#include "wash_mockapi.hpp"

void wash_set_update_kernel(t_update_kernel update_kernel){
    update_kernel_ptr = update_kernel;
}

void wash_set_force_kernel(t_force_kernel force_kernel) {
    force_kernel_ptr = force_kernel;
}

void wash_set_init_kernel(t_init init){
    init_kernel_ptr = init;
}

void wash_set_precision(std::string precision){
    return;
}

void wash_set_dimensions(uint8_t dimensions){
    return;
}

void wash_add_force(std::string force) {
    forces_scalar.push_back(force);
}

void wash_add_force(std::string force, uint8_t dim) {
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

void wash_add_par(Particle p) {
    particles.push_back(p);
}

void wash_set_influence_radius(double radius) {
    return;
}

double wash_eucdist(Particle& p, Particle& q) {
    wash::vec2d pos = p.wash_get_pos() - q.wash_get_pos();
    return sqrt(pos.magnitude());
}

Particle::Particle (wash::vec2d pos, double density) {
    this->pos = pos;
    this->density = density;

    for (std::string force : forces_scalar) {
        *(double *)this->forcesv[force] = 0.0;
    }

    for (std::string force : forces_vector) {
        *(wash::vec2d *)this->forcesv[force] = wash::vec2d();
    }
}

void* Particle::wash_get_force(std::string& force) {
    return nullptr;
}

double Particle::wash_get_force_scalar(std::string force) {
    return *(double*)this->forcesv[force];
}

wash::vec2d Particle::wash_get_force_vector(std::string force) {
    return *(wash::vec2d *)this->forcesv[force];
}

void Particle::wash_set_force(std::string force, void* value) {
    return;
}

void Particle::wash_set_force_scalar(std::string force, double value) {
    *(double *)(this->forcesv[force]) = value;
}

void Particle::wash_set_force_vector(std::string force, wash::vec2d value) {
    *(wash::vec2d *)(this->forcesv[force]) = value;
}

wash::vec2d Particle::wash_get_pos() {
    return this->pos;
}

void Particle::wash_set_pos(wash::vec2d pos) {
    this->pos = pos;
}

wash::vec2d Particle::wash_get_vel() {
    return this->vel;
}

void Particle::wash_set_vel(wash::vec2d vel) {
    this->vel = vel;
}

wash::vec2d Particle::wash_get_acc() {
    return this->acc;
}

void Particle::wash_set_acc(wash::vec2d acc) {
    this->acc = acc;
}

double Particle::wash_get_density() {
    return this->density;
}

void Particle::wash_set_density(double density) {
    this->density = density;
}

void wash_start() {
    std::cout << "Hello World" << std::endl;
}