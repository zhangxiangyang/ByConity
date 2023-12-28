/*
 * Copyright 2016-2023 ClickHouse, Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*
 * This file may have been modified by Bytedance Ltd. and/or its affiliates (“ Bytedance's Modifications”).
 * All Bytedance's Modifications are Copyright (2023) Bytedance Ltd. and/or its affiliates.
 */

#include <Poco/Net/DNS.h>
#include <common/getFQDNOrHostName.h>


namespace
{
    std::string getFQDNOrHostNameImpl()
    {
        try
        {
            return Poco::Net::DNS::thisHost().name();
        }
        catch (...)
        {
            return Poco::Net::DNS::hostName();
        }
    }

    std::string getIPOrFQDNOrHostNameImpl()
    {
        try
        {
            auto this_host = Poco::Net::DNS::thisHost();
            if (this_host.addresses().size() > 0)
                return this_host.addresses().front().toString();
            else
                return this_host.name();
        }
        catch (...)
        {
            return Poco::Net::DNS::hostName();
        }
    }
}


const std::string & getFQDNOrHostName()
{
    static std::string result = getFQDNOrHostNameImpl();
    return result;
}

const std::string & getIPOrFQDNOrHostName()
{
    static std::string result = getIPOrFQDNOrHostNameImpl();
    return result;
}

std::string getHostFromHostPort(const std::string & host_port)
{
    if (host_port.starts_with("["))
    {
            auto pos = host_port.find(']');
            return host_port.substr(1, pos - 1);
    }

    auto pos = host_port.find(':');
    return host_port.substr(0, pos - 1);
}
