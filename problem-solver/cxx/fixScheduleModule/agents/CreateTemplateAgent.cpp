#include <sc-agents-common/utils/GenerationUtils.hpp>
#include <sc-agents-common/utils/AgentUtils.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>
#include <sc-agents-common/keynodes/coreKeynodes.hpp>

#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "CreateTemplateAgent.hpp"

using namespace std;
using namespace utils;

namespace fixScheduleModule
{

SC_AGENT_IMPLEMENTATION(CreateTemplateAgent)
{
  ScAddr actionNode = otherAddr;
  SC_LOG_ERROR("my agent started!");
  ScAddr train, station, time;
  ScTemplate getInfo;

  getInfo.Triple(
    actionNode, 
    ScType::EdgeAccessVarPosPerm >> "y1",
    ScType::Unknown >> "train"
  );

  getInfo.Triple(
    actionNode, 
    ScType::EdgeAccessVarPosPerm >> "y2",
    ScType::Unknown >> "station"
  );

  getInfo.Triple(
    actionNode, 
    ScType::EdgeAccessVarPosPerm >> "y3",
    ScType::Unknown >> "time"
  );

  ScTemplateSearchResult resultInfo;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getInfo, resultInfo);
  if(is_success){
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInfo[0]["train"]));
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInfo[0]["station"]));
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(resultInfo[0]["time"]));
    train = resultInfo[0]["train"];
    station = resultInfo[0]["station"];
    time = resultInfo[0]["time"];
    m_memoryCtx.EraseElement(resultInfo[0]["y1"]);
    m_memoryCtx.EraseElement(resultInfo[0]["y2"]);
    m_memoryCtx.EraseElement(resultInfo[0]["y3"]);
  }
  else{
    ScTemplate getEdge;
    getEdge.Triple(
      Keynodes::question_create_template,
      ScType::EdgeAccessVarPosPerm >> "y",
      actionNode
    );
    ScTemplateSearchResult resultEdge;
    is_success = m_memoryCtx.HelperSearchTemplate(getEdge, resultEdge);
    if(is_success){
      m_memoryCtx.EraseElement(resultEdge[0]["y"]);
    }
    SC_LOG_ERROR("wrong parameters");
    return SC_RESULT_ERROR;
  }

  CreateTemplate(train, station, time);

  ScTemplate getEdge;
  getEdge.Triple(
    Keynodes::question_create_template,
    ScType::EdgeAccessVarPosPerm >> "y",
    actionNode
  );

  ScTemplateSearchResult resultEdge;
  is_success = m_memoryCtx.HelperSearchTemplate(getEdge, resultEdge);
  if(is_success){
    m_memoryCtx.EraseElement(resultEdge[0]["y"]);
  }

  return SC_RESULT_OK;
}

void CreateTemplateAgent::CreateTemplate(ScAddr train, ScAddr station, ScAddr time){
  ScAddr actual;
  ScTemplate getActual;

  getActual.Quintuple(
    train,
    ScType::EdgeDCommonVar,
    ScType::Unknown >> "actual",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_actual
  );

  ScTemplateSearchResult resultActual;
  bool is_success = m_memoryCtx.HelperSearchTemplate(getActual, resultActual);
  if(is_success){
    actual = resultActual[0]["actual"];
    SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(actual));
  }

  string name = m_memoryCtx.HelperGetSystemIdtf(train) + "_at_" + m_memoryCtx.HelperGetSystemIdtf(station);
  SC_LOG_ERROR(name);
  ScAddr new_node = m_memoryCtx.HelperFindBySystemIdtf(name);
  SC_LOG_ERROR(m_memoryCtx.HelperGetSystemIdtf(new_node));

  ScTemplate checkInfo;
  checkInfo.Quintuple(
    actual,
    ScType::EdgeAccessVarPosPerm,
    new_node,
    ScType::EdgeAccessVarPosPerm,
    ScType::Unknown
  );
  ScTemplateSearchResult resultCheck;
  is_success = m_memoryCtx.HelperSearchTemplate(checkInfo, resultCheck);
  if(is_success){
    SC_LOG_ERROR("this is one of the previous stations");
    return;
  }

  ScTemplate getLast;
  getLast.Quintuple(
    actual,
    ScType::EdgeAccessVarPosPerm >> "edge",
    ScType::Unknown >> "train_at",
    ScType::EdgeAccessVarPosPerm,
    Keynodes::rrel_last
  );
  ScTemplateSearchResult resultLast;
  is_success = m_memoryCtx.HelperSearchTemplate(getLast, resultLast);
  if(is_success){
    m_memoryCtx.EraseElement(resultLast[0]["edge"]);
    ScTemplate createPrevious;
    createPrevious.Quintuple(
      actual,
      ScType::EdgeAccessVarPosPerm,
      resultLast[0]["train_at"],
      ScType::EdgeAccessVarPosPerm,
      Keynodes::rrel_previous
    );
    ScTemplateGenResult resultCreatePrevious;
    m_memoryCtx.HelperGenTemplate(createPrevious, resultCreatePrevious);
  }
  
  ScTemplate createLast;
  createLast.Quintuple(
    actual,
    ScType::EdgeAccessVarPosPerm,
    new_node,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::rrel_last
  );
  createLast.Quintuple(
    new_node,
    ScType::EdgeDCommonVar,
    station,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_arrived
  );
  createLast.Quintuple(
    new_node,
    ScType::EdgeDCommonVar,
    time,
    ScType::EdgeAccessVarPosPerm,
    Keynodes::nrel_time
  );
  ScTemplateGenResult resultCreateLast;
  m_memoryCtx.HelperGenTemplate(createLast, resultCreateLast);
  ScAddr empty = m_memoryCtx.CreateNode(ScType::NodeConst);

  ScTemplate startAgent;

  startAgent.Triple(
    empty,
    ScType::EdgeAccessVarPosPerm,
    new_node
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

  ScTemplateGenResult resultStart;
  m_memoryCtx.HelperGenTemplate(startAgent, resultStart);

  return; 
}

}