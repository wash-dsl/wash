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

double wash_eucdist(Particle& p, Particle& q) {
    wash::vec2d pos = p.wash_get_pos() - q.wash_get_pos();
    return sqrt(pos.magnitude());
}