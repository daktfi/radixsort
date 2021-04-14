#include "vector"
#include "radixsort.hpp"

#define TEST_SIZE 1000000
typedef unsigned __int128 uint128_t;

main()
{
    std::vector<uint16_t> keys_16( TEST_SIZE );
    std::vector<uint32_t> keys_32( TEST_SIZE );
    std::vector<uint64_t> keys_64( TEST_SIZE );
    std::vector<uint128_t> keys_128( TEST_SIZE );
    std::vector<double> keys_double( TEST_SIZE );
    std::vector<__int128> data( TEST_SIZE );
    
    for( size_t row = 0; row < TEST_SIZE; ++row ) {
    }
    
    return 0;
}
