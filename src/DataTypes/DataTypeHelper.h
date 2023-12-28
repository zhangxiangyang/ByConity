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

#include <Core/NamesAndTypes.h>
#include <Core/Types.h>
#include <DataTypes/IDataType.h>

namespace DB
{

namespace Protos
{
    class DataType;
}

class ReadBuffer;
class WriteBuffer;

DataTypePtr createBaseDataTypeFromTypeIndex(TypeIndex index);

void serializeDataTypeV1(const DataTypePtr & data_type, WriteBuffer & buf);
DataTypePtr deserializeDataTypeV1(ReadBuffer & buf);

void serializeDataType(const DataTypePtr & data_type, WriteBuffer & buf);
DataTypePtr deserializeDataType(ReadBuffer & buf);

void serializeDataTypes(const DataTypes & data_types, WriteBuffer & buf);
DataTypes deserializeDataTypes(ReadBuffer & buf);
void serializeDataTypeToProto(const DataTypePtr & data_type, Protos::DataType & proto);
DataTypePtr deserializeDataTypeFromProto(const Protos::DataType & proto);
}
