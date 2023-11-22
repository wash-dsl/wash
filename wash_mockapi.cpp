
#include "wash_mockapi.hpp"

#define DENSITY_SMOOTH_RAD 1

/*
 Defining kernel function & attributes globals
*/

// todo: separate particle out into its own hpp class

namespace wash {
    // Define these inside the namespace so we can refer to them with wash::func_name

    std::string simulation_name = "serial_test";
    std::string output_file_name = "ca";

    t_update_kernel update_kernel_ptr = nullptr;
    t_force_kernel force_kernel_ptr = nullptr;
    t_init init_kernel_ptr = nullptr;
    t_force_kernel density_kernel_ptr = nullptr;

    std::vector<std::string> forces_scalar;
    std::vector<std::string> forces_vector;
    std::vector<Particle> particles;
    uint64_t max_iterations;
    double influence_radius;

    void set_update_kernel(const t_update_kernel update_kernel) { update_kernel_ptr = update_kernel; }

    void set_force_kernel(const t_force_kernel force_kernel) { force_kernel_ptr = force_kernel; }

    void set_density_kernel(const t_force_kernel density_kernel) { density_kernel_ptr = density_kernel; }

    void set_init_kernel(const t_init init) { init_kernel_ptr = init; }

    void set_precision(const std::string precision) { return; }

    void set_dimensions(const uint8_t dimensions) { return; }

    void set_max_iterations(const uint64_t iterations) { max_iterations = iterations; }

    void set_simulation_name(const std::string name) { simulation_name = name; }

    void set_output_file_name(const std::string name) { output_file_name = name; }

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

    Particle::Particle(const Vec2D pos, const double mass) {
        this->pos = pos;
        this->mass = mass;
        this->density = mass;

        this->density = 0.0;
        this->vel = Vec2D({0.0, 0.0});
        this->acc = Vec2D({0.0, 0.0});

        this->force_scalars = std::unordered_map<std::string, double>({});
        this->force_vectors = std::unordered_map<std::string, wash::Vec2D>({});

        for (std::string& force : forces_scalar) {
            this->force_scalars[force] = 0.0;
        }

        for (std::string& force : forces_vector) {
            this->force_vectors[force] = Vec2D({0.0, 0.0});
        }
    }

    void Particle::init_force_scalar(const std::string& force) { this->force_scalars[force] = 0.0; }

    void Particle::init_force_vector(const std::string& force) { this->force_vectors[force] = wash::Vec2D({0.0, 0.0}); }

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

    double Particle::get_vol() const { return this->mass / this->density; }

    double density_smoothing(const double radius, const double dist) {
        double vol = 3.141592654 * pow(radius, 8) / 4;
        double value = std::max(0.0, radius * radius - dist * dist);
        return value * value * value / vol;
    }

    void density_kernel(Particle& p, const std::vector<Particle>& neighbors) {
        double newDensity = p.get_mass();
        for (auto& q : neighbors) {
            const double dist = eucdist(p, q);
            newDensity += q.get_mass() * density_smoothing(DENSITY_SMOOTH_RAD, dist);
        }
        p.set_density(newDensity);
    }

    void start() {
        std::cout << "INIT" << std::endl;

        std::string output_path = "./" + simulation_name + std::string("/") + output_file_name;

        std::cout << "Output Path " << output_path << std::endl;

        // auto awriter = get_file_writer("ascii");
        auto hwriter = get_file_writer("hdf5");

        // // Add forces to each particle's force map
        // for (Particle& p : particles) {
        //     for (std::string& force_name : forces_scalar) {
        //         p.init_force_scalar(force_name);
        //     }

        //     for (std::string& force_name : forces_vector) {
        //         p.init_force_vector(force_name);
        //     }
        // }

        t_force_kernel d_kernel = density_kernel_ptr;
        if (d_kernel == nullptr) {
            d_kernel = &density_kernel;
        }

        init_kernel_ptr();

        for (uint64_t iter = 0; iter < max_iterations; iter++) {
            std::cout << "Iteration " << iter << std::endl;

            // Compute densities
            // this has to be done before force kernel for e.g. fluid sim
            // since the force might be dependent on the other particles densities.
            size_t i = 0;
            for (auto& p : particles) {
                std::vector<Particle> neighbors;
                for (auto& q : particles) {
                    if (eucdist(p, q) <= influence_radius && &p != &q) {
                        neighbors.push_back(q);
                    }
                }
                // std::cout << "DENSITY particle " << i++ << " with " << neighbors.size() << " neighbors" << std::endl;
                d_kernel(p, neighbors);
            }
            

            // Compute forces
            //size_t i = 0;
            i = 0;
            for (auto& p : particles) {
                std::vector<Particle> neighbors;
                for (auto& q : particles) {
                    if (eucdist(p, q) <= influence_radius && &p != &q) {
                        neighbors.push_back(q);
                    }
                }
                // std::cout << "DENSITY particle " << i++ << " with " << neighbors.size() << " neighbors" << std::endl;
                // d_kernel(p, neighbors);

                // std::cout << "FORCE particle " << i << " with " << neighbors.size() << " neighbors"; //<< std::endl;
                force_kernel_ptr(p, neighbors);
                // std::cout << " px=" << *p.get_force_vector("pressure")[0] << " py=" <<
                // *p.get_force_vector("pressure")[1] << std::endl;
            }

            // Update the positions (and derivatives) of each particle
            i = 0;
            for (auto& p : particles) {
                // std::cout << "UPDATE particle " << i++; //<< std::endl;
                // std::cout << " x=" << *p.get_pos()[0] << " y=" << *p.get_pos()[1];// << std::endl;
                // std::cout << " rho=" << p.get_density() << std::endl;
                update_kernel_ptr(p);
            }

            hwriter->begin_iteration(iter, output_path);
        }
    }

    const std::vector<std::string>& sim_get_forces_scalar() {
        return forces_scalar;
    }

    const std::vector<std::string>& sim_get_forces_vector() {
        return forces_vector;
    }

    const std::vector<Particle>& sim_get_particles() {
        return particles;
    }

    uint64_t sim_get_max_iterations() { return max_iterations; }

    double sim_get_influence_radius() { return influence_radius; }

}  // namespace wash
