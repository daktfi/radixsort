#include <vector>
#include <chrono>

#include "radixsort_works.hpp"

typedef unsigned __int128 uint128_t;

template <typename Key, unsigned width>
double test_sort_key_inner( Key max_key, UIndex length, std::vector<Key> &keys )
{
	std::vector<Key> duplicate = keys;
	high_resolution_clock::time_point t0 = high_resolution_clock::now();
	Sorting<Key, width>::sort_key( max_key, length, duplicate );
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	duration<double> delta = t1 - t0;
	return delta.count();
}

template <typename Key, unsigned width>
double test_sort_key_outer( Key max_key, UIndex length, std::vector<Key> &keys, unsigned runs )
{
	double average = 0;

	if( runs > 1 ) {
		double times[runs], diff = 0;
		unsigned worst = -1;

		for( unsigned run = 0; run < runs; ++run ) {
			times[run] = test_sort_key_inner<Key, width>( max_key, length, keys );
			average += times[run];
		}

		average /= runs;

		if( runs > 3 ) {
			for( unsigned run = 0; run < runs; ++run )
				if( std::abs( average - times[run] ) > diff ) {
					diff = std::abs( average - times[run] );
					worst = run;
				}

			average = 0;

			for( unsigned run = 0; run < runs; ++run )
				average += ( run == worst ) ? 0 : times[run];

			average /= runs - 1;
		}
	} else
		average = test_sort_key_inner<Key, width>( max_key, length, keys );

	return average;
}

#define test_key( x )                                                             \
	if( ( x <= key_width || first ) && x >= digit_start && x <= digit_end ) {     \
	    timing[x] = test_sort_key_outer<Key, x>( max_key, length, source, runs ); \
	    first = false;                                                            \
    }

template <typename Key>
void test_sort_key( UIndex length, unsigned key_width, unsigned runs, unsigned digit_start,
                    unsigned digit_end )
{
	std::vector<Key> source( length );
	Key max_key;
	double timing[19] = { 0.0 };
	bool first = true;

	for( UIndex row = 0; row < length; ++row ) {
		source[row] = random<Key>( key_width );
		max_key = source[row] > max_key ? source[row] : max_key;
	}

	test_key( 4 );
	test_key( 5 );
	test_key( 6 );
	test_key( 7 );
	test_key( 8 );
	test_key( 9 );
	test_key( 10 );
	test_key( 11 );
	test_key( 12 );
	test_key( 13 );
	test_key( 14 );
	test_key( 15 );
	test_key( 16 );
	test_key( 17 );
	test_key( 18 );

	printf( "%s;empty;%u;%lu", t_name<Key>().c_str(), key_width, length );

	for( unsigned w = 4; w <= 18; ++w )
		if( w <= key_width || w == 4 )
			printf( ";%0.6lf", timing[w] );

	printf( "\n" );
}

template <typename Key, typename Data, unsigned width>
double test_sort_both_inner( Key max_key, UIndex length, std::vector<Key> &keys, std::vector<Data> &values )
{
	std::vector<Key> duplicate = keys;
	high_resolution_clock::time_point t0 = high_resolution_clock::now();
	Sorting<Key, width>::sort_both( max_key, length, duplicate, values );
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	duration<double> delta = t1 - t0;
	return delta.count();
}

template <typename Key, typename Data, unsigned width>
double test_sort_both_outer( Key max_key, UIndex length, std::vector<Key> &keys, std::vector<Data> &values,
                             unsigned runs )
{
	double average = 0;

	if( runs > 1 ) {
		double times[runs], diff = 0;
		unsigned worst = -1;

		for( unsigned run = 0; run < runs; ++run ) {
			times[run] = test_sort_both_inner<Key, Data, width>( max_key, length, keys, values );
			average += times[run];
		}

		average /= runs;

		if( runs > 3 ) {
			for( unsigned run = 0; run < runs; ++run )
				if( std::abs( average - times[run] ) > diff ) {
					diff = std::abs( average - times[run] );
					worst = run;
				}

			average = 0;

			for( unsigned run = 0; run < runs; ++run )
				average += ( run == worst ) ? 0 : times[run];

			average /= runs - 1;
		}
	} else
		average = test_sort_both_inner<Key, Data, width>( max_key, length, keys, values );

	return average;
}

#define test_both( x )                                                                           \
	if( ( x <= key_width || first ) && x >= digit_start && x <= digit_end ) {                    \
	    timing[x] = test_sort_both_outer<Key, Data, x>( max_key, length, source, values, runs ); \
	    first = false;                                                                           \
    }

template <typename Key, typename Data, unsigned width = 0>
void test_sort_both( UIndex length, unsigned key_width, unsigned runs, unsigned digit_start,
                     unsigned digit_end )
{
	std::vector<Key> source( length );
	std::vector<Data> values( length );
	Key max_key;
	double timing[19] = { 0.0 };
	bool first = true;

	for( UIndex row = 0; row < length; ++row ) {
		source[row] = random<Key>( key_width );
		max_key = source[row] > max_key ? source[row] : max_key;
	}

	test_both( 4 );
	test_both( 5 );
	test_both( 6 );
	test_both( 7 );
	test_both( 8 );
	test_both( 9 );
	test_both( 10 );
	test_both( 11 );
	test_both( 12 );
	test_both( 13 );
	test_both( 14 );
	test_both( 15 );
	test_both( 16 );
	test_both( 17 );
	test_both( 18 );

	printf( "%s;%s;%u;%lu", t_name<Key>().c_str(), t_name<Data>().c_str(), key_width, length );

	for( unsigned w = 4; w <= 18; ++w )
		if( w <= key_width || w == 4 )
			printf( ";%0.6lf", timing[w] );

	printf( "\n" );
}

main()
{
	// TODO: Length, number of passes, starting/ending key and digigt widths must be settable...
	UIndex length = 10000000;
	unsigned runs = 4, key_start = 1, key_end = 128, digit_start = 4, digit_end = 18;

	if( key_start > 128 )
		key_start = 128;

	if( key_end < key_start )
		key_end = key_start;

	if( key_start == 0 || key_end == 0 ) {
		key_start = 1;
		key_end = 128;
	}

	if( runs == 0 )
		runs = 4;

	if( runs > 100 )	// Some common sense limits...
		runs = 100;

	if( digit_start < 4 )
		digit_start = 4;

	if( digit_end > 18 )
		digit_end = 18;

	if( key_start <= 16 ) {
		unsigned key16_start = std::max<unsigned>( key_start, 1 ),
		         key16_end = std::min<unsigned>( key_end, 16 );

		for( unsigned key_width = key16_start; key_width <= key16_end; ++key_width ) {
			test_sort_key<uint16_t>( length, key_width, runs, digit_start, digit_end );
			test_sort_both<uint16_t, uint32_t>( length, key_width, runs, digit_start, digit_end );
			test_sort_both<uint16_t, uint64_t>( length, key_width, runs, digit_start, digit_end );
			test_sort_both<uint16_t, uint128_t>( length, key_width, runs, digit_start, digit_end );
		}
	}

	if( key_start <= 32 && key_end > 16 ) {
		unsigned key32_start = std::max<unsigned>( key_start, 17 ),
		         key32_end = std::min<unsigned>( key_end, 32 );

		for( unsigned key_width = key32_start; key_width <= key32_end; ++key_width ) {
			test_sort_key<uint32_t>( length, key_width, runs, digit_start, digit_end );
			test_sort_both<uint32_t, uint32_t>( length, key_width, runs, digit_start, digit_end );
			test_sort_both<uint32_t, uint64_t>( length, key_width, runs, digit_start, digit_end );
			test_sort_both<uint32_t, uint128_t>( length, key_width, runs, digit_start, digit_end );
		}
	}

	if( key_start <= 64 && key_end > 32 ) {
		unsigned key64_start = std::max<unsigned>( key_start, 33 ),
		         key64_end = std::min<unsigned>( key_end, 64 );

		for( unsigned key_width = key64_start; key_width <= key64_end; ++key_width ) {
			test_sort_key<uint64_t>( length, key_width, runs, digit_start, digit_end );
			test_sort_both<uint64_t, uint32_t>( length, key_width, runs, digit_start, digit_end );
			test_sort_both<uint64_t, uint64_t>( length, key_width, runs, digit_start, digit_end );
			test_sort_both<uint64_t, uint128_t>( length, key_width, runs, digit_start, digit_end );
		}
	}

	if( key_end > 64 ) {
		unsigned key128_start = std::max<unsigned>( key_start, 65 ),
		         key128_end = std::min<unsigned>( key_end, 128 );

		for( unsigned key_width = key128_start; key_width <= key128_end; ++key_width ) {
			test_sort_key<uint128_t>( length, key_width, runs, digit_start, digit_end );
			test_sort_both<uint128_t, uint32_t>( length, key_width, runs, digit_start, digit_end );
			test_sort_both<uint128_t, uint64_t>( length, key_width, runs, digit_start, digit_end );
			test_sort_both<uint128_t, uint128_t>( length, key_width, runs, digit_start, digit_end );
		}
	}
}
