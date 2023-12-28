#include "HiveExternalCatalog.h"
#include <memory>
#include <vector>
#include <hive_metastore_types.h>
#include <Core/UUID.h>
#include <Interpreters/Context.h>
#include <Interpreters/InterpreterCreateQuery.h>
#include <Parsers/ASTCreateQuery.h>
#include <Parsers/ASTFunction.h>
#include <Parsers/ASTSetQuery.h>
#include <Parsers/IAST_fwd.h>
#include <Parsers/ParserCreateQuery.h>
#include <Parsers/formatAST.h>
#include <Parsers/formatTenantDatabaseName.h>
#include <Parsers/parseQuery.h>
#include <Storages/ColumnsDescription.h>
#include <Storages/Hive/HiveSchemaConverter.h>
#include <Storages/Hive/Metastore/HiveMetastore.h>
#include <Storages/Hive/Metastore/IMetaClient.h>
#include <Storages/IStorage.h>
#include <Storages/StorageFactory.h>
#include <boost/algorithm/string/predicate.hpp>
#include <common/logger_useful.h>
#include "IExternalCatalogMgr.h"
#include "Interpreters/Context_fwd.h"
#include "Parsers/formatTenantDatabaseName.h"
#include "Storages/Hive/Metastore/GlueMetastore.h"
namespace DB::ErrorCodes
{
extern const int BAD_ARGUMENTS;
}

namespace DB::ExternalCatalog
{

static void addCreateQuerySettings(ASTCreateQuery & create_query, PlainConfigsPtr configs);

static std::string key_hive_uri = "hive.metastore.uri";
HiveExternalCatalog::HiveExternalCatalog(const std::string & _catalog_name, [[maybe_unused]] PlainConfigsPtr conf)
    : catalog_name(_catalog_name), configs(new PlainConfigs())
{
    conf->copyTo(*this->configs);
    configs->forEachKey([this](const std::string & key, const std::string & value) { LOG_TRACE(log, "{} - {}", key, value); });
    auto type = configs->getString("type");
    if (boost::iequals(type, "hive"))
    {
        std::string hive_uri = configs->getString(key_hive_uri);
        if (!configs->has(key_hive_uri))
        {
            throw Exception(fmt::format("{} is not configured for {}", key_hive_uri, catalog_name), ErrorCodes::BAD_ARGUMENTS);
        }
        hms_client = HiveMetastoreClientFactory::instance().getOrCreate(hive_uri, {});
    }
    else
    {
        hms_client = GlueMetastoreClientFactory::instance().getOrCreate(configs);
    }
}

std::vector<std::string> HiveExternalCatalog::listDbNames()
{
    return hms_client->getAllDatabases();
}
std::vector<std::string> HiveExternalCatalog::listTableNames(const std::string & db_name)
{
    auto db_name_without_tenant = getOriginalDatabaseName(db_name);
    return hms_client->getAllTables(db_name_without_tenant);
}
std::vector<std::string>
HiveExternalCatalog::listPartitionNames([[maybe_unused]] const std::string & db_name, [[maybe_unused]] const std::string & table_name)
{
    return {};
}
std::vector<ApacheHive::Partition>
HiveExternalCatalog::getPartionsByFilter(const std::string & db_name, const std::string & table_name, const std::string & filter)
{
    return hms_client->getPartitionsByFilter(db_name, table_name, filter);
}


DB::StoragePtr HiveExternalCatalog::getTable(
    [[maybe_unused]] const std::string & db_name,
    [[maybe_unused]] const std::string & table_name,
    [[maybe_unused]] ContextPtr local_context)
{
    auto db_name_without_tenant = getOriginalDatabaseName(db_name);
    auto hive_table = hms_client->getTable(db_name_without_tenant, table_name);
    // hive_table->dbName = formatTenantDatabaseName(hive_table->dbName);
    HiveSchemaConverter converter(local_context, hive_table);
    //TODO(ExternalCatalog):: we set the metastore uri here to make the StorageCnchHive run. We need further modification.
    // auto create_query_ast = converter.createQueryAST(configs->getString(key_hive_uri, configs->getString("aws.glue.endpoint", "default")));
    auto create_query_ast = converter.createQueryAST(name());
    create_query_ast.database = formatTenantDatabaseName(formatCatalogDatabaseName(hive_table->dbName, name()));
    create_query_ast.table = hive_table->tableName;
    create_query_ast.uuid = getTableUUID(create_query_ast.database, create_query_ast.table);
    ContextMutablePtr mutable_context = Context::createCopy(local_context->shared_from_this());

    StorageFactory::HiveParamsPtr hive_params(new StorageFactory::HiveParams{.hive_client = hms_client});

    addCreateQuerySettings(create_query_ast, configs);
    auto create_query = serializeAST(create_query_ast);
    LOG_TRACE(log, "{}.{}.{} create query: {}", name(), db_name, table_name, create_query);
    auto columns = InterpreterCreateQuery::getColumnsDescription(*create_query_ast.columns_list->columns, mutable_context, false);
    auto ret = StorageFactory::instance().get(
        create_query_ast,
        "",
        mutable_context,
        mutable_context->getGlobalContext(),
        InterpreterCreateQuery::getColumnsDescription(*create_query_ast.columns_list->columns, mutable_context, false),
        InterpreterCreateQuery::getConstraintsDescription(create_query_ast.columns_list->constraints),
        {},{},
        false,
        std::move(hive_params));

    ret->setCreateTableSql(create_query);
    return ret;
}

bool HiveExternalCatalog::isTableExist(
    [[maybe_unused]] const std::string & db_name,
    [[maybe_unused]] const std::string & table_name,
    [[maybe_unused]] ContextPtr local_context)
{
    return hms_client->isTableExist(db_name, table_name);
}

UUID HiveExternalCatalog::getTableUUID(const std::string & db_name, const std::string & table_name)
{
    return UUIDHelpers::hashUUIDfromString(fmt::format("{}.{}.{}", catalog_name, db_name, table_name));
}

void addCreateQuerySettings(ASTCreateQuery & create_query, PlainConfigsPtr configs)
{
    if (!create_query.storage)
        return;
    ASTSetQuery * set_query = create_query.storage->settings;
    if (set_query == nullptr)
    {
        create_query.storage->set(create_query.storage->settings, std::make_shared<ASTSetQuery>());
        set_query = create_query.storage->settings;
    }
    set_query->is_standalone = false;

    std::vector<std::pair<std::string, std::string>> settings_map = {
        {"endpoint", "aws.s3.endpoint"}, {"ak_id", "aws.s3.access_key"}, {"ak_secret", "aws.s3.secret_key"}, {"region", "aws.s3.region"}};
    for (const auto & p : settings_map)
    {
        if (configs->has(p.second))
        {
            set_query->changes.emplace_back(p.first, configs->getString(p.second));
        }
    }
}
}
