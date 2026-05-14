#include <sc-agents-common/utils/GenerationUtils.hpp>
#include <sc-agents-common/utils/AgentUtils.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>
#include <sc-agents-common/keynodes/coreKeynodes.hpp>

#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "AnalyzerAgent.hpp"

using namespace std;
using namespace utils;

namespace fixScheduleModule
{

SC_AGENT_IMPLEMENTATION(AnalyzerAgent)
{
  ScAddr actionNode = otherAddr;
  SC_LOG_ERROR("analyzer started!");

  Analyze(actionNode);

  CheckStack(actionNode);

  ScTemplate getEdge;

  getEdge.Triple(
    Keynodes::question_analyze,
    ScType::EdgeAccessVarPosPerm >> "edge",
    actionNode
  );

  ScTemplateSearchResult resultEdge;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getEdge, resultEdge);
  if(is_success){
    m_memoryCtx.EraseElement(resultEdge[0]["edge"]);
  }
  return SC_RESULT_OK;
}

void AnalyzerAgent::CheckStack(ScAddr forecast){
  ScTemplate check;

  check.Triple(
    Keynodes::stack,
    ScType::EdgeAccessVarPosPerm >> "edge",
    ScType::Unknown >> "node"
  );

  ScTemplateSearchResult result;
  bool is_success = m_memoryCtx.HelperSearchTemplate(check, result);
  if(is_success){
    ScAddr node = result[0]["node"];
    m_memoryCtx.EraseElement(result[0]["edge"]);

    ScTemplate startAgent;

    startAgent.Triple(
      Keynodes::question_repair,
      ScType::EdgeAccessVarPosPerm,
      node
    );

    ScTemplateGenResult resultAgent;
    m_memoryCtx.HelperGenTemplate(startAgent, resultAgent);
    
  }

  else{
    ScTemplate startAgent;

    startAgent.Triple(
      Keynodes::question_finish,
      ScType::EdgeAccessVarPosPerm,
      Keynodes::temporary_schedule
    );

    ScTemplateGenResult resultAgent;
    m_memoryCtx.HelperGenTemplate(startAgent, resultAgent);
  }
  return;
}

void AnalyzerAgent::Analyze(ScAddr forecast){
  ScTemplate getInfo;

  getInfo.Triple(
    forecast,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "for_station"
  );  

  getInfo.Quintuple(
    "for_station",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "station",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_arrived
  );

  getInfo.Quintuple(
    "for_station",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  getInfo.Quintuple(
    "for_station",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "train",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train
  );
  vector<ScAddr> stations;
  vector<ScAddr> times;
  ScTemplateSearchResult resultInfo;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getInfo, resultInfo);
  if(is_success){
    for(int i = 0; i < resultInfo.Size(); i++){
      ScAddr train1, station, time1;
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInfo[i]["station"]));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInfo[i]["time"]));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInfo[i]["train"]));
      train1 = resultInfo[i]["train"];
      station = resultInfo[i]["station"];
      time1 = resultInfo[i]["time"];
      //check max interval before
      if(CheckCollision(station, train1, time1)){
        break;
      }
      //if(i == resultInfo.Size()-1){
        CreateTemporarySchedule(train1, station, time1);
     // }
      stations.push_back(station);
      times.push_back(time1);
    }
  }

  return;
}

void AnalyzerAgent::CreateTemporarySchedule(ScAddr train, ScAddr station, ScAddr time){
  ScAddr type, priority, direction;
  ScTemplate getType;

  getType.Triple(
    ScType::Unknown >> "type",
    ScType::EdgeAccessVarPosPerm,
    train
  );

  ScTemplateSearchResult resultType;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getType, resultType);
  if(is_success){
    type = resultType[0]["type"];
  }

  ScTemplate getPriority;
  getPriority.Quintuple(
    train,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "normative",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative
  );

  getPriority.Quintuple(
    "normative",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "at_station",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown 
  );

  getPriority.Quintuple(
    "at_station",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative_time
  );

  getPriority.Quintuple(
    "at_station",
    ScType::EdgeDCommonVar,
    station,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );

  getPriority.Quintuple(
    "at_station",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "priority",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_priority
  );

  getPriority.Quintuple(
    "at_station",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "direction",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_direction
  );

  ScTemplateSearchResult resultPriority;
  is_success = m_memoryCtx.HelperSearchTemplate(getPriority, resultPriority);
  if(is_success){
    priority = resultPriority[0]["priority"];
    direction = resultPriority[0]["direction"];
  }

  string name = "temporary_" + m_memoryCtx.HelperGetSystemIdtf(train) + "_" + m_memoryCtx.HelperGetSystemIdtf(station);
  ScAddr node = m_memoryCtx.HelperFindBySystemIdtf(name);
  if(node.IsValid()){
    ScTemplate getEdge;
    getEdge.Quintuple(
      node,
      ScType::EdgeDCommonVar >> "edge",
      ScType::Unknown,
      ScType::EdgeAccessVarPosPerm,
      ScType::Unknown
    );

    ScTemplateSearchResult resultEdge;
    bool is_success = m_memoryCtx.HelperSearchTemplate(getEdge, resultEdge);
    if(is_success){
      for(int i = 0; i < resultEdge.Size(); i++){
        m_memoryCtx.EraseElement(resultEdge[i]["edge"]);
      }
    }
  }
  else{
    node = m_memoryCtx.CreateNode(ScType::NodeConst);
    m_memoryCtx.HelperSetSystemIdtf(name, node);
  }

  ScTemplate createTemporary;

  createTemporary.Triple(
    Keynodes::temporary_schedule,
    ScType::EdgeAccessVarPosPerm,
    node
  );

  createTemporary.Quintuple(
    node,
    ScType::EdgeDCommonVar,
    train,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train
  );

  createTemporary.Quintuple(
    node,
    ScType::EdgeDCommonVar,
    type,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train_type
  );

  createTemporary.Quintuple(
    node,
    ScType::EdgeDCommonVar,
    time,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  createTemporary.Quintuple(
    node,
    ScType::EdgeDCommonVar,
    priority,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_priority
  );

  createTemporary.Quintuple(
    node,
    ScType::EdgeDCommonVar,
    direction,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_direction
  );

  createTemporary.Quintuple(
    node,
    ScType::EdgeDCommonVar,
    station,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );

  ScTemplateGenResult resultCreate;
  m_memoryCtx.HelperGenTemplate(createTemporary, resultCreate);
  return;
}


bool AnalyzerAgent::CheckCollision(ScAddr station, ScAddr train1, ScAddr time1){
  int stop_time, braking_time;
  ScTemplate getTimes;

  getTimes.Quintuple(
    station,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "stop",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_stop_time
  );

  getTimes.Quintuple(
    station,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "braking",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_braking_time
  );

  ScTemplateSearchResult resultTimes;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getTimes, resultTimes);
  if(is_success){
    stop_time = stoi(m_memoryCtx.HelperGetSystemIdtf(resultTimes[0]["stop"]));
    braking_time = stoi(m_memoryCtx.HelperGetSystemIdtf(resultTimes[0]["braking"]));
    SC_LOG_ERROR("times");
    SC_LOG_ERROR(stop_time);
    SC_LOG_ERROR(braking_time);
  }

  ScTemplate getForecastSchedule;

  getForecastSchedule.Triple(
    Keynodes::forecast_schedule,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "for_station"
  );

  getForecastSchedule.Quintuple(
    "for_station",
    ScType::EdgeDCommonVar,
    station,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );

  
  ScAddr for_station;
  ScTemplateSearchResult resultForecastSchedule;
  is_success = m_memoryCtx.HelperSearchTemplate(getForecastSchedule, resultForecastSchedule);
  if(is_success){
    for_station = resultForecastSchedule[0]["for_station"];
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(for_station));
  }

  ScTemplate getInfo;

  getInfo.Triple(
    for_station,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "for_train"
  );

  getInfo.Quintuple(
    "for_train",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "train",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train
  );

  getInfo.Quintuple(
    "for_train",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "type",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train_type
  );

  getInfo.Quintuple(
    "for_train",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  getInfo.Quintuple(
    "for_train",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "priority",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_priority
  );

  getInfo.Quintuple(
    "for_train",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "direction",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_direction
  );

  ScTemplateSearchResult resultInfo;
  is_success = m_memoryCtx.HelperSearchTemplate(getInfo, resultInfo);
  if(is_success){
    for(int i = 0; i < resultInfo.Size(); i++){

      if(m_memoryCtx.HelperGetSystemIdtf(train1) == m_memoryCtx.HelperGetSystemIdtf(resultInfo[i]["train"])){
        continue;
      }

      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train1));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(time1));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInfo[i]["train"]));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInfo[i]["time"]));
      int time_1, time_2;
      time_1 = GetTime(time1);
      time_2 = GetTime(resultInfo[i]["time"]);

      if(time_2 == time_1){
        //create stack
        SC_LOG_ERROR("create stack, error stop");
        CreateStack(train1, time1, station, resultInfo[i]["for_train"], Keynodes::error_stop);
        return true;
      }
      if(time_2 < time_1 && time_1 < time_2 + stop_time){
        //create stack
        SC_LOG_ERROR("create stack, error stop");
        CreateStack(train1, time1, station, resultInfo[i]["for_train"], Keynodes::error_stop);
        return true;
      }
      if(time_1 < time_2 && time_2 < time_1 + stop_time){
        //create stack
        SC_LOG_ERROR("create stack, error stop");
        CreateStack(train1, time1, station, resultInfo[i]["for_train"], Keynodes::error_stop);
        return true;
      }

      if(time_2 - braking_time < time_1 && time_1 < time_2){
        SC_LOG_ERROR("create stack, error braking");
        CreateStack(train1, time1, station, resultInfo[i]["for_train"], Keynodes::error_braking);
        return true;
      }

      if(time_1 - braking_time < time_2 && time_2 < time_1){
        SC_LOG_ERROR("create stack, error braking");
        CreateStack(train1, time1, station, resultInfo[i]["for_train"], Keynodes::error_braking);
        return true;
      }

      if(time_2 - braking_time < time_1 && time_1 < time_2){
        SC_LOG_ERROR("create stack, error braking");
        CreateStack(train1, time1, station, resultInfo[i]["for_train"], Keynodes::error_braking);
        return true;
      }

      if(time_2 + stop_time < time_1 && time_1 < time_2 + stop_time + braking_time){
        SC_LOG_ERROR("create stack, error braking");
        CreateStack(train1, time1, station, resultInfo[i]["for_train"], Keynodes::error_braking);
        return true;
      }

      if(time_1 + stop_time < time_2 && time_2 < time_1 + stop_time + braking_time){
        SC_LOG_ERROR("create stack, error braking");
        CreateStack(train1, time1, station, resultInfo[i]["for_train"], Keynodes::error_braking);
        return true;
      }


    }
  }

  return false;
}

int AnalyzerAgent::GetTime(ScAddr time){
  string name = m_memoryCtx.HelperGetSystemIdtf(time);

  int hours1 = stoi(name.substr(0, name.find('.')));
  int minutes1 = stoi(name.substr(name.find('.') + 1));
  int time_ = hours1 * 60 + minutes1;

  return time_;
}

void AnalyzerAgent::CreateStack(ScAddr train, ScAddr time, ScAddr station, ScAddr for_train, ScAddr flag){
  ScAddr empty_node_1 = m_memoryCtx.CreateNode(ScType::NodeConst);
  ScAddr empty_node_2 = m_memoryCtx.CreateNode(ScType::NodeConst);
  ScTemplate create;

  create.Triple(
    Keynodes::stack,
    ScType::EdgeAccessVarPosPerm,
    empty_node_1
  );

  create.Quintuple(
    empty_node_1,
    ScType::EdgeAccessVarPosPerm,
    empty_node_2,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::rrel_1
  );

  create.Quintuple(
    empty_node_1,
    ScType::EdgeAccessVarPosPerm,
    for_train,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::rrel_2
  );

  create.Triple(
    empty_node_1,
    ScType::EdgeAccessVarPosPerm,
    flag
  );

  create.Quintuple(
    empty_node_2,
    ScType::EdgeDCommonVar,
    train,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train
  );

  create.Quintuple(
    empty_node_2,
    ScType::EdgeDCommonVar,
    time,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  create.Quintuple(
    empty_node_2,
    ScType::EdgeDCommonVar,
    station,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );

  ScTemplateGenResult result;
  m_memoryCtx.HelperGenTemplate(create, result);
  return;
}
}