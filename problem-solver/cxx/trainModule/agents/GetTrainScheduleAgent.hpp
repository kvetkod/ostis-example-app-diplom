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
  static bool comparePairs(const std::pair<std::string, ScAddr>& a,const  std::pair<std::string, ScAddr>& b);
  void TrainCatchesUp(ScAddr &train, ScAddr &time, ScAddr &trainType, ScAddr &track, ScAddr lastStation, ScAddr &nextStation, ScAddr &normative);
  bool CheckTime(std::string interval, std::string time_1, std::string& time_2);
  void WhichTrainsAreComing();
  void CreateForecastSchedule(ScAddr &train, ScAddr &trainType, std::string &time, ScAddr &station);
  void FinishForecastSchedule();
  bool CheckActualSchedule(ScAddr train, ScAddr station);
  void CheckCollisionsOnStation(std::vector <ScAddr> trains, std::vector <ScAddr> times, std::vector<ScAddr> priorities, std::vector<ScAddr> for_trains, std::vector <ScAddr> directions, ScAddr station); 
  ScAddr EditEdge(ScAddr for_train, int time, int time2, ScAddr train, ScAddr station);
  bool CheckLines(ScAddr train1, ScAddr train2, ScAddr station, ScAddr for_train_1, ScAddr for_train_2);
  void CheckPriority(ScAddr priority1, ScAddr priority2, ScAddr for_train1, ScAddr for_train2, int time1, int time2, ScAddr train1, ScAddr train2, ScAddr station);
  bool IfLineIsBusy(ScAddr station, ScAddr line);
  void CheckLineBeforeStation(std::vector<ScAddr> for_trains, std::vector<ScAddr> directions, std::vector<ScAddr> &times, std::vector<ScAddr> trains, ScAddr station);
  void addDirecationAndPriorityOnForecast();
  void CreateNewTimeNode(ScAddr time, ScAddr for_train);
  void CheckMaxInterval(ScAddr train, ScAddr station, ScAddr time);
  bool CheckNormativeSchedule(ScAddr train1, ScAddr train2, ScAddr station);
};

} // namespace trainModule
