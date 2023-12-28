drop table if exists test.10084_t1;
drop table if exists test.10084_t2;
create table test.10084_t1(c1 UInt64, c2 UInt64) ENGINE=CnchMergeTree() order by c1;
create table test.10084_t2(c1 UInt64, c2 UInt64) ENGINE=CnchMergeTree() order by c1;
insert into test.10084_t1 values(1, 1);
insert into test.10084_t1 values(2, 2);
insert into test.10084_t1 values(3, 3);
insert into test.10084_t2 values(1, 1);
insert into test.10084_t2 values(2, 2);
insert into test.10084_t2 values(3, 3);

select 10084_t1.c1 from test.10084_t1 inner join test.10084_t2 on 10084_t1.c1=10084_t2.c1 settings enable_optimizer=1,max_plan_segment_num=1; -- { serverError 3010 }
drop table if exists test.10084_t1;
drop table if exists test.10084_t2;
