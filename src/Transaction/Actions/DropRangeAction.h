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

#include <Storages/MergeTree/MergeTreeDataPartCNCH.h>
#include <Transaction/Actions/IAction.h>
#include <Storages/MergeTree/DeleteBitmapMeta.h>
#include <Transaction/TransactionCommon.h>

namespace DB
{

class DropRangeAction : public IAction
{
public:
    DropRangeAction(const ContextPtr & query_context_, const TxnTimestamp & txn_id_, TransactionRecord record, const StoragePtr table_)
    :
    IAction(query_context_, txn_id_),
    txn_record(std::move(record)),
    table(table_),
    log(&Poco::Logger::get("DropRangeAction"))
    {}

    ~DropRangeAction() override = default;

    void appendPart(MutableMergeTreeDataPartCNCHPtr part);
    void appendDeleteBitmap(DeleteBitmapMetaPtr delete_bitmap);

    /// v1 APIs
    void executeV1(TxnTimestamp commit_time) override;

    /// V2 APIs
    void executeV2() override;
    void postCommit(TxnTimestamp commit_time) override;
    void abort() override;

    UInt32 getSize() const override { return parts.size() + delete_bitmaps.size(); }
private:
    TransactionRecord txn_record;
    const StoragePtr table;
    Poco::Logger * log;

    MutableMergeTreeDataPartsCNCHVector parts;
    DeleteBitmapMetaPtrVector delete_bitmaps;
    bool executed{false};
};

}

