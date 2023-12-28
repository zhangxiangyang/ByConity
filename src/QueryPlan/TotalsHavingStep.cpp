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

#include <cstdint>
#include <IO/Operators.h>
#include <IO/WriteHelpers.h>
#include <Interpreters/ExpressionActions.h>
#include <Parsers/ASTSerDerHelper.h>
#include <Parsers/IAST_fwd.h>
#include <Processors/QueryPipeline.h>
#include <Processors/Transforms/DistinctTransform.h>
#include <Processors/Transforms/ExpressionTransform.h>
#include <Processors/Transforms/TotalsHavingTransform.h>
#include <QueryPlan/FilterStep.h>
#include <QueryPlan/IQueryPlanStep.h>
#include <QueryPlan/TotalsHavingStep.h>
#include <Common/JSONBuilder.h>
#include "DataStreams/finalizeBlock.h"

namespace DB
{

static ITransformingStep::Traits getTraits(bool has_filter)
{
    return ITransformingStep::Traits
    {
        {
            .preserves_distinct_columns = true,
            .returns_single_stream = true,
            .preserves_number_of_streams = false,
            .preserves_sorting = true,
        },
        {
            .preserves_number_of_rows = !has_filter,
        }
    };
}

TotalsHavingStep::TotalsHavingStep(
    const DataStream & input_stream_,
    bool overflow_row_,
    const ActionsDAGPtr & actions_dag_,
    const std::string & filter_column_,
    TotalsMode totals_mode_,
    double auto_include_threshold_,
    bool final_)
    : ITransformingStep(
        input_stream_,
        TotalsHavingTransform::transformHeader(input_stream_.header, actions_dag_.get(), final_),
        getTraits(!filter_column_.empty()))
    , overflow_row(overflow_row_)
    , actions_dag(actions_dag_)
    , filter_column_name(filter_column_)
    , totals_mode(totals_mode_)
    , auto_include_threshold(auto_include_threshold_)
    , final(final_)
{
}

TotalsHavingStep::TotalsHavingStep(
    const DataStream & input_stream_,
    bool overflow_row_,
    const ConstASTPtr & having_filter_,
    TotalsMode totals_mode_,
    double auto_include_threshold_,
    bool final_)
    : ITransformingStep(
        input_stream_, TotalsHavingTransform::transformHeader(input_stream_.header, nullptr, final_), getTraits(having_filter_ != nullptr))
    , overflow_row(overflow_row_)
    , having_filter(having_filter_)
    , totals_mode(totals_mode_)
    , auto_include_threshold(auto_include_threshold_)
    , final(final_)
{
}

void TotalsHavingStep::setInputStreams(const DataStreams & input_streams_)
{
    input_streams = input_streams_;
    output_stream->header = TotalsHavingTransform::transformHeader(input_streams_[0].header, actions_dag.get(), final);
}

void TotalsHavingStep::transformPipeline(QueryPipeline & pipeline, const BuildQueryPipelineSettings & settings)
{
    if (!actions_dag && having_filter)
    {
        auto rewrite_filter = FilterStep::rewriteRuntimeFilter(having_filter, pipeline, settings);
        actions_dag = IQueryPlanStep::createFilterExpressionActions(
            settings.context, rewrite_filter->clone(), TotalsHavingTransform::transformHeader(input_streams[0].header, nullptr, final));
        filter_column_name = rewrite_filter->getColumnName();
    }
    auto expression_actions = actions_dag ? std::make_shared<ExpressionActions>(actions_dag, settings.getActionsSettings()) : nullptr;

    auto totals_having = std::make_shared<TotalsHavingTransform>(
        pipeline.getHeader(),
        overflow_row,
        expression_actions,
        filter_column_name,
        totals_mode,
        auto_include_threshold,
        final);

    pipeline.addTotalsHavingTransform(std::move(totals_having));

    if (!blocksHaveEqualStructure(pipeline.getHeader(), output_stream->header))
    {
        auto convert_actions_dag = ActionsDAG::makeConvertingActions(
            pipeline.getHeader().getColumnsWithTypeAndName(),
            output_stream->header.getColumnsWithTypeAndName(),
            ActionsDAG::MatchColumnsMode::Name);
        auto convert_actions = std::make_shared<ExpressionActions>(convert_actions_dag, settings.getActionsSettings());

        pipeline.addSimpleTransform([&](const Block & header) { return std::make_shared<ExpressionTransform>(header, convert_actions); });
    }
}

static String totalsModeToString(TotalsMode totals_mode, double auto_include_threshold)
{
    switch (totals_mode)
    {
        case TotalsMode::BEFORE_HAVING:
            return "before_having";
        case TotalsMode::AFTER_HAVING_INCLUSIVE:
            return "after_having_inclusive";
        case TotalsMode::AFTER_HAVING_EXCLUSIVE:
            return "after_having_exclusive";
        case TotalsMode::AFTER_HAVING_AUTO:
            return "after_having_auto threshold " + std::to_string(auto_include_threshold);
    }

    __builtin_unreachable();
}

void TotalsHavingStep::describeActions(FormatSettings & settings) const
{
    String prefix(settings.offset, ' ');
    settings.out << prefix << "Filter column: " << filter_column_name << '\n';
    settings.out << prefix << "Mode: " << totalsModeToString(totals_mode, auto_include_threshold) << '\n';

    if (actions_dag)
    {
        bool first = true;
        auto expression = std::make_shared<ExpressionActions>(actions_dag);
        for (const auto & action : expression->getActions())
        {
            settings.out << prefix << (first ? "Actions: "
                                             : "         ");
            first = false;
            settings.out << action.toString() << '\n';
        }
    }
}

void TotalsHavingStep::describeActions(JSONBuilder::JSONMap & map) const
{
    map.add("Mode", totalsModeToString(totals_mode, auto_include_threshold));
    if (actions_dag)
    {
        map.add("Filter column", filter_column_name);
        auto expression = std::make_shared<ExpressionActions>(actions_dag);
        map.add("Expression", expression->toTree());
    }
}

std::shared_ptr<TotalsHavingStep> TotalsHavingStep::fromProto(const Protos::TotalsHavingStep &proto, ContextPtr)
{
    auto [step_description, base_input_stream] = ITransformingStep::deserializeFromProtoBase(proto.query_plan_base());
    auto having_filter = proto.has_having_filter() ? deserializeASTFromProto(proto.having_filter()): nullptr;
    auto totals_mode = TotalsModeConverter::fromProto(proto.totals_mode());
    auto step = std::make_shared<TotalsHavingStep>(
        base_input_stream, proto.overflow_row(), having_filter, totals_mode, proto.auto_include_threshold(), proto.final());
    step->setStepDescription(step_description);
    return step;
}

void TotalsHavingStep::toProto(Protos::TotalsHavingStep & proto, bool) const
{
    if (actions_dag)
    {
        throw Exception("actions dag is not supported in protobuf", ErrorCodes::PROTOBUF_BAD_CAST);
    }

    ITransformingStep::serializeToProtoBase(*proto.mutable_query_plan_base());
    if (having_filter)
        serializeASTToProto(having_filter, *proto.mutable_having_filter());
    proto.set_overflow_row(overflow_row);
    proto.set_filter_column_name(filter_column_name);
    proto.set_totals_mode(TotalsModeConverter::toProto(totals_mode));
    proto.set_auto_include_threshold(auto_include_threshold);
    proto.set_final(final);
}

std::shared_ptr<IQueryPlanStep> TotalsHavingStep::copy(ContextPtr) const
{
    return std::make_shared<TotalsHavingStep>(input_streams[0], overflow_row, having_filter, totals_mode, auto_include_threshold, final);
}

}
