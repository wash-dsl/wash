#include "wash_vector.hpp"
#include "wash_extra_functions.cpp"

template<typename T, int dim>
std::ostream& operator<< (std::ostream& s, const wash::vector<T, dim>& vec){
   std::string str = std::string("vector { ");
   for (int i = 0; i < dim; i++) {
      s << wash::string_format("%f ", vec[i]);
   }
   s << std::string("}");
};