#include <iostream>
#include <map>
#include "../../wash/enum.h"

namespace wash{
    // These two better enum values should be populated by the source2source translation
    BETTER_ENUM(ScalarForces, int, DENSITY, MASS, SMOOTHING_LENGTH, NEAR_DENSITY, MAX)
    BETTER_ENUM(VectorForces, int, VELOCITY, ACCELERATION, PRESSURE, VISCOSITY, MAX)

    void load_forces(){
        for (ScalarForces forceVal : ScalarForces::_values()){
            std::cout << "Adding: " << forceVal << std::endl;
        }
        for (VectorForces forceVal : VectorForces::_values()){
            std::cout << "Adding: " << forceVal << std::endl;
        }
    }

}

std::ostream& operator << (std::ostream& os, const wash::ScalarForces& obj)
{
    os << "ScalarForce(" << obj._to_string() << ")";
    return os;
}

std::ostream& operator << (std::ostream& os, const wash::VectorForces& obj)
{
    os << "VectorForce(" << obj._to_string() << ")";
    return os;
}