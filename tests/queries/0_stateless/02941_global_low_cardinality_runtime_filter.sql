set enable_optimizer=1;
set create_stats_time_output=0;

drop table if exists lc_rf1;
drop table if exists lc_rf2;
drop table if exists lc_rf1_all;
drop table if exists lc_rf2_all;

create table lc_rf1 (c1 UInt32, c2 GlobalLowCardinality(Nullable(UInt32))) engine = MergeTree order by c1;
create table lc_rf2 (c1 UInt32, c2 GlobalLowCardinality(Nullable(UInt32))) engine = MergeTree order by c1;
create table lc_rf1_all as lc_rf1 engine=Distributed('test_shard_localhost', currentDatabase(), 'lc_rf1', c1);
create table lc_rf2_all as lc_rf2 engine=Distributed('test_shard_localhost', currentDatabase(), 'lc_rf2', c1);

insert into lc_rf1_all select 1, number from system.numbers limit 10000;
insert into lc_rf2_all select 1, number from system.numbers limit 10000;

create stats lc_rf1_all;
create stats lc_rf2_all;

select count(*) from lc_rf1_all a,lc_rf2_all b where a.c2=b.c2 and a.c1=1 settings enable_runtime_filter_cost=0, runtime_filter_min_filter_rows=0, runtime_filter_min_filter_factor=0;
select count(*) from lc_rf1_all a,lc_rf2_all b where a.c2=b.c2 and a.c1=1 settings enable_runtime_filter_cost=0, runtime_filter_min_filter_rows=0, runtime_filter_min_filter_factor=0, runtime_filter_bloom_build_threshold=0,runtime_filter_in_build_threshold=100000;

drop table lc_rf1;
drop table lc_rf2;
drop table lc_rf1_all;
drop table lc_rf2_all;
