#pragma once

#include <chrono>
#include <stdexcept>
#include <string>

namespace wash {

    template<typename T>
    int64_t diff_ms(std::chrono::time_point<T> time1, std::chrono::time_point<T> time2) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(time2 - time1).count();
    }

    /**
     * @brief Helper function to format to a c++ string
     *
     * @tparam Args
     * @param format standard style format string
     * @param args arguments for the format string
     * @return std::string
     */
    template <typename... Args>
    std::string string_format(const std::string& format, Args... args) {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;  // Extra space for '\0'
        if (size_s <= 0) {
            throw std::runtime_error("Error during formatting.");
        }
        auto size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1);  // We don't want the '\0' inside
    }

    /**
     * @brief Helper function for the pure sign of a numerical type
     *
     * @tparam T
     * @param val numerical value
     * @return +1 if positive, -1 if negative
     */
    template <typename T>
    int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }

    template <typename T, size_t N, size_t... I>
    auto make_tuple(std::array<T, N>& arr, std::index_sequence<I...>) {
        return std::forward_as_tuple(arr[I]...);
    }

    template <typename T, size_t N>
    auto make_tuple(std::array<T, N>& arr) {
        return wash::make_tuple(arr, std::make_index_sequence<N>{});
    }

    template <typename T, size_t N, size_t M>
    auto make_tuple(std::array<T, N>& arr) {
        return wash::make_tuple(arr, std::make_index_sequence<M>{});
    }
}
