/*
* This source file is part of an OSTIS project. For the latest info, see http://ostis.net
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include <sc-memory/kpm/sc_agent.hpp>

#include "keynodes/keynodes.hpp"
#include "RepairAgent.generated.hpp"

namespace fixScheduleModule
{

class RepairAgent : public ScAgent
{
  SC_CLASS(Agent, Event(Keynodes::question_repair, ScEvent::Type::AddOutputEdge))
  SC_GENERATED_BODY()

  void CollisionOnStation(ScAddr first, ScAddr second);
  void CollisionBeforeStation(ScAddr first, ScAddr second);
  std::vector <ScAddr> GetLines(ScAddr station, ScAddr type);
  int GetTime(ScAddr time);
  void StartForecast(ScAddr train, ScAddr station, ScAddr time);
  void CreateTemporarySchedule(ScAddr train, ScAddr type, ScAddr station, ScAddr time, ScAddr priority, ScAddr direction);
};

} // namespace fixScheduleModule
