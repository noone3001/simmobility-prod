/*
 * StatusOfWorldDao.cpp
 *
 *  Created on: Nov 13, 2015
 *      Author: gishara
 */
#include "SimulationStoppedPointDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

SimulationStoppedPointDao::SimulationStoppedPointDao(DB_Connection& connection): SqlAbstractDao<SimulationStoppedPoint>(connection, DB_TABLE_SIM_STOPPED_POINT, EMPTY_STR,EMPTY_STR, EMPTY_STR, EMPTY_STR, EMPTY_STR) {
}

SimulationStoppedPointDao::~SimulationStoppedPointDao() {
}

void SimulationStoppedPointDao::fromRow(Row& result, SimulationStoppedPoint& outObj)
{
	outObj.simVersionId = result.get<BigSerial>("sim_version_id", INVALID_ID);
	outObj.postcode = result.get<BigSerial>("postcode", INVALID_ID);
	outObj.buildingId = result.get<BigSerial>("building_id", INVALID_ID);
	outObj.unitId = result.get<BigSerial>("unit_id",INVALID_ID);
	outObj.projectId = result.get<BigSerial>("project_id",INVALID_ID);
}

void SimulationStoppedPointDao::toRow(SimulationStoppedPoint& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getSimVersionId());
	outParams.push_back(data.getPostcode());
	outParams.push_back(data.getBuildingId());
	outParams.push_back(data.getUnitId());
	outParams.push_back(data.getProjectId());
}

void SimulationStoppedPointDao::insertSimulationStoppedPoints(SimulationStoppedPoint& simulationStoppedPoint,std::string schema)
{

	const std::string DB_INSERT_SIM_STOPPED_POINT = "INSERT INTO " + APPLY_SCHEMA(schema, ".simulation_stopped_point")
	        		+ " (" + "sim_version_id" + ", " + "postcode" + ", " + "building_id"+ ", " + "unit_id" + ", " + "project_id"
	        		+ ") VALUES (:v1, :v2, :v3, :v4, :v5)";
	insertViaQuery(simulationStoppedPoint,DB_INSERT_SIM_STOPPED_POINT);

}



