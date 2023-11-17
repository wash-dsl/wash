
.. _program_listing_file_docs_doc_out_sphinx__sources_api_program_listing_file_sedov_blast_wave_sedov.hpp.rst.txt:

Program Listing for File program_listing_file_sedov_blast_wave_sedov.hpp.rst.txt
================================================================================

|exhale_lsh| :ref:`Return to documentation for file <file_docs_doc_out_sphinx__sources_api_program_listing_file_sedov_blast_wave_sedov.hpp.rst.txt>` (``docs/doc_out/sphinx/_sources/api/program_listing_file_sedov_blast_wave_sedov.hpp.rst.txt``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   
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
