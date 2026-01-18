/*
* This source file is part of an OSTIS project. For the latest info, see http://ostis.net
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

#include "trainModule.hpp"
#include "keynodes/keynodes.hpp"
#include "agents/GetTrainScheduleAgent.hpp"
#include "agents/AnalyzeForecastScheduleAgent.hpp"

using namespace trainModule;

SC_IMPLEMENT_MODULE(TrainModule)

sc_result TrainModule::InitializeImpl()
{
  if (!trainModule::Keynodes::InitGlobal())
    return SC_RESULT_ERROR;

  SC_AGENT_REGISTER(GetTrainScheduleAgent)
  SC_AGENT_REGISTER(AnalyzeForecastScheduleAgent)
  return SC_RESULT_OK;
}

sc_result TrainModule::ShutdownImpl()
{
  SC_AGENT_UNREGISTER(GetTrainScheduleAgent)
  SC_AGENT_UNREGISTER(AnalyzeForecastScheduleAgent)
  

  return SC_RESULT_OK;
}