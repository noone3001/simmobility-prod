/*
 * MobilityServiceControllerManager.hpp
 *
 *  Created on: Apr 12, 2017
 *      Author: Akshay Padmanabha
 */

#pragma once

#include <boost/shared_ptr.hpp>
#include <map>

#include "entities/Agent.hpp"
#include "message/Message.hpp"
#include "MobilityServiceController.hpp"

namespace sim_mob
{

enum MobilityServiceControllerType
{
	SERVICE_CONTROLLER_UNKNOWN = 0b0000,
	SERVICE_CONTROLLER_GREEDY = 0b0001,
	SERVICE_CONTROLLER_SHARED = 0b0010,
	SERVICE_CONTROLLER_ON_HAIL = 0b0100
};

class MobilityServiceControllerManager : public Agent
{
public:
	/**
	 * Initialize a single MobilityServiceControllerManager with the given MutexStrategy
	 */
	static bool RegisterMobilityServiceControllerManager(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered);

	~MobilityServiceControllerManager();
	
	/**
	 * Get global singleton instance of MobilityServiceControllerManager
	 * @return pointer to the global instance of MobilityServiceControllerManager
	 */
	static MobilityServiceControllerManager* GetInstance();

	/**
	 * Checks if the MobilityServiceControllerManager instance exists
	 */
	static bool HasMobilityServiceControllerManager();
	
	/**
	 * Adds a MobilityServiceController to the list of controllers
	 * @param  id                        ID of controller
	 * @param  type                      Type of controller
	 * @param  scheduleComputationPeriod Schedule computation period of controller
	 * @return                           Sucess
	 */
	bool addMobilityServiceController(unsigned int id, unsigned int type, unsigned int scheduleComputationPeriod);

	/**
	 * Removes a MobilityServiceController 
	 * @param  id ID of the MobilityServiceController to remove
	 * @return    Success
	 */
	bool removeMobilityServiceController(unsigned int id);

	/**
	 * Signals are non-spatial in nature.
	 */
	bool isNonspatial();

	/**
	 * Returns a list of enabled controllers
	 */
	std::map<unsigned int, MobilityServiceController*> getControllers();

protected:
	explicit MobilityServiceControllerManager(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered) :
			Agent(mtxStrat, -1)
	{
	}

	/**
	 * Inherited from base class agent to initialize
	 * parameters for MobilityServiceControllerManager
	 */
	Entity::UpdateStatus frame_init(timeslice now);

	/**
	 * Inherited from base class to update this agent
	 */
	Entity::UpdateStatus frame_tick(timeslice now);

	/**
	 * Inherited from base class to output result
	 */
	void frame_output(timeslice now);

private:
	/** Store list of controllers */
	std::map<unsigned int, MobilityServiceController*> controllers;

	/** Store self instance */
	static MobilityServiceControllerManager* instance;
};

}

