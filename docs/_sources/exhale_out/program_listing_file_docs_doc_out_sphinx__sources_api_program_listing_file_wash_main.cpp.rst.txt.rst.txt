
.. _program_listing_file_docs_doc_out_sphinx__sources_api_program_listing_file_wash_main.cpp.rst.txt:

Program Listing for File program_listing_file_wash_main.cpp.rst.txt
===================================================================

|exhale_lsh| :ref:`Return to documentation for file <file_docs_doc_out_sphinx__sources_api_program_listing_file_wash_main.cpp.rst.txt>` (``docs/doc_out/sphinx/_sources/api/program_listing_file_wash_main.cpp.rst.txt``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   
   .. _program_listing_file_wash_main.cpp:
   
   Program Listing for File wash_main.cpp
   ======================================
   
   |exhale_lsh| :ref:`Return to documentation for file <file_wash_main.cpp>` (``wash_main.cpp``)
   
   .. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS
   
   .. code-block:: cpp
   
      #include <algorithm> // for std::max
      
      #include "wash_serial.hpp"
      
      #define DELTA_TIME 0.1
      #define SMOOTH_RAD 20
      
      // TODO:
      // - Kernel function for density calculation?
      //
      
      double user_smoothing_derivative(double radius, double dist) {
          double value = std::max(0.0, radius * radius - dist * dist);
          return value * value * value;
      }
      
      wash::Vec2D user_bounds_check(wash::Vec2D pos) {
          return pos; // TODO: bounds check
      }
      
      void force_kernel(wash::Particle &p, std::vector<wash::Particle> &neighbours) {
          wash::Vec2D pressure_force;
          for (wash::Particle &q : neighbours) {
              double dist = wash::eucdist(p, q);
              wash::Vec2D dir = (p.get_pos() - q.get_pos()) / dist;
              double slope = user_smoothing_derivative(SMOOTH_RAD, dist);
              pressure_force += dir * -q.get_force_scalar("pressure") * q.get_force_scalar("vol") * slope;
          }
      
          p.set_force_vector("pressure", pressure_force);
      }
      
      void update_kernel(wash::Particle &p) {
          p.set_acc(p.get_force_vector("pressure") / p.get_density());
          p.set_vel(p.get_vel() + p.get_acc() * DELTA_TIME);
          p.set_pos(user_bounds_check(p.get_pos() + p.get_vel() * DELTA_TIME));
      }
      
      void init() {
          std::uniform_real_distribution<double> unif(0, 1);
          std::default_random_engine re;
          size_t num_particles = 100;
          for (size_t i = 0; i < num_particles; i++) {
              double xpos = unif(re);
              double ypos = unif(re);
      
              wash::Particle p({xpos, ypos}, 0.01);
              wash::add_par(p);
          }
      }
      
      int main(int argc, char **argv) {
          wash::set_precision("double");
          wash::set_influence_radius(0.1);
          wash::set_dimensions(2);
          wash::set_max_iterations(100);
          wash::add_force("temp");
          wash::add_force("pressure");
          wash::add_force("vol");
      
          wash::set_init_kernel(&init);
          wash::set_force_kernel(&force_kernel);
          wash::set_update_kernel(&update_kernel);
      
          wash::start();
      }
