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

#include <memory>
#include <Processors/Chunk.h>
#include <Processors/Exchange/DataTrans/BroadcastSenderProxy.h>
#include <Processors/Exchange/DataTrans/BroadcastSenderProxyRegistry.h>
#include <Processors/Exchange/DataTrans/DataTrans_fwd.h>
#include <Processors/Exchange/DataTrans/IBroadcastReceiver.h>
#include <Processors/Exchange/DataTrans/IBroadcastSender.h>
#include <Processors/Exchange/DataTrans/Local/LocalBroadcastChannel.h>
#include <Processors/Exchange/DataTrans/Local/LocalChannelOptions.h>
#include <Processors/Exchange/ExchangeDataKey.h>
#include <Processors/tests/gtest_processers_utils.h>
#include <gtest/gtest.h>
#include <Common/tests/gtest_global_context.h>
#include <Common/tests/gtest_utils.h>
#include <Common/tests/gtest_global_context.h>

namespace UnitTest
{
using namespace DB;

TEST(ExchangeLocalBroadcast, LocalBroadcastRegistryTest)
{
    initLogger();
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 1000 * 1000000;
    LocalChannelOptions options{10, ts, false};
    auto data_key = std::make_shared<ExchangeDataKey>(1, 1, 1);
    auto context = getContext().context;
    auto queue = std::make_shared<MultiPathBoundedQueue>(options.queue_size);
    auto channel
        = std::make_shared<LocalBroadcastChannel>(data_key, options, LocalBroadcastChannel::generateNameForTest(), std::move(queue));
    BroadcastSenderProxyPtr local_sender = BroadcastSenderProxyRegistry::instance().getOrCreate(data_key);
    local_sender->becomeRealSender(channel);

    ASSERT_TRUE(BroadcastSenderProxyRegistry::instance().countProxies() == 1);

    local_sender.reset();
    ASSERT_TRUE(BroadcastSenderProxyRegistry::instance().countProxies() == 0);
}


TEST(ExchangeLocalBroadcast, NormalSendRecvTest)
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 1000 * 1000000;
    LocalChannelOptions options{10, ts, false};
    auto data_key = std::make_shared<ExchangeDataKey>(1, 1, 1);
    auto context = getContext().context;
    auto queue = std::make_shared<MultiPathBoundedQueue>(options.queue_size);
    auto channel
        = std::make_shared<LocalBroadcastChannel>(data_key, options, LocalBroadcastChannel::generateNameForTest(), std::move(queue));
    BroadcastSenderProxyPtr local_sender = BroadcastSenderProxyRegistry::instance().getOrCreate(data_key);
    local_sender->becomeRealSender(channel);
    BroadcastReceiverPtr local_receiver = std::dynamic_pointer_cast<IBroadcastReceiver>(channel);

    Chunk chunk = createUInt8Chunk(10, 10, 8);
    auto total_bytes = chunk.bytes();
    setQueryDuration();
    BroadcastStatus status = local_sender->send(std::move(chunk));
    ASSERT_TRUE(status.code == BroadcastStatusCode::RUNNING);

    RecvDataPacket recv_res = local_receiver->recv(100);
    ASSERT_TRUE(std::holds_alternative<Chunk>(recv_res));
    Chunk & recv_chunk = std::get<Chunk>(recv_res);
    ASSERT_TRUE(recv_chunk.getNumRows() == 10);
    ASSERT_TRUE(recv_chunk.bytes() == total_bytes);
}

TEST(ExchangeLocalBroadcast, SendTimeoutTest)
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 200 * 1000000;
    LocalChannelOptions options{1, ts, false};
    auto data_key = std::make_shared<ExchangeDataKey>(1, 1, 1);
    auto context = getContext().context;
    auto queue = std::make_shared<MultiPathBoundedQueue>(options.queue_size);
    auto channel
        = std::make_shared<LocalBroadcastChannel>(data_key, options, LocalBroadcastChannel::generateNameForTest(), std::move(queue));
    BroadcastSenderProxyPtr local_sender = BroadcastSenderProxyRegistry::instance().getOrCreate(data_key);
    local_sender->becomeRealSender(channel);
    BroadcastReceiverPtr local_receiver = std::dynamic_pointer_cast<IBroadcastReceiver>(channel);

    Chunk chunk = createUInt8Chunk(10, 10, 8);
    setQueryDuration();
    BroadcastStatus status = local_sender->send(chunk.clone());
    ASSERT_TRUE(status.code == BroadcastStatusCode::RUNNING);
    BroadcastStatus timeout_status = local_sender->send(chunk.clone());
    ASSERT_TRUE(timeout_status.code == BroadcastStatusCode::SEND_TIMEOUT);
    ASSERT_TRUE(timeout_status.is_modifer == true);
}

TEST(ExchangeLocalBroadcast, AllSendDoneTest)
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 1000 * 1000000;
    LocalChannelOptions options{10, ts, false};
    auto data_key = std::make_shared<ExchangeDataKey>(1, 1, 1);
    auto context = getContext().context;
    auto queue = std::make_shared<MultiPathBoundedQueue>(options.queue_size);
    auto channel
        = std::make_shared<LocalBroadcastChannel>(data_key, options, LocalBroadcastChannel::generateNameForTest(), std::move(queue));
    BroadcastSenderProxyPtr local_sender = BroadcastSenderProxyRegistry::instance().getOrCreate(data_key);
    local_sender->becomeRealSender(channel);
    BroadcastReceiverPtr local_receiver = std::dynamic_pointer_cast<IBroadcastReceiver>(channel);

    Chunk chunk = createUInt8Chunk(10, 10, 8);
    auto total_bytes = chunk.bytes();

    setQueryDuration();
    ASSERT_TRUE(local_sender->send(chunk.clone()).code == BroadcastStatusCode::RUNNING);
    ASSERT_TRUE(local_sender->send(chunk.clone()).code == BroadcastStatusCode::RUNNING);
    local_sender->finish(BroadcastStatusCode::ALL_SENDERS_DONE, "Test graceful close");

    ASSERT_TRUE(std::get<Chunk>(local_receiver->recv(100)).bytes() == total_bytes);
    ASSERT_TRUE(std::get<Chunk>(local_receiver->recv(100)).bytes() == total_bytes);

    /// after consume all data, receiver get the ALL_SENDER_DONE status;
    RecvDataPacket res = local_receiver->recv(100);
    ASSERT_TRUE(std::holds_alternative<BroadcastStatus>(res));

    BroadcastStatus & final_status = std::get<BroadcastStatus>(res);
    ASSERT_TRUE(final_status.code == BroadcastStatusCode::ALL_SENDERS_DONE);
    ASSERT_TRUE(final_status.is_modifer == false);
}

}
