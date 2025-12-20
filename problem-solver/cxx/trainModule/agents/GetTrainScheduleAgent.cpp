/*
* This source file is part of an OSTIS project. For the latest info, see http://ostis.net
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

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
vector<ScAddr> all_trains;

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
  bool const is_success = m_memoryCtx.HelperSearchTemplate(findAllActualSchedules, resultStructure);

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

  FinishForecastSchedule();
  SC_LOG_ERROR("forecast schedule is formed");
  WhichTrainsAreComing();
  utils::AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, true);
  return SC_RESULT_OK;
}


void GetTrainScheduleAgent::Analysis(ScAddr schedule){
  SC_LOG_ERROR("/////////////////////////////////////////");
  //get train
  ScTemplate getTrain;
  getTrain.Quintuple(
    ScType::Unknown >> "y",
    ScType::EdgeDCommonVar,
    schedule,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_actual
  );
  ScTemplateSearchResult resultGetTrain;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getTrain, resultGetTrain);
  ScAddr train;
  if (is_success)
  {
    for (size_t i = 0; i < resultGetTrain.Size(); ++i)
    {
      train = resultGetTrain[i]["y"];
      all_trains.push_back(train);
    }
  }
  //get train type
  ScTemplate getTrainType;
  getTrainType.Triple(
    ScType::Unknown >> "y",
    ScType::EdgeAccessVarPosPerm,
    train
  );

  ScTemplateSearchResult resultGetTrainType;
  is_success = m_memoryCtx.HelperSearchTemplate(getTrainType, resultGetTrainType);
  ScAddr trainType;
  if (is_success)
  {
    for (size_t i = 0; i < resultGetTrainType.Size(); ++i)
    {
      trainType = resultGetTrainType[i]["y"];
    }
  }
  SC_LOG_ERROR("train type");
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(trainType));

  // get normative schedule
  ScTemplate getNormative;
  getNormative.Quintuple(
    train,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "y",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative
  );
  ScTemplateSearchResult resultNormative;
  is_success = m_memoryCtx.HelperSearchTemplate(getNormative, resultNormative);
  ScAddr normative;
  if (is_success)
  {
    for (size_t i = 0; i < resultNormative.Size(); ++i)
    {
      normative = resultNormative[i]["y"];
    }
  }

  // get last at_station
  ScTemplate search;
  search.Quintuple(
    schedule,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "y",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::rrel_last
  );
  ScTemplateSearchResult result;
  is_success = m_memoryCtx.HelperSearchTemplate(search, result);
  ScAddr lastAtStation;
  if (is_success)
  {
    for (size_t i = 0; i < result.Size(); ++i)
    {
      lastAtStation = result[i]["y"];
    }
  }

  // get time when train arrived

  ScTemplate getTime;
  getTime.Quintuple(
    lastAtStation,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "y",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  ScTemplateSearchResult resultGetTime;
  is_success = m_memoryCtx.HelperSearchTemplate(getTime, resultGetTime);
  ScAddr time;
  if (is_success)
  {
    for (size_t i = 0; i < resultGetTime.Size(); ++i)
    {
      time = resultGetTime[i]["y"];
    }
  }
  SC_LOG_ERROR("train");
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train));

  // get normative time
  ScTemplate GetNTime;
  GetNTime.Quintuple(
    normative,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "at_station",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown
  );
  GetNTime.Quintuple(
    "at_station",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "y",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative_time
  );
  
  vector<pair<string, ScAddr>> times;
  times.push_back({m_memoryCtx.HelperGetSystemIdtf(time), time});
  ScTemplateSearchResult resultGetNTime;
  is_success = m_memoryCtx.HelperSearchTemplate(GetNTime, resultGetNTime);
  if (is_success)
  {
    for (size_t i = 0; i < resultGetNTime.Size(); ++i)
    { 
      ScAddr node_time = resultGetNTime[i]["y"];
      if(m_memoryCtx.HelperGetSystemIdtf(time) != m_memoryCtx.HelperGetSystemIdtf(node_time)){
        times.push_back({m_memoryCtx.HelperGetSystemIdtf(node_time), node_time});
      }
    }
  }
  sort(times.begin(), times.end(), comparePairs);
  ScAddr next_time;
  for(int i=0;i<times.size();i++){
    if(times[i].first == m_memoryCtx.HelperGetSystemIdtf(time) && i != times.size()-1){
      next_time = times[i+1].second;
      break;
    }
    if(i == times.size()-1){
      SC_LOG_ERROR("last station");
      return;
    }
  }


  SC_LOG_ERROR("last time");
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(time));
  SC_LOG_ERROR("after analysis");
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(next_time));

  // differences between times
  string time1 = m_memoryCtx.HelperGetSystemIdtf(time);
  string time2 = m_memoryCtx.HelperGetSystemIdtf(next_time);

  int hours1 = stoi(time1.substr(0, 2));
  int minutes1 = stoi(time1.substr(3, 2));
  int hours2 = stoi(time2.substr(0, 2));
  int minutes2 = stoi(time2.substr(3, 2));
  ////////////////////////////////////////////
  // добавила время для отбытия, пока фиксированное 10 минут
  int totalMinutes1 = hours1 * 60 + minutes1 + 10;
  int totalMinutes2 = hours2 * 60 + minutes2;

  int difference = totalMinutes2 - totalMinutes1;

  SC_LOG_ERROR("Difference between times:");
  SC_LOG_ERROR(totalMinutes1);
  SC_LOG_ERROR(totalMinutes2);
  SC_LOG_ERROR(difference);

  //get last and next station
  ScTemplate getLStation;
  getLStation.Quintuple(
    lastAtStation,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "y",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_arrived
  );
  ScAddr lastStation;
  ScTemplateSearchResult resultGetLStation;
  is_success = m_memoryCtx.HelperSearchTemplate(getLStation, resultGetLStation);
  if (is_success)
  {
    for (size_t i = 0; i < resultGetLStation.Size(); ++i)
    { 
      lastStation = resultGetLStation[i]["y"];
    }
  }

  ScTemplate getNStation;
  getNStation.Triple(
    normative,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "at_station"
  );
  getNStation.Quintuple(
    "at_station",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "y",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );
  getNStation.Quintuple(
    "at_station",
    ScType::EdgeDCommonVar,
    next_time,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative_time
  );
  ScAddr nextStation;
  ScTemplateSearchResult resultGetNStation;
  is_success = m_memoryCtx.HelperSearchTemplate(getNStation, resultGetNStation);
  if (is_success)
  {
    for (size_t i = 0; i < resultGetNStation.Size(); ++i)
    { 
      nextStation = resultGetNStation[i]["y"];
    }
  }

  SC_LOG_ERROR("last station");
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(lastStation));
  SC_LOG_ERROR("next station");
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(nextStation));
  if (m_memoryCtx.HelperGetSystemIdtf(lastStation) == m_memoryCtx.HelperGetSystemIdtf(nextStation)){
    SC_LOG_ERROR("no errors");
    return;
  }

  //get track
  ScTemplate getTrack;
  getTrack.Quintuple(
    normative,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "y",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_track
  );
  ScAddr track;
  ScTemplateSearchResult resultGetTrack;
  is_success = m_memoryCtx.HelperSearchTemplate(getTrack, resultGetTrack);
  if (is_success)
  {
    for (size_t i = 0; i < resultGetTrack.Size(); ++i)
    { 
      track = resultGetTrack[i]["y"];
    }
  }

  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(track));

  //get interval
  ScTemplate getInterval;
   getInterval.Quintuple(
    normative,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "_track",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_track
  );
  getInterval.Quintuple(
    "_track",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "_name",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown
  );
  getInterval.Triple(
    "_name",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "y"
  );
  getInterval.Quintuple(
    "_name",
    ScType::EdgeAccessVarPosPerm,
    lastStation,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::rrel_1
  );
  getInterval.Quintuple(
    "_name",
    ScType::EdgeAccessVarPosPerm,
    nextStation,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::rrel_2
  );
  getInterval.Quintuple(
    "y",
    ScType::EdgeDCommonVar,
    trainType,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train_type
  );
  getInterval.Quintuple(
    "y",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "_time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  ScAddr interval;
  ScTemplateSearchResult resultInterval;
  is_success = m_memoryCtx.HelperSearchTemplate(getInterval, resultInterval);
  if (is_success)
  {
    for (size_t i = 0; i < resultInterval.Size(); ++i)
    { 
      interval = resultInterval[i]["_time"];
    }
  }
  
  SC_LOG_ERROR("interval between stations");
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(interval));


  int interval_value = stoi(m_memoryCtx.HelperGetSystemIdtf(interval));
  if(interval_value <= difference){
    SC_LOG_ERROR("no errors");
  }
  else {
    SC_LOG_ERROR("error, fix schedule");
    TrainCatchesUp(train, time, trainType, track, lastStation, nextStation, normative);
  }
  return;
}


bool GetTrainScheduleAgent::comparePairs(const pair<string, ScAddr>& a,const pair<string, ScAddr>& b) {
    return a.first < b.first; 
}

bool GetTrainScheduleAgent::CheckTime(string interval, string time_1, string& time_2){
  SC_LOG_ERROR(time_1);
  SC_LOG_ERROR(time_2);
  int hours1 = stoi(time_1.substr(0, 2));
  int minutes1 = stoi(time_1.substr(3, 2));
  int hours2 = stoi(time_2.substr(0, 2));
  int minutes2 = stoi(time_2.substr(3, 2));
  ////////////////////////////////////////////
  // добавила время для отбытия, пока фиксированное 10 минут
  int totalMinutes1 = hours1 * 60 + minutes1 + 10;
  int totalMinutes2 = hours2 * 60 + minutes2;

  int time = stoi(interval);
  int new_time = time + totalMinutes1;
  int hours = new_time / 60;
  int minutes = new_time % 60;
  ostringstream oss;
  oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
  time_2 = oss.str();
  SC_LOG_ERROR("time in check function");
  SC_LOG_ERROR(time_1);
  SC_LOG_ERROR(time_2);
  SC_LOG_ERROR(interval);
  if(new_time <= totalMinutes2)
    return true;
  else return false;
}

void GetTrainScheduleAgent::TrainCatchesUp(ScAddr &train, ScAddr &time, ScAddr &trainType, ScAddr &track, ScAddr lastStation, ScAddr &nextStation, ScAddr &normative){
  SC_LOG_ERROR("      ");
  SC_LOG_ERROR("when train catches up");
  //get all times from normative

  vector<string> times;
  ScTemplate getSchedule;
  getSchedule.Quintuple(
    normative,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "_time",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown
  );
  getSchedule.Quintuple(
    "_time",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "y",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative_time
  );
  ScTemplateSearchResult resultSchedule;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getSchedule, resultSchedule);
  if (is_success)
  {
    for (size_t i = 0; i < resultSchedule.Size(); ++i)
    { 
      times.push_back(m_memoryCtx.HelperGetSystemIdtf(resultSchedule[i]["y"]));
      //priority = resultSchedule[i]["_prior"];
    }
  }
  for(int i=0; i<times.size(); i++){
    if(times[i] <= m_memoryCtx.HelperGetSystemIdtf(time)){
      times.erase(times.begin() + i);
      i--;
    }
  }
  times.push_back(m_memoryCtx.HelperGetSystemIdtf(time));
  sort(times.begin(), times.end());
  SC_LOG_ERROR("normative time:");
  for(int i=0; i<times.size(); i++){
    SC_LOG_ERROR(times[i]);
  }

  //try to get interval

  ScAddr stat_1 = lastStation;
  bool check = true;

  ///////////////////////////////
  ScTemplate getInterval;
  getInterval.Quintuple(
    normative,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "_track",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_track
  );
  getInterval.Quintuple(
    "_track",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "_name",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown
  );
  getInterval.Triple(
    "_name",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "y"
  );
  getInterval.Quintuple(
    "y",
    ScType::EdgeDCommonVar,
    trainType,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train_type
  );
  getInterval.Quintuple(
    "y",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "_time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );

  
  ScTemplateSearchResult resultInterval;
  is_success = m_memoryCtx.HelperSearchTemplate(getInterval, resultInterval);
  if (is_success)
  {
    for (size_t i = 0; i < resultInterval.Size(); ++i)
    { 
      
      ScAddr track_id = resultInterval[i]["_name"];
     
      check = true;
      while(check){
        ScTemplate getStationL;
        getStationL.Quintuple(
          track_id,
          ScType::EdgeAccessVarPosPerm,
          ScType::Unknown >> "y",
          ScType::EdgeAccessVarPosPerm,
          Keynodes::rrel_1
        );
        getStationL.Quintuple(
          track_id,
          ScType::EdgeAccessVarPosPerm,
          ScType::Unknown >> "_next",
          ScType::EdgeAccessVarPosPerm,
          Keynodes::rrel_2
        );

        ScTemplateSearchResult resultStationL;
        bool success = m_memoryCtx.HelperSearchTemplate(getStationL, resultStationL);
        if(success){
          for (size_t j = 0; j < resultStationL.Size(); ++j){
            if(m_memoryCtx.HelperGetSystemIdtf(resultStationL[j]["y"]) == m_memoryCtx.HelperGetSystemIdtf(stat_1)){
              SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInterval[i]["_track"]));
              SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInterval[i]["_name"]));
              SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInterval[i]["y"]));
              SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInterval[i]["_time"]));
              SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultStationL[j]["y"]));
              SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultStationL[j]["_next"]));
              string interval = m_memoryCtx.HelperGetSystemIdtf(resultInterval[i]["_time"]);
              stat_1 = resultStationL[j]["_next"];
              SC_LOG_ERROR(times.size());
              if(CheckTime(interval, times[0], times[1])){
                times.erase(times.begin() + 0);
                SC_LOG_ERROR("train will catches station:");
                SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(stat_1));
                SC_LOG_ERROR("arrival time");
                SC_LOG_ERROR(times[0]);
                //WhichTrainsAreComing(times[0], stat_1);
                CreateForecastSchedule(train, trainType, times[0], stat_1);
                return;
              } else{
                times.erase(times.begin() + 0);
                SC_LOG_ERROR("arrival time");
                SC_LOG_ERROR(times[0]);
                //WhichTrainsAreComing(times[0], stat_1);
                CreateForecastSchedule(train, trainType, times[0], stat_1);

              }
            }
            else {
              break;
            }
          }
        }
        check = false;
      }
    }
  }
  
  ///////////////////////////////
  
  SC_LOG_ERROR("end of the cicle");
  
  return;
}

void GetTrainScheduleAgent::WhichTrainsAreComing(){
  /*
  //get station stop time
  ScTemplate getInterval;
  getInterval.Quintuple(
    station,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "_time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_stop_time
  );

  ScTemplateSearchResult resultInterval;
  string stop_time;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getInterval, resultInterval);
  if (is_success)
  {
    for (size_t i = 0; i < resultInterval.Size(); ++i)
    { 
      stop_time = m_memoryCtx.HelperGetSystemIdtf(resultInterval[i]["_time"]);
    }
  }

  //get last time
  string time_2;
  int hours1 = stoi(time.substr(0, 2));
  int minutes1 = stoi(time.substr(3, 2));
  int interval = stoi(stop_time);

  int new_time = hours1 * 60 + minutes1 + interval;

  int hours = new_time / 60;
  int minutes = new_time % 60;
  ostringstream oss;
  oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
  time_2 = oss.str();

  //get trains

  ScTemplate getNormativeTime;
  getNormativeTime.Triple(
    Keynodes::normative_schedule,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "_normative"
  );
  getNormativeTime.Quintuple(
    "_normative",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "_name",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown
  );
  getNormativeTime.Quintuple(
    "_name",
    ScType::EdgeDCommonVar,
    station,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );
  getNormativeTime.Quintuple(
    "_name",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "y",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative_time
  );
  getNormativeTime.Quintuple(
    ScType::Unknown >> "_train",
    ScType::EdgeDCommonVar,
    "_normative",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative
  );

  vector<ScAddr> trains;
  ScTemplateSearchResult resultTime;
  is_success = m_memoryCtx.HelperSearchTemplate(getNormativeTime, resultTime);
  if (is_success)
  {
    for (size_t i = 0; i < resultTime.Size(); ++i){
      string time_ = m_memoryCtx.HelperGetSystemIdtf(resultTime[i]["y"]);
      //check between
      if(time_ > time && time_ < time_2){
        //SC_LOG_ERROR("1 check");
        SC_LOG_ERROR(time_);
        trains.push_back(resultTime[i]["_train"]);
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultTime[i]["_train"]));
      }
      //check if train is in station 
      int hours2 = stoi(time_.substr(0, 2));
      int minutes2 = stoi(time_.substr(3, 2));

      new_time = hours2 * 60 + minutes2 + interval;

      hours = new_time / 60;
      minutes = new_time % 60;
      ostringstream oss;
      oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
      string time2_ = oss.str();
      if(time > time_ && time < time2_){
        //SC_LOG_ERROR("2 check");
        SC_LOG_ERROR(time_);
        trains.push_back(resultTime[i]["_train"]);
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultTime[i]["_train"]));
      }
      
    }
  }

  return;*/
  
  SC_LOG_ERROR("starting analyze forecast schedule");
  ScTemplate getFSchedule;
  getFSchedule.Triple(
    Keynodes::forecast_schedule,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "for_stat"
  );
  getFSchedule.Quintuple(
    "for_stat",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "station",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );

  ScTemplateSearchResult resultFSchedule;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getFSchedule, resultFSchedule);
  if(is_success){
    SC_LOG_ERROR(resultFSchedule.Size());
    for (size_t i = 0; i < resultFSchedule.Size(); ++i){
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultFSchedule[i]["for_stat"]));
      vector<ScAddr> trains;
      vector<ScAddr> times;
      vector<ScAddr> priorities;
      vector<ScAddr> for_trains;
      vector<ScAddr> directions;
      ScTemplate getInfo;
      getInfo.Triple(
        resultFSchedule[i]["for_stat"],
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
        ScType::Unknown >> "time",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_time
      );
      getInfo.Quintuple(
        "for_train",
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "direction",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_direction
      );
      ScTemplateSearchResult resultInfo;
      bool success = m_memoryCtx.HelperSearchTemplate(getInfo, resultInfo);
      if(success){
        SC_LOG_ERROR(resultInfo.Size());
        for (size_t j = 0; j < resultInfo.Size(); ++j){
          trains.push_back(resultInfo[j]["train"]);
          times.push_back(resultInfo[j]["time"]);
          for_trains.push_back(resultInfo[j]["for_train"]);
          directions.push_back(resultInfo[j]["direction"]);

          ScTemplate getPriority;
          getPriority.Quintuple(
            resultInfo[j]["for_train"],
            ScType::EdgeDCommonVar,
            ScType::Unknown >> "priority",
            ScType::EdgeAccessVarPosPerm,
            Keynodes::nrel_priority
          );
          ScTemplateSearchResult resultPriority;
          bool is_okay = m_memoryCtx.HelperSearchTemplate(getPriority, resultPriority);
          if(is_okay){
            //SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultPriority[0]["priority"]));
            priorities.push_back(resultPriority[0]["priority"]);
          }
        }
      }
      if (times.size() > 1){
        CheckCollisionsOnStation(trains, times, priorities, for_trains, directions, resultFSchedule[i]["station"]);
      }
      
    }
  }
  
  return;
}

void GetTrainScheduleAgent::CheckCollisionsOnStation(std::vector <ScAddr> trains, std::vector <ScAddr> times, std::vector<ScAddr> priorities, std::vector<ScAddr> for_trains, vector<ScAddr> directions, ScAddr station){
  SC_LOG_ERROR("check collisions");
  for(int i = 0; i < trains.size() - 1; i++){
    SC_LOG_ERROR("check");
    string time = m_memoryCtx.HelperGetSystemIdtf(times[i]);
    int time1 = stoi(time.substr(0, 2)) * 60 + stoi(time.substr(3, 2));
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(trains[i]));
    SC_LOG_ERROR(time1);
    for (int j=i+1; j < trains.size(); j++){
      time = m_memoryCtx.HelperGetSystemIdtf(times[j]);
      int time2 = stoi(time.substr(0, 2)) * 60 + stoi(time.substr(3, 2));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(trains[j]));
      SC_LOG_ERROR(time2);
      if (time2 < time1 && time2 > time1 - 10){
        SC_LOG_ERROR("check_1");
        SC_LOG_ERROR(time1);
        SC_LOG_ERROR(time2);
        if(CheckLines(trains[i], trains[j], station, for_trains[i], for_trains[j])){
          times[i] = EditEdge(for_trains[i], time1, time2, trains[i], station);
        }
      }
      if (time2 > time1 && time2 < time1 + 10){
        SC_LOG_ERROR("check_2");
        SC_LOG_ERROR(time1);
        SC_LOG_ERROR(time2);
        if(CheckLines(trains[i], trains[j], station, for_trains[i], for_trains[j])){
          times[j] = EditEdge(for_trains[j], time2, time1, trains[i], station);
        }
      }
      if (time1 == time2){
        SC_LOG_ERROR("check_3");
        SC_LOG_ERROR(time1);
        SC_LOG_ERROR(time2);
        if(CheckLines(trains[i], trains[j], station, for_trains[i], for_trains[j])){
          CheckPriority(priorities[i], priorities[i], for_trains[i], for_trains[j], time1, time2, trains[i], trains[j], station);
        }
      }
    }
  }
  SC_LOG_ERROR("check if time is okay");
  for(int i = 0; i < times.size(); i++){
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(times[i]));
  }
  ////////фиксануть вектор times с новыми значениями
  CheckLineBeforeStation(for_trains, directions, times, trains, station);
  for(int i = 0; i < trains.size(); i++){
    CheckMaxInterval(trains[i], station, times[i]);
  }
}

void GetTrainScheduleAgent::CheckMaxInterval(ScAddr train, ScAddr station, ScAddr time){
  SC_LOG_ERROR("check max interval");
  ScTemplate getType;
  getType.Triple(
    ScType::Unknown >> "type",
    ScType::EdgeAccessVarPosPerm,
    train
  );
  ScAddr type;
  ScTemplateSearchResult resultType;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getType, resultType);
  if(is_success){
    type = resultType[0]["type"];
  }

  ScTemplate getTrack;
  getTrack.Quintuple(
    train, 
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "normative",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative
  );
  getTrack.Quintuple(
    "normative", 
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "track",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_track
  );

  ScTemplateSearchResult resultTrack;
  is_success = m_memoryCtx.HelperSearchTemplate(getTrack, resultTrack);
  ScAddr track;
  if(is_success){
    track = resultTrack[0]["track"];
    ScTemplate searchTrack;
    searchTrack.Quintuple(
      track, 
      ScType::EdgeAccessVarPosPerm,
      ScType::Unknown >> "track_x",
      ScType::EdgeAccessVarPosPerm,
      ScType::Unknown 
    );
    searchTrack.Quintuple(
      "track_x",
      ScType::EdgeAccessVarPosPerm,
      station,
      ScType::EdgeAccessVarPosPerm,
      Keynodes::rrel_2
    );
    searchTrack.Triple(
      "track_x",
      ScType::EdgeAccessVarPosPerm,
      ScType::Unknown >> "name"
    );
    searchTrack.Quintuple(
      "name",
      ScType::EdgeDCommonVar,
      type,
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_train_type
    );
    searchTrack.Quintuple(
      "name",
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "max_time",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_max_time
    );

    ScTemplateSearchResult resultSearch;
    bool success = m_memoryCtx.HelperSearchTemplate(searchTrack, resultSearch);
    if(success){
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(track));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(station));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultSearch[0]["max_time"]));
      int maxtime = stoi(m_memoryCtx.HelperGetSystemIdtf(resultSearch[0]["max_time"]));
      //get station
      ScAddr prevStation;
      ScTemplate getStation;
      getStation.Quintuple(
        resultSearch[0]["track_x"],
        ScType::EdgeAccessVarPosPerm,
        ScType::Unknown >> "station_1",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::rrel_1
      );

      ScTemplateSearchResult resultStation;
      success = m_memoryCtx.HelperSearchTemplate(getStation, resultStation);
      if(success){
        prevStation = resultStation[0]["station_1"];
      }
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(prevStation));

      ScTemplate getStopTime;
      getStopTime.Quintuple(
        prevStation, 
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "time",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_stop_time
      );
      int stopTime;
      ScTemplateSearchResult resultStopTime;
      success = m_memoryCtx.HelperSearchTemplate(getStopTime, resultStopTime);
      if(success){
        stopTime = stoi(m_memoryCtx.HelperGetSystemIdtf(resultStopTime[0]["time"]));
      }
      //get time
      string name = "for_" + m_memoryCtx.HelperGetSystemIdtf(train) + "_" + m_memoryCtx.HelperGetSystemIdtf(prevStation);
      ScAddr for_train_ = m_memoryCtx.HelperFindBySystemIdtf(name);
      ScTemplate getTime;
      if(for_train_.IsValid()){
        getTime.Quintuple(
          for_train_, 
          ScType::EdgeDCommonVar,
          ScType::Unknown >> "time",
          ScType::EdgeAccessVarPosPerm,
          Keynodes::nrel_time
        );
        ScAddr prevtime;
        ScTemplateSearchResult resultTime;
        success = m_memoryCtx.HelperSearchTemplate(getTime, resultTime);
        if(success){
          prevtime = resultTime[0]["time"];
          SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(prevtime));
        

          SC_LOG_ERROR("stop time");
          SC_LOG_ERROR(stopTime);
          
          int time1 = stoi(m_memoryCtx.HelperGetSystemIdtf(prevtime).substr(0, 2)) * 60 + stoi(m_memoryCtx.HelperGetSystemIdtf(prevtime).substr(3, 2)) + stopTime;
          int time2 = stoi(m_memoryCtx.HelperGetSystemIdtf(time).substr(0, 2)) * 60 + stoi(m_memoryCtx.HelperGetSystemIdtf(time).substr(3, 2));
          SC_LOG_ERROR(time1);
          SC_LOG_ERROR(time2);

          if((time2 - time1) > maxtime){
            int difference = (time2 - time1) - maxtime;
            time1 = time1 - difference - stopTime;
            int hours = time1 / 60;
            int minutes = time1 % 60;
            ostringstream oss;
            oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
            string new_time = oss.str();
            ScTemplate getEdge;
            getEdge.Quintuple(
              for_train_, 
              ScType::EdgeDCommonVar >> "edge",
              ScType::Unknown >> "time",
              ScType::EdgeAccessVarPosPerm,
              Keynodes::nrel_time
            );
            ScTemplateSearchResult resultEdge;
            success = m_memoryCtx.HelperSearchTemplate(getEdge, resultEdge);
            if(success){
              m_memoryCtx.EraseElement(resultEdge[0]["edge"]);
              ScAddr time_node = m_memoryCtx.HelperFindBySystemIdtf(new_time);
              if(!time_node.IsValid()){
                time_node = m_memoryCtx.CreateNode(ScType::NodeConst);
                m_memoryCtx.HelperSetSystemIdtf(new_time, time_node);
              }
              ScTemplate genTime;
              genTime.Quintuple(
                for_train_, 
                ScType::EdgeDCommonVar,
                time_node,
                ScType::EdgeAccessVarPosPerm,
                Keynodes::nrel_time
              );

              ScTemplateGenResult resGenTime;
              m_memoryCtx.HelperGenTemplate(genTime, resGenTime);
            }
          }
        }
      } else{
        getTime.Quintuple(
          train, 
          ScType::EdgeDCommonVar,
          ScType::Unknown >> "actual",
          ScType::EdgeAccessVarPosPerm,
          Keynodes::nrel_actual
        );
        getTime.Quintuple(
          "actual", 
          ScType::EdgeAccessVarPosPerm,
          ScType::Unknown >> "station",
          ScType::EdgeAccessVarPosPerm,
          Keynodes::rrel_last
        );
        getTime.Quintuple(
          "station", 
          ScType::EdgeDCommonVar,
          ScType::Unknown >> "time",
          ScType::EdgeAccessVarPosPerm,
          Keynodes::nrel_time
        );
        ScAddr prevtime;
        ScTemplateSearchResult resultTime;
        success = m_memoryCtx.HelperSearchTemplate(getTime, resultTime);
        if(success){
          prevtime = resultTime[0]["time"];
          SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(prevtime));
        

          SC_LOG_ERROR("stop time");
          SC_LOG_ERROR(stopTime);
          
          int time1 = stoi(m_memoryCtx.HelperGetSystemIdtf(prevtime).substr(0, 2)) * 60 + stoi(m_memoryCtx.HelperGetSystemIdtf(prevtime).substr(3, 2)) + stopTime;
          int time2 = stoi(m_memoryCtx.HelperGetSystemIdtf(time).substr(0, 2)) * 60 + stoi(m_memoryCtx.HelperGetSystemIdtf(time).substr(3, 2));
          SC_LOG_ERROR(time1);
          SC_LOG_ERROR(time2);

          if((time2 - time1) > maxtime){
            int difference = (time2 - time1) - maxtime;
            time2 = time2 + difference;
            int hours = time2 / 60;
            int minutes = time2 % 60;
            ostringstream oss;
            oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
            string new_time = oss.str();
            string name = "for_" + m_memoryCtx.HelperGetSystemIdtf(train) + "_" + m_memoryCtx.HelperGetSystemIdtf(station);
            ScAddr for_train = m_memoryCtx.HelperFindBySystemIdtf(name);
            ScTemplate getEdge;
            getEdge.Quintuple(
              for_train, 
              ScType::EdgeDCommonVar >> "edge",
              ScType::Unknown >> "time",
              ScType::EdgeAccessVarPosPerm,
              Keynodes::nrel_time
            );
            ScTemplateSearchResult resultEdge;
            success = m_memoryCtx.HelperSearchTemplate(getEdge, resultEdge);
            if(success){
              m_memoryCtx.EraseElement(resultEdge[0]["edge"]);
              ScAddr time_node = m_memoryCtx.HelperFindBySystemIdtf(new_time);
              if(!time_node.IsValid()){
                time_node = m_memoryCtx.CreateNode(ScType::NodeConst);
                m_memoryCtx.HelperSetSystemIdtf(new_time, time_node);
              }
              ScTemplate genTime;
              genTime.Quintuple(
                for_train, 
                ScType::EdgeDCommonVar,
                time_node,
                ScType::EdgeAccessVarPosPerm,
                Keynodes::nrel_time
              );

              ScTemplateGenResult resGenTime;
              m_memoryCtx.HelperGenTemplate(genTime, resGenTime);
            }
          }
        }
      }
    }
  }

  return;
}

//// добавить если время равно 
void GetTrainScheduleAgent::CheckLineBeforeStation(std::vector<ScAddr> for_trains, std::vector<ScAddr> directions, std::vector<ScAddr> &times, std::vector<ScAddr> trains, ScAddr station){
  SC_LOG_ERROR("////////");
  SC_LOG_ERROR("check line before station");

  ScTemplate getBrakingTime;
  getBrakingTime.Quintuple(
    station,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_braking_time
  );
  getBrakingTime.Quintuple(
    station,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "stop_time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_stop_time
  );
  ScTemplateSearchResult resultBrakingTime;
  int braking_time;
  int stop_time;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getBrakingTime, resultBrakingTime);
  if(is_success){
    braking_time = stoi(m_memoryCtx.HelperGetSystemIdtf(resultBrakingTime[0]["time"]));
    stop_time = stoi(m_memoryCtx.HelperGetSystemIdtf(resultBrakingTime[0]["stop_time"]));
  }
  SC_LOG_ERROR(braking_time);
  for(int i = 0; i < for_trains.size(); i++){
    for(int j = i+1; j < for_trains.size(); j++){
      string time = m_memoryCtx.HelperGetSystemIdtf(times[i]);
      int time1 = stoi(time.substr(0, 2)) * 60 + stoi(time.substr(3, 2));
      time = m_memoryCtx.HelperGetSystemIdtf(times[j]);
      int time2 = stoi(time.substr(0, 2)) * 60 + stoi(time.substr(3, 2));
      int time_1 = time1;
      int time_2 = time2;
      SC_LOG_ERROR(time1);
      SC_LOG_ERROR(time2);
      SC_LOG_ERROR(braking_time);
      if(m_memoryCtx.HelperGetSystemIdtf(directions[i]) == "forward_direction" && m_memoryCtx.HelperGetSystemIdtf(directions[j]) == "forward_direction"){
       
        if(abs(time1 - time2) < braking_time){
          if(time1 < time2){
            time1 = time2 - braking_time;
          }
          if(time2 < time1){
            time2 = time1 - braking_time;
          }
        }
        SC_LOG_ERROR("new times");
        SC_LOG_ERROR(time1);
        SC_LOG_ERROR(time2);
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(for_trains[i]));
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(for_trains[j]));
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(trains[i]));
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(trains[j]));
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(times[i]));
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(times[j]));
      }
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(directions[i]));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(directions[j]));
      if(m_memoryCtx.HelperGetSystemIdtf(directions[i]) != m_memoryCtx.HelperGetSystemIdtf(directions[j])){
        if(m_memoryCtx.HelperGetSystemIdtf(directions[i]) == "forward_direction"){
          int time_from_stat = time2 + stop_time;
          if(abs(time_from_stat - time1) < braking_time){
            if(time1 < time_from_stat){
              time1 = time_from_stat - braking_time;
            }
            if(time_from_stat < time1){
              time2 = time1 - braking_time - stop_time;
            }
          }
        }
        if(m_memoryCtx.HelperGetSystemIdtf(directions[j]) == "forward_direction"){
          int time_from_stat = time1 + stop_time;
          if(abs(time_from_stat - time1) < braking_time){
            if(time2 < time_from_stat){
              time2 = time_from_stat - braking_time;
            }
            if(time_from_stat < time1){
              time1 = time2 - braking_time - stop_time;
            }
          }
        }
        SC_LOG_ERROR("with stop time");
        SC_LOG_ERROR("new times");
        SC_LOG_ERROR(time1);
        SC_LOG_ERROR(time2);
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(times[i]));
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(times[j]));
      }
      if(time1 != time_1){
        int hours = time1 / 60;
        int minutes = time1 % 60;
        ostringstream oss;
        oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
        string new_time = oss.str();
        ScAddr time_node = m_memoryCtx.HelperFindBySystemIdtf(new_time);
        if (!time_node.IsValid()){
          time_node = m_memoryCtx.CreateNode(ScType::NodeConst);
          m_memoryCtx.HelperSetSystemIdtf(new_time, time_node);
        }
        times[i] = time_node;
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(times[i]));
        CreateNewTimeNode(times[i], for_trains[i]);
      }
      if(time2 != time_2){
        int hours = time2 / 60;
        int minutes = time2 % 60;
        ostringstream oss;
        oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
        string new_time = oss.str();
        ScAddr time_node = m_memoryCtx.HelperFindBySystemIdtf(new_time);
        if (!time_node.IsValid()){
          time_node = m_memoryCtx.CreateNode(ScType::NodeConst);
          m_memoryCtx.HelperSetSystemIdtf(new_time, time_node);
        }
        times[j] = time_node;
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(times[j]));
        CreateNewTimeNode(times[j], for_trains[j]);
      }
    }
  }
}

void GetTrainScheduleAgent::CreateNewTimeNode(ScAddr time, ScAddr for_train){
  ScTemplate getTime;
  getTime.Quintuple(
    for_train,
    ScType::EdgeDCommonVar >> "edge",
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );
  ScAddr time_1;
  ScTemplateSearchResult resultTime;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getTime, resultTime);
  if(is_success){
    for(size_t i = 0; i < resultTime.Size(); ++i){
      m_memoryCtx.EraseElement(resultTime[i]["edge"]);
      ScTemplate genTime;
      genTime.Quintuple(
        for_train,
        ScType::EdgeDCommonVar,
        time,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_time
      );
      ScTemplateGenResult resGenTime;
      m_memoryCtx.HelperGenTemplate(genTime, resGenTime);
    }
  }
}

void GetTrainScheduleAgent::CheckPriority(ScAddr priority1, ScAddr priority2, ScAddr for_train1, ScAddr for_train2, int time1, int time2, ScAddr train1, ScAddr train2, ScAddr station){

  if(m_memoryCtx.HelperGetSystemIdtf(priority1) > m_memoryCtx.HelperGetSystemIdtf(priority2)){
    EditEdge(for_train1, time1, time2, train1, station);
  }
  if(m_memoryCtx.HelperGetSystemIdtf(priority1) < m_memoryCtx.HelperGetSystemIdtf(priority2)){
    EditEdge(for_train2, time2, time1, train2, station);
  }
  if(m_memoryCtx.HelperGetSystemIdtf(priority1) == m_memoryCtx.HelperGetSystemIdtf(priority2)){
    //fix with normative schedule
    if(CheckNormativeSchedule(train1, train2, station)) {
      EditEdge(for_train1, time1, time2, train1, station);
    }
    else EditEdge(for_train2, time2, time1, train2, station);
  }
}

bool GetTrainScheduleAgent::CheckNormativeSchedule(ScAddr train1, ScAddr train2, ScAddr station){
  //for train 1
  string time1;
  string time2;
  ScTemplate getTime1;
  getTime1.Quintuple(
    train1,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "normative",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative
  );

  getTime1.Quintuple(
    "normative",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "train_at",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown 
  );

  getTime1.Quintuple(
    "train_at",
    ScType::EdgeDCommonVar,
    station,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );

  getTime1.Quintuple(
    "train_at",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative_time
  );

  ScTemplateSearchResult resultTime1;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getTime1, resultTime1);
  if(is_success){
    time1 = m_memoryCtx.HelperGetSystemIdtf(resultTime1[0]["time"]);
  }

  ScTemplate getTime2;
  getTime2.Quintuple(
    train2,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "normative",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative
  );

  getTime2.Quintuple(
    "normative",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "train_at",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown 
  );

  getTime2.Quintuple(
    "train_at",
    ScType::EdgeDCommonVar,
    station,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );

  getTime2.Quintuple(
    "train_at",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative_time
  );

  ScTemplateSearchResult resultTime2;
  is_success = m_memoryCtx.HelperSearchTemplate(getTime2, resultTime2);
  if(is_success){
    time2 = m_memoryCtx.HelperGetSystemIdtf(resultTime2[0]["time"]);
  }
  SC_LOG_ERROR("//////////////////////////////");
  SC_LOG_ERROR("//////////////////////////////");
  SC_LOG_ERROR("//////////////////////////////");
  SC_LOG_ERROR("//////////////////////////////");
  SC_LOG_ERROR("//////////////////////////////");
  SC_LOG_ERROR("//////////////////////////////");
  SC_LOG_ERROR("//////////////////////////////");
  SC_LOG_ERROR("//////////////////////////////");
  SC_LOG_ERROR("//////////////////////////////");
  SC_LOG_ERROR("//////////////////////////////");
  SC_LOG_ERROR("//////////////////////////////");
  SC_LOG_ERROR(time1);
  SC_LOG_ERROR(time2);

  float time_1 = stof(time1);
  float time_2 = stof(time2);

  if(time_1 > time_2){
    return true;
  }
  else return false;
}

bool GetTrainScheduleAgent::CheckLines(ScAddr train1, ScAddr train2, ScAddr station, ScAddr for_train_1, ScAddr for_train_2){
  bool check = true;
  vector<ScAddr>lines1;
  vector<ScAddr> lines2;
  ScAddr type1;
  ScAddr type2;
  ScTemplate getFLine;
  getFLine.Quintuple(
    ScType::Unknown >> "for_train",
    ScType::EdgeDCommonVar,
    train2,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train
  );
  getFLine.Quintuple(
    ScType::Unknown >> "for_train",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "line",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_line
  );
  ScAddr fLine;
  ScTemplateSearchResult resultFLine;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getFLine, resultFLine);
  if(is_success){
    check == false;
    for(size_t i = 0; i < resultFLine.Size(); ++i){
      fLine = resultFLine[i]["line"];
    }
  }
  else {
    ScTemplate getType1;
    getType1.Triple(
      ScType::Unknown >> "type1",
      ScType::EdgeAccessVarPosPerm,
      train1
    );
    ScTemplate getType2;
    getType2.Triple(
      ScType::Unknown >> "type2",
      ScType::EdgeAccessVarPosPerm,
      train2
    );
    ScTemplateSearchResult resultType1;
    m_memoryCtx.HelperSearchTemplate(getType1, resultType1);
    ScTemplateSearchResult resultType2;
    m_memoryCtx.HelperSearchTemplate(getType2, resultType2);
    SC_LOG_ERROR("check lines");
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train1));
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train2));
    type1 = resultType1[0]["type1"];
    type2 = resultType2[0]["type2"];
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(type1));

    ScTemplate getSLine1;
    getSLine1.Triple(
      station,
      ScType::EdgeAccessVarPosPerm,
      ScType::Unknown >> "stat"
    );
    getSLine1.Quintuple(
      "stat",
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "line",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_line
    );  
    getSLine1.Quintuple(
      "stat",
      ScType::EdgeDCommonVar,
      type1,
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_train_type
    ); 
    ScTemplate getSLine2;
    getSLine2.Triple(
      station,
      ScType::EdgeAccessVarPosPerm,
      ScType::Unknown >> "stat"
    );
    getSLine2.Quintuple(
      "stat",
      ScType::EdgeDCommonVar,
      ScType::Unknown >> "line",
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_line
    );  
    getSLine2.Quintuple(
      "stat",
      ScType::EdgeDCommonVar,
      type2,
      ScType::EdgeAccessVarPosPerm,
      Keynodes::nrel_train_type
    ); 
    
    ScTemplateSearchResult resultSLine1;
    m_memoryCtx.HelperSearchTemplate(getSLine1, resultSLine1);
    for(size_t i = 0; i < resultSLine1.Size(); ++i){
      lines1.push_back(resultSLine1[i]["line"]);
    }
    ScTemplateSearchResult resultSLine2;
    m_memoryCtx.HelperSearchTemplate(getSLine2, resultSLine2);
    for(size_t i = 0; i < resultSLine2.Size(); ++i){
      lines2.push_back(resultSLine2[i]["line"]);
    }
    for(int i = 0; i < lines1.size(); i++){
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(lines1[i]));
    }
    for(int i = 0; i < lines2.size(); i++){
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(lines2[i]));
    }
  }
  if(check){
    if(lines2.size()){
      bool is_check = false;
      int index;
      for(int i = 0; i < lines2.size(); i++){
        if(IfLineIsBusy(station, lines2[i])){
            ScTemplate genLine2;
            genLine2.Quintuple(
            for_train_2,
            ScType::EdgeDCommonVar,
            lines2[i],
            ScType::EdgeAccessVarPosPerm,
            Keynodes::nrel_line
          );

          ScTemplateGenResult resGenLine2;
          m_memoryCtx.HelperGenTemplate(genLine2, resGenLine2);
          is_check = true;
          index = i;
          break;
        }
      }
      if(is_check){
      for(int i = 0; i < lines1.size(); i++){
        SC_LOG_ERROR(index);
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(type1));
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(type2));
        if(m_memoryCtx.HelperGetSystemIdtf(type1) == m_memoryCtx.HelperGetSystemIdtf(type2)){
          if(i == index){
            SC_LOG_ERROR("check for index");
            if(i != lines1.size()-1){
              i++;
            }
            else break;
          }
        }
        if(IfLineIsBusy(station, lines1[i])){
          ScTemplate genLine1;
          genLine1.Quintuple(
            for_train_1,
            ScType::EdgeDCommonVar,
            lines1[i],
            ScType::EdgeAccessVarPosPerm,
            Keynodes::nrel_line
          );
          ScTemplateGenResult resGenLine1;
          m_memoryCtx.HelperGenTemplate(genLine1, resGenLine1);
          return false;
      }
    }
    }
    }
    else {
      for(int i = 0; i < lines1.size(); i++){
        if(m_memoryCtx.HelperGetSystemIdtf(lines1[i]) != m_memoryCtx.HelperGetSystemIdtf(fLine) && IfLineIsBusy(station, lines1[i])){
          ScTemplate genLine1;
          genLine1.Quintuple(
            for_train_1,
            ScType::EdgeDCommonVar,
            lines1[i],
            ScType::EdgeAccessVarPosPerm,
            Keynodes::nrel_line
          );
          ScTemplateGenResult resGenLine1;
          m_memoryCtx.HelperGenTemplate(genLine1, resGenLine1);
          return false;
        }
      }
    }
  }
  return true;
}
/// я хз нужна ли эта проверка, она не катируется временем
bool GetTrainScheduleAgent::IfLineIsBusy(ScAddr station, ScAddr line){
  ScTemplate getStat;
  getStat.Triple(
    Keynodes::forecast_schedule,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "for_stat"
  );
  getStat.Quintuple(
    "for_stat",
    ScType::EdgeDCommonVar,
    station,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );
  ScTemplateSearchResult resultStat;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getStat, resultStat);
  if(is_success){
    for(size_t i = 0; i < resultStat.Size(); ++i){
      ScTemplate getLine;

      getLine.Triple(
        resultStat[i]["for_stat"],
        ScType::EdgeAccessVarPosPerm,
        ScType::Unknown >> "for_train"
      );
      getLine.Quintuple(
        "for_train",
        ScType::EdgeDCommonVar,
        line,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_line
      );

      ScTemplateSearchResult resultLine;
      bool success = m_memoryCtx.HelperSearchTemplate(getLine, resultLine);
      if(success){
        SC_LOG_ERROR("line is busy");
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(line));
        return false;
      }
    } 
  }
  SC_LOG_ERROR("line free");
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(line));
  return true;
}

ScAddr GetTrainScheduleAgent::EditEdge(ScAddr for_train, int time, int time2, ScAddr train, ScAddr station){
  ScTemplate getTime;
  getTime.Quintuple(
    for_train,
    ScType::EdgeDCommonVar >> "edge",
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );
  ScAddr time_1;
  ScTemplateSearchResult resultTime;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getTime, resultTime);
  if(is_success){
    for(size_t i = 0; i < resultTime.Size(); ++i){
      m_memoryCtx.EraseElement(resultTime[i]["edge"]);
      int difference = time2 + 10 + 1; ///////////////////////////одна минута запаса
      int hours = difference / 60;
      int minutes = difference % 60;
      ostringstream oss;
      oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
      string new_time = oss.str();
      time_1 = m_memoryCtx.HelperFindBySystemIdtf(new_time);
      if (!time_1.IsValid()){
        time_1 = m_memoryCtx.CreateNode(ScType::NodeConst);
        m_memoryCtx.HelperSetSystemIdtf(new_time, time_1);
      }

      ScTemplate genTime;
      genTime.Quintuple(
        for_train,
        ScType::EdgeDCommonVar,
        time_1,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_time
      );
      ScTemplateGenResult resGenTime;
      m_memoryCtx.HelperGenTemplate(genTime, resGenTime);

    }
  }
  return time_1;
} 




void GetTrainScheduleAgent::CreateForecastSchedule(ScAddr &train, ScAddr &trainType, string &time, ScAddr &station ){
  SC_LOG_ERROR("//////////////////////////////////////////////////////////////////////////////////");
  //get schedule with late arriving
  ScTemplate getSchedule;
  getSchedule.Triple(
    Keynodes::forecast_schedule,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "for_"
  );
  getSchedule.Quintuple(
    "for_",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "stat_",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );

  ScTemplateSearchResult resultSchedule;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getSchedule, resultSchedule);
  if (is_success)
  {
    for (size_t i = 0; i < resultSchedule.Size(); ++i){
      //SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultSchedule[i]["for_"]));
      //SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultSchedule[i]["stat_"]));
      if(m_memoryCtx.HelperGetSystemIdtf(station) == m_memoryCtx.HelperGetSystemIdtf(resultSchedule[i]["stat_"])){
        ScTemplate result_struct;
        string name_node = "for_" + m_memoryCtx.HelperGetSystemIdtf(train) + "_" + m_memoryCtx.HelperGetSystemIdtf(station);
        ScAddr node = m_memoryCtx.HelperFindBySystemIdtf(name_node);
        if (!node.IsValid()){
          node = m_memoryCtx.CreateNode(ScType::NodeConst);
          m_memoryCtx.HelperSetSystemIdtf(name_node, node);
        }
        ScAddr node_time = m_memoryCtx.HelperFindBySystemIdtf(time);
        if (!node_time.IsValid()){
          node_time = m_memoryCtx.CreateNode(ScType::NodeConst);
          m_memoryCtx.HelperSetSystemIdtf(time, node_time);
        }
        SC_LOG_ERROR(name_node);
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(node));
        SC_LOG_ERROR(time);
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(node_time));
        result_struct.Triple(
          resultSchedule[i]["for_"],
          ScType::EdgeAccessVarPosPerm,
          node
        );
        result_struct.Quintuple(
          node, 
          ScType::EdgeDCommonVar,
          train,
          ScType::EdgeAccessVarPosPerm,
          Keynodes::nrel_train
        );
        result_struct.Quintuple(
          node, 
          ScType::EdgeDCommonVar,
          node_time,
          ScType::EdgeAccessVarPosPerm,
          Keynodes::nrel_time
        );
        ScTemplateGenResult genClassConstruction;
        m_memoryCtx.HelperGenTemplate(result_struct, genClassConstruction);
      }
    }
  }
  SC_LOG_ERROR("//////////////////////////////////////////////////////////////////////////////////");
  return;
}

void GetTrainScheduleAgent::FinishForecastSchedule(){
  ScTemplate getSchedule;
  getSchedule.Triple(
    Keynodes::normative_schedule,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "normative"
  );
  getSchedule.Quintuple(
    "normative",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "train_at",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown 
  );
  getSchedule.Quintuple(
    "train_at",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "time",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_normative_time
  );
  getSchedule.Quintuple(
    "train_at",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "station",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );

  ScTemplateSearchResult resultSchedule;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getSchedule, resultSchedule);
  if(is_success){
    for(size_t i = 0; i < resultSchedule.Size(); ++i){
      ScAddr train;
      ScTemplate getTrain;
      getTrain.Quintuple(
        ScType::Unknown >> "train",
        ScType::EdgeDCommonVar,
        resultSchedule[i]["normative"],
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_normative
      );
      ScTemplateSearchResult resTrain;
      bool success = m_memoryCtx.HelperSearchTemplate(getTrain, resTrain);
      if(success){
        train = resTrain[0]["train"];
      }
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train));
      SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultSchedule[i]["station"]));
      if(!CheckActualSchedule(train, resultSchedule[i]["station"])){
        SC_LOG_ERROR("actual schedule");
        continue;
      }
      string name = "for_" + m_memoryCtx.HelperGetSystemIdtf(train) + "_" + m_memoryCtx.HelperGetSystemIdtf(resultSchedule[i]["station"]);
      ScAddr forecast_train = m_memoryCtx.HelperFindBySystemIdtf(name);
      if(!forecast_train.IsValid()){
        ScAddr for_train_ = m_memoryCtx.CreateNode(ScType::NodeConst);
        m_memoryCtx.HelperSetSystemIdtf(name, for_train_);
        string for_stat_name = "for_" + m_memoryCtx.HelperGetSystemIdtf(resultSchedule[i]["station"]);
        ScAddr for_station = m_memoryCtx.HelperFindBySystemIdtf(for_stat_name);
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(for_train_));
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(for_station));
        ScTemplate createForecast;
        createForecast.Triple(
          for_station,
          ScType::EdgeAccessVarPosPerm,
          for_train_
        );
        createForecast.Quintuple(
          for_train_,
          ScType::EdgeDCommonVar,
          train,
          ScType::EdgeAccessVarPosPerm,
          Keynodes::nrel_train
        );
        createForecast.Quintuple(
          for_train_,
          ScType::EdgeDCommonVar,
          resultSchedule[i]["time"],
          ScType::EdgeAccessVarPosPerm,
          Keynodes::nrel_time
        );
        ScTemplateGenResult genCreating;
        m_memoryCtx.HelperGenTemplate(createForecast, genCreating);
      }
    }
  }
  addDirecationAndPriorityOnForecast();
}

void GetTrainScheduleAgent::addDirecationAndPriorityOnForecast(){
  SC_LOG_ERROR("add direction on forecast");
  SC_LOG_ERROR("add priority");
  ScTemplate getForecast;
  getForecast.Triple(
    Keynodes::forecast_schedule,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "for_stat"
  );
  getForecast.Quintuple(
    "for_stat",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "station",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_station
  );
  getForecast.Triple(
    "for_stat",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "for_train"
  );
  getForecast.Quintuple(
    "for_train",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "train",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_train
  );

  ScTemplateSearchResult resultForecast;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getForecast, resultForecast);
  if(is_success){
    for(size_t i = 0; i < resultForecast.Size(); ++i){
      //SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultForecast[i]["train"]));
      //SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultForecast[i]["station"]));
      ScTemplate getDirection;
      getDirection.Quintuple(
        resultForecast[i]["train"],
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "normative",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_normative
      );
      getDirection.Quintuple(
        "normative",
        ScType::EdgeAccessVarPosPerm,
        ScType::Unknown >> "for_train",
        ScType::EdgeAccessVarPosPerm,
        ScType::Unknown
      );
      getDirection.Quintuple(
        "for_train",
        ScType::EdgeDCommonVar,
        resultForecast[i]["station"],
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_station
      );
      getDirection.Quintuple(
        "for_train",
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "direction",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_direction
      );
      getDirection.Quintuple(
        "for_train",
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "priority",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_priority
      );
      ScTemplateSearchResult resultDirection;
      bool success = m_memoryCtx.HelperSearchTemplate(getDirection, resultDirection);
      if(success){
        for(size_t j = 0; j < resultDirection.Size(); ++j){
          //SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultDirection[j]["direction"]));
          ScTemplate addForecast;
          addForecast.Quintuple(
            resultForecast[i]["for_train"],
            ScType::EdgeDCommonVar,
            resultDirection[j]["direction"],
            ScType::EdgeAccessVarPosPerm,
            Keynodes::nrel_direction
          );
          addForecast.Quintuple(
            resultForecast[i]["for_train"],
            ScType::EdgeDCommonVar,
            resultDirection[j]["priority"],
            ScType::EdgeAccessVarPosPerm,
            Keynodes::nrel_priority
          );

          ScTemplateGenResult genCreating;
          m_memoryCtx.HelperGenTemplate(addForecast, genCreating);
        }
      }

      //get priority


    }
  }



}

bool GetTrainScheduleAgent::CheckActualSchedule(ScAddr train, ScAddr station){
  ScTemplate getPrevStation;
  getPrevStation.Quintuple(
    train,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "schedule",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_actual
  );
  getPrevStation.Quintuple(
    "schedule",
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "train_at",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::rrel_previous
  );  
  getPrevStation.Quintuple(
    "train_at",
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "station",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_arrived
  );  
  ScTemplateSearchResult resultPrevStation;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getPrevStation, resultPrevStation);
  if(is_success){
    for(size_t i = 0; i < resultPrevStation.Size(); ++i){
      if(m_memoryCtx.HelperGetSystemIdtf(resultPrevStation[i]["station"]) == m_memoryCtx.HelperGetSystemIdtf(station)){
        return false;
      }
    }
  }
  ScTemplate getLastStation;
  getLastStation.Quintuple(
    train,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "schedule",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_actual
  );
  getLastStation.Quintuple(
    "schedule",
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
  ScTemplateSearchResult resultLastStation;
  is_success = m_memoryCtx.HelperSearchTemplate(getLastStation, resultLastStation);
  if(is_success){
    for(size_t i = 0; i < resultLastStation.Size(); ++i){
      if(m_memoryCtx.HelperGetSystemIdtf(resultLastStation[i]["station"]) == m_memoryCtx.HelperGetSystemIdtf(station)){
        return false;
      }
    }
  }

  return true;
}
}
