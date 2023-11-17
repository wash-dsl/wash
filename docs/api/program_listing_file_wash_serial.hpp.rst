
.. _program_listing_file_wash_serial.hpp:

Program Listing for File wash_serial.hpp
========================================

|exhale_lsh| :ref:`Return to documentation for file <file_wash_serial.hpp>` (``wash_serial.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   /*
    Header file of all the DSL functions the API spec seemed to use
    with some best guesses for types, etc.
   
    Written in the browser based vscode so might not be actually great
    c++ code -- should probably check that with a compiler.
   
    Took some liberties with adding a few more underscores.
    No implementations at this point.
   */
   
   // TODO: Issues
   // 1. What is the force type? How do we know what it is?
   // 2. Forces being string indexed might be more readable
   //    but without a DSL we really need some translation func
   // 3. I'll think of some more later
   
   /*
       Includes etc..
   */
   #pragma once
   
   #include "wash_mockapi.hpp"
   
   using namespace wash;
   
   void force_kernel(Particle& p, std::vector<Particle>& neighbours);
   void update_kernel(Particle& p);
   void init();
