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

#include <Columns/ColumnConst.h>
#include <DataTypes/Serializations/ISerialization.h>
#include <Optimizer/Utils.h>
#include <QueryPlan/SetOperationStep.h>

namespace DB
{

ColumnPtr getCommonColumnForUnion(const std::vector<const ColumnWithTypeAndName *> & columns)
{
    ColumnWithTypeAndName result = *columns[0];
    size_t num_const = 0;
    DataTypes types(columns.size());
    for (size_t i = 0; i < columns.size(); ++i)
    {
        types[i] = columns[i]->type;
        if (isColumnConst(*columns[i]->column))
            ++num_const;
    }

    static auto same_constants = [](const IColumn & a, const IColumn & b) {
        return assert_cast<const ColumnConst &>(a).getField() == assert_cast<const ColumnConst &>(b).getField();
    };

    /// Create supertype column saving constness if possible.
    bool save_constness = false;
    if (columns.size() == num_const)
    {
        save_constness = true;
        for (size_t i = 1; i < columns.size(); ++i)
        {
            const ColumnWithTypeAndName & first = *columns[0];
            const ColumnWithTypeAndName & other = *columns[i];

            if (!same_constants(*first.column, *other.column))
            {
                save_constness = false;
                break;
            }
        }
    }

    ColumnPtr column = result.type->createColumn();
    if (save_constness)
        column = result.type->createColumnConst(0, assert_cast<const ColumnConst &>(*columns[0]->column).getField());

    return column;
}

SetOperationStep::SetOperationStep(DataStreams input_streams_, DataStream output_stream_, OutputToInputs output_to_inputs_)
    : output_to_inputs(std::move(output_to_inputs_))
{
    input_streams = std::move(input_streams_);

    if (output_stream_.header.getNamesAndTypes().empty())
        output_stream = input_streams.front();
    else
    {
        output_stream = output_stream_;
    }

    size_t num_selects = input_streams.size();
    std::vector<const ColumnWithTypeAndName *> columns(num_selects);
    for (size_t column_num = 0; column_num < output_stream->header.columns(); ++column_num)
    {
        ColumnWithTypeAndName & result_elem = output_stream->header.getByPosition(column_num);
        for (size_t i = 0; i < num_selects; ++i)
        {
            if (output_to_inputs.contains(result_elem.name))
            {
                for (auto & input_name : output_to_inputs[result_elem.name])
                {
                    if (input_streams[i].header.findByName(input_name))
                        columns[i] = input_streams[i].header.findByName(input_name);
                }
            }
            else
                columns[i] = &input_streams[i].header.getByPosition(column_num);
        }
        result_elem.column = getCommonColumnForUnion(columns);
    }

    if (output_to_inputs.empty())
    {
        for (size_t i = 0; i < output_stream->header.columns(); ++i)
        {
            String output_symbol = output_stream->header.getByPosition(i).name;
            std::vector<String> inputs;
            for (auto & input_stream : input_streams)
            {
                String input_symbol = input_stream.header.getByPosition(i).name;
                inputs.emplace_back(input_symbol);
            }
            output_to_inputs[output_symbol] = inputs;
        }
    }

    for (const auto & value : output_to_inputs)
    {
        Utils::checkArgument(
            value.second.size() == input_streams.size(), "Every source needs to map its symbols to an output operation symbol");
    }

    // Make sure each source positionally corresponds to their Symbol values in the Multimap
    for (size_t i = 0; i < input_streams.size(); i++)
    {
        for (auto value : output_to_inputs)
        {
            const Names & input_symbols = input_streams[i].header.getNames();
            String symbol = value.second[i];
            Utils::checkArgument(
                std::find(input_symbols.begin(), input_symbols.end(), symbol) != input_symbols.end(),
                "Every source needs to map its symbols to an output operation symbol");
        }
    }
}

void SetOperationStep::setInputStreams(const DataStreams & input_streams_)
{
    input_streams = input_streams_;
}

const OutputToInputs & SetOperationStep::getOutToInputs() const
{
    return output_to_inputs;
}

void SetOperationStep::serializeToProtoBase(Protos::SetOperationStep & proto) const
{
    for (const auto & element : input_streams)
        element.toProto(*proto.add_input_streams());
    if (!output_stream.has_value())
        throw Exception("empty output stream", ErrorCodes::LOGICAL_ERROR);
    output_stream.value().toProto(*proto.mutable_output_stream());
    serializeMapToProto(output_to_inputs, *proto.mutable_output_to_inputs());
}

std::tuple<DataStreams, DataStream, std::unordered_map<String, std::vector<String>>>
SetOperationStep::deserializeFromProtoBase(const Protos::SetOperationStep & proto)
{
    DataStreams input_streams;
    for (const auto & proto_element : proto.input_streams())
    {
        DataStream element;
        element.fillFromProto(proto_element);
        input_streams.emplace_back(std::move(element));
    }
    DataStream output_stream;
    output_stream.fillFromProto(proto.output_stream());
    auto output_to_inputs = deserializeMapFromProto<String, std::vector<String>>(proto.output_to_inputs());

    return std::make_tuple(input_streams, output_stream, output_to_inputs);
}
}
