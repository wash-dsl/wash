#include <iostream>

// Mass, density, position & derivatives are implicity
wash_setprecision("double")
wash_setinfluenceradius(50f)
wash_setdimensions(2)
wash_addforce("Temp")
// Can set dimensions with second argument
wash_addforce("Concentration", 1)

using std::pair<double, double> = Vector2D;

// Can implement helper functions for common particle use cases (later)
void init(){
    size_t numParticles = 10000;
    for (size_t i = 0; i < numParticles; i++){
        double xpos = getRand(0f, 1f);
        double ypos = getRand(0f, 1f);
        Particle p(Vec2d(xpos, ypos), 0.01, Vec2d(0,0))
        
        wash_addpar(p);
    }
}

// 
void force_kernel(Particle p, std::iterator<Particle> neighbours){
    std::pair<double, double> pressureForce; 
    
    for (Particle q : neighbours) {
        double dist = wash_eucdist(p, q); // magnitude of 2D distance
        Vector2D dir = (p.wash_getpos() - q.wash_getpos()) / dist;
        double slope = user_smoothing_derivative(smoothRad, dist);
        // N.B. a particle density (& vol) will be recalculated by us
        pressureForce += -q.wash_getforce("pressure") * q.wash_getforce("vol") * dir * slope;
    }
    
    p.wash_setforce(forces::PRESSURE, pressureForce);
}

void update_kernel(Particle p){
    // acceleration, velocity, position
    p.wash_setacc(p.wash_getforce("pressure") / p.wash_getdensity());
    p.wash_setvel(p.wash_getvel() + p.wash_getacc() * DELTA_TIME);
    p.wash_setpos(enforce_in_bounds(p.position + p.wash_getvel() * DELTA_TIME));
}

wash_setinitkernel(init)
wash_setforcekernel(force_kernel)
wash_setupdatekernel(update_kernel)

// pickup forceKernel and updateKernel. Then our system would call these kernels for each particle
