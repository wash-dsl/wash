#include "wash_vector.hpp"
#include "wash_extra_functions.hpp"

template<typename T, int dim>
std::ostream& operator<< (std::ostream& s, const wash::Vec<T, dim>& vec){
   std::string str = std::string("vector { ");
   for (int i = 0; i < dim; i++) {
      s << wash::string_format("%f ", vec.at(i));
   }
   s << std::string("}");
   return s;

};