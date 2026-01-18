#include <sc-agents-common/utils/GenerationUtils.hpp>
#include <sc-agents-common/utils/AgentUtils.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>
#include <sc-agents-common/keynodes/coreKeynodes.hpp>

#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "CreateForecastFromNormative.hpp"

using namespace std;
using namespace utils;

namespace fixScheduleModule
{

SC_AGENT_IMPLEMENTATION(CreateForecastFromNormative)
{
  ScAddr actionNode = otherAddr;
  SC_LOG_ERROR("my agent started!");

  ScTemplate getNormative;

  getNormative.Triple(
    actionNode,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown >> "normative"
  );

  ScTemplateSearchResult resultNormative;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getNormative, resultNormative);

  if(is_success){
    for(int i = 0; i < resultNormative.Size(); i++){
        Analysis(resultNormative[i]["normative"]);
    }
  }

  ScTemplate getEdge;

  getEdge.Triple(
    Keynodes::question_create_forecast_from_normative,
    ScType::EdgeAccessVarPosPerm >> "edge",
    actionNode
  );

  ScTemplateSearchResult resultEdge;
  is_success = m_memoryCtx.HelperSearchTemplate(getEdge, resultEdge);
  if (is_success) {
    m_memoryCtx.EraseElement(resultEdge[0]["edge"]);
  }

  return SC_RESULT_OK;
}

void CreateForecastFromNormative::Analysis(ScAddr normative){
    ScAddr train;
    ScAddr type;
    ScTemplate getTrain;

    getTrain.Quintuple(
        ScType::Unknown >> "train",
        ScType::EdgeDCommonVar,
        normative,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_normative
    );

    ScTemplateSearchResult resultTrain;
    bool is_success = m_memoryCtx.HelperSearchTemplate(getTrain, resultTrain);
    if(is_success){
        train = resultTrain[0]["train"];
    }

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

    ScTemplate getTrainAt;

    getTrainAt.Quintuple(
        normative,
        ScType::EdgeAccessVarPosPerm,
        ScType::Unknown >> "train_at",
        ScType::EdgeAccessVarPosPerm,
        ScType::Unknown 
    );

    getTrainAt.Quintuple(
        "train_at",
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "time",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_normative_time
    );

    getTrainAt.Quintuple(
        "train_at",
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "station",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_station
    );

    getTrainAt.Quintuple(
        "train_at",
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "priority",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_priority
    );

    getTrainAt.Quintuple(
        "train_at",
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "direction",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_direction
    );
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(train));
    ScTemplateSearchResult resultTrainAt;
    is_success = m_memoryCtx.HelperSearchTemplate(getTrainAt, resultTrainAt);
    if(is_success){
        for(int i = 0; i < resultTrainAt.Size(); i++){
            ScAddr time, station, priority, direction;
            time = resultTrainAt[i]["time"];
            station = resultTrainAt[i]["station"];
            priority = resultTrainAt[i]["priority"];
            direction = resultTrainAt[i]["direction"];
            SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(station));
            CreateForecast(type, train, time, station, priority, direction);
        }
    }
    return;
}

void CreateForecastFromNormative::CreateForecast(ScAddr type, ScAddr train, ScAddr time, ScAddr station, ScAddr priority, ScAddr direction){
    
    vector<ScAddr> prevStations = GetPreviousStations(train);

    for(int i = 0; i < prevStations.size(); i++){
        if(m_memoryCtx.HelperGetSystemIdtf(prevStations[i]) == m_memoryCtx.HelperGetSystemIdtf(station)){
            return;
        }
    }
    
    ScTemplate getForecast;

    getForecast.Triple(
        Keynodes::forecast_schedule,
        ScType::EdgeAccessVarPosPerm,
        ScType::Unknown >> "for_station"
    );

    getForecast.Quintuple(
        "for_station",
        ScType::EdgeDCommonVar,
        station,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_station
    );

    ScAddr for_station;
    ScTemplateSearchResult resultForecast;
    bool is_success = m_memoryCtx.HelperSearchTemplate(getForecast, resultForecast);
    if(is_success){
        for_station = resultForecast[0]["for_station"];
        SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultForecast[0]["for_station"]));
    }

    string name = "for_" + m_memoryCtx.HelperGetSystemIdtf(station) + "_" + m_memoryCtx.HelperGetSystemIdtf(train);
    ScAddr for_train = m_memoryCtx.CreateNode(ScType::NodeConst);
    m_memoryCtx.HelperSetSystemIdtf(name, for_train);

    ScTemplate addForecast;
    addForecast.Triple(
        for_station,
        ScType::EdgeAccessVarPosPerm,
        for_train
    );  

    addForecast.Quintuple(
        for_train,
        ScType::EdgeDCommonVar,
        train,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_train
    );

    addForecast.Quintuple(
        for_train,
        ScType::EdgeDCommonVar,
        type,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_train_type
    );

    addForecast.Quintuple(
        for_train,
        ScType::EdgeDCommonVar,
        time,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_time
    );

    addForecast.Quintuple(
        for_train,
        ScType::EdgeDCommonVar,
        priority,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_priority
    );

    addForecast.Quintuple(
        for_train,
        ScType::EdgeDCommonVar,
        direction,
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_direction
    );
    
    ScTemplateGenResult resultAddForecast;
    m_memoryCtx.HelperGenTemplate(addForecast, resultAddForecast);

    return;
}

vector<ScAddr> CreateForecastFromNormative::GetPreviousStations(ScAddr train){
    vector<ScAddr> stations;

    ScTemplate getStations;

    getStations.Quintuple(
        train,
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "actual",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_actual
    );

    getStations.Quintuple(
        "actual",
        ScType::EdgeAccessVarPosPerm,
        ScType::Unknown >> "for_stat",
        ScType::EdgeAccessVarPosPerm,
        ScType::Unknown 
    );

    getStations.Quintuple(
        "for_stat",
        ScType::EdgeDCommonVar,
        ScType::Unknown >> "station",
        ScType::EdgeAccessVarPosPerm,
        Keynodes::nrel_arrived
    );

    ScTemplateSearchResult resultStations;
    bool is_success = m_memoryCtx.HelperSearchTemplate(getStations, resultStations);
    if(is_success){
        for(int i = 0; i < resultStations.Size(); i++){
            stations.push_back(resultStations[i]["station"]);
        }
    }

    return stations;
}
}