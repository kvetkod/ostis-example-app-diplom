#include <sc-agents-common/utils/GenerationUtils.hpp>
#include <sc-agents-common/utils/AgentUtils.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>
#include <sc-agents-common/keynodes/coreKeynodes.hpp>

#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "ForecastingAgent.hpp"

using namespace std;
using namespace utils;

namespace fixScheduleModule
{

SC_AGENT_IMPLEMENTATION(ForecastingAgent)
{
  ScAddr actionNode = otherAddr;
  SC_LOG_ERROR("forecasting started!");
  ScAddr train, info;
  ScTemplate getInfo;
  getInfo.Triple(
    actionNode,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "info"
  );
  getInfo.Triple(
    actionNode,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "train"
  );
  ScTemplateSearchResult resultInfo;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getInfo, resultInfo);
  if(is_success){
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInfo[0]["train"]));
    train = resultInfo[0]["train"];
    info = resultInfo[0]["info"];
  }

  ScTemplate clearForecast;

    clearForecast.Triple(
        Keynodes::forecast,
        ScType::EdgeAccessVarPosPerm >> "edge",
        ScType::Unknown >> "node"
    );

    ScTemplateSearchResult resultClear;
    is_success = m_memoryCtx.HelperSearchTemplate(clearForecast, resultClear);
    if(is_success){
        for(int i = 0; i < resultClear.Size(); i++){
            m_memoryCtx.EraseElement(resultClear[i]["edge"]);
            m_memoryCtx.EraseElement(resultClear[i]["node"]);
        }
    }
  //clear forecast
  Forecast(train, info);

  ScTemplate startAgent;

  startAgent.Triple(
    Keynodes::question_analyze,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::forecast
  );

  ScTemplateGenResult resultStart;
  m_memoryCtx.HelperGenTemplate(startAgent, resultStart);

  ScTemplate getEdge;
  getEdge.Triple(
    Keynodes::question_forecasting,
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

void ForecastingAgent::Forecast(ScAddr train, ScAddr lastInfo){
    ScAddr normative, track;

    ScTemplate getNormative;
    getNormative.Quintuple(
        train,
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "normative",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_normative
    );

    ScTemplateSearchResult resultNormative;
    bool is_success = m_memoryCtx.HelperSearchTemplate(getNormative, resultNormative);
    if(is_success){
        normative = resultNormative[0]["normative"];
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(normative));
    }

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
    if(is_success){
        track = resultTrack[0]["track"];
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(track));
    }

    //if not actual -> ?
    ScAddr lastStation, lastTime;
    ScTemplate getInfo;
    getInfo.Quintuple(
        lastInfo,
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "station",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_arrived
    );  
    getInfo.Quintuple(
        lastInfo,
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "time",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_time
    );  
    ScTemplateSearchResult resultInfo;
    is_success = m_memoryCtx.HelperSearchTemplate(getInfo, resultInfo);
    if(is_success){
        lastStation = resultInfo[0]["station"];
        lastTime = resultInfo[0]["time"];
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(lastStation));
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(lastTime));
    }


    //get route
    bool check = true;
    ScAddr last = lastStation;
    vector<ScAddr> stations;
    vector<int> intervals;
    while(check){
        ScTemplate getFirst;

        getFirst.Quintuple(
            track,
            ScType::EdgeAccessVarPosPerm,
            ScType::Unknown >> "track_",
            ScType::EdgeAccessVarPosPerm,
            ScType::Unknown
        );
        getFirst.Quintuple(
            "track_",
            ScType::EdgeAccessVarPosPerm,
            last,
            ScType::EdgeAccessVarPosPerm,
            Keynodes::rrel_1
        );
        getFirst.Quintuple(
            "track_",
            ScType::EdgeAccessVarPosPerm,
            ScType::Unknown >> "next",
            ScType::EdgeAccessVarPosPerm,
            Keynodes::rrel_2
        );

        ScTemplateSearchResult resultFirst;
        check = m_memoryCtx.HelperSearchTemplate(getFirst, resultFirst);
        if(check){
            SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultFirst[0]["track_"]));
            stations.push_back(resultFirst[0]["next"]);
            ScTemplate getTime;
            getTime.Triple(
                resultFirst[0]["track_"],
                ScType::EdgeAccessVarPosPerm,
                ScType::Unknown >>  "id"
            );
            getTime.Quintuple(
                "id",
                ScType::EdgeDCommonVar,
                ScType::Unknown >> "time",
                ScType::EdgeAccessVarPosPerm,
                Keynodes::nrel_time
            );
            ScTemplateSearchResult resultTime;
            is_success = m_memoryCtx.HelperSearchTemplate(getTime, resultTime);
            if(is_success){
                intervals.push_back(stoi(m_memoryCtx.HelperGetSystemIdtf(resultTime[0]["time"])));
                SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultTime[0]["time"]));
            }
            last = resultFirst[0]["next"];
        }
    }
    last = lastStation;
    int last_time = 0;
    for(int i = 0; i < stations.size(); i++){
        SC_LOG_ERROR(i);
        int time1;
        if(i == 0){
            string name = m_memoryCtx.HelperGetSystemIdtf(lastTime);
            int hours1 = stoi(name.substr(0, name.find('.')));
            int minutes1 = stoi(name.substr(name.find('.') + 1));
            time1 = hours1 * 60 + minutes1 + GetStopTime(last);
        }
        else { time1 = last_time + GetStopTime(last); }
        int time2 = GetTime(normative, stations[i]);
        last = stations[i];
         
        SC_LOG_ERROR(time1);
        SC_LOG_ERROR(time2);
        if(time1 + intervals[i] > time2){
            SC_LOG_ERROR("ошибка");
            time2 = time1 + intervals[i];
            int hours = time2 / 60;
            int minutes = time2 % 60;
            ostringstream oss;
            oss << setw(2) << setfill('0') << hours << "." << setw(2) << setfill('0') << minutes;
            string time_2 = oss.str();
            SC_LOG_ERROR(time_2);
            CreateForecast(time_2, train, stations[i]);
        }
        else {
            break;
        }
        last_time = time2; 
    }


    return;
}

void ForecastingAgent::CreateForecast(string time, ScAddr train, ScAddr station){

    //clear forecast
    ScAddr time_node = m_memoryCtx.HelperFindBySystemIdtf(time);
    if(!time_node.IsValid()){
        time_node = m_memoryCtx.CreateNode(ScType::NodeConst);
        m_memoryCtx.HelperSetSystemIdtf(time, time_node);
    }

    string name = "forecast_" + m_memoryCtx.HelperGetSystemIdtf(train) + "_" + m_memoryCtx.HelperGetSystemIdtf(station);
    ScAddr node_name = m_memoryCtx.HelperFindBySystemIdtf(name);
    if(node_name.IsValid()){
        m_memoryCtx.EraseElement(node_name);
    }
    
    node_name = m_memoryCtx.CreateNode(ScType::NodeConst);
    m_memoryCtx.HelperSetSystemIdtf(name, node_name);

    ScTemplate creating;

    creating.Triple(
        Keynodes::forecast,
        ScType::EdgeAccessVarPosPerm,
        node_name
    );

    creating.Quintuple(
        node_name,
        ScType::EdgeDCommonVar,
        station,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_arrived
    );

    creating.Quintuple(
        node_name,
        ScType::EdgeDCommonVar,
        time_node,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_time
    );

    creating.Quintuple(
        node_name,
        ScType::EdgeDCommonVar,
        train,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_train
    );

    ScTemplateGenResult result;
    m_memoryCtx.HelperGenTemplate(creating, result);
    return;
}

int ForecastingAgent::GetTime(ScAddr normative, ScAddr station){
    int time;
    string name;
    ScTemplate getTime;

    getTime.Quintuple(
        normative, 
        ScType::EdgeAccessVarPosPerm,
        ScType::Unknown >> "train_at",
        ScType::EdgeAccessVarPosPerm,
        ScType::Unknown 
    );
    getTime.Quintuple(
        "train_at",
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "time",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_normative_time
    );
    getTime.Quintuple(
        "train_at",
        ScType::EdgeDCommonVar,
        station,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_station
    );

    ScTemplateSearchResult resultTime;
    bool is_success = m_memoryCtx.HelperSearchTemplate(getTime, resultTime);
    if(is_success){
        name = m_memoryCtx.HelperGetSystemIdtf(resultTime[0]["time"]);
        SC_LOG_ERROR(name);
    }

    int hours1 = stoi(name.substr(0, name.find('.')));
    int minutes1 = stoi(name.substr(name.find('.') + 1));
    time = hours1 * 60 + minutes1;

    return time;
}

int ForecastingAgent::GetStopTime(ScAddr station){
    int stop_time;
    ScTemplate getStopTime;
    getStopTime.Quintuple(
        station,
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "stop",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_stop_time
    );
    ScTemplateSearchResult resultStopTime;
    bool is_success = m_memoryCtx.HelperSearchTemplate(getStopTime, resultStopTime);
    if(is_success){
        stop_time = stoi(m_memoryCtx.HelperGetSystemIdtf(resultStopTime[0]["stop"]));
    }

    return stop_time;
}
}