/*
 * Copyright (2022) Bytedance Ltd. and/or its affiliates
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <variant>
#include <Parsers/ASTDropQuery.h>
#include <Storages/MergeTree/MergeTreeDataPartCNCH_fwd.h>
#include <Transaction/Actions/IAction.h>
#include <Poco/Logger.h>

namespace DB
{

struct DropDatabaseParams
{
    String name;
    TxnTimestamp prev_version;
};

struct DropSnapshotParams
{
    String name;
    UUID db_uuid;
};

struct DropDictionaryParams
{
    String database;
    String name;
    bool is_detach = false;
};

struct DropTableParams
{
    StoragePtr storage;
    TxnTimestamp prev_version;
    bool is_detach = false;
    UUID db_uuid = UUIDHelpers::Nil;
};

using DropActionParams = std::variant<DropDatabaseParams, DropSnapshotParams, DropDictionaryParams, DropTableParams>;

class DDLDropAction : public IAction
{
public:
    DDLDropAction(const ContextPtr & query_context_, const TxnTimestamp & txn_id_, DropActionParams params_)
        : IAction(query_context_, txn_id_), params(std::move(params_)), log(&Poco::Logger::get("DropAction"))
    {
    }

    ~DDLDropAction() override = default;

    void executeV1(TxnTimestamp commit_time) override;

private:
    void dropDataParts(const MergeTreeDataPartsCNCHVector & parts);
    // void updateTsCache(const UUID & uuid, const TxnTimestamp & commit_time) override;

    DropActionParams params;
    Poco::Logger * log;
};

using DDLDropActionPtr = std::shared_ptr<DDLDropAction>;

}
