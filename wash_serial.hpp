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

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

t_update_kernel update_kernel;

t_force_kernel force_kernel;

t_init init;