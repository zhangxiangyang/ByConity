SET enable_optimizer=1;
SET optimizer_projection_support=1;
SET force_optimize_projection=1;

CREATE DATABASE IF NOT EXISTS test_proj_support_db;
USE test_proj_support_db;
SET max_threads=8;
SET exchange_source_pipeline_threads=1;

SELECT '-- 1. use projection data only';
DROP TABLE IF EXISTS test_proj_support;

CREATE TABLE test_proj_support
(
    `part` Int32,
    `key1` Int32,
    `key2` Int32,
    `val` Int64
)
ENGINE = CnchMergeTree
PARTITION BY part
ORDER BY tuple()
SETTINGS index_granularity = 1000;

ALTER TABLE test_proj_support ADD PROJECTION proj1
(
SELECT
    key1,
    sum(val)
    GROUP BY key1
);

INSERT INTO test_proj_support
SELECT
    number / 100000,
    number % 10,
    number % 3357,
    1
FROM system.numbers LIMIT 100000;

SELECT
    key1, sum(val)
FROM test_proj_support
GROUP BY key1
ORDER BY key1;

SELECT '-- 2. use raw data only';
SELECT
    key2, sum(val)
FROM test_proj_support
GROUP BY key2
ORDER BY key2
LIMIT 20;

SELECT '-- 3. use projection & raw data';
DROP TABLE IF EXISTS test_proj_support;

CREATE TABLE test_proj_support
(
    `part` Int32,
    `key1` Int32,
    `key2` Int32,
    `val` Int64
)
ENGINE = CnchMergeTree
PARTITION BY part
ORDER BY tuple()
SETTINGS index_granularity = 1000;

INSERT INTO test_proj_support
SELECT
    number / 100000,
    number % 10,
    number % 3357,
    1
FROM system.numbers LIMIT 100000;

ALTER TABLE test_proj_support ADD PROJECTION proj1
(
    SELECT
        key1,
        key2 + 1,
        sum(val),
        sum(val + 1)
    GROUP BY key1, key2 + 1
);

INSERT INTO test_proj_support
SELECT
    number / 100000,
    number % 10,
    number % 3357,
    1
FROM system.numbers LIMIT 100000 OFFSET 100000;

SELECT key1, key2 + 1, sum(val), sum(val + 1)
FROM test_proj_support
GROUP BY key1, key2 + 1
ORDER BY key1, key2 + 1
LIMIT 20;

SELECT '-- 4.1 test where';
SELECT key1, key2 + 1, sum(val), sum(val + 1)
FROM test_proj_support
WHERE key1 = 0 AND (key2 + 1) % 3 = 0
GROUP BY key1, key2 + 1
ORDER BY key1, key2 + 1
LIMIT 20;

SELECT '-- 4.2 test rollup';
SELECT
    key1, sum(val), sum(val + 1)
FROM test_proj_support
GROUP BY key1
ORDER BY key1;

SELECT '-- 4.3 test derived grouping key';
SELECT
    key1, (key2 + 1) % 2, sum(val), sum(val + 1)
FROM test_proj_support
GROUP BY key1, (key2 + 1) % 2
ORDER BY key1, (key2 + 1) % 2;

SELECT '-- 4.4 test use multiple projections';
ALTER TABLE test_proj_support ADD PROJECTION proj2
(
SELECT
    key1,
    sum(val),
    sum(val + 1)
    GROUP BY key1
);

INSERT INTO test_proj_support
SELECT
    number / 100000,
    number % 10,
    number % 3357,
    1
FROM system.numbers LIMIT 100000 OFFSET 200000;

SELECT
    key1, sum(val), sum(val + 1)
FROM test_proj_support
GROUP BY key1
ORDER BY key1;

SELECT '-- 4.5 test missing column';
ALTER TABLE test_proj_support ADD COLUMN `key3` Int32 AFTER `key2`;

ALTER TABLE test_proj_support ADD PROJECTION proj3
(
SELECT
    key1,
    key3,
    sum(val)
    GROUP BY key1, key3
);

INSERT INTO test_proj_support
SELECT
    number / 100000,
    number % 10,
    number % 3357,
    number % 10,
    1
FROM system.numbers LIMIT 100000 OFFSET 300000;

SELECT
    key1, key3, sum(val)
FROM test_proj_support
GROUP BY key1, key3
ORDER BY key1, key3;

DROP TABLE IF EXISTS test_proj_support;
