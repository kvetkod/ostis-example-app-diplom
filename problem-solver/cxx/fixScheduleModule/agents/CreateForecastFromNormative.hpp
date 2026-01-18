/*
* This source file is part of an OSTIS project. For the latest info, see http://ostis.net
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include <sc-memory/kpm/sc_agent.hpp>

#include "keynodes/keynodes.hpp"
#include "CreateForecastFromNormative.generated.hpp"

namespace fixScheduleModule
{

class CreateForecastFromNormative : public ScAgent
{
  SC_CLASS(Agent, Event(Keynodes::question_create_forecast_from_normative, ScEvent::Type::AddOutputEdge))
  SC_GENERATED_BODY()

  void Analysis(ScAddr normative);
  void CreateForecast(ScAddr type, ScAddr train, ScAddr time, ScAddr station, ScAddr priority, ScAddr direction);
  std::vector<ScAddr> GetPreviousStations(ScAddr train);
};

} // namespace fixScheduleModule
