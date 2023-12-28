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
#include <DataStreams/SizeLimits.h>
#include <QueryPlan/ITransformingStep.h>

namespace DB
{

struct AggregatingTransformParams;
using AggregatingTransformParamsPtr = std::shared_ptr<AggregatingTransformParams>;

/// WITH ROLLUP. See RollupTransform.
class RollupStep : public ITransformingStep
{
public:
    RollupStep(const DataStream & input_stream_, AggregatingTransformParamsPtr params_);

    String getName() const override { return "Rollup"; }

    Type getType() const override { return Type::Rollup; }
    AggregatingTransformParamsPtr getParams() const { return params; }

    void transformPipeline(QueryPipeline & pipeline, const BuildQueryPipelineSettings &) override;

    std::shared_ptr<IQueryPlanStep> copy(ContextPtr ptr) const override;
    void setInputStreams(const DataStreams & input_streams_) override;

private:
    size_t keys_size;
    AggregatingTransformParamsPtr params;
};

}
