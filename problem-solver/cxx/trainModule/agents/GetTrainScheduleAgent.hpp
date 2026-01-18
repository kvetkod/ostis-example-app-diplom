/*
* This source file is part of an OSTIS project. For the latest info, see http://ostis.net
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include <sc-memory/kpm/sc_agent.hpp>

#include "keynodes/keynodes.hpp"
#include "GetTrainScheduleAgent.generated.hpp"

namespace trainModule
{

class GetTrainScheduleAgent : public ScAgent
{
  SC_CLASS(Agent, Event(Keynodes::question_get_schedule, ScEvent::Type::AddOutputEdge))
  SC_GENERATED_BODY()

  void Analysis(ScAddr schedule);
  std::string CalculateInterval(std::string interval, std::string time1, std::string time2, ScAddr station2);
  void CreateForecast(ScAddr train, ScAddr type, std::vector<std::string> times, std::vector<ScAddr> stations, std::vector<ScAddr> train_at);
  void RemoveOldForecast(ScAddr for_train);
};

} // namespace trainModule
