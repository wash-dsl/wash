
.. _program_listing_file_docs_doc_out_sphinx__sources_api_program_listing_file_wash_mockapi.hpp.rst.txt:

Program Listing for File program_listing_file_wash_mockapi.hpp.rst.txt
======================================================================

|exhale_lsh| :ref:`Return to documentation for file <file_docs_doc_out_sphinx__sources_api_program_listing_file_wash_mockapi.hpp.rst.txt>` (``docs/doc_out/sphinx/_sources/api/program_listing_file_wash_mockapi.hpp.rst.txt``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   
   .. _program_listing_file_wash_mockapi.hpp:
   
   Program Listing for File wash_mockapi.hpp
   =========================================
   
   |exhale_lsh| :ref:`Return to documentation for file <file_wash_mockapi.hpp>` (``wash_mockapi.hpp``)
   
   .. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS
   
   .. code-block:: cpp
   
      #pragma once
      
      #include <iostream>
      #include <list>
      #include <random>
      #include <string>
      #include <unordered_map>
      
      #include "wash_vector.hpp"
      
      namespace wash {
          class Particle {
          private:
              Vec2D pos;
              Vec2D vel;
              Vec2D acc;
              double density;
              double mass;
              std::unordered_map<std::string, double> force_scalars;
              std::unordered_map<std::string, Vec2D> force_vectors;
      
          public:
              Particle(){};
              Particle(const Vec2D pos, double density);
      
              // Return the force value
              void* get_force(const std::string& force) const;
      
              double get_force_scalar(const std::string& force) const;
              Vec2D get_force_vector(const std::string& force) const;
      
              // Set the force value
              void set_force(const std::string& force, void* value);
      
              void set_force_scalar(const std::string& force, const double value);
              void set_force_vector(const std::string& force, const Vec2D value);
      
              Vec2D get_pos() const;
              void set_pos(const Vec2D pos);
      
              Vec2D get_vel() const;
              void set_vel(const Vec2D vel);
      
              Vec2D get_acc() const;
              void set_acc(const Vec2D acc);
      
              double get_density() const;
              void set_density(const double density);
      
              double get_mass() const;
              void set_mass(const double mass);
          };
      
          typedef void (*t_update_kernel)(Particle&);
          typedef void (*t_force_kernel)(Particle&, std::vector<Particle>&);
          typedef void (*t_init)();
      
          /*
           Simulation parameter setup functions.
           These define the parameters of the simulation
          */
      
          /*
           Set precision to double/float
           In the non-DSL version hardcode to double?
          */
          void set_precision(const std::string precision);
      
          /*
           Set radius of nearest neighbour particles which
           influence other particles.
      
           Expected to be constant over the simulation
          */
          void set_influence_radius(const double radius);
      
          /*
           Get radius of nearest neighbour particles which
           influence other particles.
          */
          double get_influence_radius();
      
          /*
           Set the number of dimensions of the particle class
           (e.g. size of pos vector, etc.)
      
           In the non-DSL version hardcode to 2?
          */
          void set_dimensions(const uint8_t dimensions);
      
          /*
           Set the maximum number of iterations
           Later we can implement different stopping criteria
          */
          void set_max_iterations(const uint64_t iterations);
      
          /*
           Register a force with the DSL which will be present
           on particle object. Will be a scalar `precision` type.
          */
          void add_force(const std::string force);
      
          // Similarly, but set to be a vector `precision` type size `dim`
          void add_force(const std::string force, const uint8_t dim);
      
          /*
           Add a particle to the simulation
          */
          void add_par(const Particle p);
      
          /*
           Compute the euclidean distance
          */
          double eucdist(const Particle& p, const Particle& q);
      
          /*
           Register the init kernel function
          */
          void set_init_kernel(const t_init init);
      
          /*
           Register the force update kernel
          */
          void set_force_kernel(const t_force_kernel force_kernel);
      
          /*
           Register the particle position update kernel
          */
          void set_update_kernel(const t_update_kernel update_kernel);
      
          /*
           The smoothing kernel used for density computations
           (may be worth letting the user define this in future, although we'll provide a standard implementation for now)
          */
          double density_smoothing(const double radius, const double dist);
      
          /*
           The density update kernel
           (assuming a fixed smoothing kernel, this will be invariant between different particle simulations)
          */
          void density_kernel(Particle& p, std::vector<const Particle>& neighbors);
      
          /*
           Start Simulation
          */
      
          void start();
      
      }
