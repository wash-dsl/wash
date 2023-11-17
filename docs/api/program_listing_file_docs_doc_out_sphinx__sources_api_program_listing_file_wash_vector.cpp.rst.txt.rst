
.. _program_listing_file_docs_doc_out_sphinx__sources_api_program_listing_file_wash_vector.cpp.rst.txt:

Program Listing for File program_listing_file_wash_vector.cpp.rst.txt
=====================================================================

|exhale_lsh| :ref:`Return to documentation for file <file_docs_doc_out_sphinx__sources_api_program_listing_file_wash_vector.cpp.rst.txt>` (``docs/doc_out/sphinx/_sources/api/program_listing_file_wash_vector.cpp.rst.txt``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   
   .. _program_listing_file_wash_vector.cpp:
   
   Program Listing for File wash_vector.cpp
   ========================================
   
   |exhale_lsh| :ref:`Return to documentation for file <file_wash_vector.cpp>` (``wash_vector.cpp``)
   
   .. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS
   
   .. code-block:: cpp
   
      #include "wash_vector.hpp"
      
      #include "wash_extra_functions.hpp"
      
      template <typename T, int dim>
      std::ostream& operator<<(std::ostream& s, const wash::Vec<T, dim>& vec) {
          std::string str = std::string("vector { ");
          for (int i = 0; i < dim; i++) {
              s << wash::string_format("%f ", vec.at(i));
          }
          s << std::string("}");
          return s;
      };
