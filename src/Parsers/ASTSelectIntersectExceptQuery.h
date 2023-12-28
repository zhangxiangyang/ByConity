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

#include <Parsers/ASTSelectQuery.h>
#include <Parsers/ExpressionListParsers.h>


namespace DB
{

class ASTSelectIntersectExceptQuery : public ASTSelectQuery
{
public:
    String getID(char) const override { return "SelectIntersectExceptQuery"; }

    ASTType getType() const override { return ASTType::ASTSelectIntersectExceptQuery; }

    ASTPtr clone() const override;

    ENUM_WITH_PROTO_CONVERTER(
        Operator, // enum name
        Protos::IntersectExceptOperator, // proto enum message
        (UNKNOWN, 0),
        (EXCEPT_ALL, 1),
        (EXCEPT_DISTINCT, 2),
        (INTERSECT_ALL, 3),
        (INTERSECT_DISTINCT, 4));

    void formatImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const override;

    ASTs getListOfSelects() const;

    void serialize(WriteBuffer & buf) const override;
    void deserializeImpl(ReadBuffer & buf) override;
    static ASTPtr deserialize(ReadBuffer & buf);
    static const char * fromOperator(Operator op);
    /// Final operator after applying visitor.
    Operator final_operator = Operator::UNKNOWN;
};

}
