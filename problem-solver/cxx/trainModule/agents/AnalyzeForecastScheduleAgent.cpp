#include <sc-agents-common/utils/GenerationUtils.hpp>
#include <sc-agents-common/utils/AgentUtils.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>
#include <sc-agents-common/keynodes/coreKeynodes.hpp>

#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "AnalyzeForecastScheduleAgent.hpp"

using namespace std;
using namespace utils;

namespace trainModule
{

SC_AGENT_IMPLEMENTATION(AnalyzeForecastScheduleAgent)
{
  ScAddr forecast = otherAddr;
  SC_LOG_ERROR("my agent for forecast started!");
  bool check = false;

  
  //cycle 
  while(!check){
    check = AnalyzeForecast(forecast);
  }
    
  ScTemplate getEdge;
  getEdge.Triple(
    Keynodes::question_analyze_forecast,
    ScType::EdgeAccessVarPosPerm >> "y",
    forecast
  );
  ScTemplateSearchResult resultEdge;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getEdge, resultEdge);
  if (is_success) {
    m_memoryCtx.EraseElement(resultEdge[0]["y"]);
  }
    
  return SC_RESULT_OK;
}

bool AnalyzeForecastScheduleAgent::AnalyzeForecast(ScAddr forecast){
    bool check = true;

    //get all info about each station
    
    ScTemplate getStation;

    getStation.Triple(
        forecast,
        ScType::EdgeAccessVarPosPerm,
        ScType::Unknown >> "for_station"
    );
    getStation.Quintuple(
        "for_station",
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "station",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_station
    );

    ScTemplateSearchResult resultStation;
    bool is_success = m_memoryCtx.HelperSearchTemplate(getStation, resultStation);
    if(is_success){
        for(int i = 0; i < resultStation.Size(); i++){
            SC_LOG_ERROR("/////////////////");
            
            vector<ScAddr> trains;
            vector<ScAddr> types;
            vector<ScAddr> times;
            vector<ScAddr> priorities;
            vector<ScAddr> directions;
            ScAddr station;

            station = resultStation[i]["station"];
            SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultStation[i]["for_station"]));
            SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(station));

            ScTemplate getInfo;

            getInfo.Triple(
                resultStation[i]["for_station"],
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
            bool success = m_memoryCtx.HelperSearchTemplate(getInfo, resultInfo);
            if(success){
                for(int j = 0; j < resultInfo.Size(); j++){
                    trains.push_back(resultInfo[j]["train"]);
                    types.push_back(resultInfo[j]["type"]);
                    times.push_back(resultInfo[j]["time"]);
                    priorities.push_back(resultInfo[j]["priority"]);
                    directions.push_back(resultInfo[j]["direction"]);
                }
            }
            /*
            for(int j = 0; j < trains.size(); j++){
                SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(trains[j]));
                SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(types[j]));
                SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(times[j]));
                SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(priorities[j]));
                SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(directions[j]));
            }
            */
            if (trains.size() > 1) {
                check = CheckStation(station, trains, types, times, priorities, directions);
            }
            SC_LOG_ERROR("               ");
        }
    }

    return check;
}

bool AnalyzeForecastScheduleAgent::CheckStation(ScAddr station, vector<ScAddr> trains, vector<ScAddr> types, vector<ScAddr> times, vector<ScAddr> priorities, vector<ScAddr> directions){
    bool check = true;
    int stop_time;
    ScTemplate getStopTime;

    getStopTime.Quintuple(
        station,
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "stop_time",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_stop_time
    );

    ScTemplateSearchResult resultStopTime;
    bool is_success = m_memoryCtx.HelperSearchTemplate(getStopTime, resultStopTime);
    if(is_success){
        stop_time = stoi(m_memoryCtx.HelperGetSystemIdtf(resultStopTime[0]["stop_time"]));
    }

    SC_LOG_ERROR(stop_time);

    for(int i = 0; i < trains.size() - 1; i++){
        int time1 = FromStringToTime(times[i]);
        int time2 = FromStringToTime(times[i+1]);


        //1. проверка что они к одному времени + остановка, если да то 2
        //2. проверка на линии (то есть 1 < 2, 2 < 1, 1 = 2)
            //2.1. если свободны обе то ставим оба
            //2.2. если свободная одна то приоритет и торможение (4)
            //2.3. если заняты, тормозим оба (4)
        //3. проверка прихода на станцию:
            //3.1. одно направление: если 2 поезда состыкуются, то тормозим не приоритетный
            //3.2 разные: если разные направления, то тоже тормозим по приоритету, но тут прикол что если поезд задержать на станции менять время прихода на некст станцию
        //4. проверка торможения:
            //4.1. если не вмещается в макс промежуток оставляем на прошлой станции
            //4.2. если все ок, то все ок
    }

    return check;
}

int AnalyzeForecastScheduleAgent::FromStringToTime(ScAddr time){
    string name = m_memoryCtx.HelperGetSystemIdtf(time);

    int hours1 = stoi(name.substr(0, name.find('.')));
    int minutes1 = stoi(name.substr(name.find('.') + 1));

    int new_time = hours1 * 60 + minutes1;

    return new_time;
}


string AnalyzeForecastScheduleAgent::FromTimeToString(int time){
    int hours = time / 60;
    int minutes = time % 60;
    ostringstream oss;
    oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
    string new_time = oss.str();

    return new_time;
}
}