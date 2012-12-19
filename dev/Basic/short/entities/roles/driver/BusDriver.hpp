/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "Driver.hpp"
#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"
#include <vector>

namespace sim_mob {

//Forward declarations
class DriverUpdateParams;
class PackageUtils;
class UnPackageUtils;
class BusStop;
class Person;
class Bus;
class Passenger;


/**
 * This simple BusDriver class maintains a single, non-looping route with a series of
 *   stops. Most driving behavior is re-used from the Driver class. At bus stops, the
 *   BusDriver will simply pull over to the left-most lane and stop for a length of time.
 *
 * \author Seth N. Hetu
 */
class BusDriver : public sim_mob::Driver {
public:
	BusDriver(sim_mob::Person* parent, sim_mob::MutexStrategy mtxStrat);

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	/// Return the distance (m) to the (next) bus stop.
	/// A negative return value indicates that there is no relevant bus stop nearby.
	double distanceToNextBusStop();

	/// get distance to bus stop of particular segment (meter)
	/// A negative return value indicates that there is no relevant bus stop nearby.
	double getDistanceToBusStopOfSegment(const RoadSegment* rs);

	//double no_passengers_boarding,no_passengers_alighting;

	bool isBusFarawayBusStop();
	bool isBusApproachingBusStop();
	bool isBusArriveBusStop();
	bool isBusLeavingBusStop();
	double busAccelerating(DriverUpdateParams& p);
	//mutable double lastTickDistanceToBusStop;
	//void passengers_distribution(Bus* bus);
	void passengers_Board(Bus* bus);
	void passengers_Alight(Bus* bus);
	double passengerGeneration(Bus* bus);
	double dwellTimeCalculation(int busline_i, int trip_k, int busstopSequence_j,int A,int B,int delta_bay,int delta_full,int Pfront,int no_of_passengers); // dwell time calculation module
	std::vector<const sim_mob::BusStop*> findBusStopInPath(const std::vector<const sim_mob::RoadSegment*>& path) const;

	double getPositionX() const;
	double getPositionY() const;

	double getWaitTime_BusStop() { return BUS_STOP_WAIT_PASSENGER_TIME_SEC; }
	void setWaitTime_BusStop(double time) { BUS_STOP_WAIT_PASSENGER_TIME_SEC = time; }
	Vehicle* initializePath_bus(bool allocateVehicle);
	Shared<BusStop_RealTimes>* getCurrentBusStopRealTimes() {
		return curr_busStopRealTimes;
	}
	std::vector<Shared<BusStop_RealTimes>* >& getBusStop_RealTimes() {
		return busStopRealTimes_vec_bus;
	}

	double lastTickDistanceToBusStop;
	Shared<const BusStop*> lastVisited_BusStop; // can get some passenger count, passenger information and busStop information
	Shared<int> lastVisited_BusStopSequenceNum; // last visited busStop sequence number m, reset by BusDriver, What Time???(needed for query the last Stop m -->realStop Times)---> move to BusTrip later
	Shared<double> real_DepartureTime; // set by BusController, reset once stop at only busStop j (j belong to the small set of BusStops)
	Shared<double> real_ArrivalTime; // set by BusDriver, reset once stop at any busStop
	Shared<BusStop_RealTimes>* curr_busStopRealTimes; // current BusStop real Times, convenient for reset
	Shared<double> DwellTime_ijk; // set by BusDriver, reset once stop at any busStop
	double dwellTime_record;// set by BusDriver(temporary), only needed by BusDriver
	Shared<int> busstop_sequence_no; // set by BusDriver, has 0.1sec delay

	std::vector<Shared<BusStop_RealTimes>* > busStopRealTimes_vec_bus;// can be different for different pair<busLine_id,busTripRun_sequenceNum>

	bool first_busstop;
	bool last_busstop;
	bool passengerCountOld_display_flag;
	size_t no_passengers_boarding;
	size_t no_passengers_alighting;

protected:
	//Override the following behavior
	virtual double linkDriving(DriverUpdateParams& p);

//Basic data
private:

	//BusRoute route;
	const DemoBusStop* nextStop;
	std::vector<DemoBusStop> stops;
	std::vector<DemoBusStop> arrivedStops;
	double waitAtStopMS;
	std::vector<const BusStop*> busStops;

	double BUS_STOP_WAIT_PASSENGER_TIME_SEC;


	//Serialization, not implemented
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil){};
	virtual void unpack(UnPackageUtils& unpackageUtil){};

	virtual void packProxy(PackageUtils& packageUtil){};
	virtual void unpackProxy(UnPackageUtils& unpackageUtil){};
#endif

};



}
