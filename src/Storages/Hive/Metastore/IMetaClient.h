#pragma once

#include "Common/config.h"
#if USE_HIVE

#include "Core/Types.h"
#include "hivemetastore/hive_metastore_types.h"

namespace ApacheHive = Apache::Hadoop::Hive;

namespace DB
{


struct HiveTableStats
{
    int64_t row_count;
     ApacheHive::TableStatsResult table_stats;
};

class IMetaClient
{
public:
    IMetaClient() = default;
    virtual ~IMetaClient() = default;

    virtual Strings getAllDatabases() = 0;
    virtual Strings getAllTables(const String & db_name) = 0;
    virtual std::shared_ptr<ApacheHive::Table> getTable(const String & db_name, const String & table_name) = 0;
    virtual bool isTableExist(const String & db_name, const String & table_name) = 0;
    virtual std::vector<ApacheHive::Partition> getPartitionsByFilter(const String & db_name, const String & table_name, const String & filter) = 0;
    virtual HiveTableStats getTableStats(const String & db_name, const String & table_name, const Strings& col_names, const bool merge_all_partition = false) = 0 ;
    // virtual ApacheHive::TableStatsResult getPartitionedTableStats(const String & db_name, const String & table_name, const Strings& col_names, const std::vector<ApacheHive::Partition>& partitions) = 0;

    // each partition is identified by a `key1=val1/key2=val2`
    virtual ApacheHive::PartitionsStatsResult getPartitionStats(const String & db_name, const String & table_name, const Strings& col_names, const Strings& partition_keys, const std::vector<Strings>& partition_vals ) = 0;
};
using IMetaClientPtr = std::shared_ptr<IMetaClient>;

}

#endif
