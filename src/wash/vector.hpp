#pragma once

#include <array>
#include <cmath>
#include <initializer_list>
#include <memory>
#include <string>
#include <iostream>
#include <ostream>
#include <assert.h>     /* assert */


// DIM is the compile-time flag for the dimensionality of the simulation, dictating
// the dimensionality of the vector to use. If it's not defined as a flag, we default
// it to 2 here.
#ifndef DIM
#define DIM 2
#endif

namespace wash {
    template <typename T, int dim>
    class Vec {
        /**
         * Vector Class for WASH
         *
         */
    private:
        std::array<T, dim> data;

    public:
        constexpr Vec() {
            for (int i = 0; i < dim; i++) {
                data[i] = 0;
            }
        }

        constexprVec(std::initializer_list<T> l) {
            assert(l.size() == dim);

            for (T item : l) {
                data[i] = item;
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
        Vec<T, dim> operator-(Vec<T, dim> v) const {
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
            T sum = T(0);
            for (int i = 0; i < dim; i++) {
                sum += data[i] * data[i];
            }
            return std::sqrt(sum);
        }

        T at(const size_t i) const { return data.at(i); }

        Vec<T, dim> abs() const {
            auto vec = Vec<T, dim>();
            for (size_t i = 0; i < dim; i++) {
                *(vec[i]) = std::abs(data[i]);
            }

            return vec;
        }
    };

    using SimulationVecT = Vec<double, DIM>;
    
    typedef Vec<double, 2> Vec2D;
    typedef Vec<double, 3> Vec3D;
}

template <typename T, int dim>
std::ostream& operator<<(std::ostream& s, const wash::Vec<T, dim>& vec) {
    s << std::string("vector [");
    for (int i = 0; i < dim; i++) {
        s << vec.at(i);
        if (i < dim - 1) s << std::string(", ");
    }
    s << std::string("]");
    return s;
};