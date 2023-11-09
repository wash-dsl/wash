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
    class WashVector {
    private:
        std::array<T, dim> data;
    public:
        WashVector() {
            for (int i = 0; i < dim; i++) {
                data[i] = 0;
            }
        }

        WashVector(std::initializer_list<T> l) {
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
        WashVector<T, dim> operator+(T d) {
            WashVector<T, dim> v;
            for (int i = 0; i < dim; i++) {
                *(v[i]) = data[i] + d;
            }
            return v;
        }

        // Elementwise vector addition
        WashVector<T, dim> operator+(WashVector<T, dim> v) {
            WashVector<T, dim> vp;
            for (int i = 0; i < dim; i++) {
                *(vp[i]) = data[i] + *(v[i]);
            }
            return vp;
        }

        // Elementwise vector addition
        void operator+=(WashVector<T, dim> v) {
            double* idx = data.begin();
            for (int i = 0; i < dim; i++) {
                *idx += *(v[i]);
                std::advance(idx, 1);
            }
        }

        // Elementwise vector subtraction
        WashVector<T, dim> operator-(WashVector<T, dim> v) {
            WashVector<T, dim> vp;
            for (int i = 0; i < dim; i++) {
                *(vp[i]) = data[i] - *(v[i]);
            }
            return vp;
        }

        // Scalar division
        WashVector<T, dim> operator/(T d) {
            WashVector<T, dim> v;
            for (int i = 0; i < dim; i++) {
                *(v[i]) = data[i] / d;
            }
            return v;
        }

        // Scalar multiplication
        WashVector<T, dim> operator*(T d) {
            WashVector<T, dim> v;
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

        T at(const size_t i) const{
            return data[i];
        }
    };

    typedef WashVector<double, 2> Vec2D;
    typedef WashVector<double, 3> Vec3D;
}

#endif