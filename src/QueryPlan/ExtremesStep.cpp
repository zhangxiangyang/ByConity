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

#include <QueryPlan/ExtremesStep.h>
#include <Processors/QueryPipeline.h>

namespace DB
{

static ITransformingStep::Traits getTraits()
{
    return ITransformingStep::Traits
    {
        {
            .preserves_distinct_columns = true,
            .returns_single_stream = false,
            .preserves_number_of_streams = true,
            .preserves_sorting = true,
        },
        {
            .preserves_number_of_rows = true,
        }
    };
}

ExtremesStep::ExtremesStep(const DataStream & input_stream_)
    : ITransformingStep(input_stream_, input_stream_.header, getTraits())
{
}

void ExtremesStep::setInputStreams(const DataStreams & input_streams_)
{
    input_streams = input_streams_;
    output_stream->header = input_streams_[0].header;
}

void ExtremesStep::transformPipeline(QueryPipeline & pipeline, const BuildQueryPipelineSettings &)
{
    pipeline.addExtremesTransform();
}

std::shared_ptr<ExtremesStep> ExtremesStep::fromProto(const Protos::ExtremesStep & proto, ContextPtr)
{
    auto [step_description, base_input_stream] = ITransformingStep::deserializeFromProtoBase(proto.query_plan_base());
    auto step = std::make_shared<ExtremesStep>(base_input_stream);
    step->setStepDescription(step_description);
    return step;
}

void ExtremesStep::toProto(Protos::ExtremesStep & proto, bool) const
{
    ITransformingStep::serializeToProtoBase(*proto.mutable_query_plan_base());
}

std::shared_ptr<IQueryPlanStep> ExtremesStep::copy(ContextPtr) const
{
    return std::make_shared<ExtremesStep>(input_streams[0]);
}

}
