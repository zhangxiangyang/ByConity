/*
 * Copyright 2016-2023 ClickHouse, Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*
 * This file may have been modified by Bytedance Ltd. and/or its affiliates (“ Bytedance's Modifications”).
 * All Bytedance's Modifications are Copyright (2023) Bytedance Ltd. and/or its affiliates.
 */

#pragma once

#include <Core/UUID.h>
#include <Databases/Snapshot.h>
#include <Interpreters/Context_fwd.h>
#include <Parsers/IAST_fwd.h>
#include <Storages/IStorage_fwd.h>
#include <Common/Exception.h>
#include <common/types.h>

#include <ctime>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <map>


namespace DB
{

struct Settings;
struct ConstraintsDescription;
struct IndicesDescription;
struct StorageInMemoryMetadata;
struct StorageID;
class ASTCreateQuery;
class AlterCommands;
class SettingsChanges;
using DictionariesWithID = std::vector<std::pair<String, UUID>>;

namespace ErrorCodes
{
    extern const int NOT_IMPLEMENTED;
    extern const int CANNOT_GET_CREATE_TABLE_QUERY;
}

class IDatabaseTablesIterator
{
public:
    virtual void next() = 0;
    virtual bool isValid() const = 0;

    virtual const String & name() const = 0;

    /// This method can return nullptr if it's Lazy database
    /// (a database with support for lazy tables loading
    /// - it maintains a list of tables but tables are loaded lazily).
    virtual const StoragePtr & table() const = 0;

    IDatabaseTablesIterator(const String & database_name_) : database_name(database_name_) { }
    IDatabaseTablesIterator(String && database_name_) : database_name(std::move(database_name_)) { }

    virtual ~IDatabaseTablesIterator() = default;

    virtual UUID uuid() const { return UUIDHelpers::Nil; }

    const String & databaseName() const { assert(!database_name.empty()); return database_name; }

protected:
    const String database_name;
};

class EmptyDatabaseTablesIterator : public IDatabaseTablesIterator
{
public:
    EmptyDatabaseTablesIterator(const std::string & name) : IDatabaseTablesIterator(name) { }
    void next() override { }
    bool isValid() const override { return false; }
    const String & name() const override {return database_name;}
    const StoragePtr & table() const override { return dummy; }


private:
    StoragePtr dummy = nullptr;
};


/// Copies list of tables and iterates through such snapshot.
class DatabaseTablesSnapshotIterator : public IDatabaseTablesIterator
{
private:
    Tables tables;
    Tables::iterator it;

protected:
    DatabaseTablesSnapshotIterator(DatabaseTablesSnapshotIterator && other)
    : IDatabaseTablesIterator(std::move(other.database_name))
    {
        size_t idx = std::distance(other.tables.begin(), other.it);
        std::swap(tables, other.tables);
        other.it = other.tables.end();
        it = tables.begin();
        std::advance(it, idx);
    }

public:
    DatabaseTablesSnapshotIterator(const Tables & tables_, const String & database_name_)
    : IDatabaseTablesIterator(database_name_), tables(tables_), it(tables.begin())
    {
    }

    DatabaseTablesSnapshotIterator(Tables && tables_, String && database_name_)
    : IDatabaseTablesIterator(std::move(database_name_)), tables(std::move(tables_)), it(tables.begin())
    {
    }

    void next() override { ++it; }

    bool isValid() const override { return it != tables.end(); }

    const String & name() const override { return it->first; }

    const StoragePtr & table() const override { return it->second; }
};

using DatabaseTablesIteratorPtr = std::unique_ptr<IDatabaseTablesIterator>;


/** Database engine.
  * It is responsible for:
  * - initialization of set of known tables and dictionaries;
  * - checking existence of a table and getting a table object;
  * - retrieving a list of all tables;
  * - creating and dropping tables;
  * - renaming tables and moving between databases with same engine.
  */

class IDatabase : public std::enable_shared_from_this<IDatabase>
{
public:
    IDatabase() = delete;
    IDatabase(String database_name_) : database_name(std::move(database_name_)) {}

    /// Get name of database engine.
    virtual String getEngineName() const = 0;

    virtual bool canContainMergeTreeTables() const { return true; }

    virtual bool canContainDistributedTables() const { return true; }

    /// Load a set of existing tables.
    /// You can call only once, right after the object is created.
    virtual void loadStoredObjects(ContextMutablePtr /*context*/, bool /*has_force_restore_data_flag*/, bool /*force_attach*/ = false) {}

    /// Check the existence of the table.
    virtual bool isTableExist(const String & name, ContextPtr context) const = 0;

    /// Get the table for work. Return nullptr if there is no table.
    virtual StoragePtr tryGetTable(const String & name, ContextPtr context) const = 0;

    virtual UUID tryGetTableUUID(const String & /*table_name*/) const { return UUIDHelpers::Nil; }

    using FilterByNameFunction = std::function<bool(const String &)>;

    /// Get an iterator that allows you to pass through all the tables.
    /// It is possible to have "hidden" tables that are not visible when passing through, but are visible if you get them by name using the functions above.
    virtual DatabaseTablesIteratorPtr getTablesIterator(ContextPtr context, const FilterByNameFunction & filter_by_table_name = {}) = 0;

    /// Is the database empty.
    virtual bool empty() const = 0;

    /// Add the table to the database. Record its presence in the metadata.
    virtual void createTable(
        ContextPtr /*context*/,
        const String & /*name*/,
        const StoragePtr & /*table*/,
        const ASTPtr & /*query*/)
    {
        throw Exception("There is no CREATE TABLE query for Database" + getEngineName(), ErrorCodes::NOT_IMPLEMENTED);
    }

    /// Delete the table from the database, drop table and delete the metadata.
    virtual void dropTable(
        ContextPtr /*context*/,
        const String & /*name*/,
        [[maybe_unused]] bool no_delay = false)
    {
        throw Exception("There is no DROP TABLE query for Database" + getEngineName(), ErrorCodes::NOT_IMPLEMENTED);
    }

    /// Add a table to the database, but do not add it to the metadata. The database may not support this method.
    ///
    /// Note: ATTACH TABLE statement actually uses createTable method.
    virtual void attachTable(const String & /*name*/, const StoragePtr & /*table*/, [[maybe_unused]] const String & relative_table_path = {})
    {
        throw Exception("There is no ATTACH TABLE query for Database" + getEngineName(), ErrorCodes::NOT_IMPLEMENTED);
    }

    /// Forget about the table without deleting it, and return it. The database may not support this method.
    virtual StoragePtr detachTable(const String & /*name*/)
    {
        throw Exception("There is no DETACH TABLE query for Database" + getEngineName(), ErrorCodes::NOT_IMPLEMENTED);
    }

    /// Forget about the table without deleting it's data, but rename metadata file to prevent reloading it
    /// with next restart. The database may not support this method.
    virtual void detachTablePermanently(ContextPtr /*context*/, const String & /*name*/)
    {
        throw Exception("There is no DETACH TABLE PERMANENTLY query for Database" + getEngineName(), ErrorCodes::NOT_IMPLEMENTED);
    }

    /// Rename the table and possibly move the table to another database.
    virtual void renameTable(
        ContextPtr /*context*/,
        const String & /*name*/,
        IDatabase & /*to_database*/,
        const String & /*to_name*/,
        bool /*exchange*/,
        bool /*dictionary*/)
    {
        throw Exception(getEngineName() + ": renameTable() is not supported", ErrorCodes::NOT_IMPLEMENTED);
    }

    using ASTModifier = std::function<void(IAST &)>;

    /// Change the table structure in metadata.
    /// You must call under the alter_lock of the corresponding table . If engine_modifier is empty, then engine does not change.
    virtual void alterTable(
        ContextPtr /*context*/,
        const StorageID & /*table_id*/,
        const StorageInMemoryMetadata & /*metadata*/)
    {
        throw Exception(getEngineName() + ": alterTable() is not supported", ErrorCodes::NOT_IMPLEMENTED);
    }

    /// Returns time of table's metadata change, 0 if there is no corresponding metadata file.
    virtual time_t getObjectMetadataModificationTime(const String & /*name*/) const
    {
        return static_cast<time_t>(0);
    }

    /// Get the CREATE TABLE query for the table. It can also provide information for detached tables for which there is metadata.
    ASTPtr tryGetCreateTableQuery(const String & name, ContextPtr context) const noexcept
    {
        return getCreateTableQueryImpl(name, context, false);
    }

    ASTPtr getCreateTableQuery(const String & name, ContextPtr context) const
    {
        return getCreateTableQueryImpl(name, context, true);
    }

    /// Get the CREATE DATABASE query for current database.
    virtual ASTPtr getCreateDatabaseQuery() const = 0;

    /// Get name of database.
    String getDatabaseName() const
    {
        std::lock_guard lock{mutex};
        return database_name;
    }
    /// Get UUID of database.
    virtual UUID getUUID() const { return UUIDHelpers::Nil; }

    virtual void renameDatabase(ContextPtr /*context*/, const String & /*nnew_name*/)
    {
        throw Exception(getEngineName() + ": RENAME DATABASE is not supported", ErrorCodes::NOT_IMPLEMENTED);
    }

    /// Returns path for persistent data storage if the database supports it, empty string otherwise
    virtual String getDataPath() const { return {}; }

    /// Returns path for persistent data storage for table if the database supports it, empty string otherwise. Table must exist
    virtual String getTableDataPath(const String & /*table_name*/) const { return {}; }
    /// Returns path for persistent data storage for CREATE/ATTACH query if the database supports it, empty string otherwise
    virtual String getTableDataPath(const ASTCreateQuery & /*query*/) const { return {}; }
    /// Returns metadata path if the database supports it, empty string otherwise
    virtual String getMetadataPath() const { return {}; }
    /// Returns metadata path of a concrete table if the database supports it, empty string otherwise
    virtual String getObjectMetadataPath(const String & /*table_name*/) const { return {}; }

    /// All tables and dictionaries should be detached before detaching the database.
    virtual bool shouldBeEmptyOnDetach() const { return true; }

    virtual void assertCanBeDetached(bool /*cleanup*/) {}

    virtual void waitDetachedTableNotInUse(const UUID & /*uuid*/) { }
    virtual void checkDetachedTableNotInUse(const UUID & /*uuid*/) { }

    /// Ask all tables to complete the background threads they are using and delete all table objects.
    virtual void shutdown() = 0;

    /// Delete data and metadata stored inside the database, if exists.
    virtual void drop(ContextPtr /*context*/) {}

    /// tables marked as broken if can not be loaded when instance startup.
    virtual std::map<String, String> getBrokenTables() {return {};}
    virtual void clearBrokenTables() {}

    virtual void applySettingsChanges(const SettingsChanges &, ContextPtr)
    {
        throw Exception(
            ErrorCodes::NOT_IMPLEMENTED,
            "Database engine {} either does not support settings, or does not support altering settings",
            getEngineName());
    }

    virtual bool supportSnapshot() const { return false; }

    virtual void dropSnapshot(ContextPtr /*local_context*/, const String & /*snapshot_name*/)
    {
        assertSupportSnapshot();
        throw Exception(ErrorCodes::NOT_IMPLEMENTED, "{}: DROP SNAPSHOT is not supported");
    }

    virtual SnapshotPtr tryGetSnapshot(const String & /*snapshot_name*/) const
    {
        assertSupportSnapshot();
        throw Exception(ErrorCodes::NOT_IMPLEMENTED, "{}: GET SNAPSHOT is not supported");
    }

    virtual Snapshots getAllSnapshots() const
    {
        assertSupportSnapshot();
        throw Exception(ErrorCodes::NOT_IMPLEMENTED, "{}: SHOW SNAPSHOTS is not supported");
    }

    virtual Snapshots getAllSnapshotsForStorage(UUID /*storage_uuid*/) const
    {
        assertSupportSnapshot();
        throw Exception(ErrorCodes::NOT_IMPLEMENTED, "{}: SHOW SNAPSHOTS is not supported");
    }

    virtual ~IDatabase() = default;

protected:
    void assertSupportSnapshot() const
    {
        if (!supportSnapshot())
            throw Exception(ErrorCodes::LOGICAL_ERROR, "Database engine {} doesn't support snapshots", getEngineName());
    }

    virtual ASTPtr getCreateTableQueryImpl(const String & /*name*/, ContextPtr /*context*/, bool throw_on_error) const
    {
        if (throw_on_error)
            throw Exception("There is no SHOW CREATE TABLE query for Database" + getEngineName(), ErrorCodes::CANNOT_GET_CREATE_TABLE_QUERY);
        return nullptr;
    }

    mutable std::mutex mutex;
    String database_name;
};

using DatabasePtr = std::shared_ptr<IDatabase>;
using Databases = std::map<String, DatabasePtr>;

}
