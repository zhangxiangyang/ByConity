#pragma once

#include <common/singleton.h>

#if __has_include(<Common/config.h>)
    #include <Common/config.h>
#endif

#if USE_JEMALLOC
    #include <atomic>
    #include <Core/Types.h>
    #include <Poco/Logger.h>
    #include <Poco/Util/AbstractConfiguration.h>
#endif

namespace DB
{
class JeprofControl : public ext::singleton<JeprofControl>
{
private:
    bool jemalloc_prof_active {false};

public:
    bool jeprofEnabled()
    {
        return jemalloc_prof_active;
    }

#if USE_JEMALLOC
public:
    bool jeprofInitialize(bool enable_by_conf);
    void setJeprofStatus(bool active);
    void setProfPath(const String & prof_path_);
    void dump();

    void loadFromConfig(const Poco::Util::AbstractConfiguration & config);
private:
    template<typename T>
    void setJemallocMetric(const String & name, T value);

    String prof_path{"/tmp/"};
    mutable std::atomic<size_t> index{0};
    Poco::Logger * log{&Poco::Logger::get("JeprofControl")};
#endif

};

inline bool jeprofEnabled() { return DB::JeprofControl::instance().jeprofEnabled(); }
}
