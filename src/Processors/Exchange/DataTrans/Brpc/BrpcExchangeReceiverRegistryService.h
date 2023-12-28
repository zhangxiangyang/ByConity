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

#include <Interpreters/Context.h>
#include <Interpreters/Context_fwd.h>
#include <Processors/Exchange/DataTrans/BroadcastSenderProxyRegistry.h>
#include <Processors/Exchange/DataTrans/DataTrans_fwd.h>
#include <Processors/Executors/PipelineExecutor.h>
#include <Protos/registry.pb.h>
#include <brpc/server.h>
#include <brpc/stream.h>
#include <Common/Brpc/BrpcServiceDefines.h>

namespace DB
{
class BrpcExchangeReceiverRegistryService : public Protos::RegistryService
{
public:
    enum RegisterMode
    {
        BRPC = 0,
        DISK_READER = 1
    };
    explicit BrpcExchangeReceiverRegistryService(ContextMutablePtr context_)
        : context(context_), max_buf_size(context_->getSettingsRef().exchange_stream_max_buf_size)
    {
    }
    explicit BrpcExchangeReceiverRegistryService(int max_buf_size_) : max_buf_size(max_buf_size_)
    {
    }

    /// register the brpc sender of pipeline mode,
    /// exchange data is loaded from in-memory pipeline executions
    void registry(
        ::google::protobuf::RpcController * controller,
        const ::DB::Protos::RegistryRequest * request,
        ::DB::Protos::RegistryResponse * response,
        ::google::protobuf::Closure * done) override;

    /// register the brpc sender of bsp mode,
    /// exchange data is loaded from disk dumped by previous execution
    void registerBRPCSenderFromDisk(
        ::google::protobuf::RpcController * controller,
        const ::DB::Protos::RegistryDiskSenderRequest * request,
        ::DB::Protos::RegistryResponse * response,
        ::google::protobuf::Closure * done) override;

    /// cancel exchange data reader(only bsp mode)
    void cancelExchangeDataReader(
        ::google::protobuf::RpcController * controller,
        const ::DB::Protos::CancelExchangeDataReaderRequest * request,
        ::DB::Protos::CancelExchangeDataReaderResponse * response,
        ::google::protobuf::Closure * done) override;

    void cleanupExchangeData(
        ::google::protobuf::RpcController * controller,
        const ::DB::Protos::CleanupExchangeDataRequest * request,
        ::DB::Protos::CleanupExchangeDataResponse * response,
        ::google::protobuf::Closure * done) override;

    void sendExchangeDataHeartbeat(
        ::google::protobuf::RpcController * controller,
        const ::DB::Protos::ExchangeDataHeartbeatRequest * request,
        ::DB::Protos::ExchangeDataHeartbeatResponse * response,
        ::google::protobuf::Closure * done) override;

    void setContext(ContextMutablePtr context_)
    {
        context = context_;
    }

private:
    ContextMutablePtr context;
    int max_buf_size;
    Poco::Logger * log = &Poco::Logger::get("BrpcExchangeReceiverRegistryService");

    /// stream will be accepted, but the host socket of the accpeted stream
    /// is not really set yet until done->Run() is called
    void acceptStream(
        brpc::Controller * cntl,
        uint64_t accept_timeout_ms,
        BroadcastSenderProxyPtr sender,
        const String & query_id,
        brpc::StreamId & sender_stream_id);

    /// proxy will become real sender in this method
    void registerSenderToProxy(
        brpc::Controller * cntl,
        const DiskExchangeDataManagerPtr & mgr,
        const BroadcastSenderProxyPtr & sender_proxy,
        const String & query_id,
        const brpc::StreamId & sender_stream_id,
        Processors processors,
        const ExchangeDataKeyPtr & key,
        bool read_from_disk);
};

REGISTER_SERVICE_IMPL(BrpcExchangeReceiverRegistryService);
}
