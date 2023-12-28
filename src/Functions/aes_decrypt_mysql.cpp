#if !defined(ARCADIA_BUILD)
#    include <Common/config.h>
#endif

#if USE_SSL

#include <Functions/FunctionFactory.h>
#include <Functions/FunctionsAES.h>

namespace
{

struct DecryptMySQLModeImpl
{
    static constexpr auto name = "aes_decrypt_mysql";
    static constexpr auto compatibility_mode = OpenSSLDetails::CompatibilityMode::MySQL;
};

}

namespace DB
{

REGISTER_FUNCTION(AESDecryptMysql)
{
    factory.registerFunction<FunctionDecrypt<DecryptMySQLModeImpl>>();
}

}

#endif
