#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

#include "radixsort.hpp"

#define TEST_SIZE 1000000
typedef unsigned __int128 uint128_t;

int main()
{
	std::vector<uint32_t> keys_32( TEST_SIZE );
	std::vector<uint64_t> keys_64( TEST_SIZE );
	std::vector<__int128> data( TEST_SIZE );

	for( size_t row = 0; row < TEST_SIZE; ++row ) {
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

	std::cout << "Sorted " << TEST_SIZE << " uint64_t keys in " << d1.count() << "s ("
	          << size_t( TEST_SIZE / d1.count() ) << " items/sec)" << std::endl;
	std::cout << "Sorted " << TEST_SIZE << " uint32_t keys with uint128_t load in " << d2.count()
	          << "s (" << size_t( TEST_SIZE / d2.count() ) << " items/sec)" << std::endl;

	return 0;
}
