#include <sc-agents-common/utils/GenerationUtils.hpp>
#include <sc-agents-common/utils/AgentUtils.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>
#include <sc-agents-common/keynodes/coreKeynodes.hpp>

#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "FinishAgent.hpp"

using namespace std;
using namespace utils;

namespace fixScheduleModule
{

SC_AGENT_IMPLEMENTATION(FinishAgent)
{
  ScAddr actionNode = otherAddr;
  SC_LOG_ERROR("finish agent started!");

  ScTemplate getInfo;

  getInfo.Triple(
    actionNode,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "node"
  );

  ScTemplateSearchResult resultInfo;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getInfo, resultInfo);
  if(is_success){
    for(int i = 0; i < resultInfo.Size(); i++){
      CreateForecast(resultInfo[i]["node"]);
    }
  }
  

  ScTemplate getEdge;
  getEdge.Triple(
    Keynodes::question_finish,
    ScType::EdgeAccessVarPosPerm >> "edge",
    actionNode
  );
  ScTemplateSearchResult resultEdge;
  is_success = m_memoryCtx.HelperSearchTemplate(getEdge, resultEdge);
  if(is_success){
    m_memoryCtx.EraseElement(resultEdge[0]["edge"]);
  }
  return SC_RESULT_OK;
}

void FinishAgent::CreateForecast(ScAddr info){
  ScTemplate getInfo;

  getInfo.Quintuple(
    info,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "train",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train
  );

  getInfo.Quintuple(
    info,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "type",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train_type
  );

  getInfo.Quintuple(
    info,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  getInfo.Quintuple(
    info,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "priority",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_priority
  );

  getInfo.Quintuple(
    info,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "direction",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_direction
  );

  getInfo.Quintuple(
    info,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "station",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );

  ScAddr train, time, station, type;
  ScTemplateSearchResult resultInfo;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getInfo, resultInfo);
  if(is_success){
    train = resultInfo[0]["train"];
    station = resultInfo[0]["station"];
    time = resultInfo[0]["time"];
    type = resultInfo[0]["type"];
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train));
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(station));
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(time));
  }

  ScTemplate findForecast;

  findForecast.Triple(
    Keynodes::forecast_schedule,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "node"
  );

  findForecast.Quintuple(
    "node",
    ScType::EdgeDCommonVar,
    station,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );
  ScAddr for_station;
  ScTemplateSearchResult resultForecast;
  is_success = m_memoryCtx.HelperSearchTemplate(findForecast, resultForecast);
  if(is_success){
    for_station = resultForecast[0]["node"];
  }

  ScTemplate getTime;

  getTime.Triple(
    for_station,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "for_train"
  );

  getTime.Quintuple(
    "for_train",
    ScType::EdgeDCommonVar,
    train,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train
  );

  getTime.Quintuple(
    "for_train",
    ScType::EdgeDCommonVar,
    type,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train_type
  );

  getTime.Quintuple(
    "for_train",
    ScType::EdgeDCommonVar >> "edge",
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  ScTemplateSearchResult resultTime;
  is_success = m_memoryCtx.HelperSearchTemplate(getTime, resultTime);
  if(is_success){
    SC_LOG_ERROR("find old time");
    m_memoryCtx.EraseElement(resultTime[0]["edge"]);

    ScTemplate genNew;

    genNew.Quintuple(
      resultTime[0]["for_train"],
      ScType::EdgeDCommonVar,
      time,
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_time
    );

    ScTemplateGenResult result;
    m_memoryCtx.HelperGenTemplate(genNew, result);
  }
}
}