//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "entities/roles/driver/DriverFacets.hpp"
#include "entities/roles/driver/driverCommunication/DriverComm.hpp"

namespace sim_mob
{

class DriverCommMovement : public DriverMovement
{
public:
    explicit DriverCommMovement();
    virtual ~DriverCommMovement();

    //Virtual overrides
    virtual void frame_init();
};
}
