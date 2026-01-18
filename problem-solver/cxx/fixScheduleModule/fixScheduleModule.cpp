/*
* This source file is part of an OSTIS project. For the latest info, see http://ostis.net
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

#include "fixScheduleModule.hpp"
#include "keynodes/keynodes.hpp"
#include "agents/CreateTemplateAgent.hpp"
#include "agents/CreateForecastFromNormative.hpp"
#include "agents/ForecastingAgent.hpp"
#include "agents/AnalyzerAgent.hpp"
#include "agents/RepairAgent.hpp"
#include "agents/FinishAgent.hpp"


using namespace fixScheduleModule;

SC_IMPLEMENT_MODULE(FixScheduleModule)

sc_result FixScheduleModule::InitializeImpl()
{
  if (!fixScheduleModule::Keynodes::InitGlobal())
    return SC_RESULT_ERROR;

  SC_AGENT_REGISTER(CreateTemplateAgent)
  SC_AGENT_REGISTER(CreateForecastFromNormative)
  SC_AGENT_REGISTER(ForecastingAgent)
  SC_AGENT_REGISTER(AnalyzerAgent)
  SC_AGENT_REGISTER(RepairAgent)
  SC_AGENT_REGISTER(FinishAgent)
  
  return SC_RESULT_OK;
}

sc_result FixScheduleModule::ShutdownImpl()
{
  SC_AGENT_UNREGISTER(CreateTemplateAgent)
  SC_AGENT_UNREGISTER(CreateForecastFromNormative)
  SC_AGENT_UNREGISTER(ForecastingAgent)
  SC_AGENT_UNREGISTER(AnalyzerAgent)
  SC_AGENT_UNREGISTER(RepairAgent)
  SC_AGENT_UNREGISTER(FinishAgent)
  
  return SC_RESULT_OK;
}