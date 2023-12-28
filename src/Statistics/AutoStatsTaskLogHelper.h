#pragma once
#include <optional>
#include <Interpreters/AutoStatsTaskLog.h>
#include <Statistics/AutoStatisticsHelper.h>
#include <Statistics/AutoStatisticsTaskQueue.h>
#include <Statistics/StatsTableIdentifier.h>

namespace DB::Statistics::AutoStats
{

struct TaskInfoLog : public TaskInfoCore
{
    DateTime64 event_time;
};


std::vector<TaskInfoLog> batchReadTaskLog(ContextPtr context, DateTime64 min_event_time);

void writeTaskLog(ContextPtr context, const TaskInfoCore & core, const String & extra_message = {});
void writeTaskLog(ContextPtr context, const TaskInfo & task, const String & extra_message = {});

}
