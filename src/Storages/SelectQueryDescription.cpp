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

#include <Storages/SelectQueryDescription.h>

#include <Parsers/ASTSelectWithUnionQuery.h>
#include <Parsers/ASTSelectQuery.h>
#include <Interpreters/getTableExpressions.h>
#include <Interpreters/AddDefaultDatabaseVisitor.h>
#include <Interpreters/Context.h>


namespace DB
{

namespace ErrorCodes
{
extern const int QUERY_IS_NOT_SUPPORTED_IN_MATERIALIZED_VIEW;
extern const int LOGICAL_ERROR;
}

SelectQueryDescription::SelectQueryDescription(const SelectQueryDescription & other)
    : select_table_id(other.select_table_id)
    , base_table_ids(other.base_table_ids)
    , select_query(other.select_query ? other.select_query->clone() : nullptr)
    , inner_query(other.inner_query ? other.inner_query->clone() : nullptr)
{
}

SelectQueryDescription & SelectQueryDescription::SelectQueryDescription::operator=(const SelectQueryDescription & other)
{
    if (&other == this)
        return *this;

    select_table_id = other.select_table_id;
    base_table_ids = other.base_table_ids;
    if (other.select_query)
        select_query = other.select_query->clone();
    else
        select_query.reset();

    if (other.inner_query)
        inner_query = other.inner_query->clone();
    else
        inner_query.reset();
    return *this;
}


namespace
{

std::vector<std::shared_ptr<StorageID>> getBaseTableIds(ASTSelectQuery & query, ContextPtr context)
{
    std::vector<std::shared_ptr<StorageID>> table_ids;
    std::vector<ASTPtr> all_tables;

    bool has_table_function = false;
    ASTSelectQuery::collectAllTables(&query, all_tables, has_table_function);
    for (auto & table_ast : all_tables)
    {
        DatabaseAndTableWithAlias db_and_table(table_ast, context->getCurrentDatabase());
        table_ids.emplace_back(std::make_shared<StorageID>(db_and_table.database, db_and_table.table));
    }

    return table_ids;
}

StorageID extractDependentTableFromSelectQuery(ASTSelectQuery & query, ContextPtr context, bool add_default_db = true)
{
    if (add_default_db)
    {
        AddDefaultDatabaseVisitor visitor(context, context->getCurrentDatabase(), false, false, nullptr);
        visitor.visit(query);
    }

    if (auto db_and_table = getDatabaseAndTable(query, 0))
    {
        return StorageID(db_and_table->database, db_and_table->table/*, db_and_table->uuid*/);
    }
    else if (auto subquery = extractTableExpression(query, 0))
    {
        auto * ast_select = subquery->as<ASTSelectWithUnionQuery>();
        if (!ast_select)
            throw Exception("Logical error while creating StorageMaterializedView. "
                            "Could not retrieve table name from select query.",
                            DB::ErrorCodes::LOGICAL_ERROR);
        if (ast_select->list_of_selects->children.size() != 1)
            throw Exception("UNION is not supported for MATERIALIZED VIEW",
                  ErrorCodes::QUERY_IS_NOT_SUPPORTED_IN_MATERIALIZED_VIEW);

        auto & inner_query = ast_select->list_of_selects->children.at(0);

        return extractDependentTableFromSelectQuery(inner_query->as<ASTSelectQuery &>(), context, false);
    }
    else
        return StorageID::createEmpty();
}


void checkAllowedQueries(const ASTSelectQuery & query)
{
    if (query.prewhere() || query.final() || query.sampleSize())
        throw Exception("MATERIALIZED VIEW cannot have PREWHERE, SAMPLE or FINAL.", DB::ErrorCodes::QUERY_IS_NOT_SUPPORTED_IN_MATERIALIZED_VIEW);

    ASTPtr subquery = extractTableExpression(query, 0);
    if (!subquery)
        return;

    if (const auto * ast_select = subquery->as<ASTSelectWithUnionQuery>())
    {
        if (ast_select->list_of_selects->children.size() != 1)
            throw Exception("UNION is not supported for MATERIALIZED VIEW", ErrorCodes::QUERY_IS_NOT_SUPPORTED_IN_MATERIALIZED_VIEW);

        const auto & inner_query = ast_select->list_of_selects->children.at(0);

        checkAllowedQueries(inner_query->as<ASTSelectQuery &>());
    }
}

}

/// check if only one single select query in SelectWithUnionQuery
static bool isSingleSelect(const ASTPtr & select, ASTPtr & res)
{
    auto new_select = select->as<ASTSelectWithUnionQuery &>();
    if (new_select.list_of_selects->children.size() != 1)
        return false;
    auto & new_inner_query = new_select.list_of_selects->children.at(0);
    if (new_inner_query->as<ASTSelectQuery>())
    {
        res = new_inner_query;
        return true;
    }
    else
        return isSingleSelect(new_inner_query, res);
}

SelectQueryDescription SelectQueryDescription::getSelectQueryFromASTForMatView(const ASTPtr & select, ContextPtr context)
{
    ASTPtr new_inner_query;

    if (!isSingleSelect(select, new_inner_query))
        throw Exception("UNION is not supported for MATERIALIZED VIEW", ErrorCodes::QUERY_IS_NOT_SUPPORTED_IN_MATERIALIZED_VIEW);

    auto & select_query = new_inner_query->as<ASTSelectQuery &>();
    checkAllowedQueries(select_query);

    SelectQueryDescription result;
    result.select_table_id = extractDependentTableFromSelectQuery(select_query, context);
    result.base_table_ids = getBaseTableIds(select_query, context);
    result.select_query = select->as<ASTSelectWithUnionQuery &>().clone();
    result.inner_query = new_inner_query->clone();
    return result;
}

}

