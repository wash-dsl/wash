#pragma once
#ifndef WASH_VECTOR_H
#define WASH_VECTOR_H

#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <stdexcept>

namespace wash {

    template<typename T, int dim>
    class vector {
    private:
        std::array<T, dim> data;
    public:
        vector() {
            for (int i = 0; i < dim; i++) {
                data[i] = 0;
            }
        }

        vector(std::initializer_list<T> l) {
            size_t i = 0;

            for (T item : l) {
                data[i] = item;
                i++;
            }
        }

        T* operator[](int i) const {
            T* idx = data.begin();
            std::advance(idx, i);
            return idx; 
        }

        // Scalar addition (broadcast a T to all components)
        vector<T, dim> operator+(T d) {
            vector<T, dim> v;
            for (int i = 0; i < dim; i++) {
                *(v[i]) = data[i] + d;
            }
            return v;
        }

        // Elementwise vector addition
        vector<T, dim> operator+(vector<T, dim> v) {
            vector<T, dim> vp;
            for (int i = 0; i < dim; i++) {
                *(vp[i]) = data[i] + *(v[i]);
            }
            return vp;
        }

        // Elementwise vector addition
        void operator+=(vector<T, dim> v) {
            double* idx = data.begin();
            for (int i = 0; i < dim; i++) {
                *idx += *(v[i]);
                std::advance(idx, 1);
            }
        }

        // Elementwise vector subtraction
        vector<T, dim> operator-(vector<T, dim> v) {
            vector<T, dim> vp;
            for (int i = 0; i < dim; i++) {
                *(vp[i]) = data[i] - *(v[i]);
            }
            return vp;
        }

        // Scalar division
        vector<T, dim> operator/(T d) {
            vector<T, dim> v;
            for (int i = 0; i < dim; i++) {
                *(v[i]) = data[i] / d;
            }
            return v;
        }

        // Scalar multiplication
        vector<T, dim> operator*(T d) {
            vector<T, dim> v;
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
    };

    typedef vector<double, 2> vec2d;
    typedef vector<double, 3> vec3d;
}

#endif