/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * BusController.hpp
 *
 *  Created on: 2012-6-11
 *      Author: Yao Jin
 */

#pragma once

#include <vector>

#include "entities/Agent.hpp"

#include "buffering/Shared.hpp"
#include "entities/UpdateParams.hpp"
#include "vehicle/Bus.hpp"
#include "roles/driver/Driver.hpp"
#include "util/DynamicVector.hpp"

namespace sim_mob
{
class Bus;

class BusController : public sim_mob::Agent
{
public:
	static BusController & getInstance() {return sim_mob::BusController::instance_;}
	static BusController * get_pInstance() {return &(sim_mob::BusController::instance_);}
	virtual Entity::UpdateStatus update(frame_t frameNumber);
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);
	void updateBusInformation(DPoint pt);
	bool isTobeUpdated() { return TobeUpdated; }
	void setTobeUpdated() { TobeUpdated = true; }

	///Retrieve a reference to the list of managedBuses.
	std::vector<sim_mob::Bus*>& getManagedBuses() { return managedBuses; }
	//const sim_mob::RoadNetwork& getNetwork() { return network; }

private:
	explicit BusController(const MutexStrategy& mtxStrat = sim_mob::ConfigParams::GetInstance().mutexStategy, int id=-1);
	~BusController();
	void frame_init(frame_t frameNumber);
	void frame_tick_output(frame_t frameNumber);
	static BusController instance_;

	frame_t frameNumberCheck;// check some frame number to do control
	bool firstFrameTick;  ///Determines if frame_init() has been done.
	bool TobeUpdated;// Determines whether Xml has buscontroller thus to be updated in output file
	std::vector<sim_mob::Bus*> managedBuses;// Saved managedBuses
	//sim_mob::RoadNetwork network;// Saved RoadNetwork
	DPoint posBus;// The sent position of a given bus ,only for test

};



}
