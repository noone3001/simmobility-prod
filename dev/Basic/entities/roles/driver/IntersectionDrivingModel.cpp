/* Copyright Singapore-MIT Alliance for Research and Technology */


#include "IntersectionDrivingModel.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

using namespace sim_mob;

void IntersectionDrivingModel::pack(PackageUtils& package, const SimpleIntDrivingModel* params) {
	if (params == NULL) {
		bool is_NULL = true;
		package.packBasicData(is_NULL);
		return;
	} else {
		bool is_NULL = false;
		package.packBasicData(is_NULL);
	}

	package.packDynamicVector(params->intTrajectory);
	package.packBasicData<double>(params->totalMovement);
	}

void IntersectionDrivingModel::unpack(UnPackageUtils& unpackage, SimpleIntDrivingModel* params) {
	bool is_NULL = unpackage.unpackBasicData<bool> ();
	if (is_NULL) {
		return NULL;
	}

	unpackage.unpackDynamicVector(params->intTrajectory);
	params->totalMovement = unpackage.unpackBasicData<double>();
}
