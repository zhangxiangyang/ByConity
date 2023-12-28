DROP TABLE IF EXISTS tableCommon;
DROP TABLE IF EXISTS tableTrees;
DROP TABLE IF EXISTS tableFlowers;

CREATE TABLE tableCommon (`key` FixedString(15), `value` Nullable(Int8)) ENGINE = CnchMergeTree() order by tuple();
CREATE TABLE tableTrees (`key` FixedString(15), `name` Nullable(Int8), `name2` Nullable(Int8)) ENGINE = CnchMergeTree() order by tuple();
CREATE TABLE tableFlowers (`key` FixedString(15), `name` Nullable(Int8)) ENGINE = CnchMergeTree() order by tuple();

SELECT * FROM (
    SELECT common.key, common.value, trees.name, trees.name2
    FROM (
	SELECT *
	FROM tableCommon
    ) as common
    INNER JOIN (
	SELECT *
	FROM tableTrees
    ) trees ON (common.key = trees.key)
)
UNION ALL
(
    SELECT common.key, common.value, 
    null as name, null as name2 
    
    FROM (
	SELECT *
	FROM tableCommon
    ) as common
    INNER JOIN (
	SELECT *
	FROM tableFlowers
    ) flowers ON (common.key = flowers.key)
);

SELECT * FROM (
    SELECT common.key, common.value, trees.name, trees.name2
    FROM (
	SELECT *
	FROM tableCommon
    ) as common
    INNER JOIN (
	SELECT *
	FROM tableTrees
    ) trees ON (common.key = trees.key)
)
UNION ALL
(
    SELECT common.key, common.value, 
    flowers.name, null as name2

    FROM (
	SELECT *
	FROM tableCommon
    ) as common
    INNER JOIN (
	SELECT *
	FROM tableFlowers
    ) flowers ON (common.key = flowers.key)
);

DROP TABLE IF EXISTS tableCommon;
DROP TABLE IF EXISTS tableTrees;
DROP TABLE IF EXISTS tableFlowers;
