
.. _program_listing_file_wash_vector.hpp:

Program Listing for File wash_vector.hpp
========================================

|exhale_lsh| :ref:`Return to documentation for file <file_wash_vector.hpp>` (``wash_vector.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   
   #include <array>
   #include <initializer_list>
   #include <iostream>
   #include <memory>
   #include <stdexcept>
   #include <string>
   #include <vector>
   
   namespace wash {
   
       template <typename T, int dim>
       class Vec {
       private:
           std::array<T, dim> data;
   
       public:
           Vec() {
               for (int i = 0; i < dim; i++) {
                   data[i] = 0;
               }
           }
   
           Vec(std::initializer_list<T> l) {
               size_t i = 0;
   
               for (T item : l) {
                   data[i] = item;
                   i++;
               }
           }
   
           T* operator[](int i) {
               T* idx = data.begin();
               std::advance(idx, i);
               return idx;
           }
   
           // Scalar addition (broadcast a T to all components)
           Vec<T, dim> operator+(T d) {
               Vec<T, dim> v;
               for (int i = 0; i < dim; i++) {
                   *(v[i]) = data[i] + d;
               }
               return v;
           }
   
           // Elementwise vector addition
           Vec<T, dim> operator+(Vec<T, dim> v) {
               Vec<T, dim> vp;
               for (int i = 0; i < dim; i++) {
                   *(vp[i]) = data[i] + *(v[i]);
               }
               return vp;
           }
   
           // Elementwise vector addition
           void operator+=(Vec<T, dim> v) {
               double* idx = data.begin();
               for (int i = 0; i < dim; i++) {
                   *idx += *(v[i]);
                   std::advance(idx, 1);
               }
           }
   
           // Elementwise vector subtraction
           Vec<T, dim> operator-(Vec<T, dim> v) {
               Vec<T, dim> vp;
               for (int i = 0; i < dim; i++) {
                   *(vp[i]) = data[i] - *(v[i]);
               }
               return vp;
           }
   
           // Scalar division
           Vec<T, dim> operator/(T d) {
               Vec<T, dim> v;
               for (int i = 0; i < dim; i++) {
                   *(v[i]) = data[i] / d;
               }
               return v;
           }
   
           // Scalar multiplication
           Vec<T, dim> operator*(T d) {
               Vec<T, dim> v;
               for (int i = 0; i < dim; i++) {
                   *(v[i]) = data[i] * d;
               }
               return v;
           }
   
           T magnitude() {
               T sum;
               for (int i = 0; i < dim; i++) {
                   sum += data[i] * data[i];
               }
               return sum;
           }
   
           T at(const size_t i) const { return data.at(i); }
       };
   
       typedef Vec<double, 2> Vec2D;
       typedef Vec<double, 3> Vec3D;
   }
