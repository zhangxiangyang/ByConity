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
#include <Core/Types.h>

namespace DB
{
constexpr auto DATA_FILE_EXTENSION = ".bin";
constexpr auto INDEX_FILE_EXTENSION = ".idx";
constexpr auto MARKS_FILE_EXTENSION = ".mrk";

constexpr auto BLOOM_FILTER_FILE_EXTENSION = ".bmf";
constexpr auto RANGE_BLOOM_FILTER_FILE_EXTENSION = ".rbmf";

constexpr auto COMPRESSION_COLUMN_EXTENSION = "_encoded";
constexpr auto COMPRESSION_DATA_FILE_EXTENSION = "_encoded.bin";
constexpr auto COMPRESSION_MARKS_FILE_EXTENSION = "_encoded.mrk";

constexpr auto BITMAP_IDX_EXTENSION = ".adx";
constexpr auto BITMAP_IRK_EXTENSION = ".ark";

constexpr auto SEGMENT_BITMAP_IDX_EXTENSION = ".bitidx";
constexpr auto SEGMENT_BITMAP_TABLE_EXTENSION = ".bittab";
constexpr auto SEGMENT_BITMAP_DIRECTORY_EXTENSION = ".bitdir";

constexpr auto DELETE_BITMAP_FILE_EXTENSION = ".del";
constexpr auto DELETE_BITMAP_FILE_NAME = "delete.del";

constexpr auto KLL_FILE_EXTENSION = ".kll";

constexpr auto UKI_FILE_NAME = "unique_key.idx";   /// name of unique key index file

constexpr auto UNIQUE_ROW_STORE_DATA_NAME = "row_store.data";
constexpr auto UNIQUE_ROW_STORE_META_NAME = "row_store.meta";

constexpr auto BITENGINE_COLUMN_EXTENSION = "_encoded_bitmap";
constexpr auto BITENGINE_DATA_FILE_EXTENSION = "_encoded_bitmap.bin";
constexpr auto BITENGINE_DATA_MARKS_EXTENSION = "_encoded_bitmap.mrk";

constexpr auto COMPRESSED_DATA_INDEX_EXTENSION = ".compress_idx";

constexpr auto GIN_SEGMENT_ID_FILE_EXTENSION = ".gin_sid";
constexpr auto GIN_SEGMENT_METADATA_FILE_EXTENSION = ".gin_seg";
constexpr auto GIN_DICTIONARY_FILE_EXTENSION = ".gin_dict";
constexpr auto GIN_POSTINGS_FILE_EXTENSION = ".gin_post";

bool isEngineReservedWord(const String & column);

/// Type `ONLY_SOURCE` is used in encoding and select, it's **DEFAULT** type
/// Type `ONLY_ENCODE` used in BitEngine parts merge
/// Type `BOTH` is used in BitEngineDictionaryManager::checkEncodedPart
enum class BitEngineReadType : uint8_t { ONLY_SOURCE, ONLY_ENCODE, BOTH, };
}
