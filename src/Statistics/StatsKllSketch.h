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
#include <DataTypes/IDataType.h>
#include <Statistics/Base64.h>
#include <Statistics/BucketBounds.h>
#include <Statistics/StatisticsBaseImpl.h>
#include <Statistics/StatsNdvBucketsResult.h>
#include <Common/Exception.h>

namespace DB::Statistics
{
template <typename T>
class BucketBoundsImpl;


template <typename T>
class StatsKllSketchImpl;

class StatsKllSketch : public StatisticsBase
{
public:
    static constexpr auto tag = StatisticsTag::KllSketch;

    StatisticsTag getTag() const override { return tag; }

    virtual bool isEmpty() const = 0;
    virtual std::optional<double> minAsDouble() const = 0;
    virtual std::optional<double> maxAsDouble() const = 0;

    virtual SerdeDataType getSerdeDataType() const = 0;

    template <typename T>
    using Impl = StatsKllSketchImpl<T>;

    virtual std::shared_ptr<BucketBounds> getBucketBounds(UInt64 histogram_bucket_size) const = 0;

    virtual std::shared_ptr<StatsNdvBucketsResult> generateNdvBucketsResult(double total_ndv, UInt64 histogram_bucket_size) const = 0;

    virtual int64_t getCount() const = 0;

private:
};


}
