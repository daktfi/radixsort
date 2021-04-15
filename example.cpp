#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

#include "radixsort.hpp"

#define TEST_MIN 1000
#define TEST_SIZE 1000000
#define TEST_MAX 1000000000
typedef unsigned __int128 uint128_t;

int main( int argc, char *argv[] )
{
	size_t test_length = TEST_SIZE;

	// Check if there is non-default length provided.
	if( argc > 1 )
		test_length = std::min<size_t>( std::max<size_t>( atoll( argv[1] ), TEST_MIN ), TEST_MAX );

	std::vector<uint32_t> keys_32( test_length );
	std::vector<uint64_t> keys_64( test_length );
	std::vector<__int128> data( test_length );

	for( size_t row = 0; row < test_length; ++row ) {
		keys_32[row] = rand();
		keys_64[row] = rand();
		data[row] = rand();
	}

	std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

	radixsort( keys_64 );

	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

	radixsort( keys_32, data );

	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> d1 = t1 - t0, d2 = t2 - t1;

	// Check sorting correctness.
	for( size_t row = 1; row < test_length; ++row ) {
		if( keys_32[row - 1] > keys_32[row] ) {
			std::cerr << "Keys 32 sort broken at " << row << std::endl;
			break;
		}

		if( keys_64[row - 1] > keys_64[row] ) {
			std::cerr << "Keys 64 sort broken at " << row << std::endl;
			break;
		}
	}

	std::cout << "Sorted " << test_length << " uint64_t keys in " << d1.count() << "s ("
	          << size_t( test_length / d1.count() ) << " items/sec)" << std::endl;
	std::cout << "Sorted " << test_length << " uint32_t keys with uint128_t load in " << d2.count()
	          << "s (" << size_t( test_length / d2.count() ) << " items/sec)" << std::endl;

	return 0;
}
