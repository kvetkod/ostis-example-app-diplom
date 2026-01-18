#include <sc-agents-common/utils/GenerationUtils.hpp>
#include <sc-agents-common/utils/AgentUtils.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>
#include <sc-agents-common/keynodes/coreKeynodes.hpp>

#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "GetTrainScheduleAgent.hpp"

using namespace std;
using namespace utils;

namespace trainModule
{

SC_AGENT_IMPLEMENTATION(GetTrainScheduleAgent)
{
  ScAddr actionNode = otherAddr;
  SC_LOG_ERROR("my agent started!");
  ScTemplate findAllActualSchedules;
  findAllActualSchedules.Triple(
    actionNode,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "y"
  );

  ScTemplateSearchResult resultStructure;
  bool is_success = m_memoryCtx.HelperSearchTemplate(findAllActualSchedules, resultStructure);

  vector <ScAddr> actualShedules;

  if (is_success)
  {
    for (size_t i = 0; i < resultStructure.Size(); ++i)
    {
      actualShedules.push_back(resultStructure[i]["y"]);
    }
  }

  for (int i=0; i<actualShedules.size(); i++){
    Analysis(actualShedules[i]);
    //break;
  }

  ScTemplate getEdge;

  getEdge.Triple(
    Keynodes::question_get_schedule,
    ScType::EdgeAccessVarPosPerm >> "edge",
    actionNode
  );
  
  ScTemplateSearchResult resultEdge;
  is_success = m_memoryCtx.HelperSearchTemplate(getEdge, resultEdge);
  if (is_success){
    m_memoryCtx.EraseElement(resultEdge[0]["edge"]);
  }

  ScTemplate createForecast;
  createForecast.Triple(
    Keynodes::question_initiated,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::forecast_schedule
  );

  createForecast.Triple(
    Keynodes::question_analyze_forecast,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::forecast_schedule
  );

  ScTemplateGenResult resultForecast;
  m_memoryCtx.HelperGenTemplate(createForecast, resultForecast);
  return SC_RESULT_OK;
}

void GetTrainScheduleAgent::Analysis(ScAddr schedule){
  //all variables
  ScAddr train;
  ScAddr type;
  ScAddr normative;
  ScAddr track;
  ScAddr lastStation;
  ScAddr lastTime;
  vector<ScAddr> prevStations;
  vector<ScAddr> train_at;
  vector<ScAddr> stations;
  vector<string> times;

  //get train

  ScTemplate getTrain;
  getTrain.Quintuple(
    ScType::Unknown >> "train",
    ScType::EdgeDCommonVar,
    schedule,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_actual
  );

  ScTemplateSearchResult resultTrain;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getTrain, resultTrain);
  if(is_success){
    train = resultTrain[0]["train"];
  }

  //get type

  ScTemplate getType;
  getType.Triple(
    ScType::Unknown >> "type",
    ScType::EdgeAccessVarPosPerm,
    train
  );  

  ScTemplateSearchResult resultType;
  is_success = m_memoryCtx.HelperSearchTemplate(getType, resultType);
  if(is_success){
    type = resultType[0]["type"];
  }

  //get last station and time

  ScTemplate getLastStation;

  getLastStation.Quintuple(
    schedule,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "train_at",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::rrel_last
  );

  getLastStation.Quintuple(
    "train_at",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "station",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_arrived
  );

  getLastStation.Quintuple(
    "train_at",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  ScTemplateSearchResult resultLastStation;
  is_success = m_memoryCtx.HelperSearchTemplate(getLastStation, resultLastStation);
  if(is_success){
    lastStation = resultLastStation[0]["station"];
    lastTime = resultLastStation[0]["time"];
  }

  //get prev stations

  ScTemplate getPrevs;

  getPrevs.Quintuple(
    schedule,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "train_at",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::rrel_previous
  );

  getPrevs.Quintuple(
    "train_at",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "station",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_arrived
  );

  ScTemplateSearchResult resultPrevs;
  is_success = m_memoryCtx.HelperSearchTemplate(getPrevs, resultPrevs);
  if(is_success){
    for(int i = 0; i < resultPrevs.Size(); i++){
      prevStations.push_back(resultPrevs[i]["station"]);
    }
  }

  //get normative

  ScTemplate getNormative;

  getNormative.Quintuple(
    train,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "normative",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative
  );

  getNormative.Quintuple(
    "normative",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "train_at",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown
  );

  getNormative.Quintuple(
    "train_at",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "station",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );

  ScTemplateSearchResult resultNormative;
  is_success = m_memoryCtx.HelperSearchTemplate(getNormative, resultNormative);
  if(is_success){
    normative = resultNormative[0]["normative"];
    for(int i = 0; i < resultNormative.Size(); i++){
      bool check = true;
      if(m_memoryCtx.HelperGetSystemIdtf(lastStation) == m_memoryCtx.HelperGetSystemIdtf(resultNormative[i]["station"])){
        check = false;
      }
      for(int j = 0; j < prevStations.size(); j++){
        if(m_memoryCtx.HelperGetSystemIdtf(prevStations[j]) == m_memoryCtx.HelperGetSystemIdtf(resultNormative[i]["station"])){
          check = false;
          break;
        }
      }

      if(check){
        train_at.push_back(resultNormative[i]["train_at"]);
      }
    }
  }

  //get stations
  //get times

  stations.push_back(lastStation);
  times.push_back(m_memoryCtx.HelperGetSystemIdtf(lastTime));

  for(int i = 0; i < train_at.size(); i++){
    ScTemplate getStation;

    getStation.Quintuple(
      train_at[i],
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "station",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_station
    );

    getStation.Quintuple(
      train_at[i],
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "time",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_normative_time
    );

    ScTemplateSearchResult resultStation;
    is_success = m_memoryCtx.HelperSearchTemplate(getStation, resultStation);
    if (is_success) {
      stations.push_back(resultStation[0]["station"]);
      times.push_back(m_memoryCtx.HelperGetSystemIdtf(resultStation[0]["time"]));
    }
  }
  //get track

  ScTemplate getTrack;

  getTrack.Quintuple(
    normative,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "track",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_track
  );

  ScTemplateSearchResult resultTrack;
  is_success = m_memoryCtx.HelperSearchTemplate(getTrack, resultTrack);
  if (is_success) {
    track = resultTrack[0]["track"];
  }

  //get interval

  for (int i = 0; i < stations.size()-1; i++){
    ScTemplate getInterval;

    getInterval.Quintuple(
      track,
      ScType::EdgeAccessVarPosPerm,
      ScType::Unknown >> "name",
      ScType::EdgeAccessVarPosPerm,
      ScType::Unknown 
    );

    getInterval.Triple(
      "name",
      ScType::EdgeAccessVarPosPerm,
      ScType::Unknown >> "node"
    );

    getInterval.Quintuple(
      "node",
      ScType::EdgeDCommonVar,
      type,
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_train_type
    );

    getInterval.Quintuple(
      "node",
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "interval",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_time
    );

    getInterval.Quintuple(
      "name",
      ScType::EdgeAccessVarPosPerm,
      stations[i],
      ScType::EdgeAccessVarPosPerm,
      Keynodes::rrel_1
    );

    getInterval.Quintuple(
      "name",
      ScType::EdgeAccessVarPosPerm,
      stations[i+1],
      ScType::EdgeAccessVarPosPerm,
      Keynodes::rrel_2
    );

    ScTemplateSearchResult resultInterval;
    is_success = m_memoryCtx.HelperSearchTemplate(getInterval, resultInterval);
    if (is_success){
      string interval = m_memoryCtx.HelperGetSystemIdtf(resultInterval[0]["interval"]);
      times[i+1] = CalculateInterval(interval, times[i], times[i+1], stations[i+1]);
      //idk calculate interval for the next if the first was ok or not
    }
  }

  //create forecast

  SC_LOG_ERROR("///////");
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train));
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(type));
  /*
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(lastStation));
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(lastTime));
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(normative));
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(track));
  for(int i = 0; i < prevStations.size(); i++){
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(prevStations[i]));
  }
  for(int i = 0; i < train_at.size(); i++){
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train_at[i]));
  }
  SC_LOG_ERROR(stations.size());
  SC_LOG_ERROR(times.size());
  for(int i = 0; i < times.size(); i++){
    SC_LOG_ERROR(times[i]);
  }
  
  */
  CreateForecast(train, type, times, stations, train_at);
  SC_LOG_ERROR("       ");
  return;
}

string GetTrainScheduleAgent::CalculateInterval(std::string interval, std::string time1, std::string time2, ScAddr station2){
  //get stop time

  int stop_time; 

  ScTemplate getTime;

  getTime.Quintuple(
    station2,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_stop_time
  );

  ScTemplateSearchResult resultTime;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getTime, resultTime);
  if(is_success){
    stop_time = stoi(m_memoryCtx.HelperGetSystemIdtf(resultTime[0]["time"]));
  }
  
  int interval_ = stoi(interval);

  int hours1 = stoi(time1.substr(0, time1.find('.')));
  int minutes1 = stoi(time1.substr(time1.find('.') + 1));
  int hours2 = stoi(time2.substr(0, time2.find('.')));
  int minutes2 = stoi(time2.substr(time2.find('.') + 1));

  int time_1 = hours1 * 60 + minutes1 + stop_time;
  int time_2 = hours2 * 60 + minutes2;

  int result_time = time_1 + interval_;

  int hours = result_time / 60;
  int minutes = result_time % 60;
  ostringstream oss;
  oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
  string new_time = oss.str();

  //SC_LOG_ERROR("time 1");
  //SC_LOG_ERROR(time1);
  //SC_LOG_ERROR(time_1);
  //SC_LOG_ERROR("time 2");
  //SC_LOG_ERROR(time2);
  //SC_LOG_ERROR(time_2);
  //SC_LOG_ERROR("result time");
  //SC_LOG_ERROR(new_time);
  //SC_LOG_ERROR(result_time);

  if(time_2 < result_time){
    return new_time;
  }
  else return time2;
}

void GetTrainScheduleAgent::CreateForecast(ScAddr train, ScAddr type, std::vector<string> times, std::vector<ScAddr> stations, std::vector<ScAddr> train_at){
  for(int i = 1; i < stations.size(); i++){
    ScTemplate getAtStation;

    getAtStation.Triple(
      Keynodes::forecast_schedule,
      ScType::EdgeAccessVarPosPerm,
      ScType::Unknown >> "for_station"
    );

    getAtStation.Quintuple(
      "for_station",
      ScType::EdgeDCommonVar,
      stations[i],
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_station
    );

    ScTemplateSearchResult resultAtStation;
    bool is_success = m_memoryCtx.HelperSearchTemplate(getAtStation, resultAtStation);
    if(is_success){
      ScAddr for_station = resultAtStation[0]["for_station"];
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(for_station));
      string name = "for_" + m_memoryCtx.HelperGetSystemIdtf(train) + "_" + m_memoryCtx.HelperGetSystemIdtf(stations[i]);
      ScAddr new_for_train = m_memoryCtx.HelperFindBySystemIdtf(name);
      if(new_for_train.IsValid()){
        //removing old forecast schedule
        RemoveOldForecast(new_for_train);
      }
      else {
        //if not in forecast schedule
        new_for_train = m_memoryCtx.CreateNode(ScType::NodeConst);
        m_memoryCtx.HelperSetSystemIdtf(name, new_for_train);
        
        ScTemplate createNewNode;

        createNewNode.Triple(
          for_station,
          ScType::EdgeAccessVarPosPerm,
          new_for_train
        );

        ScTemplateGenResult resultCreateNewNode;
        m_memoryCtx.HelperGenTemplate(createNewNode, resultCreateNewNode);
      }
      
      //create time node

      ScAddr time_node = m_memoryCtx.HelperFindBySystemIdtf(times[i]);
      if(!time_node.IsValid()){
        time_node = m_memoryCtx.CreateNode(ScType::NodeConst);
        m_memoryCtx.HelperSetSystemIdtf(times[i], time_node);
      }
      //create direction node
      //create priority node

      ScAddr priority, direction;

      ScTemplate getInfo;

      getInfo.Quintuple(
        train_at[i-1],
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "priority",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_priority
      );

      getInfo.Quintuple(
        train_at[i-1],
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "direction",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_direction
      );

      ScTemplateSearchResult resultInfo;
      bool success = m_memoryCtx.HelperSearchTemplate(getInfo, resultInfo);
      if(success){
        priority = resultInfo[0]["priority"];
        direction = resultInfo[0]["direction"];
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(priority));
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(direction));
      }

      //finish forecast schedule
      ScTemplate createForecast;

      createForecast.Quintuple(
        new_for_train,
        ScType::EdgeDCommonVar,
        train,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_train
      );

      createForecast.Quintuple(
        new_for_train,
        ScType::EdgeDCommonVar,
        type,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_train_type
      );

      createForecast.Quintuple(
        new_for_train,
        ScType::EdgeDCommonVar,
        time_node,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_time
      );

      createForecast.Quintuple(
        new_for_train,
        ScType::EdgeDCommonVar,
        priority,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_priority
      );

      createForecast.Quintuple(
        new_for_train,
        ScType::EdgeDCommonVar,
        direction,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_direction
      );

      ScTemplateGenResult resultForecast;
      m_memoryCtx.HelperGenTemplate(createForecast, resultForecast);
    }
  }
  return;
}

void GetTrainScheduleAgent::RemoveOldForecast(ScAddr for_train){
  ScTemplate removing;

  removing.Quintuple(
    for_train,
    ScType::EdgeDCommonVar >> "edge1",
    ScType::Unknown,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train
  );

  removing.Quintuple(
    for_train,
    ScType::EdgeDCommonVar >> "edge2",
    ScType::Unknown,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train_type
  );

  removing.Quintuple(
    for_train,
    ScType::EdgeDCommonVar >> "edge3",
    ScType::Unknown,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  removing.Quintuple(
    for_train,
    ScType::EdgeDCommonVar >> "edge4",
    ScType::Unknown,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_priority
  );

  removing.Quintuple(
    for_train,
    ScType::EdgeDCommonVar >> "edge5",
    ScType::Unknown,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_direction
  );

  ScTemplateSearchResult resultRemoving;
  bool is_success = m_memoryCtx.HelperSearchTemplate(removing, resultRemoving);
  if (is_success){
    m_memoryCtx.EraseElement(resultRemoving[0]["edge1"]);
    m_memoryCtx.EraseElement(resultRemoving[0]["edge2"]);
    m_memoryCtx.EraseElement(resultRemoving[0]["edge3"]);
    m_memoryCtx.EraseElement(resultRemoving[0]["edge4"]);
    m_memoryCtx.EraseElement(resultRemoving[0]["edge5"]);
  }
  return;
}
}