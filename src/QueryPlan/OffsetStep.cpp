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

#include <QueryPlan/OffsetStep.h>
#include <Processors/OffsetTransform.h>
#include <Processors/QueryPipeline.h>
#include <IO/Operators.h>
#include <Common/JSONBuilder.h>

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
            .preserves_number_of_rows = false,
        }
    };
}

OffsetStep::OffsetStep(const DataStream & input_stream_, size_t offset_)
    : ITransformingStep(input_stream_, input_stream_.header, getTraits())
    , offset(offset_)
{
}

void OffsetStep::setInputStreams(const DataStreams & input_streams_)
{
    input_streams = input_streams_;
    output_stream->header = input_streams_[0].header;
}

void OffsetStep::transformPipeline(QueryPipeline & pipeline, const BuildQueryPipelineSettings &)
{
    auto transform = std::make_shared<OffsetTransform>(
            pipeline.getHeader(), offset, pipeline.getNumStreams());

    pipeline.addTransform(std::move(transform));
}

void OffsetStep::describeActions(FormatSettings & settings) const
{
    settings.out << String(settings.offset, ' ') << "Offset " << offset << '\n';
}

void OffsetStep::describeActions(JSONBuilder::JSONMap & map) const
{
    map.add("Offset", offset);
}

std::shared_ptr<OffsetStep> OffsetStep::fromProto(const Protos::OffsetStep & proto, ContextPtr)
{
    auto [step_description, base_input_stream] = ITransformingStep::deserializeFromProtoBase(proto.query_plan_base());
    auto offset = proto.offset();
    auto step = std::make_shared<OffsetStep>(base_input_stream, offset);
    step->setStepDescription(step_description);
    return step;
}

void OffsetStep::toProto(Protos::OffsetStep & proto, bool) const
{
    ITransformingStep::serializeToProtoBase(*proto.mutable_query_plan_base());
    proto.set_offset(offset);
}

std::shared_ptr<IQueryPlanStep> OffsetStep::copy(ContextPtr) const
{
    return std::make_shared<OffsetStep>(input_streams[0], offset);
}

}
