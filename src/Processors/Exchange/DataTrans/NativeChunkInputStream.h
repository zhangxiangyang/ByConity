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

#include <memory>
#include <DataStreams/IBlockInputStream.h>
#include <DataStreams/MarkInCompressedFile.h>
#include <DataStreams/NativeBlockInputStream.h>
#include <Processors/Chunk.h>
#include <Common/PODArray.h>

namespace DB
{
class CompressedReadBufferFromFile;

/** Deserializes the stream of chunks from the native binary format (with names and column types).
  */
class NativeChunkInputStream
{
public:
    /// For cases when data structure (header) is known in advance.
    NativeChunkInputStream(ReadBuffer & istr_, const Block & header_);

    static void readData(const IDataType & type, ColumnPtr & column, ReadBuffer & istr, size_t rows, double avg_value_size_hint);

    Chunk readImpl();

private:
    ReadBuffer & istr;
    Block header;
    PODArray<double> avg_value_size_hints;
};

using NativeChunkInputStreamHolder = std::unique_ptr<NativeChunkInputStream>;
}
