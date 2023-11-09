
#include "wash_mockapi.hpp"

#define DENSITY_SMOOTH_RAD 20

/*
 Defining kernel function & attributes globals
*/

t_update_kernel update_kernel_ptr;
t_force_kernel force_kernel_ptr;
t_init init_kernel_ptr;

std::vector<std::string> forces_scalar;
std::vector<std::string> forces_vector;
std::vector<Particle> particles;
uint64_t max_iterations;
double influence_radius;

void wash_set_update_kernel(const t_update_kernel update_kernel){
    update_kernel_ptr = update_kernel;
}

void wash_set_force_kernel(const t_force_kernel force_kernel) {
    force_kernel_ptr = force_kernel;
}

void wash_set_init_kernel(const t_init init){
    init_kernel_ptr = init;
}

void wash_set_precision(const std::string precision){
    return;
}

void wash_set_dimensions(const uint8_t dimensions){
    return;
}

void wash_set_max_iterations(const uint64_t iterations) {
    max_iterations = iterations;
}

void wash_add_force(const std::string force) {
    forces_scalar.push_back(force);
}

void wash_add_force(const std::string force, const uint8_t dim) {
    switch (dim) {
        case 1:
        wash_add_force(force);
        break;
        case 2:
        forces_vector.push_back(force);
        break;
        default:
        printf("too many dimension vector force\n");
    }
}

void wash_add_par(const Particle p) {
    particles.push_back(p);
}

void wash_set_influence_radius(const double radius) {
    influence_radius = radius;
}

// p and q don't change during this method, so can be marked as const
double wash_eucdist(const Particle& p, const Particle& q) {
    wash::Vec2D pos = p.wash_get_pos() - q.wash_get_pos();
    return sqrt(pos.magnitude());
}

Particle::Particle (const wash::Vec2D pos, const double density) {
    this->pos = pos;
    this->density = density;

    for (std::string& force : forces_scalar) {
        this->force_scalars[force] = 0.0;
    }

    for (std::string& force : forces_vector) {
        this->force_vectors[force] = wash::Vec2D();
    }
}

void* Particle::wash_get_force(const std::string& force) const {
    return nullptr;
}

double Particle::wash_get_force_scalar(const std::string& force) const {
    // need to check whether force exists and otherwise return default value
    return this->force_scalars.at(force);
}

wash::Vec2D Particle::wash_get_force_vector(const std::string& force) const {
    return this->force_vectors.at(force);
}

void Particle::wash_set_force(const std::string& force, void* value) {
    return;
}

void Particle::wash_set_force_scalar(const std::string& force, const double value) {
    this->force_scalars[force] = value;
}

void Particle::wash_set_force_vector(const std::string& force, const wash::Vec2D value) {
    this->force_vectors[force] = value;
}

wash::Vec2D Particle::wash_get_pos() const  {
    return this->pos;
}

void Particle::wash_set_pos(const wash::Vec2D pos) {
    this->pos = pos;
}

wash::Vec2D Particle::wash_get_vel() const {
    return this->vel;
}

void Particle::wash_set_vel(const wash::Vec2D vel) {
    this->vel = vel;
}

wash::Vec2D Particle::wash_get_acc() const  {
    return this->acc;
}

void Particle::wash_set_acc(const wash::Vec2D acc) {
    this->acc = acc;
}

double Particle::wash_get_density() const  {
    return this->density;
}

void Particle::wash_set_density(const double density) {
    this->density = density;
}

void Particle::wash_set_mass(const double mass) {
    this->mass = mass;
}

double Particle::wash_get_mass() const  {
    return this->mass;
}

double density_smoothing(const double radius, const double dist) {
    // TODO: This function
    return 0;
}

void density_kernel(Particle& p, const std::vector<Particle>& neighbors) {
    double newDensity = 0;
    for (auto& q : neighbors) {
        double dist = wash_eucdist(p, q);
        newDensity += q.wash_get_mass() * density_smoothing(DENSITY_SMOOTH_RAD, dist);
    }
    p.wash_set_density(newDensity);
}

void wash_start() {
    std::cout << "INIT" << std::endl;
    init_kernel_ptr();

    for (uint64_t iter = 0; iter < max_iterations; iter++) {
        std::cout << "Iteration " << iter << std::endl;
        
        // Compute densities
        // TODO: Work out whether or not this is worth including in the loop below
        // (this would help readability, but might hurt performance)
        size_t i = 0;
        for (auto p : particles) {
            std::vector<Particle> neighbors;
            for (auto q : particles) {
                if (wash_eucdist(p, q) <= influence_radius) {
                    neighbors.push_back(q);
                }
            }
            std::cout << "FORCE particle " << i++ << " with " << neighbors.size() << " neighbors" << std::endl;
            density_kernel(p, neighbors);
        }

        // Compute forces
        i = 0;
        for (auto& p : particles) {
            std::vector<Particle> neighbors;
            for (auto& q : particles) {
                if (wash_eucdist(p, q) <= influence_radius) {
                    neighbors.push_back(q);
                }
            }
            std::cout << "FORCE particle " << i++ << " with " << neighbors.size() << " neighbors" << std::endl;
            force_kernel_ptr(p, neighbors);
        }

        // Update the positions (and derivatives) of each particle
        i = 0;
        for (auto& p : particles) {
            std::cout << "UPDATE particle " << i++ << std::endl;
            update_kernel_ptr(p);
        }
    }
}   

