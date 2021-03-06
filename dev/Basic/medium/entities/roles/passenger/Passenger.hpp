//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/Person_MT.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/waitBusActivity/WaitBusActivity.hpp"
#include "geospatial/network/WayPoint.hpp"

namespace sim_mob
{

class Agent;
class Person;
class BusStop;

namespace medium
{

class PassengerBehavior;
class PassengerMovement;
class Driver;

/**
 * A medium-term Passenger.
 * \author Seth N. Hetu
 * \author zhang huai peng
 */
class Passenger : public sim_mob::Role<Person_MT>
{
public:

    explicit Passenger(Person_MT *parent, 
                    sim_mob::medium::PassengerBehavior* behavior = nullptr,
                    sim_mob::medium::PassengerMovement* movement = nullptr,
                    std::string roleName = std::string("Passenger_"),
                    Role<Person_MT>::Type roleType = Role<Person_MT>::RL_PASSENGER);

    virtual ~Passenger()
    {
    }

    //Virtual overrides
    virtual Role<Person_MT>* clone(Person_MT *parent) const;

    virtual void make_frame_tick_params(timeslice now)
    {
    }
    
    virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
    /**
     * make a decision for alighting.
     * @param nextStop is the next stop which bus will arrive at
     */
    void makeAlightingDecision(const sim_mob::BusStop* nextStop);

    /**
     * message handler which provide a chance to handle message transfered from parent agent.
     * @param type of the message.
     * @param message data received.
     */
    virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message);

    /**
     * collect travel time for current role
     */
    virtual void collectTravelTime();

    /**
     * collect walking time
     */
    void collectWalkingTime();

    bool canAlightBus() const
    {
        return alightBus;
    }

    void setAlightBus(bool alightBus)
    {
        this->alightBus = alightBus;
    }

    const sim_mob::medium::Driver* getDriver() const
    {
        return driver;
    }

    void setDriver(const Driver* driver)
    {
        this->driver = driver;
    }

    const sim_mob::WayPoint& getEndPoint() const
    {
        return endPoint;
    }

    void setEndPoint(const sim_mob::WayPoint& endPoint)
    {
        this->endPoint = endPoint;
    }

    const sim_mob::WayPoint& getStartPoint() const
    {
        return startPoint;
    }

    void setStartPoint(const sim_mob::WayPoint& startPoint)
    {
        this->startPoint = startPoint;
    }

    double getWalkTimeToPlatform() const
    {
        return remainingWalkTime;
    }
    void setWalkTimeToPlatform(double walkTime)
    {
        remainingWalkTime = walkTime;
        originalWalkTime = walkTime;
    }

    double reduceWalkingTime(double value)
    {
        return remainingWalkTime -= value;
    }

    void setService(const std::vector<WayPoint>& lines)
    {
        std::stringstream sst;
        sst << "[";
        for(auto i=lines.begin(); i!=lines.end(); i++){
            if(i->type==WayPoint::LINK){
                sst<<i->link->getLinkId()<<" ";
            }
        }
        sst << "]";
        this->service = sst.str();
    }


    double getStartPointDriverDistance() const
    {
        return startPointDriverDistance;
    }

    void setStartPointDriverDistance(double DriverDistance)
    {
        startPointDriverDistance = DriverDistance;
    }

    double getFinalPointDriverDistance() const
    {
        return finalPointDriverDistance;
    }

    void setFinalPointDriverDistance(double DriverDistance)
    {
        finalPointDriverDistance = DriverDistance;
    }

    const unsigned long getSharingCount() const
    {
        return maxSharingCount;
    }

    void setSharingCount(const unsigned long currentCount)
    {
        maxSharingCount = std::max(maxSharingCount, currentCount);
    }

    void resetSharingCount()
    {
        maxSharingCount = 0;
    }

private:
    friend class PassengerBehavior;
    friend class PassengerMovement;

    /** Driver who is driving the vehicle of this passenger*/
    const Driver* driver;

    /**flag to indicate whether the passenger has decided to alight the bus*/
    bool alightBus;

    /** starting point of passenger - for travel time storage */
    sim_mob::WayPoint startPoint;

    /** ending node of passenger - for travel time storage */
    sim_mob::WayPoint endPoint;

    /**record service info*/
    std::string service;

    /**walking time to the platform*/
    double remainingWalkTime = 0.0;

    /**original walk time*/
    double originalWalkTime = 0.0;

    /**To get Passenger Travel distance in Taxi, Here it is Driver offset distance . Taxi Passenger Travel Distance
     * Taxi Passenger Travel Distance = finalPointDriverDistance - startPointDriverDistance
     */
    double startPointDriverDistance =0.0;
    double finalPointDriverDistance=0.0;

    /**Max number of people this person shares an on-call ride with*/
    unsigned long maxSharingCount = 0;
};

}
}
