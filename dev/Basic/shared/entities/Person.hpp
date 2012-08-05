/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "GenConfig.h"

#include "Agent.hpp"
#include "roles/Role.hpp"
#include "buffering/Shared.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/misc/TripChain.hpp"

namespace sim_mob
{

class TripChainItem;

#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
#endif

/**
 * Basic Person class.
 *
 * \author Seth N. Hetu
 * \author Wang Xinyuan
 * \author Luo Linbo
 * \author Li Zhemin
 * \author Xu Yan
 * \author Harish Loganathan
 *
 * A person may perform one of several roles which
 *  change over time. For example: Drivers, Pedestrians, and Passengers are
 *  all roles which a Person may fulfil.
 */
class Person : public sim_mob::Agent {
public:
	explicit Person(const MutexStrategy& mtxStrat, int id=-1);
	virtual ~Person();

	///Update Person behavior
	virtual Entity::UpdateStatus update(frame_t frameNumber);

	///Load a Person's config-specified properties, creating a placeholder trip chain if
	/// requested.
	virtual void load(const std::map<std::string, std::string>& configProps);

    ///Update a Person's subscription list.
    virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

    ///Change the role of this person: Driver, Passenger, Pedestrian
    void changeRole(sim_mob::Role* newRole);
    sim_mob::Role* getRole() const;

    ///Check if any role changing is required.
    Entity::UpdateStatus checkAndReactToTripChain(unsigned int currTimeMS);

    ///get this person's trip chain
    const std::vector<const TripChainItem*>& getTripChain() const
    {
        return tripChain;
    }

    ///Set this person's trip chain
    void setTripChain(const std::vector<const TripChainItem*>& tripChain)
    {
        this->tripChain = tripChain;
    }

	sim_mob::Link* getCurrLink();
	void setCurrLink(sim_mob::Link* link);

    void getNextSubTripInTrip();
    void findNextItemInTripChain();

    const TripChainItem* currTripChainItem; // pointer to current item in trip chain
    const SubTrip* currSubTrip; //pointer to current subtrip in the current trip (if  current item is trip)

    //Used for passing various debug data. Do not rely on this for anything long-term.
    std::string specialStr;

private:
    //Properties
    sim_mob::Role* prevRole; ///< To be deleted on the next time tick.
    sim_mob::Role* currRole;
    sim_mob::Link* currLink;


    int currTripChainSequenceNumber;
    std::vector<const TripChainItem*> tripChain;
    bool firstFrameTick;
    ///Determines if frame_init() has been done.
    friend class PartitionManager;
    friend class BoundaryProcessor;

#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);

#endif
};

}
