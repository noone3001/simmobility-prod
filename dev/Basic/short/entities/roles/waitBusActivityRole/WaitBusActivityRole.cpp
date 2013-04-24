#include "WaitBusActivityRole.hpp"

#include "entities/Person.hpp"
#include "entities/vehicle/Bus.hpp"

using std::vector;
using namespace sim_mob;

BusStop* getbusStop(const Node* node,sim_mob::RoadSegment* segment)
{
 	 std::map<centimeter_t, const RoadItem*>::const_iterator ob_it;
 	 const std::map<centimeter_t, const RoadItem*> & obstacles =segment->obstacles;
 	 for (ob_it = obstacles.begin(); ob_it != obstacles.end(); ++ob_it) {
 		RoadItem* ri = const_cast<RoadItem*>(ob_it->second);
 		BusStop *bs = dynamic_cast<BusStop*>(ri);
 		if (bs && ((segment->getStart() == node) || (segment->getEnd() == node) )) {
 			return bs;
 		}
 	 }

 	 return nullptr;
}

sim_mob::WaitBusActivityRole::WaitBusActivityRole(Agent* parent, std::string buslineid, std::string roleName) :
		Role(parent,  roleName), params(parent->getGenerator()), remainingTime(0),
		busStopAgent(nullptr), registered(false), TimeOfReachingBusStop(0), buslineid(buslineid) {
	boarding_Frame = 0;
	busDriver = nullptr;
	nextRole = nullptr;
}

sim_mob::WaitBusActivityRole::~WaitBusActivityRole() {

}

Role* sim_mob::WaitBusActivityRole::clone(Person* parent) const
{
	return new WaitBusActivityRole(parent);
}

void sim_mob::WaitBusActivityRole::frame_init(UpdateParams& p) {
	sim_mob::BusStop* busStop = setBusStopPos(parent->originNode);
	busStopAgent = busStop->generatedBusStopAgent;
	parent->xPos.set(busStop->xPos);
	parent->yPos.set(busStop->yPos);
	TimeOfReachingBusStop = p.now.ms();
	buslineid = "7_2";// hardcoded now, later change
	initializeRemainingTime();
}

void sim_mob::WaitBusActivityRole::frame_tick(UpdateParams& p) {
	if(0!=boarding_Frame) {
		//updateBoardingTime();
		if(boarding_Frame == p.now.frame()) {
			parent->setToBeRemoved();
			boarding_Frame = 0;
			Person* person = dynamic_cast<Person*> (parent);
			if(person) {
				person->setTempRoleFlag(false);
			}
		}
	}
//	updateRemainingTime();
//	if(remainingTime <= 0){
//		parent->setToBeRemoved();
//	}
//	else
//	{
////		if(roleFlag) {
////			if(roleFlag->getRoleName() == "passenger") {
////				std::cout << "WaitBusActivity has a passenger flag! " << std::endl;
////			}
////		}
//		if(p.now.frame() % 3 == 0 && (!registered)) {
//
//		}
//	}
}

void sim_mob::WaitBusActivityRole::frame_tick_output(const UpdateParams& p) {
	if (ConfigParams::GetInstance().is_run_on_many_computers) {
		return;
	}

	//Reset our offset if it's set to zero
	if (DisplayOffset.getX()==0 && DisplayOffset.getY()==0) {
	   boost::mt19937 gen(static_cast<unsigned int>(parent->getId()*parent->getId()));
	   boost::uniform_int<> distX(0, 249);
	   boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varX(gen, distX);
	   boost::uniform_int<> distY(0, 99);
	   boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varY(gen, distY);
	   unsigned int value = (unsigned int)varX();
	   DisplayOffset.setX(value+1);
	   value= (unsigned int)varY();
	   DisplayOffset.setY(value+1);
	}

//	LogOut("(\"WaitBusActivity\""
//			<<","<<p.now.frame()
//			<<","<<parent->getId()
//			<<",{"
//			<<"\"xPos\":\""<<static_cast<int>(parent->xPos)
//			<<"\",\"yPos\":\""<<static_cast<int>(parent->yPos)
//			<<"\"})"<<std::endl);
//	LogOut("(\"Activity\""
//				<<","<<p.now.frame()
//				<<","<<parent->getId()
//				<<",{"
//				<<"\"xPos\":\""<<static_cast<int>(parent->xPos)
//				<<"\",\"yPos\":\""<<static_cast<int>(parent->yPos)
//				<<"\"})"<<std::endl);
	LogOut("("<<"\"passenger\","<<p.now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<(parent->xPos.get()+DisplayOffset.getX()+DisplayOffset.getX())<<"\"," <<"\"yPos\":\""<<(parent->yPos.get()+DisplayOffset.getY()+DisplayOffset.getY())<<"\",})"<<std::endl);
}

void sim_mob::WaitBusActivityRole::frame_tick_output_mpi(timeslice now) {

}

UpdateParams& sim_mob::WaitBusActivityRole::make_frame_tick_params(timeslice now) {
	params.reset(now);
	return params;
}

std::vector<sim_mob::BufferedBase*> sim_mob::WaitBusActivityRole::getSubscriptionParams() {
	vector<BufferedBase*> res;
	return res;
}

void sim_mob::WaitBusActivityRole::initializeRemainingTime()
{
//	remainingTime = activityEndTime.offsetMS_From(ConfigParams::GetInstance().simStartTime)
//			- activityStartTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);
	remainingTime = 3000000;
}

void sim_mob::WaitBusActivityRole::updateRemainingTime()
{
	remainingTime = std::max(0, remainingTime - int(ConfigParams::GetInstance().baseGranMS));
}

void sim_mob::WaitBusActivityRole::updateBoardingTime()
{
	//boarding_Time = std::max(0, boarding_Time - int(ConfigParams::GetInstance().baseGranMS));
}

BusStop* sim_mob::WaitBusActivityRole::setBusStopPos(const Node* node)//to find the nearest busstop to a node
{
 	 const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (node);
 	 double dist=0;
 	 BusStop*bs1;
 	 if(currEndNode)
 	 {
 		 const std::set<sim_mob::RoadSegment*>& segments_ = currEndNode->getRoadSegments();
 		 BusStop* busStop_ptr = nullptr;
 		 for(std::set<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i !=segments_.end();i++)
 		 {
 			sim_mob::BusStop* bustop_ = (*i)->getBusStop();
 			busStop_ptr = getbusStop(node,(*i));
 			if(busStop_ptr)
 			{
 			double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,node->location.getX(), node->location.getY());
 			if((newDist<dist || dist==0)&& busStop_ptr->BusLines.size()!=0)
 			   {
 			     dist=newDist;
 				 bs1=busStop_ptr;
 			   }
 			}
 		 }
 	 }
 	 else
 	 {
 		 Point2D point = node->location;
 		 const StreetDirectory::LaneAndIndexPair lane_index =  StreetDirectory::instance().getLane(point);
 		 if(lane_index.lane_)
 		 {
 			 sim_mob::Link* link_= lane_index.lane_->getRoadSegment()->getLink();
 			 const sim_mob::Link* link_2 = StreetDirectory::instance().searchLink(link_->getEnd(),link_->getStart());
 			 BusStop* busStop_ptr = nullptr;

 			 std::vector<sim_mob::RoadSegment*> segments_ ;

 			 if(link_)
 			 {
 				 segments_= const_cast<Link*>(link_)->getSegments();
 				 for(std::vector<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i != segments_.end();i++)
 				 {
 					sim_mob::BusStop* bustop_ = (*i)->getBusStop();
 					busStop_ptr = getbusStop(node,(*i));
 					if(busStop_ptr)
 					{
 						double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,point.getX(), point.getY());
 						if((newDist<dist || dist==0)&& busStop_ptr->BusLines.size()!=0)
 						 {
 						 	dist=newDist;
 						 	bs1=busStop_ptr;
 						 }
 					}
 				 }
 			 }

 			 if(link_2)
 			 {
 				 segments_ = const_cast<Link*>(link_2)->getSegments();
 				 for(std::vector<sim_mob::RoadSegment*>::const_iterator i = segments_.begin();i != segments_.end();i++)
 				 {
 					sim_mob::BusStop* bustop_ = (*i)->getBusStop();
 					busStop_ptr = getbusStop(node,(*i));
 					if(busStop_ptr)
 					{
 						double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos,point.getX(), point.getY());
 						if((newDist<dist || dist==0)&& busStop_ptr->BusLines.size()!=0)
 					    {
 						   dist=newDist;
 						   bs1=busStop_ptr;
 						 }
 					}
 				 }
 			 }
 		 }

 	 }

 	 return bs1;
}
