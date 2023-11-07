#include "wash_mockapi.hpp"

void wash_set_update_kernel(t_update_kernel update_kernel){
    update_kernel_ptr = update_kernel;
}

void wash_set_init_kernel(t_init init){
    init_kernel_ptr = init;
}

void wash_set_precision(char* precision){
    return;
}

void wash_set_dimensions(uint8_t dimensions){
    return;
}
