#pragma once

#include <iostream>
#include <unordered_map>
#include <string>
#include <list>
#include <random>

typedef std::pair<double, double> Vector2D;

typedef void (*t_update_kernel)(Particle&);
typedef void (*t_force_kernel)(Particle&, std::list<Particle>&);
typedef void (*t_init)();

class Particle {
private:
    Vector2D pos;
    Vector2D vel;
    Vector2D acc;
    double density;
    std::unordered_map<std::string, void*> forcesv;
public:
    Particle(Vector2D pos, double density) : pos(pos), density(density) {};

    // Return the force value
    void* wash_get_force(std::string& force);

    double wash_get_force_scalar(char* force);
    Vector2D wash_get_force_vector(char* force);

    // Set the force value
    void wash_set_force(std::string& force, void* value);

    void wash_set_force_scalar(char* force, double value);
    Vector2D wash_set_force_vector(char* force, Vector2D value);

    Vector2D wash_get_pos();
    void wash_set_pos(Vector2D pos);

    Vector2D wash_get_vel();
    void wash_set_vel(Vector2D vel);

    Vector2D wash_get_acc();
    void wash_set_acc(Vector2D acc);

    double wash_get_density();
    void wash_set_density(double density);
};

/*
 Simulation parameter setup functions. 
 These define the parameters of the simulation
*/

/*
 Set precision to double/float 
 In the non-DSL version hardcode to double?
*/
void wash_set_precision(char* precision);

/*
 Set radius of nearest neighbour particles which
 influence other particles. 

 Expected to be constant over the simulation
*/
void wash_set_influence_radius(double radius); 

/*
 Set the number of dimensions of the particle class
 (e.g. size of pos vector, etc.)

 In the non-DSL version hardcode to 2?
*/
void wash_set_dimensions(uint8_t dimensions);

/*
 Register a force with the DSL which will be present
 on particle object. Will be a scalar `precision` type.
*/
void wash_add_force(char* force);

// Similarly, but set to be a vector `precision` type size `dim`
void wash_add_force(char* force, uint8_t dim);

/*
 Add a particle to the simulation
*/
void wash_add_par(Particle p);

/*
 Compute the euclidean distance
*/
double wash_eucdist(Particle& p, Particle& q);

/*
 Register the init kernel function
*/
void wash_set_init_kernel(t_init init);

/*
 Register the force update kernel
*/
void wash_set_force_kernel(t_force_kernel force_kernel);

/*
 Register the particle position update kernel
*/
void wash_set_update_kernel(t_update_kernel update_kernel);

/*
 Start Simulation
*/

void start();

/*
 Defining kernel function & attributes globals
*/

t_update_kernel update_kernel_ptr;
t_force_kernel force_kernel_ptr;
t_init init_kernel_ptr;

std::vector<Particle> particles;
