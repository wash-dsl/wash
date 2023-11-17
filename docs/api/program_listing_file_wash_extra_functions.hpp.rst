
.. _program_listing_file_wash_extra_functions.hpp:

Program Listing for File wash_extra_functions.hpp
=================================================

|exhale_lsh| :ref:`Return to documentation for file <file_wash_extra_functions.hpp>` (``wash_extra_functions.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   #include <string>
   
   namespace wash {
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
   }  // namespace wash
