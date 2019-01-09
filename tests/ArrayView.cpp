#include <catch.hpp>

#include "ArrayView.h"

using namespace Utils;


template< typename ArrayView, size_t DimY, size_t DimX >
void test_2d_array() 
{
    int array[DimY][DimX];
    
    int idx = 0;
    for (int y=0; y < DimY; ++y) {
        for (int x=0; x < DimX; ++x, ++idx) {
            array[y][x] = idx;
        }
    }
    
    ArrayView empty;
    REQUIRE_FALSE(empty);
    REQUIRE(empty.size() == 0);
    REQUIRE(empty.size(0) == 0);
    REQUIRE(empty.size(1) == 0);
    
    ArrayView view = array;
    
    REQUIRE(view);
    REQUIRE(view.size() == DimY);
    REQUIRE(view[0].size() == DimX);
    REQUIRE(view.size(0) == DimY);
    REQUIRE(view.size(1) == DimX);
    
    idx = 0;
    for (int y=0; y < DimY; ++y) {
        for (int x=0; x < DimX; ++x, ++idx) {
            REQUIRE(view[y][x] == idx);
        }
    }
    
    idx = 0;
    for (auto y : view) {
        for (int val : y) {
            REQUIRE(val == idx);
            ++idx;
        }
    }
}

TEST_CASE("ArrayView")
{
    test_2d_array<ArrayView<int, 4,4>,4,4>();
    test_2d_array<ArrayView<int, 0,4>,4,4>();
    test_2d_array<ArrayView<int, 4,0>,4,4>();
    test_2d_array<ArrayView<int, 0,0>,4,4>();
}

