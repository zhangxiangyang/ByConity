#pragma once

namespace DB
{
using String = std::string;
struct WorkerId 
{
    WorkerId(const String & vw_name_, const String & wg_name_, const String & id_) : vw_name(vw_name_), wg_name(wg_name_), id(id_) { }
    WorkerId() = default;
    String vw_name;
    String wg_name;
    String id;
    const String ToString() const
    {
        return vw_name + "." + wg_name + "." + id;
    }
};
struct WorkerIdEqual
{
    bool operator()(const WorkerId & lhs, const WorkerId & rhs) const
    {
        return lhs.vw_name == rhs.vw_name && lhs.wg_name == rhs.wg_name && lhs.id == rhs.id;
    }
};
struct WorkerIdHash 
{
    std::size_t operator()(const WorkerId & worker_id) const
    {
        return std::hash<String>()(worker_id.ToString());
    }
}; 
} // namespace DB
