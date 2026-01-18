/*
* This source file is part of an OSTIS project. For the latest info, see http://ostis.net
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include <sc-memory/kpm/sc_agent.hpp>

#include "keynodes/keynodes.hpp"
#include "AnalyzeForecastScheduleAgent.generated.hpp"

namespace trainModule
{

class AnalyzeForecastScheduleAgent : public ScAgent
{
  SC_CLASS(Agent, Event(Keynodes::question_analyze_forecast, ScEvent::Type::AddOutputEdge))
  SC_GENERATED_BODY()

  bool AnalyzeForecast(ScAddr forecast);
  bool CheckStation(ScAddr station, std::vector<ScAddr> trains, std::vector<ScAddr> types, std::vector<ScAddr> times, std::vector<ScAddr> priorities, std::vector<ScAddr> directions);
  int FromStringToTime(ScAddr time);
  std::string FromTimeToString(int time);
};

} // namespace trainModule
