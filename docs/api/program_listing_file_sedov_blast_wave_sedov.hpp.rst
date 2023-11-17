
.. _program_listing_file_sedov_blast_wave_sedov.hpp:

Program Listing for File sedov.hpp
==================================

|exhale_lsh| :ref:`Return to documentation for file <file_sedov_blast_wave_sedov.hpp>` (``sedov_blast_wave/sedov.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   
   #include "../wash_mockapi.hpp"
   
   using namespace wash;
   
   void force_kernel(Particle& p, std::vector<Particle>& neighbours);
   void update_kernel(Particle& p);
   void init();
