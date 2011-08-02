/*
 * Driver.cpp
 *
 *  Created on: 2011-7-5
 *      Author: wangxy
 */


#include "Driver.hpp"
#include "../entities/Signal.hpp"
#include <math.h>

using namespace sim_mob;
using std::numeric_limits;
using std::max;

double DS[] = {0.6, 0.5, 0.4, 0.7};
double DS_av = (DS[0]+DS[1]+DS[2]+DS[3])/4;

//Some static properties require initialization in the CPP file. ~Seth
const double sim_mob::Driver::maxLaneSpeed[] = {120,140,180};
//const double sim_mob::Driver::lane[] = {310,320,330};
const double sim_mob::Driver::MAX_NUM = numeric_limits<double>::max();
//const double sim_mob::Driver::laneWidth = 10;

const double sim_mob::Driver::CF_parameters[2][6] = {
//        alpha   beta    gama    lambda  rho     stddev
		{ 0.0400, 0.7220, 0.2420, 0.6820, 0.6000, 0.8250},
		{-0.0418, 0.0000, 0.1510, 0.6840, 0.6800, 0.8020}
};

const double sim_mob::Driver::GA_parameters[4][9] = {
//	    scale alpha lambda beta0  beta1  beta2  beta3  beta4  stddev
		{ 1.00, 0.0, 0.000, 0.508, 0.000, 0.000,-0.420, 0.000, 0.488},	//Discretionary,lead
		{ 1.00, 0.0, 0.000, 2.020, 0.000, 0.000, 0.153, 0.188, 0.526},	//Discretionary,lag
		{ 1.00, 0.0, 0.000, 0.384, 0.000, 0.000, 0.000, 0.000, 0.859},	//Mandatory,lead
		{ 1.00, 0.0, 0.000, 0.587, 0.000, 0.000, 0.048, 0.356, 1.073}	//Mandatory,lag
};
const double sim_mob::Driver::MLC_parameters[5] = {
		1320.0,		//feet, lower bound
	    5280.0,		//feet, delta
		   0.5,		//coef for number of lanes
		   1.0,		//coef for congestion level
		   1.0		//minimum time in lane
};

//for unit conversion
double feet2Unit(double feet)
{
	return feet*0.158;
}
double unit2Feet(double unit)
{
	return unit/0.158;
}

const badArea sim_mob::Driver::badareas[] = {
		//{200,350,0},
		//{750,800,1},
		//{200,350,2}
};

const link_ sim_mob::Driver::testLinks[] = {
		{ 0,   0, 270, 460, 270,10,3},
		{ 1, 530,   0, 530, 260,10,3},
		{ 2,1000, 330, 540, 330,10,3},
		{ 3, 470, 600, 470, 340,10,3},
		{ 4, 460, 330,   0, 330,10,3},
		{ 5, 470, 260, 470,   0,10,3},
		{ 6, 540, 270,1000, 270,10,3},
		{ 7, 530, 340, 530, 600,10,3}
};

//initiate
sim_mob::Driver::Driver(Agent* parent) : Role(parent), leader(nullptr), sig(0)
{
	//Set random seed
	//NOTE: This will reset the sequence returned by rand(); it's not a good idea.
	//      I moved srand() initialization into main.cpp; we'll need to make our own
	//      random data management classes later.
	//srand(parent->getId());

	//Set default speed in the range of 1m/s to 1.4m/s
	speed_ = 1+((double)(rand()%10))/10;
	//speed up
	speed_ *= 16;

	//Set default data for acceleration
	acc_ = 0;
	maxAcceleration = MAX_ACCELERATION;
	normalDeceleration = -maxAcceleration*0.6;
	maxDeceleration = -maxAcceleration;

	//Other basic parameters
	xPos = 0;
	yPos = 0;
	xVel = 0;
	yVel = 0;
	xAcc = 0;
	yAcc = 0;

	//assume that all the car has the same size
	length=12;
	width=8;

	targetSpeed=speed_*2;
	satisfiedDistance=100;
	avoidBadAreaDistance=1.5*satisfiedDistance;
	dis2stop=MAX_NUM;

	timeStep=0.1;			//assume that time step is constant
	isGoalSet = false;
	isOriginSet = false;
	LF=nullptr;LB=nullptr;RF=nullptr;RB=nullptr;

	//Need to init
	currentLink = (parent->getId()/3)%4;
	nextLane=-1;
	nextLink=-1;

	ischanging=false;
	isback=false;
	isWaiting=false;
	fromLane=getLane();
	toLane=getLane();
	changeDecision=0;


	currPhase = 0;
	phaseCounter = 0;
	sig.initializeSignal();
	angle = 0;
	inIntersection=false;

}


//Main update functionality
void sim_mob::Driver::update(frame_t frameNumber)
{
	getFromParent();
	updateCurrentLink();
	abs2relat();
	updateCurrentLane();

	//Set the goal of agent
	if(!isGoalSet){
		setGoal();
		isGoalSet = true;
	}
	//Set the origin of agent
	if(!isOriginSet){
		setOrigin();
		isOriginSet=true;
	}

	//if reach the goal, get back to the origin
	if(isGoalReached()){
		currentLink=originLink;
		//currentLink=(currentLink+1)%8;
		//double fallback=0;
		xPos_=origin.xPos;
		yPos_=origin.yPos;
		//isGoalSet=false;
		relat2abs();
		setToParent();
		return;
	}

	//To check if the vehicle reaches the lane it wants to move to
	if(getLane()==toLane){
		ischanging=false;
		isWaiting=false;
		isback=false;
		fromLane=toLane=getLane();
		changeDecision=0;
	}

	//update signal information
	updateSignalInfo();



	VelOfLaneChanging=	testLinks[currentLink].laneWidth/5;		//assume that 5 time steps is need when changing lane

	//update information
	updateLeadingDriver();

	//accelerating part
	makeAcceleratingDecision();


	//lane changing part
	if(!isInTheIntersection() && xPos_ > 50){
		excuteLaneChanging();
	}

	//update
	updateAcceleration();
	updateVelocity();
	updatePosition();

	modifyPosition();

	if(!isInTheIntersection()){
		relat2abs();
	}

	setToParent();

	updateAngle();

	boost::mutex::scoped_lock local_lock(BufferedBase::global_mutex);
	std::cout <<"(" <<parent->getId() <<"," <<frameNumber<<","<<parent->xPos.get()<<"," <<parent->yPos.get() <<","<<sig.getcurrPhase()<<","<<DS_av<<","<<floor(sig.getnextCL())<<","<<sig.getphaseCounter()<<","<<angle <<")" <<std::endl;
}

void sim_mob::Driver::getFromParent()
{
	xPos=parent->xPos.get();
	yPos=parent->yPos.get();
	xVel=parent->xVel.get();
	yVel=parent->yVel.get();
	xAcc=parent->xAcc.get();
	yAcc=parent->yAcc.get();
}

void sim_mob::Driver::setToParent()
{
	parent->xPos.set(xPos);
	parent->yPos.set(yPos);
	parent->xVel.set(xVel);
	parent->yVel.set(yVel);
	parent->xAcc.set(xAcc);
	parent->yAcc.set(yAcc);
	parent->currentLink.set(currentLink);
}

void sim_mob::Driver::abs2relat()
{
	double xDir=testLinks[currentLink].endX-testLinks[currentLink].startX;
	double yDir=testLinks[currentLink].endY-testLinks[currentLink].startY;
	double xOffset=xPos-testLinks[currentLink].startX;
	double yOffset=yPos-testLinks[currentLink].startY;
	double magnitude=sqrt(xDir*xDir+yDir*yDir);
	xDirection=xDir/magnitude;
	yDirection=yDir/magnitude;
	xPos_= xOffset*xDirection+yOffset*yDirection;
	yPos_=-xOffset*yDirection+yOffset*xDirection;
	xVel_= xVel*xDirection+yVel*yDirection;
	yVel_=-xVel*yDirection+yVel*xDirection;
	xAcc_= xAcc*xDirection+yAcc*yDirection;
	yAcc_=-xAcc*yDirection+yAcc*xDirection;
}

void sim_mob::Driver::relat2abs()
{
	double xDir=testLinks[currentLink].endX-testLinks[currentLink].startX;
	double yDir=testLinks[currentLink].endY-testLinks[currentLink].startY;
	double magnitude=sqrt(xDir*xDir+yDir*yDir);
	xDirection=xDir/magnitude;
	yDirection=yDir/magnitude;
	xPos=xPos_*xDirection-yPos_*yDirection+testLinks[currentLink].startX;
	yPos=xPos_*yDirection+yPos_*xDirection+testLinks[currentLink].startY;
	xVel=xVel_*xDirection-yVel_*yDirection;
	yVel=xVel_*yDirection+yVel_*xDirection;
	xAcc=xAcc_*xDirection-yAcc_*yDirection;
	yAcc=xAcc_*yDirection+yAcc_*xDirection;
}

void sim_mob::Driver::setOrigin()
{
	originLink = currentLink;
	origin.xPos = 0;
	origin.yPos = yPos_;
}

void sim_mob::Driver::setGoal()
{
	if(currentLink%2==0) {
		goal.xPos = 460;			//all the cars move in x direction to reach the goal
	} else{
		goal.xPos = 260;
	}
}

bool sim_mob::Driver::isGoalReached()
{
	return (xPos < 0 || xPos > 1000 || yPos < 0 || yPos >600);
}

bool sim_mob::Driver::isReachSignal()
{
	return (!isInTheIntersection() && getLinkLength()-xPos_ < length
			&& currentLink<4);
}

void sim_mob::Driver::updateCurrentLink()
{
	for(int i=0;i<numOfLinks;i++){
		if(isOnTheLink(i)){
			currentLink=i;
		}
	}
}

bool sim_mob::Driver::isInTheIntersection()
{
	return (parent->xPos.get()>460 && parent->xPos.get() < 540
				&& parent->yPos.get() > 260 && parent->yPos.get() < 340);
}

void sim_mob::Driver::updateCurrentLane()
{
	if(!isInTheIntersection()){
		currentLane=getLane();
	}
}

bool sim_mob::Driver::isOnTheLink(int linkid)
{
	double xDir=testLinks[linkid].endX-testLinks[linkid].startX;
	double yDir=testLinks[linkid].endY-testLinks[linkid].startY;
	double xOffset=xPos-testLinks[linkid].startX;
	double yOffset=yPos-testLinks[linkid].startY;
	double magnitude=sqrt(xDir*xDir+yDir*yDir);
	double xD=xDir/magnitude;
	double yD=yDir/magnitude;
	double xP= xOffset*xD+yOffset*yD;
	double yP=-xOffset*yD+yOffset*xD;
	if(xP>=0 && xP <= magnitude && yP >= 0
			&& yP <=testLinks[linkid].laneWidth*((double)testLinks[linkid].laneNum-1)){
			return true;
	} else {
		return false;
	}
}

void sim_mob::Driver::updateAcceleration()
{
	//Set actual acceleration
	xAcc_ = acc_;
	yAcc_ = 0;//yDirection*acc_;
}


void sim_mob::Driver::updateVelocity()
{
	if(isInTheIntersection()) {
		IntersectionVelocityUpdate();
	} else {
		if(isReachSignal()){
			nextLink=(currentLane+currentLink+1)%4+4;
			nextLane=currentLane;
			if(!reachSignalDecision()){
				xVel_ = 0 ; yVel_ =0;
			} else {
			xVel_= targetSpeed/2;yVel_=0;
			}
		} else if(leader && leader_xVel_ == 0 && getDistance()<0.2*length) {
			xVel_=0;
			yVel_=0;
		} else { //when vehicle just gets back to the origin, help them to speed up
			if(xPos_<0) {
				xVel_=(0.2+((double)(rand()%10))/30)*getTargetSpeed();
				yVel_=0;
			} else{
				xVel_ = max(0.0, speed_+xAcc_*timeStep);
				yVel_ = 0;//max(0.0, yDirection*speed_+yAcc*timeStep);
			}

			if(!ischanging){
				double foward;
				if(!leader) {
					foward=MAX_NUM;
				} else {
					foward=leader_xPos_-xPos_-length;
				}
				if(foward<0) {
					xVel_=0;
				}
				yVel_=0;
			}
		}
	}
	speed_=sqrt(xVel_*xVel_+yVel_*yVel_);
}


void sim_mob::Driver::updatePosition()
{
	if(isInTheIntersection()){
		xPos = xPos + xVel*timeStep;
		yPos = yPos + yVel*timeStep;
	} else if(xVel_!=0) { //Only update if velocity is non-zero.
		xPos_ = xPos_+xVel_*timeStep+0.5*xAcc_*timeStep*timeStep;
	}
}


void sim_mob::Driver::updateLeadingDriver()
{
	/*
	 * In fact the so-called big brother can return the leading driver.
	 * Since now there is no such big brother, so I assume that each driver can know information of other vehicles.
	 * It will find it's leading vehicle itself.
	 * */
	const Agent* other = nullptr;
	double leadingDistance=MAX_NUM;
	size_t leadingID=Agent::all_agents.size();
	for (size_t i=0; i<Agent::all_agents.size(); i++) {
		//Skip self
		other = Agent::all_agents[i];
		if (other->getId()==parent->getId()
				||other->currentLink.get()!=currentLink)
		{
			continue;
		}

		double other_xOffset	= other->xPos.get()	- testLinks[currentLink].startX;
		double other_yOffset	= other->yPos.get()	- testLinks[currentLink].startY;
		double other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
		double other_yPos_		=-other_xOffset	* yDirection	+ other_yOffset	* xDirection;

		//Check. Search all the vehicle it may get crashed and find the closest one ahead.
		if(other_yPos_ < yPos_+width && other_yPos_ > yPos_-width){
			double tmpLeadingDistance=other_xPos_-xPos_;
			if(tmpLeadingDistance<leadingDistance && tmpLeadingDistance >0)	{
				leadingDistance=tmpLeadingDistance;
				leadingID=i;
			}
		}
	}

	if(leadingID == Agent::all_agents.size()) {
		leader=nullptr;
	} else {
		leader=Agent::all_agents[leadingID];
		double xOffset	= leader->xPos.get()	- testLinks[currentLink].startX;
		double yOffset	= leader->yPos.get()	- testLinks[currentLink].startY;
		leader_xPos_	= xOffset	* xDirection	+ yOffset	* yDirection;
		leader_yPos_	=-xOffset	* yDirection	+ yOffset	* xDirection;
		leader_xVel_	= leader->xVel.get()	* xDirection	+ leader->yVel.get()	* yDirection;
		leader_yVel_	=-leader->xVel.get()	* yDirection	+ leader->yVel.get()	* xDirection;
		leader_xAcc_	= leader->xAcc.get()	* xDirection	+ leader->yAcc.get()	* yDirection;
		leader_yAcc_	=-leader->xAcc.get()	* yDirection	+ leader->yAcc.get()	* xDirection;
	}
}

int sim_mob::Driver::getLane()
{
	//This function will be part of the big brother, so this is just a simple temporary function for test.
	//Later, it may be replaced by function of the big brother.
	for (int i=0;i<3;i++){
		if(yPos_==testLinks[currentLink].laneWidth*i) {
			return i;
		}
	}
	return -1;
}

double sim_mob::Driver::getLinkLength()
{
	double dx=testLinks[currentLink].endX-testLinks[currentLink].startX;
	double dy=testLinks[currentLink].endY-testLinks[currentLink].startY;
	return sqrt(dx*dx+dy*dy);
}

int sim_mob::Driver::checkIfBadAreaAhead()
{
	double disToBA=MAX_NUM;
	int BA=-1;
	// First, find the closest bad area ahead that the vehicle may knock on
	// If there is no such bad area, BA will be -1. Else, BA will be location of the bad area in the array.
	for(int i=0;i<numOfBadAreas;i++){
		if(
			yPos_ > testLinks[currentLink].laneWidth*badareas[i].lane-testLinks[currentLink].laneWidth/2-width/2
			&& yPos_ < testLinks[currentLink].laneWidth*badareas[i].lane+testLinks[currentLink].laneWidth/2+width/2
			&& badareas[i].startX > xPos_+length/2
		){
			if(badareas[i].startX - xPos_-length < disToBA)
				{disToBA=badareas[i].startX - xPos_-length/2;BA=i;}
		}
	}

	/*
	 * If the subject vehicle has no leading vehicle, it will return BA+1, the sequence number of it in the array
	 * 0 and -1 means no such bad area.
	 * If it has a leading vehicle and such bad area ahead, the distance to the leading vehicle and to the bad area.
	 * If the former one is smaller, return -(BA+2); else return (BA+1).
	 * So that return which is not 0 or -1 means there is bad area ahead,
	 *   while the positive return means bad area is closer and the sequence number will be return-1
	 *   and negative return means leading vehicle is closer and the sequence number will be -return-2.
	 * */
	if(!leader){
		return BA+1;
	} else {
		if(BA==-1) {
			return -1;
		} else {
			double temp1=badareas[BA].startX-xPos_-length/2;
			double temp2=leader_xPos_-xPos_-length;
			return (temp1<temp2)?(BA+1):-(BA+2);
		}
	}
}

double sim_mob::Driver::getDistance()
{
	int BA=checkIfBadAreaAhead();
	if( BA > 0 ) { 		//There is bad area ahead and no leading vehicle ahead.
		return badareas[BA-1].startX-xPos_-length/2;
	} else if( BA < 0 ) { 	//Leading vehicle is closer(maybe there is no bad area ahead)
		return max(0.001,leader_xPos_-xPos_-length);
	} else {				//Nothing ahead, just go as fast as it can.
		return MAX_NUM;
	}
}

void sim_mob::Driver::makeAcceleratingDecision()
{
	space = getDistance();
	int i=checkIfBadAreaAhead();
	if(space <= 0) {
		acc_=0;
	} else {
		if (speed_ == 0)headway = 2 * space * 100000;
		else headway = space / speed_;
		if( i > 0 ){
			v_lead		=	0;
			a_lead		=	0;
			space_star	=	0;
		} else if( i == 0 ) {
			v_lead		=	MAX_NUM;
			a_lead		=	MAX_NUM;
			space_star	=	MAX_NUM;
		} else {
			v_lead 		=	leader_xVel_;
			a_lead		=	leader_xAcc_;
			double dt	=	timeStep;
			space_star	=	space + v_lead * dt + 0.5 * a_lead * dt * dt;
		}
		if(headway < hBufferLower) {
			acc_ = accOfEmergencyDecelerating();
		}
		if(headway > hBufferUpper) {
			acc_ = accOfMixOfCFandFF();
		}
		if(headway <= hBufferUpper && headway >= hBufferLower) {
			acc_ = accOfCarFollowing();
		}
	}
}

double sim_mob::Driver::breakToTargetSpeed()
{
	double v 			=	xVel_;
	double dt			=	timeStep;
	if( space_star > FLT_EPSILON) {
		return (( v_lead + a_lead * dt ) * ( v_lead + a_lead * dt) - v * v) / 2 / space_star;
	} else if ( dt <= 0 ) {
		return MAX_ACCELERATION;
	} else {
		return ( v_lead + a_lead * dt - v ) / dt;
	}
}

double sim_mob::Driver::accOfEmergencyDecelerating()
{
	double v 			=	xVel_;
	double dv			=	v-v_lead;
	double epsilon_v	=	0.001;
	double aNormalDec	=	getNormalDeceleration();

	if( dv < epsilon_v ) {
		return a_lead + 0.25*aNormalDec;
	} else if ( space > 0.01 ) {
		return a_lead - dv * dv / 2 / space;
	} else {
		return breakToTargetSpeed();
	}
}

double uRandom()
{
	srand(time(0));
	long int seed_=rand();
	const long int M = 2147483647;  // M = modulus (2^31)
	const long int A = 48271;       // A = multiplier (was 16807)
	const long int Q = M / A;
	const long int R = M % A;
	seed_ = A * (seed_ % Q) - R * (seed_ / Q);
	seed_ = (seed_ > 0) ? (seed_) : (seed_ + M);
	return (double)seed_ / (double)M;
}

double nRandom(double mean,double stddev)
{
	   double r1 = uRandom(), r2 = uRandom();
	   double r = - 2.0 * log(r1);
	   if (r > 0.0) return (mean + stddev * sqrt(r) * sin(2 * 3.1415926 * r2));
	   else return (mean);
}

double sim_mob::Driver::accOfCarFollowing()
{
	const double density	=	1;		//represent the density of vehicles in front of the subject vehicle
										//now we ignore it, assuming that it is 1.
	double v				=	xVel_;
	int i = (v > v_lead) ? 1 : 0;
	double dv =(v > v_lead)?(v-v_lead):(v_lead - v);
	double acc_ = CF_parameters[i][0] * pow(v , CF_parameters[i][1]) /pow(space , CF_parameters[i][2]);
	acc_ *= pow(dv , CF_parameters[i][3])*pow(density,CF_parameters[i][4]);
	acc_ += feet2Unit(nRandom(0,CF_parameters[i][5]));
	return acc_;
}

double sim_mob::Driver::accOfFreeFlowing()
{
	double vn			=	speed_;
	double acc_;
	if ( vn < getTargetSpeed()) {
		if( vn < maxLaneSpeed[getLane()]) {
			acc_=getMaxAcceleration();
		} else {
			acc_ = getNormalDeceleration();
		}
	}
	if ( vn > getTargetSpeed()) {
		acc_ = getNormalDeceleration();
	}
	if ( vn == getTargetSpeed()) {
		if( vn < maxLaneSpeed[getLane()]) {
			acc_=getMaxAcceleration();
		} else {
			acc_ = 0;
		}
	}
	return acc_;
}

double sim_mob::Driver::accOfMixOfCFandFF()		//mix of car following and free flowing
{
	distanceToNormalStop = speed_ * speed_ / 2 /getNormalDeceleration();
	if( space > distanceToNormalStop ) {
		return accOfFreeFlowing();
	} else {
		return breakToTargetSpeed();
	}
}


Agent* sim_mob::Driver::getNextForBDriver(bool isLeft,bool isFront)
{
	int border;			//the sequence number of border lane
	double offset;		//to get the adjacent lane, yPos should minus the offset

	if(isLeft) {
		border = 0;
		offset = testLinks[currentLink].laneWidth;
	} else{
		border = 2;
		offset = -testLinks[currentLink].laneWidth;
	}

	double NFBDistance;
	if(isFront) {
		NFBDistance=MAX_NUM;
	} else {
		NFBDistance=-MAX_NUM;
	}

	size_t NFBID=Agent::all_agents.size();
	if(getLane()==border) {
		return nullptr;		//has no left side or right side
	} else {
		const Agent* other = nullptr;
		for (size_t i=0; i<Agent::all_agents.size(); i++) {
			//Skip self
			other = Agent::all_agents[i];
			if (other->getId()==parent->getId()
					||other->currentLink.get()!=currentLink) {
				//other = nullptr;
				continue;
			}
			//Check. Find the closest one
			double other_xOffset	= other->xPos.get()	- testLinks[currentLink].startX;
			double other_yOffset	= other->yPos.get()	- testLinks[currentLink].startY;
			double other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
			double other_yPos_		=-other_xOffset	* yDirection	+ other_yOffset	* xDirection;

			if(other_yPos_ == yPos_-offset){		//now it just searches vehicles on the lane
				double forward=other_xPos_-xPos_;
				if(
						(isFront && forward>0 && forward < NFBDistance)||
						((!isFront) && forward<0 && forward > NFBDistance)
						) {
					NFBDistance=forward;NFBID=i;
				}
			}
		}
	}

	if(NFBID == Agent::all_agents.size()) {
		return nullptr;
	} else {
		return Agent::all_agents[NFBID];
	}
}


int sim_mob::Driver::findClosestBadAreaAhead(int lane)
{
	double disToBA=MAX_NUM;
	int BA=-1;
	for(int j=0;j<numOfBadAreas;j++){
		if( lane==badareas[j].lane
				&& badareas[j].startX > xPos_ + length / 2
				&& badareas[j].startX - xPos_ - length / 2 < disToBA	){
			BA = j;
			disToBA = badareas[j].startX - xPos_ - length / 2;
			}
	}
	return BA;
}


double sim_mob::Driver::lcCriticalGap(int type,	double dis_, double spd_, double dv_)
{
	double k=( type < 2 ) ? 1 : 5;
	return k*-dv_*timeStep;

	//The code below is from MITSIMLab. While the calculation result not suit for current unit.
	//So now, I just put them here.
	/*double dis=unit2Feet(dis_);
	double spd=unit2Feet(spd_);
	double dv=unit2Feet(dv_);
	double rem_dist_impact = (type < 3) ?
		0.0 : (1.0 - 1.0 / (1 + exp(GA_parameters[type][2] * dis)));
	double dvNegative = (dv < 0) ? dv : 0.0;
	double dvPositive = (dv > 0) ? dv : 0.0;
	double gap = GA_parameters[type][3] + GA_parameters[type][4] * rem_dist_impact +
			GA_parameters[type][5] * dv + GA_parameters[type][6] * dvNegative + GA_parameters[type][7] *  dvPositive;
	double u = gap + nRandom(0, GA_parameters[type][8]);
	double cri_gap ;

	if (u < -4.0) {
		cri_gap = 0.0183 * GA_parameters[type][0] ;		// exp(-4)=0.0183
	}
	else if (u > 6.0) {
		cri_gap = 403.4 * GA_parameters[type][0] ;   	// exp(6)=403.4
	}
	else cri_gap = GA_parameters[type][0] * exp(u) ;

	if (cri_gap < GA_parameters[type][1]) return feet2Unit(GA_parameters[type][1]) ;
	else return feet2Unit(cri_gap) ;*/
}


unsigned int sim_mob::Driver::gapAcceptance(int type)
{
	int border[2]={0,2};
	int offset[2]={-1,1};
	bool badarea[2]={false,false};
	//check if there is bad area on the left/right side
	for(int i=0;i<numOfBadAreas;i++){
		for(int j=0;j<2;j++){
			if(getLane()+j*2-1==badareas[i].lane
				&& xPos_-length/2 < badareas[i].endX
				&& xPos_+length/2 > badareas[i].startX
				)badarea[j]=true;
		}
	}

	//[0:left,1:right][0:lead,1:lag]
	double otherSpeed[2][2];		//the speed of the closest vehicle in adjacent lane
	double otherDistance[2][2];		//the distance to the closest vehicle in adjacent lane

	double other_xOffset;
	double other_yOffset;
	double other_xPos_;
	double other_xVel_;

	LF=getNextForBDriver(true,true);
	LB=getNextForBDriver(true,false);
	RF=getNextForBDriver(false,true);
	RB=getNextForBDriver(false,false);
	Agent* F;
	Agent* B;
	for(int i=0;i<2;i++){
		if(i==0){
			F=LF;
			B=LB;
		} else {
			F=RF;
			B=RB;
		}
		if(getLane()!=border[i] && badarea[i]!=true){	//the left/right side exists
		int BA=findClosestBadAreaAhead(getLane()+offset[i]);
			if(!F) {		//no vehicle ahead
				if(BA==-1) {		//also no bad area ahead
					otherSpeed[i][0]=MAX_NUM;
					otherDistance[i][0]=MAX_NUM;
				} else {			//bad area ahead, check the gap between subject vehicle and bad area
					otherSpeed[i][0]=0;
					otherDistance[i][0]=badareas[BA].startX-xPos_-length/2;
				}
			} else {				//has vehicle ahead
				other_xOffset	= F->xPos.get()	- testLinks[currentLink].startX;
				other_yOffset	= F->yPos.get()	- testLinks[currentLink].startY;
				other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
				other_xVel_		= F->xVel.get()	* xDirection	+ F->yVel.get()	* yDirection;

				double gna1=other_xPos_-xPos_-length;
				if(BA==-1) {		//no bad area, just check the gap
					otherSpeed[i][0]=other_xVel_;
					otherDistance[i][0]=gna1;
				} else {			//bad area ahead, use the smallest gap to compute
					double gna2=badareas[BA].startX-xPos_-length/2;
					otherSpeed[i][0]=( gna1 < gna2 )?other_xVel_:0;
					otherDistance[i][0]=( gna1 < gna2 )?gna1:gna2;
				}
			}

			if(B) {		//has vehicle behind, check the gap
				other_xOffset	= B->xPos.get()	- testLinks[currentLink].startX;
				other_yOffset	= B->yPos.get()	- testLinks[currentLink].startY;
				other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
				other_xVel_		= B->xVel.get()	* xDirection	+ B->yVel.get()	* yDirection;

				otherSpeed[i][1]=other_xVel_;
				otherDistance[i][1]=xPos_-other_xPos_-length;
			} else {		//no vehicle behind
				otherSpeed[i][1]=-MAX_NUM;
				otherDistance[i][1]=MAX_NUM;
			}
		} else {			// no left/right side exists
			otherSpeed[i][0]=-MAX_NUM;
			otherDistance[i][0]=0;
			otherSpeed[i][1]=MAX_NUM;
			otherDistance[i][1]=0;
		}
	}

	//[0:left,1:right][0:lead,1:lag]
	bool flags[2][2];
	for(int i=0;i<2;i++){	//i for left / right
		for(int j=0;j<2;j++){	//j for lead / lag
			double v 	= ( j == 0 ) ? xVel_ : otherSpeed[i][1];
			double dv 	= ( j == 0 ) ? otherSpeed[i][0] - xVel_ : xVel_-otherSpeed[i][1];
			flags[i][j]= otherDistance[i][j] > lcCriticalGap(j+type,dis2stop,v,dv);
		}
	}

	//Build up a return value.
	unsigned int returnVal = 0;
	if ( flags[0][0] && flags[0][1] ) {
		returnVal |= LSIDE_LEFT;
	}
	if ( flags[1][0] && flags[1][1] ) {
		returnVal |= LSIDE_RIGHT;
	}

	return returnVal;
}

double sim_mob::Driver::calcSideLaneUtility(bool isLeft){
	int border = (isLeft) ? 0 : 2;
	int offset = (isLeft) ? -1 : 1;
	Agent* F = (isLeft) ? LF : RF;
	double other_xOffset;
	double other_yOffset;
	double other_xPos_;
	if(getLane()==border) {
		return -MAX_NUM;	//has no left/right side
	} else {
		int BA=findClosestBadAreaAhead(getLane()+offset);
		if(BA==-1) {			//no bad area ahead
			if(F)	{	//has vehicle ahead
				other_xOffset	= F->xPos.get()	- testLinks[currentLink].startX;
				other_yOffset	= F->yPos.get()	- testLinks[currentLink].startY;
				other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
				//other_xPos_=F->xPos.get();
				return other_xPos_-xPos_-length;
			} else {			//no vehicle ahead neither
				return MAX_NUM;
			}
		} else if(badareas[BA].startX-xPos_-length < avoidBadAreaDistance) {
			return -MAX_NUM;	//bad area is to close, won't choose that side
		} else {				//has bad area but is not too close
			double slr1=badareas[BA].startX-xPos_-length/2;
			if(F) {	//has vehicle ahead
				other_xOffset	= F->xPos.get()	- testLinks[currentLink].startX;
				other_yOffset	= F->yPos.get()	- testLinks[currentLink].startY;
				other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
				//other_xPos_=F->xPos.get();
				double slr2=other_xPos_-xPos_-length;
				return ( slr1 < slr2 ) ? slr1 : slr2;		//check the smaller gap
			} else {
				return slr1;	//has no vehicle ahead,just check the gap to the bad area
			}
		}
	}
}

double sim_mob::Driver::makeDiscretionaryLaneChangingDecision()
{
	// for available gaps(including current gap between leading vehicle and itself), vehicle will choose the longest
	unsigned int freeLanes = gapAcceptance(DLC);
	bool freeLeft = ((freeLanes&LSIDE_LEFT)!=0);
	bool freeRight = ((freeLanes&LSIDE_RIGHT)!=0);

	if(!freeLeft && !freeRight)return 0;		//neither gap is available, stay in current lane

	double s=getDistance();
	if(s>satisfiedDistance)return 0;	// space ahead is satisfying, stay in current lane

	//calculate the utility of both sides
	double leftUtility = calcSideLaneUtility(true);
	double rightUtility = calcSideLaneUtility(false);

	//to check if their utilities are greater than current lane
	bool left = ( s < leftUtility );
	bool right = ( s < rightUtility );

	//decide
	if(freeRight && !freeLeft && right) {
		return 1;
	}
	if(freeLeft && !freeRight && left) {
		return -1;
	}
	if(freeLeft && freeRight){
		if(right && left){
			return (leftUtility < rightUtility) ? 1 : -1;	//both side is available, choose the better one
		}
		if(right && !left) {
			return 1;
		}
		if(!right && left) {
			return -1;
		}
		if(!right && !left) {
			return 0;
		}
	}
	return 0;
}

double sim_mob::Driver::checkIfMandatory()
{
	int BA=findClosestBadAreaAhead(getLane());
	if( BA==-1 ){
		dis2stop=MAX_NUM;
		return 0;	//no bad area ahead, do not have to MLC
	}
	else{
		dis2stop=badareas[BA].startX-xPos_-length/2;
		//we first use simple rule to decide lane changing mode
		return (dis2stop < satisfiedDistance) ? 1 : 0;  //This is equivalent to the if statement below. ~Seth
		/*if(dis2stop < satisfiedDistance) {
			return 1;
		} else {
			return 0;
		}*/

		//the code below is MITSIMLab model
		/*double num		=	1;		//now we just assume that MLC only need to change to the adjacent lane
		double y		=	0.5;		//segment density/jam density, now assume that it is 0.5
		double delta0	=	feet2Unit(MLC_parameters[0]);
		double dis		=	dis2stop - delta0;
		double delta	=	1.0 + MLC_parameters[2] * num + MLC_parameters[3] * y;
		delta *= MLC_parameters[1];
		return exp(-dis * dis / (delta * delta));*/
	}
}

double sim_mob::Driver::makeMandatoryLaneChangingDecision()
{
	unsigned int freeLanes = gapAcceptance(MLC);
	bool freeLeft = ((freeLanes&LSIDE_LEFT)!=0);
	bool freeRight = ((freeLanes&LSIDE_RIGHT)!=0);

	//calculate the utility of both sides
	double leftUtility = calcSideLaneUtility(true);
	double rightUtility = calcSideLaneUtility(false);

	/*
	 * In fact, in most cases, vehicle has its desired lane to achieve.
	 * This parts of the functions will be made in later updates.
	 * Now we just compare utilities of the 2 adjacent lanes and choose the bigger one.
	 * */

	//decide
	if(freeRight && !freeLeft) {
		isWaiting=false;
		return 1;
	} else if(freeLeft && !freeRight) {
		isWaiting=false;
		return -1;
	} else if(freeLeft && freeRight) {
		isWaiting=false;
		return (leftUtility < rightUtility) ? 1 : -1;
	} else {			//when neither side is available,vehicle will decelerate to wait a proper gap.
		isWaiting=true;
		return 0;
	}
}

/*
 * In MITSIMLab, vehicle change lane in 1 time step.
 * While in sim mobility, vehicle approach the target lane in limited speed.
 * So the behavior when changing lane is quite different from MITSIMLab.
 * New behavior and model need to be discussed.
 * Here is just a simple model, which now can function.
 *
 * -wangxy
 * */
void sim_mob::Driver::excuteLaneChanging()
{
	// when vehicle is on the lane, make decision
	if(!ischanging){

		if(getLinkLength()-xPos_ < 50){
			return;			//when close to link end, do not change lane
		}
		//check if MLC is needed(vehicle has probability=checkIfMandatory() to be tagged in to MLC mode)
		double p=(double)(rand()%1000)/1000;
		if(p<checkIfMandatory()){
			changeMode = MLC;
		} else {
			changeMode = DLC;
			dis2stop=MAX_NUM;		//no crucial point ahead
		}

		//make decision depending on current lane changing mode
		if(changeMode==DLC) {
			changeDecision=makeDiscretionaryLaneChangingDecision();
		} else {
			changeDecision=makeMandatoryLaneChangingDecision();
		}

		//set original lane and target lane
		fromLane=getLane();
		toLane=getLane()+changeDecision;
	}
	if(isWaiting){			//when waiting proper gap
		ischanging=true;		//vehicle is actually changing lane
		changeDecision=makeMandatoryLaneChangingDecision();		//to make decision again
		fromLane=getLane();
		toLane=getLane()+changeDecision;
		acc_=MAX_DECELERATION;	//decelerate in max rate to wait for acceptable gap
	} else{			//when changing lane
		if(changeDecision!=0) {
			double change=1;		//for MLC

			if(checkForCrash() ) {	//if get crashed
				if(changeMode==DLC && !isback){		// in DLC mode
					//exchange the leaving lane and target lane to get back
					int tmp;
					tmp=fromLane;
					fromLane=toLane;
					toLane=tmp;
					changeDecision=-changeDecision;
					isback=true;
				}
				if(changeMode==MLC){	//in MLC mode
					change=0;			//do not change in this time step
					acc_=MAX_DECELERATION;	//decelerate to let other vehicle go
				}
			}
			ischanging=true;
			yPos_ += changeDecision*VelOfLaneChanging*change;	//change y position according to decision
		}
	}
}

bool sim_mob::Driver::checkIfOnTheLane(double y)
{
	for(int i=0;i<3;i++){
		if(y==testLinks[currentLink].laneWidth*i){
			return true;
		}
	}
	return false;
}

/*
 * When vehicles change lane slowly, crash may happen.
 * To avoid such crash, vehicle should be able to detect it.
 * While here are some rules that some crash will be ignored.
 * More discussion is needed.
 *
 * -wangxy
 * */
bool sim_mob::Driver::checkForCrash()
{
	// now, only vehicles changing lane check if crash may happen
	if(!ischanging) {
		return false;
	}
	//check other vehicles
	const Agent* other = nullptr;
	for (size_t i=0; i<Agent::all_agents.size(); i++) {
		//Skip self
		other = Agent::all_agents[i];
		if (other->getId()==parent->getId()
				||other->currentLink.get()!=currentLink) {
			//other = NULL;
			continue;
		}
		double other_xOffset	= other->xPos.get()	- testLinks[currentLink].startX;
		double other_yOffset	= other->yPos.get()	- testLinks[currentLink].startY;
		double other_xPos_		= other_xOffset	* xDirection	+ other_yOffset	* yDirection;
		double other_yPos_		=-other_xOffset	* yDirection	+ other_yOffset	* xDirection;

		//Check. When other vehicle is too close to subject vehicle, crash will happen
		if(
				(other_yPos_ < yPos_+width*1.1)
				&& (other_yPos_ > yPos_-width*1.1)
				&& (other_xPos_ < xPos_+length*0.9)
				&& (other_xPos_ > xPos_-length*0.9)
				)
		{
			//but some kind of crash can be ignored. Cases below must be regard as crash.
			if(
					((fromLane>toLane && other_yPos_ < yPos_)
				||(fromLane<toLane && other_yPos_ > yPos_))
						)
			{		//the other vehicle is on the position where subject vehicle is approaching
				if(checkIfOnTheLane(other_yPos_)){		//if other vehicle is on the lane
					return true;								//subject vehicle should avoid it
				} else if(other_xPos_>xPos_){	//if both vehicle is changing lane
					return true;		//one has bigger ID will not be affected
				} else {
					return false;
				}
			} else{
				return false;
			}
		}
	}
	//check bad areas
	for(int i=0;i<numOfBadAreas;i++) {
		if(
				badareas[i].startX < xPos_-length
				&& badareas[i].endX > xPos_+length
				&& testLinks[currentLink].laneWidth*badareas[i].lane + testLinks[currentLink].laneWidth/2+width/2 > yPos_
				&& testLinks[currentLink].laneWidth*badareas[i].lane - testLinks[currentLink].laneWidth/2-width/2 < yPos_
		) {
			return true;
		}
	}
	return false;
}

void sim_mob :: Driver :: updateSignalInfo()
{
	//update signal information
	sig.updateSignal(DS);

	//currPhase is the order for drivers
	currPhase = sig.getcurrPhase();

}

//Angle shows the velocity direction of vehicles
void sim_mob::Driver::updateAngle()
{
	if(xVel==0 && yVel==0){}
    else if(xVel>=0 && yVel>=0)angle = 360 - atan(yVel/xVel)/3.1415926*180;
	else if(xVel>=0 && yVel<0)angle = - atan(yVel/xVel)/3.1415926*180;
	else if(xVel<0 && yVel>=0)angle = 180 - atan(yVel/xVel)/3.1415926*180;
	else if(xVel<0 && yVel<0)angle = 180 - atan(yVel/xVel)/3.1415926*180;
	else{}
}



bool sim_mob :: Driver :: reachSignalDecision()
{
	//NOTE: I replaced the if statements with simple "returns", which are more concise. ~Seth

	//Phase A, drivers on Lane 0 and Lane 1 of Link 0 and Link 2 can move forward
	if(currPhase == 0 || currPhase == 10){
		return (currentLink == 0 && currentLane == 0) || (currentLink == 0 && currentLane == 1)|| (currentLink == 2 && currentLane == 0) || (currentLink == 2 && currentLane == 1);
	}

	//Phase B, drivers on Lane 2 of Link 0 and Link 2 can move forward
	else if(currPhase == 1 || currPhase == 11){
		return (currentLink == 0 && currentLane == 2) || (currentLink == 2 && currentLane == 2);
	}

	//Phase C, drivers on Lane 0 and Lane 1 of Link 1 and Link 3 can move forward
	else if(currPhase == 2 || currPhase == 12){
		return (currentLink == 1 && currentLane == 0) || (currentLink == 1 && currentLane == 1)|| (currentLink == 3 && currentLane == 0) || (currentLink == 3 && currentLane == 1);
	}

	//Phase D, drivers on Lane 2 of Link 1 and Link 3 can move forward
	else if(currPhase == 3 || currPhase == 13){
		return (currentLink == 1 && currentLane == 2) || (currentLink == 3 && currentLane == 2);
	}

	else {
		//Does this represent an error? ~Seth
		return true;
	}
}

void sim_mob :: Driver :: IntersectionVelocityUpdate()
{
	double speed = 36;

	switch (currentLane) {
		case 1:
			//vehicles that are going to go straight
			if( currentLink==0) {
				xVel = speed;yVel = 0;
			} else if( currentLink==1) {
				xVel = 0;yVel = speed;
			} else if( currentLink==2) {
				xVel = -speed;yVel = 0;
			} else if( currentLink==3) {
				xVel = 0;yVel = -speed;
			}
			break;

		case 2:
			//vehicles that are going to turn right, their routes are based on circles
			if( currentLink==0) {
				//the center of circle are (450,350)
				double xD = 450- parent->xPos.get();
				double yD = 350 - parent->yPos.get();
				double magnitude = sqrt(xD*xD + yD*yD);

				double xDirection = yD/magnitude;
				double yDirection = - xD/magnitude;

				xVel = xDirection*speed*0.7;
				yVel = yDirection*speed*0.7;
			} else if( currentLink==1) {
				//the center of circle are (450,250)
				double xD = 450 - parent->xPos.get();
				double yD = 250 - parent->yPos.get();
				double magnitude = sqrt(xD*xD + yD*yD);

				double xDirection = yD/magnitude;
				double yDirection = - xD/magnitude;

				xVel = xDirection*speed*0.7;
				yVel = yDirection*speed*0.7;
			} else if( currentLink==2) {
				//the center of circle are (550,250)
				double xD = 550 - parent->xPos.get();
				double yD = 250 - parent->yPos.get();
				double magnitude = sqrt(xD*xD + yD*yD);

				double xDirection =  yD/magnitude;
				double yDirection =  - xD/magnitude;

				xVel = xDirection*speed*0.7;
				yVel = yDirection*speed*0.7;
			} else if( currentLink==3) {
				//the center of circle are (550,350)
				double xD = 550- parent->xPos.get();
				double yD = 350 - parent->yPos.get();
				double magnitude = sqrt(xD*xD + yD*yD);

				double xDirection = yD/magnitude;
				double yDirection = - xD/magnitude;

				xVel = xDirection*speed*0.7;
				yVel = yDirection*speed*0.7;
			}
			break;

		case 0:
			//Vehicles that are going to turn left
			if(currentLink==0) {
				xVel = 0.5*speed;yVel = -0.5*speed;
			} else if(currentLink==1) {
				xVel = 0.5*speed;yVel = 0.5*speed;
			} else if(currentLink==2) {
				xVel = -0.5*speed;yVel = 0.5*speed;
			} else if(currentLink==3) {
				xVel = -0.5*speed;yVel = -0.5*speed;
			}
			break;

		default:
			//Does this represent an error condition? ~Seth
			return;
	}
}

//modify vehicles' positions when they just finishing crossing intersection,
//make sure they are on one of the 3 lanes inside link.
void sim_mob::Driver::modifyPosition()
{
	//it is in the intersection last frame, but not in the intersection now
	if(!isInTheIntersection()) {
		if(inIntersection) {
			currentLink=nextLink;
			currentLane=nextLane;
			xPos_=5;
			yPos_=currentLane*testLinks[currentLink].laneWidth;
			xVel_=targetSpeed/2;
			yVel_=0;
			xAcc_=0;
			yAcc_=0;
			nextLane=-1;
			nextLink=-1;
			inIntersection=false;
		}
	} else {
		inIntersection=true;
	}
}
