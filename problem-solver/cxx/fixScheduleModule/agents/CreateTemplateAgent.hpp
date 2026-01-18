/*
* This source file is part of an OSTIS project. For the latest info, see http://ostis.net
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include <sc-memory/kpm/sc_agent.hpp>

#include "keynodes/keynodes.hpp"
#include "CreateTemplateAgent.generated.hpp"

namespace fixScheduleModule
{

class CreateTemplateAgent : public ScAgent
{
  SC_CLASS(Agent, Event(Keynodes::question_create_template, ScEvent::Type::AddOutputEdge))
  SC_GENERATED_BODY()

  void CreateTemplate(ScAddr train, ScAddr station, ScAddr time);

};

} // namespace fixScheduleModule
