/*
* This source file is part of an OSTIS project. For the latest info, see http://ostis.net
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include <sc-memory/kpm/sc_agent.hpp>

#include "keynodes/keynodes.hpp"
#include "AnalyzerAgent.generated.hpp"

namespace fixScheduleModule
{

class AnalyzerAgent : public ScAgent
{
  SC_CLASS(Agent, Event(Keynodes::question_analyze, ScEvent::Type::AddOutputEdge))
  SC_GENERATED_BODY()


  void Analyze(ScAddr forecast);
  void CheckStack(ScAddr forecast);
  bool CheckCollision(ScAddr station, ScAddr train1, ScAddr time1);
  int GetTime(ScAddr time);
  void CreateStack(ScAddr train, ScAddr time, ScAddr station, ScAddr for_train, ScAddr flag);
  void CreateTemporarySchedule(ScAddr train, ScAddr station, ScAddr time);
};

} // namespace fixScheduleModule
