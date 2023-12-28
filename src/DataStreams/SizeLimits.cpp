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

#include <DataStreams/SizeLimits.h>
#include <IO/ReadHelpers.h>
#include <IO/WriteHelpers.h>
#include <Protos/plan_node_utils.pb.h>
#include <Common/Exception.h>
#include <Common/formatReadable.h>


namespace DB
{

bool SizeLimits::check(UInt64 rows, UInt64 bytes, const char * what, int too_many_rows_exception_code, int too_many_bytes_exception_code) const
{
    if (overflow_mode == OverflowMode::THROW)
    {
        if (max_rows && rows > max_rows)
            throw Exception("Limit for " + std::string(what) + " exceeded, max rows: " + formatReadableQuantity(max_rows)
                + ", current rows: " + formatReadableQuantity(rows), too_many_rows_exception_code);

        if (max_bytes && bytes > max_bytes)
            throw Exception(fmt::format("Limit for {} exceeded, max bytes: {}, current bytes: {}",
                std::string(what), ReadableSize(max_bytes), ReadableSize(bytes)), too_many_bytes_exception_code);

        return true;
    }

    return softCheck(rows, bytes);
}

bool SizeLimits::softCheck(UInt64 rows, UInt64 bytes) const
{
    if (max_rows && rows > max_rows)
        return false;
    if (max_bytes && bytes > max_bytes)
        return false;
    return true;
}

bool SizeLimits::check(UInt64 rows, UInt64 bytes, const char * what, int exception_code) const
{
    return check(rows, bytes, what, exception_code, exception_code);
}

void SizeLimits::serialize(WriteBuffer & buffer) const
{
    writeBinary(max_rows, buffer);
    writeBinary(max_bytes, buffer);
    writeBinary(UInt8(overflow_mode), buffer);
}

void SizeLimits::deserialize(ReadBuffer & buffer)
{
    readBinary(max_rows, buffer);
    readBinary(max_bytes, buffer);

    UInt8 mode;
    readBinary(mode, buffer);
    overflow_mode = OverflowMode(mode);
}

void SizeLimits::toProto(Protos::SizeLimits & proto) const
{
    proto.set_max_rows(max_rows);
    proto.set_max_bytes(max_bytes);
    proto.set_overflow_mode(OverflowModeConverter::toProto(overflow_mode));
}

void SizeLimits::fillFromProto(const Protos::SizeLimits & proto)
{
    max_rows = proto.max_rows();
    max_bytes = proto.max_bytes();
    overflow_mode = OverflowModeConverter::fromProto(proto.overflow_mode());
}
}
