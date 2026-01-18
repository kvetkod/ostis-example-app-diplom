/*
 * This source file is part of an OSTIS project. For the latest info, see
 * http://ostis.net Distributed under the MIT License (See accompanying file
 * COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#pragma once

#include <sc-memory/sc_addr.hpp>
#include <sc-memory/sc_object.hpp>

#include "keynodes.generated.hpp"

namespace fixScheduleModule
{
class Keynodes : public ScObject
{
  SC_CLASS()
  SC_GENERATED_BODY()

public:
  SC_PROPERTY(Keynode("question_create_template"), ForceCreate)
  static ScAddr question_create_template;

  SC_PROPERTY(Keynode("question_create_forecast_from_normative"), ForceCreate)
  static ScAddr question_create_forecast_from_normative;

  SC_PROPERTY(Keynode("question_forecasting"), ForceCreate)
  static ScAddr question_forecasting;

  SC_PROPERTY(Keynode("question_analyze"), ForceCreate)
  static ScAddr question_analyze;

  SC_PROPERTY(Keynode("questione_repair"), ForceCreate)
  static ScAddr question_repair;

  SC_PROPERTY(Keynode("question_finish"), ForceCreate)
  static ScAddr question_finish;

  SC_PROPERTY(Keynode("nrel_normative_time"), ForceCreate)
  static ScAddr nrel_normative_time;

  SC_PROPERTY(Keynode("nrel_station"), ForceCreate)
  static ScAddr nrel_station;

  SC_PROPERTY(Keynode("nrel_priority"), ForceCreate)
  static ScAddr nrel_priority;

  SC_PROPERTY(Keynode("nrel_direction"), ForceCreate)
  static ScAddr nrel_direction;

  SC_PROPERTY(Keynode("nrel_normative"), ForceCreate)
  static ScAddr nrel_normative;

  SC_PROPERTY(Keynode("forecast_schedule"), ForceCreate)
  static ScAddr forecast_schedule;

  SC_PROPERTY(Keynode("nrel_actual"), ForceCreate)
  static ScAddr nrel_actual;

  SC_PROPERTY(Keynode("nrel_arrived"), ForceCreate)
  static ScAddr nrel_arrived;

  SC_PROPERTY(Keynode("nrel_train"), ForceCreate)
  static ScAddr nrel_train;

  SC_PROPERTY(Keynode("nrel_time"), ForceCreate)
  static ScAddr nrel_time;

  SC_PROPERTY(Keynode("nrel_train_type"), ForceCreate)
  static ScAddr nrel_train_type;

  SC_PROPERTY(Keynode("rrel_last"), ForceCreate)
  static ScAddr rrel_last;

  SC_PROPERTY(Keynode("rrel_previous"), ForceCreate)
  static ScAddr rrel_previous;

  SC_PROPERTY(Keynode("nrel_track"), ForceCreate)
  static ScAddr nrel_track;

  SC_PROPERTY(Keynode("rrel_1"), ForceCreate)
  static ScAddr rrel_1;

  SC_PROPERTY(Keynode("rrel_2"), ForceCreate)
  static ScAddr rrel_2;

  SC_PROPERTY(Keynode("nrel_stop_time"), ForceCreate)
  static ScAddr nrel_stop_time;

  SC_PROPERTY(Keynode("nrel_braking_time"), ForceCreate)
  static ScAddr nrel_braking_time;

  SC_PROPERTY(Keynode("forecast"), ForceCreate)
  static ScAddr forecast;

  SC_PROPERTY(Keynode("stack"), ForceCreate)
  static ScAddr stack;

  SC_PROPERTY(Keynode("error_stop"), ForceCreate)
  static ScAddr error_stop;

  SC_PROPERTY(Keynode("error_braking"), ForceCreate)
  static ScAddr error_braking;

  SC_PROPERTY(Keynode("error_max_time"), ForceCreate)
  static ScAddr error_max_time;

  SC_PROPERTY(Keynode("nrel_line"), ForceCreate)
  static ScAddr nrel_line;

  SC_PROPERTY(Keynode("temporary_schedule"), ForceCreate)
  static ScAddr temporary_schedule;
};

}  // namespace fixScheduleModule
