#include "GenConfig.h"

#include "PackageUtils.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include "util/GeomHelpers.hpp"
#include "partitions/PartitionManager.hpp"

namespace sim_mob {
std::string PackageUtils::getPackageData() {
	return buffer.str().data();
}

PackageUtils::PackageUtils()
{
	package = new boost::archive::text_oarchive(buffer);
}

PackageUtils::~PackageUtils()
{
	buffer.clear();
	if (package) {
		delete package;
		package = NULL;
	}
}

//void PackageUtils::clearPackage() {
//	buffer.clear();
//	if (package) {
//		delete package;
//		package = NULL;
//	}
//}

//void PackageUtils::packNode(const Node* one_node) {
//	bool hasSomthing = true;
//	if (!one_node) {
//		hasSomthing = false;
//		(*package) & hasSomthing;
//		return;
//	} else {
//		(*package) & hasSomthing;
//	}
//
//	(*package) & (one_node->location);
//}

//void PackageUtils::packRoadSegment(const RoadSegment* roadsegment) {
//	bool hasSomthing = true;
//	if (!roadsegment) {
//
//		hasSomthing = false;
//		(*package) & hasSomthing;
//		return;
//	} else {
//
//		(*package) & hasSomthing;
//	}
//
//
//	(*package) & (roadsegment->getStart()->location);
//	(*package) & (roadsegment->getEnd()->location);
//}
//void PackageUtils::packLink(const Link* one_link) {
//	bool hasSomthing = true;
//	if (!one_link) {
//		hasSomthing = false;
//		(*package) & hasSomthing;
//		return;
//	} else {
//		(*package) & hasSomthing;
//	}
//
//	(*package) & (one_link->getStart()->location);
//	(*package) & (one_link->getEnd()->location);
//}
//
//void PackageUtils::packLane(const Lane* one_lane) {
//	bool hasSomthing = true;
//	if (!one_lane) {
//		hasSomthing = false;
//		(*package) & hasSomthing;
//		return;
//	} else {
//		(*package) & hasSomthing;
//	}
//
//	sim_mob::Point2D const & start = one_lane->getRoadSegment()->getStart()->location;
//	sim_mob::Point2D const & end = one_lane->getRoadSegment()->getEnd()->location;
//	int lane_id = one_lane->getLaneID();
//
//	(*package) & (start);
//	(*package) & (end);
//	(*package) & lane_id;
//}

//void PackageUtils::packTripChain(const TripChain* tripChain) {
//	bool hasSomthing = true;
//	if (!tripChain) {
//		hasSomthing = false;
//		(*package) & hasSomthing;
//		return;
//	} else {
//		(*package) & hasSomthing;
//	}
//
//	packTripActivity(&(tripChain->from));
//	packTripActivity(&(tripChain->to));
//
//	(*package) & (tripChain->primary);
//	(*package) & (tripChain->flexible);
//	(*package) & (tripChain->startTime);
//	(*package) & (tripChain->mode);
//}
//
//void PackageUtils::packTripActivity(const TripActivity* tripActivity) {
//	bool hasSomthing = true;
//	if (!tripActivity) {
//		hasSomthing = false;
//		(*package) & hasSomthing;
//		return;
//	} else {
//		(*package) & hasSomthing;
//	}
//
//	(*package) & (tripActivity->description);
//	Node::pack(*(this), *(tripActivity->location));
////	packNode(tripActivity->location);
//}

//void PackageUtils::packVehicle(const Vehicle* one_vehicle) {
//	bool hasSomthing = true;
//	if (!one_vehicle) {
//		hasSomthing = false;
//		(*package) & hasSomthing;
//		return;
//	} else {
//		(*package) & hasSomthing;
//	}
//
//	(*package) & (one_vehicle->length);
//	(*package) & (one_vehicle->width);
//
//	packGeneralPathMover(&(one_vehicle->fwdMovement));
//
//	//After fwdMovement
//	(*package) & (one_vehicle->latMovement);
//	(*package) & (one_vehicle->fwdVelocity);
//	(*package) & (one_vehicle->latVelocity);
//	(*package) & (one_vehicle->fwdAccel);
//
//	(*package) & (one_vehicle->posInIntersection);
//	(*package) & (one_vehicle->error_state);
//}
//
//void PackageUtils::packGeneralPathMover(const GeneralPathMover* fwdMovement) {
//	//transfer vector of road segment
//	//part 1
//
//	int path_size = fwdMovement->fullPath.size();
//	(*package) & (path_size);
//
//	std::vector<const sim_mob::RoadSegment*>::const_iterator itr = fwdMovement->fullPath.begin();
//	for (; itr != fwdMovement->fullPath.end(); itr++) {
//		packRoadSegment(*itr);
//	}
//
//	int current_segment = fwdMovement->currSegmentIt - fwdMovement->fullPath.begin();
//	(*package) & current_segment;
//
//
//	//part 2
//	//polypointsList
//	if (fwdMovement->polypointsList.size() > 0) {
//		bool hasPointList = true;
//		(*package) & hasPointList;
//
//		(*package) & (fwdMovement->polypointsList);
//
//		int current_poly = fwdMovement->currPolypoint - fwdMovement->polypointsList.begin();
//		(*package) & current_poly;
//
//		int next_poly = fwdMovement->nextPolypoint - fwdMovement->polypointsList.begin();
//		(*package) & next_poly;
//
//		//copy from GeneralPathMover.cpp, the design of current_zero_poly is not good.
//		//I thought current_zero_poly is the iterator of currPolypoint
//		const std::vector<Point2D>& tempLaneZero = const_cast<RoadSegment*>(*(fwdMovement->currSegmentIt))->getLaneEdgePolyline(0);
//
//		int current_zero_poly = fwdMovement->currLaneZeroPolypoint - tempLaneZero.begin();
//		(*package) & current_zero_poly;
//
//		int next_zero_poly = fwdMovement->nextLaneZeroPolypoint - tempLaneZero.begin();
//		(*package) & next_zero_poly;
//
//	} else {
//		bool hasPointList = false;
//		(*package) & hasPointList;
//	}
//
//	//part 3
//	//Others
//	(*package) & (fwdMovement->distAlongPolyline);
//	(*package) & (fwdMovement->distMovedInCurrSegment);
//	(*package) & (fwdMovement->distOfThisSegment);
//	(*package) & (fwdMovement->distOfRestSegments);
//	(*package) & (fwdMovement->inIntersection);
//	(*package) & (fwdMovement->isMovingForwardsInLink);
//	(*package) & (fwdMovement->currLaneID);
//
//	std::string value_ = fwdMovement->DebugStream.str();
//	(*package) & (value_);
//
//}
//
//void PackageUtils::packCrossing(const Crossing* one_crossing) {
//	bool hasSomthing = true;
//	if (!one_crossing) {
//		hasSomthing = false;
//		(*package) & hasSomthing;
//		return;
//	} else {
//		(*package) & hasSomthing;
//	}
//
//	//std::cout << "333.6" << std::endl;
//
//	Point2D near1 = one_crossing->nearLine.first;
//	Point2D near2 = one_crossing->nearLine.second;
//	Point2D far1 = one_crossing->farLine.first;
//	Point2D far2 = one_crossing->farLine.second;
//
//	(*package) & (near1);
//	(*package) & (near2);
//
//	//std::cout << "333.7" << std::endl;
//
//	(*package) & (far1);
//	(*package) & (far2);
//
//	//std::cout << "333.8" << std::endl;
//}

//void PackageUtils::packIntersectionDrivingModel(const SimpleIntDrivingModel* one_model) {
//	bool hasSomthing = true;
//	if (!one_model) {
//		hasSomthing = false;
//		(*package) & hasSomthing;
//		return;
//	} else {
//		(*package) & hasSomthing;
//	}
//
//	(*package) & (*one_model);
//}

void PackageUtils::packFixedDelayedDPoint(const FixedDelayed<DPoint*>& one_delay) {
	//(*package) & (one_delay.delayMS);
	(*package) & (one_delay.reclaimPtrs);

	int list_size = one_delay.history.size();
	(*package) & list_size;

	std::list<FixedDelayed<DPoint*>::HistItem>::const_iterator itr = one_delay.history.begin();
	for (; itr != one_delay.history.end(); itr++) {
		FixedDelayed<DPoint*>::HistItem one = (*itr);
		DPoint* value = one.item;
		int value2 = one.observedTime;

		(*package) & (*value);
		(*package) & value2;
	}
}

void PackageUtils::packFixedDelayedDouble(const FixedDelayed<double>& one_delay) {
	//(*package) & (one_delay.delayMS);
	(*package) & (one_delay.reclaimPtrs);


	int list_size = one_delay.history.size();
	(*package) & list_size;

	std::list<FixedDelayed<double>::HistItem>::const_iterator itr = one_delay.history.begin();
	for (; itr != one_delay.history.end(); itr++) {
		FixedDelayed<double>::HistItem one = (*itr);
		double value = one.item;
		int value2 = one.observedTime;

		(*package) & (value);
		(*package) & value2;
	}
}

void PackageUtils::packFixedDelayedInt(const FixedDelayed<int>& one_delay) {
	//(*package) & (one_delay.delayMS);
	(*package) & (one_delay.reclaimPtrs);

	int list_size = one_delay.history.size();
	(*package) & list_size;

	std::list<FixedDelayed<int>::HistItem>::const_iterator itr = one_delay.history.begin();
	for (; itr != one_delay.history.end(); itr++) {
		FixedDelayed<int>::HistItem one = (*itr);
		int value = one.item;
		int value2 = one.observedTime;

		(*package) & (value);
		(*package) & value2;
	}
}

void PackageUtils::packPoint2D(const Point2D& one_point) {
	(*package) & (one_point);
}

//void PackageUtils::packDriverUpdateParams(const DriverUpdateParams& one_driver)
//{
//	(*package) & (one_driver.frameNumber);
//	(*package) & (one_driver.currTimeMS);
//	//(*package) & (one_driver.gen);
//
//	//packLane(one_driver.currLane);
//	Lane::pack(*(this), *(one_driver.currLane));
//
//	(*package) & (one_driver.currLaneIndex);
//	(*package) & (one_driver.fromLaneIndex);
//
//	Lane::pack(*(this), *(one_driver.leftLane));
//	Lane::pack(*(this), *(one_driver.rightLane));
//
////	packLane(one_driver.leftLane);
////	packLane(one_driver.rightLane);
//
//	(*package) & (one_driver.currSpeed);
//
//	(*package) & (one_driver.currLaneOffset);
//	(*package) & (one_driver.currLaneLength);
//	(*package) & (one_driver.isTrafficLightStop);
//	(*package) & (one_driver.trafficSignalStopDistance);
//	(*package) & (one_driver.elapsedSeconds);
//
//	(*package) & (one_driver.perceivedFwdVelocity);
//	(*package) & (one_driver.perceivedLatVelocity);
//
//	(*package) & (one_driver.perceivedFwdVelocityOfFwdCar);
//	(*package) & (one_driver.perceivedLatVelocityOfFwdCar);
//	(*package) & (one_driver.perceivedAccelerationOfFwdCar);
//	(*package) & (one_driver.perceivedDistToFwdCar);
//
//	(*package) & (one_driver.laneChangingVelocity);
//
//	(*package) & (one_driver.isCrossingAhead);
//	(*package) & (one_driver.crossingFwdDistance);
//
//	(*package) & (one_driver.space);
//	(*package) & (one_driver.a_lead);
//	(*package) & (one_driver.v_lead);
//	(*package) & (one_driver.space_star);
//	(*package) & (one_driver.distanceToNormalStop);
//
//	(*package) & (one_driver.dis2stop);
//	(*package) & (one_driver.isWaiting);
//
//	(*package) & (one_driver.justChangedToNewSegment);
//	(*package) & (one_driver.TEMP_lastKnownPolypoint);
//	(*package) & (one_driver.justMovedIntoIntersection);
//	(*package) & (one_driver.overflowIntoIntersection);
//}
//
//void PackageUtils::packPedestrianUpdateParams(const PedestrianUpdateParams& one_pedestrain)
//{
//	(*package) & (one_pedestrain.frameNumber);
//	(*package) & (one_pedestrain.currTimeMS);
//	(*package) & (one_pedestrain.skipThisFrame);
//}

void PackageUtils::packDailyTime(const DailyTime& time)
{
	(*package) & time;
}

void PackageUtils::packDPoint(const DPoint& point)
{
	(*package) & point;
}

void PackageUtils::packDynamicVector(const DynamicVector& vector)
{
	(*package) & vector;
}

}
#endif
