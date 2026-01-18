#include <sc-agents-common/utils/GenerationUtils.hpp>
#include <sc-agents-common/utils/AgentUtils.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>
#include <sc-agents-common/keynodes/coreKeynodes.hpp>

#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "RepairAgent.hpp"

using namespace std;
using namespace utils;

namespace fixScheduleModule
{

SC_AGENT_IMPLEMENTATION(RepairAgent)
{
  ScAddr actionNode = otherAddr;
  SC_LOG_ERROR("repair agent started!");

  string flag;
  ScTemplate getFlag;

  getFlag.Quintuple(
    actionNode,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "first",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::rrel_1
  );

  getFlag.Quintuple(
    actionNode,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "second",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::rrel_2
  );

  getFlag.Triple(
    actionNode,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "flag"
  );

  ScAddr first, second;
  ScTemplateSearchResult resultFlag;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getFlag, resultFlag);
  if(is_success){
    first = resultFlag[0]["first"];
    second = resultFlag[0]["second"];
    flag = m_memoryCtx.HelperGetSystemIdtf(resultFlag[0]["flag"]);
    SC_LOG_ERROR(flag);
  }

  if(flag == "error_stop"){
    CollisionOnStation(first, second);
  }
  if(flag == "error_braking"){
    CollisionBeforeStation(first, second);
  }
  
  ScTemplate getEdge;
  getEdge.Triple(
    Keynodes::question_repair,
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

void RepairAgent::CollisionOnStation(ScAddr first, ScAddr second){
    ScAddr train1, time1, type1, priority1, direction1;
    ScAddr station;
    ScTemplate getFirst;

    getFirst.Quintuple(
        first, 
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "train",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_train
    );
    getFirst.Quintuple(
        first, 
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "time",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_time
    );
    getFirst.Quintuple(
        first, 
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "station",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_station
    );

    ScTemplateSearchResult resultFirst;
    bool is_success = m_memoryCtx.HelperSearchTemplate(getFirst, resultFirst);
    if(is_success){
        train1 = resultFirst[0]["train"];
        time1 = resultFirst[0]["time"];
        station = resultFirst[0]["station"];
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train1));
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(time1));
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(station));
    }

    ScTemplate getTrain1Type;

    getTrain1Type.Triple(
      ScType::Unknown >> "type",
      ScType::EdgeAccessVarPosPerm,
      train1
    );

    ScTemplateSearchResult resultType1;
    is_success = m_memoryCtx.HelperSearchTemplate(getTrain1Type, resultType1);
    if(is_success){
      type1 = resultType1[0]["type"];
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(type1));
    }

    ScTemplate getPriority;

    getPriority.Quintuple(
      train1,
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
      priority1 = resultPriority[0]["priority"];
      direction1 = resultPriority[0]["direction"];
      SC_LOG_ERROR("priority1");
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(priority1));
    }

    ScAddr train2, time2, type2;
    ScAddr priority2, direction2;

    ScTemplate getSecond;

    getSecond.Quintuple(
      second,
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "train",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_train
    );

    getSecond.Quintuple(
      second,
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "type",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_train_type
    );

    getSecond.Quintuple(
      second,
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "time",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_time
    );

    getSecond.Quintuple(
      second,
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "priority",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_priority
    );

    getSecond.Quintuple(
      second,
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "direction",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_direction
    );

    ScTemplateSearchResult resultSecond;
    is_success = m_memoryCtx.HelperSearchTemplate(getSecond, resultSecond);
    if(is_success){
      train2 = resultSecond[0]["train"];
      type2 = resultSecond[0]["type"];
      time2 = resultSecond[0]["time"];
      priority2 = resultSecond[0]["priority"];
      direction2 = resultSecond[0]["direction"];
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train2));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(type2));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(time2));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(priority2));
    }

    if(m_memoryCtx.HelperGetSystemIdtf(type1) == m_memoryCtx.HelperGetSystemIdtf(type2)){
      vector<ScAddr> lines;
      lines = GetLines(station, type1);
      if(lines.size() < 2){
        //check priority
        int pr1, pr2;
        pr1 = stoi(m_memoryCtx.HelperGetSystemIdtf(priority1));
        pr2 = stoi(m_memoryCtx.HelperGetSystemIdtf(priority2));
        int stopTime;
        ScTemplate getStop;

        getStop.Quintuple(
          station,
          ScType::EdgeDCommonVar,
          ScType::Unknown >> "stop",
          ScType::EdgeAccessVarPosPerm,
          Keynodes::nrel_stop_time
        );

        ScTemplateSearchResult resultStop;
        is_success = m_memoryCtx.HelperSearchTemplate(getStop, resultStop);
        if(is_success){
          stopTime = stoi(m_memoryCtx.HelperGetSystemIdtf(resultStop[0]["stop"]));
          SC_LOG_ERROR("stop time");
          SC_LOG_ERROR(stopTime);
        }

        if(pr1 > pr2){ 
          int t1 = GetTime(time1);
          int t2 = GetTime(time2);

          t1 = t2 + stopTime;

          int hours = t1 / 60;
          int minutes = t1 % 60;
          ostringstream oss;
          oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
          string time_2 = oss.str();
          SC_LOG_ERROR("check1");
          SC_LOG_ERROR(time_2);

          ScAddr time = m_memoryCtx.HelperFindBySystemIdtf(time_2);
          if(!time.IsValid()){
            time = m_memoryCtx.CreateNode(ScType::NodeConst);
            m_memoryCtx.HelperSetSystemIdtf(time_2, time);
          }
          CreateTemporarySchedule(train1, type1, station, time, priority1, direction1);
          StartForecast(train1, station, time);
          return;
        }
        else if(pr1 < pr2){
          int t1 = GetTime(time1);
          int t2 = GetTime(time2);

          t2 = t1 + stopTime;

          int hours = t2 / 60;
          int minutes = t2 % 60;
          ostringstream oss;
          oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
          string time_2 = oss.str();
          SC_LOG_ERROR("check2");
          SC_LOG_ERROR(time_2);

          ScAddr time = m_memoryCtx.HelperFindBySystemIdtf(time_2);
          if(!time.IsValid()){
            time = m_memoryCtx.CreateNode(ScType::NodeConst);
            m_memoryCtx.HelperSetSystemIdtf(time_2, time);
          }
          CreateTemporarySchedule(train2, type2, station, time, priority2, direction2);
          StartForecast(train2, station, time);
          return;
        }
      }
      else{
        CreateTemporarySchedule(train1, type1, station, time1, priority1, direction1);
        StartForecast(train1, station, time1);
        return;
      }
    }
    else{
      CreateTemporarySchedule(train1, type1, station, time1, priority1, direction1);
      StartForecast(train1, station, time1);
      return;
    }

    return;
}


void RepairAgent::CreateTemporarySchedule(ScAddr train, ScAddr type, ScAddr station, ScAddr time, ScAddr priority, ScAddr direction){
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


void RepairAgent::StartForecast(ScAddr train, ScAddr station, ScAddr time){
  ScAddr empty = m_memoryCtx.CreateNode(ScType::NodeConst);
  ScAddr node = m_memoryCtx.CreateNode(ScType::NodeConst);

  ScTemplate createNode;
  createNode.Quintuple(
    node,
    ScType::EdgeDCommonVar,
    station,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_arrived
  );

  createNode.Quintuple(
    node,
    ScType::EdgeDCommonVar,
    time,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  ScTemplateGenResult resultNode;
  m_memoryCtx.HelperGenTemplate(createNode, resultNode);

  ScTemplate startAgent;
  startAgent.Triple(
    empty,
    ScType::EdgeAccessVarPosPerm,
    node
  );
  startAgent.Triple(
    empty,
    ScType::EdgeAccessVarPosPerm,
    train
  );
  startAgent.Triple(
    Keynodes::question_forecasting,
    ScType::EdgeAccessVarPosPerm,
    empty
  );

  ScTemplateGenResult resultAgent;
  m_memoryCtx.HelperGenTemplate(startAgent, resultAgent);
  
  return;
}

int RepairAgent::GetTime(ScAddr time){
  string name = m_memoryCtx.HelperGetSystemIdtf(time);

  int hours1 = stoi(name.substr(0, name.find('.')));
  int minutes1 = stoi(name.substr(name.find('.') + 1));
  int time_ = hours1 * 60 + minutes1;

  return time_;
}

std::vector <ScAddr> RepairAgent::GetLines(ScAddr station, ScAddr type){
  vector<ScAddr> lines;
  ScTemplate getLines;

  getLines.Triple(
    station,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "node"
  );

  getLines.Quintuple(
    "node",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "line",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_line
  );

  getLines.Quintuple(
    "node",
    ScType::EdgeDCommonVar,
    type,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train_type
  );

  ScTemplateSearchResult resultLines;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getLines, resultLines);
  if(is_success){
    for(int i = 0; i < resultLines.Size(); i++){
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultLines[i]["line"]));
      lines.push_back(resultLines[i]["line"]);
    }
  }

  return lines;
}

void RepairAgent::CollisionBeforeStation(ScAddr first, ScAddr second){
  ScAddr train1, time1, type1, priority1, direction1;
  ScAddr station;
  ScTemplate getFirst;

  getFirst.Quintuple(
      first, 
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "train",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_train
  );
  getFirst.Quintuple(
      first, 
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "time",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_time
  );
  getFirst.Quintuple(
      first, 
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "station",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_station
  );

  ScTemplateSearchResult resultFirst;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getFirst, resultFirst);
  if(is_success){
      train1 = resultFirst[0]["train"];
      time1 = resultFirst[0]["time"];
      station = resultFirst[0]["station"];
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train1));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(time1));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(station));
  }

  ScTemplate getTrain1Type;

  getTrain1Type.Triple(
    ScType::Unknown >> "type",
    ScType::EdgeAccessVarPosPerm,
    train1
  );

  ScTemplateSearchResult resultType1;
  is_success = m_memoryCtx.HelperSearchTemplate(getTrain1Type, resultType1);
  if(is_success){
    type1 = resultType1[0]["type"];
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(type1));
  }

  ScTemplate getPriority;

  getPriority.Quintuple(
    train1,
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
    priority1 = resultPriority[0]["priority"];
    direction1 = resultPriority[0]["direction"];
    SC_LOG_ERROR("priority1");
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(priority1));
  }

  ScAddr train2, time2, type2;
  ScAddr priority2, direction2;

  ScTemplate getSecond;

  getSecond.Quintuple(
    second,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "train",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train
  );

  getSecond.Quintuple(
    second,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "type",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train_type
  );

  getSecond.Quintuple(
    second,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  getSecond.Quintuple(
    second,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "priority",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_priority
  );

  getSecond.Quintuple(
    second,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "direction",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_direction
  );

  ScTemplateSearchResult resultSecond;
  is_success = m_memoryCtx.HelperSearchTemplate(getSecond, resultSecond);
  if(is_success){
    train2 = resultSecond[0]["train"];
    type2 = resultSecond[0]["type"];
    time2 = resultSecond[0]["time"];
    priority2 = resultSecond[0]["priority"];
    direction2 = resultSecond[0]["direction"];
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train2));
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(type2));
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(time2));
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(priority2));
  }

  int stopTime, brakingTime;

  ScTemplate getTimes;

  getTimes.Quintuple(
    station,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "stop_time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_stop_time
  );

  getTimes.Quintuple(
    station,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "braking_time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_braking_time
  );

  ScTemplateSearchResult resultTimes;
  is_success = m_memoryCtx.HelperSearchTemplate(getTimes, resultTimes);
  if(is_success){
    stopTime = stoi(m_memoryCtx.HelperGetSystemIdtf(resultTimes[0]["stop_time"]));
    brakingTime = stoi(m_memoryCtx.HelperGetSystemIdtf(resultTimes[0]["braking_time"]));
  }

  int t1 = GetTime(time1);
  int t2 = GetTime(time2);
  int pr1 = stoi(m_memoryCtx.HelperGetSystemIdtf(priority1));
  int pr2 = stoi(m_memoryCtx.HelperGetSystemIdtf(priority2));

  if(m_memoryCtx.HelperGetSystemIdtf(direction1) == m_memoryCtx.HelperGetSystemIdtf(direction2)){
    if(m_memoryCtx.HelperGetSystemIdtf(direction1) == "reverse_direction"){
      t1 += stopTime;
      t2 += stopTime;
    }
  }
  else if(m_memoryCtx.HelperGetSystemIdtf(direction1) == "forward_direction"){
    t2 += stopTime;
  }
  else if(m_memoryCtx.HelperGetSystemIdtf(direction2) == "forward_direction"){
    t1 += stopTime;
  }

  if(pr1 < pr2){
    int time_2 = t1 + brakingTime;
    int hours = time_2 / 60;
    int minutes = time_2 % 60;
    ostringstream oss;
    oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
    string time_new = oss.str();
    SC_LOG_ERROR("check1");
    SC_LOG_ERROR(time_new);

    ScAddr time = m_memoryCtx.HelperFindBySystemIdtf(time_new);
    if(!time.IsValid()){
      time = m_memoryCtx.CreateNode(ScType::NodeConst);
      m_memoryCtx.HelperSetSystemIdtf(time_new, time);
    }
    CreateTemporarySchedule(train2, type2, station, time, priority2, direction2);
    StartForecast(train2, station, time);
    return;
  }
  else{
    int time_2 = t2 + brakingTime;
    int hours = time_2 / 60;
    int minutes = time_2 % 60;
    ostringstream oss;
    oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
    string time_new = oss.str();
    SC_LOG_ERROR("check2");
    SC_LOG_ERROR(time_new);

    ScAddr time = m_memoryCtx.HelperFindBySystemIdtf(time_new);
    if(!time.IsValid()){
      time = m_memoryCtx.CreateNode(ScType::NodeConst);
      m_memoryCtx.HelperSetSystemIdtf(time_new, time);
    }
    CreateTemporarySchedule(train1, type1, station, time, priority1, direction1);
    StartForecast(train1, station, time);
    return;
  }

  return;
}
}