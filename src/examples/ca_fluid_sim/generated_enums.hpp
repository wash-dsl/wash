#pragma once
#include <iostream>
#include <map>
#include "../../wash/enum.h"

namespace wash{
    // These two better enum values should be populated by the source2source translation
    BETTER_ENUM(ScalarForces, int, DENSITY, MASS, SMOOTHING_LENGTH, NEAR_DENSITY)
    BETTER_ENUM(VectorForces, int, VEL, POS, ACC, POSITION, PRESSURE, VISCOSITY)

}

inline std::ostream& operator << (std::ostream& os, const wash::ScalarForces& obj)
{
    os << "ScalarForce(" << obj._to_string() << ")";
    return os;
}

inline std::ostream& operator << (std::ostream& os, const wash::VectorForces& obj)
{
    os << "VectorForce(" << obj._to_string() << ")";
    return os;
}