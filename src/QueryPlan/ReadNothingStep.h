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
#include <QueryPlan/ISourceStep.h>

namespace DB
{

/// Create NullSource with specified structure.
class ReadNothingStep : public ISourceStep
{
public:
    explicit ReadNothingStep(Block output_header);

    String getName() const override { return "ReadNothing"; }

    Type getType() const override { return Type::ReadNothing; }

    void initializePipeline(QueryPipeline & pipeline, const BuildQueryPipelineSettings &) override;

    void toProto(Protos::ReadNothingStep & proto, bool for_hash_equals = false) const;
    static std::shared_ptr<ReadNothingStep> fromProto(const Protos::ReadNothingStep & proto, ContextPtr context);
    void setUniqueId(Int32 unique_id_) { unique_id = unique_id_; }
    std::shared_ptr<IQueryPlanStep> copy(ContextPtr ptr) const override;

private:
    Int32 unique_id;
};

}
