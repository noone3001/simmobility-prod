/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <cmath>
#include <map>

#include "util/LangHelpers.hpp"

#include "Base.hpp"

namespace sim_mob
{

//Forward declarations
class Node;
class Intersection;


namespace aimsun
{

//Forward declarations
class Section;
class Crossing;


///An AIMSUN road intersection or segment intersection.
/// \author Seth N. Hetu
class Node : public Base {
public:
	double xPos;
	double yPos;
	bool isIntersection;

	Node() : Base(),
		xPos(0), yPos(0), isIntersection(false), candidateForSegmentNode(false), generatedNode(nullptr) {}

	int getXPosAsInt() {
		return round(xPos);
	}
	int getYPosAsInt() {
		return round(yPos);
	}

	//Decorated data
	std::vector<Section*> sectionsAtNode;
	std::map<int, std::vector<Crossing*> > crossingsAtNode; //Arranged by laneID
	std::map<Node*, std::vector<int> > crossingLaneIdsByOutgoingNode;
	bool candidateForSegmentNode;

	//Reference to saved object (Maybe be UniNode or MultiNode, of course)
	sim_mob::Node* generatedNode;

};


}
}
