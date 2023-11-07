#pragma once
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>
#include <array>

#include <memory>
#include <stdexcept>

namespace wash {

    template<typename ... Args>
    std::string string_format( const std::string& format, Args ... args )
    {
        int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
        if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
        auto size = static_cast<size_t>( size_s );
        std::unique_ptr<char[]> buf( new char[ size ] );
        std::snprintf( buf.get(), size, format.c_str(), args ... );
        return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
    }

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

        T* operator[](int i) {
            return (data.begin() + i * sizeof(T));
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
                std::cout << i << *(vp[i]) << data[i] << *(v[i]) << std::endl;
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


        operator std::string() {
            std::string str = std::string("vector { ");
            for (int i = 0; i < dim; i++) {
                str += string_format("%f ", data[i]);
            }
            str += std::string("}");
            return str;
        }
        
    };

    typedef vector<double, 2> vec2d;
    typedef vector<double, 3> vec3d;
}