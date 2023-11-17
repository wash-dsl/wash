
.. _program_listing_file_wash_mockapi.cpp:

Program Listing for File wash_mockapi.cpp
=========================================

|exhale_lsh| :ref:`Return to documentation for file <file_wash_mockapi.cpp>` (``wash_mockapi.cpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   
   #include "wash_mockapi.hpp"
   
   #define DENSITY_SMOOTH_RAD 20
   
   /*
    Defining kernel function & attributes globals
   */
   
   // todo: separate particle out into its own hpp class
   
   namespace wash {
       // Define these inside the namespace so we can refer to them with wash::func_name
   
       t_update_kernel update_kernel_ptr;
       t_force_kernel force_kernel_ptr;
       t_init init_kernel_ptr;
   
       std::vector<std::string> forces_scalar;
       std::vector<std::string> forces_vector;
       std::vector<Particle> particles;
       uint64_t max_iterations;
       double influence_radius;
   
       void set_update_kernel(const t_update_kernel update_kernel) { update_kernel_ptr = update_kernel; }
   
       void set_force_kernel(const t_force_kernel force_kernel) { force_kernel_ptr = force_kernel; }
   
       void set_init_kernel(const t_init init) { init_kernel_ptr = init; }
   
       void set_precision(const std::string precision) { return; }
   
       void set_dimensions(const uint8_t dimensions) { return; }
   
       void set_max_iterations(const uint64_t iterations) { max_iterations = iterations; }
   
       void add_force(const std::string force) { forces_scalar.push_back(force); }
   
       void add_force(const std::string force, const uint8_t dim) {
           switch (dim) {
           case 1:
               add_force(std::move(force));
               break;
           case 2:
               forces_vector.push_back(std::move(force));
               break;
           default:
               printf("too many dimension vector force\n");
           }
       }
   
       void add_par(const Particle p) { particles.push_back(p); }
   
       void set_influence_radius(const double radius) { influence_radius = radius; }
   
       double get_influence_radius() { return influence_radius; }
   
       // p and q don't change during this method, so can be marked as const
       double eucdist(const Particle& p, const Particle& q) {
           Vec2D pos = p.get_pos() - q.get_pos();
           return sqrt(pos.magnitude());
       }
   
       Particle::Particle(const Vec2D pos, const double density) {
           this->pos = pos;
           this->density = density;
   
           for (std::string& force : forces_scalar) {
               this->force_scalars[force] = 0.0;
           }
   
           for (std::string& force : forces_vector) {
               this->force_vectors[force] = Vec2D();
           }
       }
   
       void* Particle::get_force(const std::string& force) const { return nullptr; }
   
       double Particle::get_force_scalar(const std::string& force) const {
           // need to check whether force exists and otherwise return default value
           return this->force_scalars.at(force);
       }
   
       Vec2D Particle::get_force_vector(const std::string& force) const { return this->force_vectors.at(force); }
   
       void Particle::set_force(const std::string& force, void* value) { return; }
   
       void Particle::set_force_scalar(const std::string& force, const double value) {
           this->force_scalars[force] = value;
       }
   
       void Particle::set_force_vector(const std::string& force, const Vec2D value) { this->force_vectors[force] = value; }
   
       Vec2D Particle::get_pos() const { return this->pos; }
   
       void Particle::set_pos(const Vec2D pos) { this->pos = pos; }
   
       Vec2D Particle::get_vel() const { return this->vel; }
   
       void Particle::set_vel(const Vec2D vel) { this->vel = vel; }
   
       Vec2D Particle::get_acc() const { return this->acc; }
   
       void Particle::set_acc(const Vec2D acc) { this->acc = acc; }
   
       double Particle::get_density() const { return this->density; }
   
       void Particle::set_density(const double density) { this->density = density; }
   
       void Particle::set_mass(const double mass) { this->mass = mass; }
   
       double Particle::get_mass() const { return this->mass; }
   
       double density_smoothing(const double radius, const double dist) {
           // TODO: This function
           return 0;
       }
   
       void density_kernel(Particle& p, const std::vector<Particle>& neighbors) {
           double newDensity = 0;
           for (auto& q : neighbors) {
               const double dist = eucdist(p, q);
               newDensity += q.get_mass() * density_smoothing(DENSITY_SMOOTH_RAD, dist);
           }
           p.set_density(newDensity);
       }
   
       void start() {
           std::cout << "INIT" << std::endl;
           init_kernel_ptr();
   
           for (uint64_t iter = 0; iter < max_iterations; iter++) {
               std::cout << "Iteration " << iter << std::endl;
   
               // Compute densities
               // TODO: Work out whether or not this is worth including in the loop below
               // (this would help readability, but might hurt performance)
               size_t i = 0;
               for (auto& p : particles) {
                   std::vector<Particle> neighbors;
                   for (auto& q : particles) {
                       if (eucdist(p, q) <= influence_radius) {
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
                       if (eucdist(p, q) <= influence_radius) {
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
   
   }  // namespace wash
