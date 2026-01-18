/*
* This source file is part of an OSTIS project. For the latest info, see http://ostis.net
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include <sc-memory/kpm/sc_agent.hpp>

#include "keynodes/keynodes.hpp"
#include "ForecastingAgent.generated.hpp"

namespace fixScheduleModule
{

class ForecastingAgent : public ScAgent
{
  SC_CLASS(Agent, Event(Keynodes::question_forecasting, ScEvent::Type::AddOutputEdge))
  SC_GENERATED_BODY()

  void Forecast(ScAddr train, ScAddr lastInfo);
  
  int GetTime(ScAddr normative, ScAddr station);
  int GetStopTime(ScAddr station);
  void CreateForecast(std::string time, ScAddr train, ScAddr station);
};

} // namespace fixScheduleModule
