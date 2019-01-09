

find_path (CATCH_PATH NAMES catch.hpp DOC "Path to catch.hpp file")



find_path (CATCH_PATH NAMES catch.hpp DOC "Path to catch.hpp file")

add_executable (VisseUtilsTests main.cpp ArrayView.cpp )


target_link_libraries (VisseUtilsTests 
    PRIVATE Visse::Utils
)
target_include_directories(VisseUtilsTests 
    PRIVATE ${CATCH_PATH}
)
