#pragma once

#ifndef NOPREFETCH
#define PREFETCH

/// x86: L1d is 8 pools of 64 rows of 64 bytes per core, shared by two logical cores.
/// So we can safely use roughly half of those rows.
/// Keep in mind that those 8 pools can be used very unevenly.
/// Adjust for other CPUs accordingly or simply define right values when compiling.
/// For example, POWERPC8 has 1/2/3/4 logical cores per physical one (adjustable at runtime).
/// x86: L2 cache is 256 kb per core (1 mb xeon), so we have to keep stastics table under 16384
/// 64-bit entries (65536 entries on server) (max digit width 14/16 bits).
/// x86: L3 cache for server CPU is big. Nothing to worry about until L4 caches.

#ifndef PF_ROWS
#define PF_ROWS 200  // Of 512 total L1d rows
#endif               // PF_ROWS

#define pf_solo ( PF_ROWS / 3 )
#define pf_solo_cnt ( 2 * PF_ROWS / 3 )
#define pf_two ( PF_ROWS / 4 )
#define pf_two_cnt ( PF_ROWS / 2 )

#endif  // NOPREFETCH

using int128_t = __int128;
using uint128_t = unsigned __int128;

// clang-format off
namespace std
{
// Dunno why GCC doesn't did that by itself...
//template <> struct __is_integral_helper<__int128> : public true_type {};
//template <> struct __is_integral_helper<unsigned __int128> : public true_type {};
}  // namespace std

template <typename T> int significant_bits( const T &v );

// Probably need to specifically exclude whatever very wide integer we're gonna use...
template <typename T> typename std::enable_if<!std::is_integral<T>::value, int>::type
significant_bits( const T & ) { return 8 * sizeof( T ); }

/// Assumes T is integer type and LSB stored and marked as signed if so.
/// We have exact specializations for (u)int16/32/64/128_t.
template <typename T>
typename std::enable_if<!std::is_same<T, int16_t>::value && !std::is_same<T, uint16_t>::value
                     && !std::is_same<T, int32_t>::value && !std::is_same<T, uint32_t>::value
                     && !std::is_same<T, int64_t>::value && !std::is_same<T, uint64_t>::value
                     && !std::is_same<T, int128_t>::value && !std::is_same<T, uint128_t>::value
                     && std::is_integral<T>::value, int>::type
significant_bits( const T &v )
{
	// clang-format on
	const uint64_t *value_ptr = reinterpret_cast<const uint64_t *>( &v );
	int length = sizeof( T ) / sizeof( uint64_t ), result;

	// Go down from highest qword.
	if( std::numeric_limits<T>::is_singed
	    && value_ptr[length - 1] > std::numeric_limits<int64_t>::max() ) {
		// Negative signed value.
		do {
			result = significant_bits<uint64_t>( ~value_ptr[--length] );
		} while( result == 0 && length > 0 );
	} else {
		// Positive or unsigned value.
		do {
			result = significant_bits<uint64_t>( value_ptr[--length] );
		} while( result == 0 && length > 0 );
	}

	return length * 8 * sizeof( uint64_t ) + result;
}

template <>
inline int significant_bits( const uint16_t &v )
{
	return v ? ( 32 - __builtin_clz( static_cast<uint32_t>( v ) ) ) : 0;
}

template <>
inline int significant_bits( const int16_t &v )
{
	if( v >= 0 )
		return significant_bits( static_cast<uint16_t>( v ) );
	else
		return significant_bits( static_cast<uint16_t>( ~v ) );
}

template <>
inline int significant_bits( const uint32_t &v )
{
	return v ? ( 32 - __builtin_clz( v ) ) : 0;
}

template <>
inline int significant_bits( const int32_t &v )
{
	if( v >= 0 )
		return significant_bits( static_cast<uint32_t>( v ) );
	else
		return significant_bits( static_cast<uint32_t>( ~v ) );
}

template <>
inline int significant_bits( const uint64_t &v )
{
	return v ? ( 64 - __builtin_clzl( v ) ) : 0;
}

template <>
inline int significant_bits( const int64_t &v )
{
	if( v >= 0 )
		return significant_bits( static_cast<uint64_t>( v ) );
	else
		return significant_bits( static_cast<uint64_t>( ~v ) );
}

template <>
inline int significant_bits( const uint128_t &v )
{
	uint64_t hi = static_cast<uint64_t>( v >> 64 );

	if( hi == 0 )
		return significant_bits( static_cast<uint64_t>( v ) );
	else
		return significant_bits( hi ) + 64;
}

template <>
inline int significant_bits( const int128_t &v )
{
	if( v >= 0 )
		return significant_bits( static_cast<uint128_t>( v ) );
	else
		return significant_bits( static_cast<uint128_t>( ~v ) );
}

/// Sorter object templated by sorting key type.
/// @param Key - key type to sort by.
template <typename Key, template <class...> class Container, unsigned width = 0, class... Args>
class Sorting
{
public:
	/// Sorts key only.
	/// @param max_key - max value of the key to sort.
	/// @param length - length of the array to sort (redundand).
	/// @param keys - array of keys to sort.
	static void sort_key( Key max_key, size_t length, Container<Key> &keys );

	/// Sorts both key and data.
	/// @param max_key - max value of the key to sort.
	/// @param length - length of the array to sort (redundand).
	/// @param keys - array of keys to sort.
	/// @param vals - array of values.
	template <typename Data>
	static void sort_both( Key max_key, size_t length, Container<Key> &keys,
	                       Container<Data> &vals );

	/// Sorts both key and data.
	/// @param max_key - max value of the key to sort.
	/// @param length - length of the array to sort (redundand).
	/// @param keys - array of keys to sort.
	/// @param vals - array of values.
	template <typename Data>
	static void sort_both( Key max_key, size_t length, Container<Key> &keys,
	                       Container<Data> &vals );

	/// Sorts vals array by keys, leaving keys array intact
	/// @param max_key - max value of the key to sort.
	/// @param length - length of the array to sort (redundand).
	/// @param keys - array of keys to sort by.
	/// @param vals - array of values to be sorted.
	template <typename Data>
	static void sort_data( Key max_key, size_t length, Container<const Key> &keys,
	                       Container<Data> &vals );

private:
	/// Internal - single pass for sort_both and sort_data (middle passes).
	template <typename Data, unsigned digit_width>
	static inline void pass_both( size_t *offsets, const Key *k0, const Key *end, const Data *d0,
	                              Key *k1, Data *d1, unsigned shift );

	/// Internal - digit-size templated sorter for key only.
	template <unsigned digit_width>
	static void sort_key_width( size_t length, Container<Key> &keys, unsigned passes_count );

	/// Internal - digit-size templated sorter for key and data.
	template <typename Data, unsigned digit_width>
	static void sort_both_width( size_t length, Container<Key> &keys, Container<Data> &vals,
	                             unsigned passes_count );

	/// Internal - digit-size templated sorter for key and data.
	template <typename Data, unsigned digit_width>
	static void sort_both_width( size_t length, Container<Key> &keys, Container<Data> &vals,
	                             unsigned passes_count );

	/// Internal - digit-size templated sorter for data only.
	template <typename Data, unsigned digit_width>
	static void sort_data_width( size_t length, Container<Key> &keys, Container<Data> &vals,
	                             unsigned passes_count );

	/// Internal - digit-size and passes count templated real sorter for key only.
	template <unsigned digit_width, unsigned passes_count>
	static void sort_key_real( size_t length, Container<Key> &keys );

	/// Internal - digit-size and passes count templated real sorter for key and data.
	template <typename Data, unsigned digit_width, unsigned passes_count>
	static void sort_both_real( size_t length, Container<Key> &keys, Container<Data> &vals );

	/// Internal - digit-size and passes count templated real sorter for key and data.
	template <typename Data, unsigned digit_width, unsigned passes_count>
	static void sort_both_real( size_t length, Container<Key> &keys, Container<Data> &vals );

	/// Internal - digit-size and passes count templated real sorter for data only.
	template <typename Data, unsigned digit_width, unsigned passes_count>
	static void sort_data_real( size_t length, const Container<Key> &keys, Container<Data> &vals );
};

/// Converts statistics to offsets.
/// @param offsets - array of statistics to convert into offsets.
/// @param stat_length - count of digits in statistics.
/// @param length - length of array to sort (to check for triviality).
/// @returns true, if pass is trivial and can be skipped.
inline bool stat2offs( size_t *offsets, size_t stat_length, size_t length )
{
	bool trivial = false;

	// We can add here early exit on trivial==true - for no noticeable speed improvement.
	for( size_t accum = 0, digit = 0; digit < stat_length; ++digit ) {
		trivial |= ( offsets[digit] == length );  // All keys have same digit.
		std::swap( accum, offsets[digit] );
		accum += offsets[digit];
	}

	return trivial;
}

/// Recursive statistics counter for key value templated by digit size and passes count.
/// @Key - key type (uint32_t, uint64_t, uint128_t; double need conversion to uint64_t).
/// @digit_width - radixsort digit's size.
/// @pass - backward passes counter to terminate template.
template <typename Key, unsigned digit_width, unsigned pass>
struct Counter {
	enum { stat_len = 1 << digit_width, digit_mask = stat_len - 1 };

	/// Counts statistic for given key and invokes itself for next digit.
	/// @key - value to count statistics for.
	/// @digs - pointer to statistics array (current digit position)
	static void count( const Key key, size_t *digs )
	{
		++digs[key & digit_mask];
		Counter<Key, digit_width, pass - 1>::count( key >> digit_width, digs + stat_len );
	}
};

/// Terminal instantiation of Counter.
template <typename Key, unsigned digit_width>
struct Counter<Key, digit_width, 1> {
	enum { stat_len = 1 << digit_width, digit_mask = stat_len - 1 };

	static void count( const Key key, size_t *digs ) { ++digs[key & digit_mask]; }
};

template <typename Key, unsigned width>
void Sorting<Key, width>::sort_key( Key max_key, size_t length, Container<Key> &keys )
{
	if( max_key == 0 || length == 0 )
		return;

	unsigned key_width = significant_bits( max_key ), digit_width, passes_count;

	// Determine number of passes and digit width.
	if( key_width <= 33 ) {
		// Smaller keys. Sorted up to 11 bits.
		passes_count = ( key_width + 10 ) / 11;
		digit_width = std::max<unsigned>( 4, ( key_width + passes_count - 1 ) / passes_count );
	} else {
		// Wider keys. Sorted up to 10 bits.
		passes_count = ( key_width + 9 ) / 10;
		digit_width = ( key_width + passes_count - 1 ) / passes_count;
	}

	if( width == 0 )
		switch( digit_width ) {
			case 4:
				return sort_key_width<4>( length, keys, passes_count );
			case 5:
				return sort_key_width<5>( length, keys, passes_count );
			case 6:
				return sort_key_width<6>( length, keys, passes_count );
			case 7:
				return sort_key_width<7>( length, keys, passes_count );
			case 8:
				return sort_key_width<8>( length, keys, passes_count );
			case 9:
				return sort_key_width<9>( length, keys, passes_count );
			case 10:
				return sort_key_width<10>( length, keys, passes_count );
			case 11:
				return sort_key_width<11>( length, keys, passes_count );
		}
	else
		return sort_key_width<width>( length, keys, passes_count );
}

template <typename Key, unsigned width>
template <typename Data>
void Sorting<Key, width>::sort_both( Key max_key, size_t length, Container<Key> &keys,
                                     Container<Data> &vals )
{
	if( max_key == 0 || length == 0 )
		return;

	unsigned key_width = significant_bits( max_key ), digit_width, passes_count;

	// Determine number of passes and digit width.
	if( key_width <= 27 ) {
		// Smaller keys. Sorted up to 9 bits.
		passes_count = ( key_width + 8 ) / 9;
		digit_width = std::max<unsigned>( 4, ( key_width + passes_count - 1 ) / passes_count );
	} else if( key_width <= 64 ) {
		// Average keys. Sorted 8 bit exactly.
		passes_count = ( key_width + 7 ) / 8;
		digit_width = 8;
	} else {
		if( key_width > 90 && key_width <= 120 && sizeof( Data ) >= sizeof( uint128_t ) )
			// Wider key with bigger data. Sorted up to 10 bits.
			passes_count = ( key_width + 9 ) / 10;
		else  // Data isn't big enough.
			passes_count = ( key_width + 8 ) / 9;

		digit_width = ( key_width + passes_count - 1 ) / passes_count;
	}

	if( width == 0 )
		switch( digit_width ) {
			case 4:
				return sort_both_width<Data, 4>( length, keys, vals, passes_count );
			case 5:
				return sort_both_width<Data, 5>( length, keys, vals, passes_count );
			case 6:
				return sort_both_width<Data, 6>( length, keys, vals, passes_count );
			case 7:
				return sort_both_width<Data, 7>( length, keys, vals, passes_count );
			case 8:
				return sort_both_width<Data, 8>( length, keys, vals, passes_count );
			case 9:
				return sort_both_width<Data, 9>( length, keys, vals, passes_count );
			case 10:
				return sort_both_width<Data, 10>( length, keys, vals, passes_count );
		}
	else
		return sort_both_width<Data, width>( length, keys, vals, passes_count );
}

template <typename Key, unsigned width>
template <typename Data>
void Sorting<Key, width>::sort_both( Key max_key, size_t length, Container<Key> &keys,
                                     Container<Data> &vals )
{
	if( max_key == 0 || length == 0 )
		return;

	unsigned key_width = significant_bits( max_key ), digit_width, passes_count;

	// Determine number of passes and digit width.
	if( key_width <= 27 ) {
		// Smaller keys. Sorted up to 9 bits.
		passes_count = ( key_width + 8 ) / 9;
		digit_width = std::max<unsigned>( 4, ( key_width + passes_count - 1 ) / passes_count );
	} else if( key_width <= 64 ) {
		// Average keys. Sorted 8 bit exactly.
		passes_count = ( key_width + 7 ) / 8;
		digit_width = 8;
	} else {
		if( key_width > 90 && key_width <= 120 && sizeof( Data ) >= sizeof( uint128_t ) )
			// Wider key with bigger data. Sorted up to 10 bits.
			passes_count = ( key_width + 9 ) / 10;
		else  // Data isn't big enough.
			passes_count = ( key_width + 8 ) / 9;

		digit_width = ( key_width + passes_count - 1 ) / passes_count;
	}

	if( width == 0 )
		switch( digit_width ) {
			case 4:
				return sort_both_width<Data, 4>( length, keys, vals, passes_count );
			case 5:
				return sort_both_width<Data, 5>( length, keys, vals, passes_count );
			case 6:
				return sort_both_width<Data, 6>( length, keys, vals, passes_count );
			case 7:
				return sort_both_width<Data, 7>( length, keys, vals, passes_count );
			case 8:
				return sort_both_width<Data, 8>( length, keys, vals, passes_count );
			case 9:
				return sort_both_width<Data, 9>( length, keys, vals, passes_count );
			case 10:
				return sort_both_width<Data, 10>( length, keys, vals, passes_count );
		}
	else
		return sort_both_width<Data, width>( length, keys, vals, passes_count );
}

template <typename Key, unsigned width>
template <typename Data>
void Sorting<Key, width>::sort_data( Key max_key, size_t length, const Container<Key> &keys,
                                     Container<Data> &vals )
{
	if( max_key == 0 || length == 0 )
		return;

	unsigned key_width = significant_bits( max_key ), digit_width, passes_count;

	// Determine number of passes and digit width.
	if( key_width <= 27 ) {
		// Smaller keys. Sorted up to 9 bits.
		passes_count = ( key_width + 8 ) / 9;
		digit_width = std::max<unsigned>( 4, ( key_width + passes_count - 1 ) / passes_count );
	} else if( key_width <= 64 ) {
		// Average keys. Sorted 8 bit exactly.
		passes_count = ( key_width + 7 ) / 8;
		digit_width = 8;
	} else {
		if( key_width > 90 && key_width <= 120 && sizeof( Data ) >= sizeof( uint128_t ) )
			// Wider key with bigger data. Sorted up to 10 bits.
			passes_count = ( key_width + 9 ) / 10;
		else  // Data isn't big enough.
			passes_count = ( key_width + 8 ) / 9;

		digit_width = ( key_width + passes_count - 1 ) / passes_count;
	}

	if( width == 0 )
		switch( digit_width ) {
			case 4:
				return sort_data_width<Data, 4>( length, keys, vals, passes_count );
			case 5:
				return sort_data_width<Data, 5>( length, keys, vals, passes_count );
			case 6:
				return sort_data_width<Data, 6>( length, keys, vals, passes_count );
			case 7:
				return sort_data_width<Data, 7>( length, keys, vals, passes_count );
			case 8:
				return sort_data_width<Data, 8>( length, keys, vals, passes_count );
			case 9:
				return sort_data_width<Data, 9>( length, keys, vals, passes_count );
			case 10:
				return sort_data_width<Data, 10>( length, keys, vals, passes_count );
		}
	else
		return sort_data_width<Data, width>( length, keys, vals, passes_count );
}

template <typename Key, unsigned width>
template <typename Data, unsigned digit_width>
void Sorting<Key, width>::pass_both( size_t *offsets, const Key *k0, const Key *end, const Data *d0,
                                     Key *k1, Data *d1, unsigned shift )
{
	enum { digit_mask = ( 1 << digit_width ) - 1 };

#ifdef PREFETCH
	const Key *e1 = end - pf_two_cnt;

	for( size_t target_offset; k0 < e1; ++k0, ++d0 ) {
		__builtin_prefetch( offsets + ( ( k0[pf_two_cnt] >> shift ) & digit_mask ), 1, 3 );
		target_offset = offsets[( k0[pf_two] >> shift ) & digit_mask];
		__builtin_prefetch( k1 + target_offset, 1, 0 );
		__builtin_prefetch( d1 + target_offset, 1, 0 );
		target_offset = offsets[( *k0 >> shift ) & digit_mask]++;
		k1[target_offset] = *k0;
		d1[target_offset] = *d0;
	}
#endif
	for( size_t target_offset; k0 < end; ++k0, ++d0 ) {
		target_offset = offsets[( *k0 >> shift ) & digit_mask]++;
		k1[target_offset] = *k0;
		d1[target_offset] = *d0;
	}
}

template <typename Key, unsigned width>
template <unsigned digit_width>
void Sorting<Key, width>::sort_key_width( size_t length, Container<Key> &keys,
                                          unsigned passes_count )
{
	// Max number of passes is 16 (128 bit key via 8 bit digits).
	// Or less.
	switch( passes_count ) {
		case 1:
			return sort_key_real<digit_width, 1>( length, keys );
		case 2:
			return sort_key_real<digit_width, 2>( length, keys );
		case 3:
			return sort_key_real<digit_width, 3>( length, keys );
		case 4:
			return sort_key_real<digit_width, 4>( length, keys );
		case 5:
			return sort_key_real<digit_width, 5>( length, keys );
		case 6:
			return sort_key_real<digit_width, 6>( length, keys );
		case 7:
			return sort_key_real<digit_width, 7>( length, keys );
		case 8:
			return sort_key_real<digit_width, 8>( length, keys );
		case 9:
			return sort_key_real<digit_width, 9>( length, keys );
		case 10:
			return sort_key_real<digit_width, 10>( length, keys );
		case 11:
			return sort_key_real<digit_width, 11>( length, keys );
		case 12:
			return sort_key_real<digit_width, 12>( length, keys );
		case 13:
			return sort_key_real<digit_width, 13>( length, keys );
		case 14:
			return sort_key_real<digit_width, 14>( length, keys );
		case 15:
			return sort_key_real<digit_width, 15>( length, keys );
		case 16:
			return sort_key_real<digit_width, 16>( length, keys );
	}
}

template <typename Key, unsigned width>
template <typename Data, unsigned digit_width>
void Sorting<Key, width>::sort_both_width( size_t length, Container<Key> &keys,
                                           Container<Data> &vals, unsigned passes_count )
{
	// Max number of passes is 16 (128 bit key via 8 bit digits).
	// Or less.
	switch( passes_count ) {
		case 1:
			return sort_both_real<Data, digit_width, 1>( length, keys, vals );
		case 2:
			return sort_both_real<Data, digit_width, 2>( length, keys, vals );
		case 3:
			return sort_both_real<Data, digit_width, 3>( length, keys, vals );
		case 4:
			return sort_both_real<Data, digit_width, 4>( length, keys, vals );
		case 5:
			return sort_both_real<Data, digit_width, 5>( length, keys, vals );
		case 6:
			return sort_both_real<Data, digit_width, 6>( length, keys, vals );
		case 7:
			return sort_both_real<Data, digit_width, 7>( length, keys, vals );
		case 8:
			return sort_both_real<Data, digit_width, 8>( length, keys, vals );
		case 9:
			return sort_both_real<Data, digit_width, 9>( length, keys, vals );
		case 10:
			return sort_both_real<Data, digit_width, 10>( length, keys, vals );
		case 11:
			return sort_both_real<Data, digit_width, 11>( length, keys, vals );
		case 12:
			return sort_both_real<Data, digit_width, 12>( length, keys, vals );
		case 13:
			return sort_both_real<Data, digit_width, 13>( length, keys, vals );
		case 14:
			return sort_both_real<Data, digit_width, 14>( length, keys, vals );
		case 15:
			return sort_both_real<Data, digit_width, 15>( length, keys, vals );
		case 16:
			return sort_both_real<Data, digit_width, 16>( length, keys, vals );
	}
}

template <typename Key, unsigned width>
template <typename Data, unsigned digit_width>
void Sorting<Key, width>::sort_both_width( size_t length, Container<Key> &keys,
                                           Container<Data> &vals, unsigned passes_count )
{
	// Max number of passes is 16 (128 bit key via 8 bit digits).
	// Or less.
	switch( passes_count ) {
		case 1:
			return sort_both_real<Data, digit_width, 1>( length, keys, vals );
		case 2:
			return sort_both_real<Data, digit_width, 2>( length, keys, vals );
		case 3:
			return sort_both_real<Data, digit_width, 3>( length, keys, vals );
		case 4:
			return sort_both_real<Data, digit_width, 4>( length, keys, vals );
		case 5:
			return sort_both_real<Data, digit_width, 5>( length, keys, vals );
		case 6:
			return sort_both_real<Data, digit_width, 6>( length, keys, vals );
		case 7:
			return sort_both_real<Data, digit_width, 7>( length, keys, vals );
		case 8:
			return sort_both_real<Data, digit_width, 8>( length, keys, vals );
		case 9:
			return sort_both_real<Data, digit_width, 9>( length, keys, vals );
		case 10:
			return sort_both_real<Data, digit_width, 10>( length, keys, vals );
		case 11:
			return sort_both_real<Data, digit_width, 11>( length, keys, vals );
		case 12:
			return sort_both_real<Data, digit_width, 12>( length, keys, vals );
		case 13:
			return sort_both_real<Data, digit_width, 13>( length, keys, vals );
		case 14:
			return sort_both_real<Data, digit_width, 14>( length, keys, vals );
		case 15:
			return sort_both_real<Data, digit_width, 15>( length, keys, vals );
		case 16:
			return sort_both_real<Data, digit_width, 16>( length, keys, vals );
	}
}

template <typename Key, unsigned width>
template <typename Data, unsigned digit_width>
void Sorting<Key, width>::sort_data_width( size_t length, Container<Key> &keys,
                                           Container<Data> &vals, unsigned passes_count )
{
	// Max number of passes is 16 (128 bit key via 8 bit digits).
	// Or less.
	switch( passes_count ) {
		case 1:
			return sort_data_real<Data, digit_width, 1>( length, keys, vals );
		case 2:
			return sort_data_real<Data, digit_width, 2>( length, keys, vals );
		case 3:
			return sort_data_real<Data, digit_width, 3>( length, keys, vals );
		case 4:
			return sort_data_real<Data, digit_width, 4>( length, keys, vals );
		case 5:
			return sort_data_real<Data, digit_width, 5>( length, keys, vals );
		case 6:
			return sort_data_real<Data, digit_width, 6>( length, keys, vals );
		case 7:
			return sort_data_real<Data, digit_width, 7>( length, keys, vals );
		case 8:
			return sort_data_real<Data, digit_width, 8>( length, keys, vals );
		case 9:
			return sort_data_real<Data, digit_width, 9>( length, keys, vals );
		case 10:
			return sort_data_real<Data, digit_width, 10>( length, keys, vals );
		case 11:
			return sort_data_real<Data, digit_width, 11>( length, keys, vals );
		case 12:
			return sort_data_real<Data, digit_width, 12>( length, keys, vals );
		case 13:
			return sort_data_real<Data, digit_width, 13>( length, keys, vals );
		case 14:
			return sort_data_real<Data, digit_width, 14>( length, keys, vals );
		case 15:
			return sort_data_real<Data, digit_width, 15>( length, keys, vals );
		case 16:
			return sort_data_real<Data, digit_width, 16>( length, keys, vals );
	}
}

template <typename Key, unsigned width>
template <unsigned digit_width, unsigned passes_count>
void Sorting<Key, width>::sort_key_real( size_t length, Container<Key> &keys )
{
	enum { stat_length = 1 << digit_width, digit_mask = stat_length - 1 };

	Container<Key> spare( length );
	Array<size_t> stat( stat_length * passes_count );
	size_t *offsets = stat.begin();

	// Count statistics. Unless it's 8-bit key it fractional, so no reason to overoptimize.
	for( size_t row = 0; row < length; ++row )
		Counter<Key, digit_width, passes_count>::count( keys[row], offsets );

	// Do passes, skipping trivial ones.
	for( unsigned pass = 0, shift = 0; pass < passes_count; ++pass ) {
		Key *end = keys.end(), *k0 = keys.begin(), *k1 = spare.begin();

		// Convert statistics to offsets & do non-trivial passes.
		if( !stat2offs( offsets, stat_length, length ) ) {
#ifdef PREFETCH
			const Key *e1 = end - pf_solo_cnt;

			for( ; k0 < e1; ++k0 ) {
				__builtin_prefetch( offsets + ( ( k0[pf_solo_cnt] >> shift ) & digit_mask ), 1, 3 );
				__builtin_prefetch( k1 + offsets[( k0[pf_solo] >> shift ) & digit_mask], 1, 0 );
				k1[offsets[( *k0 >> shift ) & digit_mask]++] = *k0;
			}
#endif
			for( ; k0 < end; ++k0 )
				k1[offsets[( *k0 >> shift ) & digit_mask]++] = *k0;

			// Swap keys with spare buffer (now contains sorted keys).
			std::swap( keys, spare );
		}

		offsets += stat_length;
		shift += digit_width;
	}
}

template <typename Key, unsigned width>
template <typename Data, unsigned digit_width, unsigned passes_count>
void Sorting<Key, width>::sort_both_real( size_t length, Container<Key> &keys,
                                          Container<Data> &vals )
{
	enum { stat_length = 1 << digit_width, digit_mask = stat_length - 1 };

	Container<Key> spare_keys( length );
	Container<Data> spare_vals( length );
	Array<size_t> stat( stat_length * passes_count );
	size_t *offsets = stat.begin();

	// Count statistics. Unless it's 8-bit key it fractional, so no reason to overoptimize.
	for( size_t row = 0; row < length; ++row )
		Counter<Key, digit_width, passes_count>::count( keys[row], offsets );

	// Do passes, skipping trivial ones.
	for( unsigned pass = 0, shift = 0; pass < passes_count; ++pass ) {
		// Convert statistics to offsets and do non-trivial passes.
		if( !stat2offs( offsets, stat_length, length ) ) {
			pass_both<Data, digit_width>( offsets, keys.begin(), keys.end(), vals.begin(),
			                              spare_keys.begin(), spare_vals.begin(), shift );

			// Swap main buffers with spare.
			std::swap( keys, spare_keys );
			std::swap( vals, spare_vals );
		}

		offsets += stat_length;
		shift += digit_width;
	}
}

template <typename Key, unsigned width>
template <typename Data, unsigned digit_width, unsigned passes_count>
void Sorting<Key, width>::sort_both_real( size_t length, Container<Key> &keys,
                                          Container<Data> &vals )
{
	enum { stat_length = 1 << digit_width, digit_mask = stat_length - 1 };

	Container<Key> spare_keys( length );
	Container<Data> spare_vals( length );
	Array<size_t> stat( stat_length * passes_count );
	size_t *offsets = stat.begin();

	// Count statistics. Unless it's 8-bit key it fractional, so no reason to overoptimize.
	for( size_t row = 0; row < length; ++row )
		Counter<Key, digit_width, passes_count>::count( keys[row], offsets );

	// Do passes, skipping trivial ones.
	for( unsigned pass = 0, shift = 0; pass < passes_count; ++pass ) {
		// Convert statistics to offsets and do non-trivial passes.
		if( !stat2offs( offsets, stat_length, length ) ) {
			pass_both<Data, digit_width>( offsets, keys.begin(), keys.end(), &vals[0],
			                              spare_keys.begin(), &spare_vals[0], shift );

			// Swap main buffers with spare.
			std::swap( keys, spare_keys );
			std::swap( vals, spare_vals );
		}

		offsets += stat_length;
		shift += digit_width;
	}
}

template <typename Key, unsigned width>
template <typename Data, unsigned digit_width, unsigned passes_count>
void Sorting<Key, width>::sort_data_real( size_t length, const Container<Key> &keys,
                                          Container<Data> &vals )
{
	enum { stat_length = 1 << digit_width, digit_mask = stat_length - 1 };

	// Get statistics size and resize buffer accordingly.
	Array<size_t> stat( stat_length * passes_count );
	size_t *stat_ptr[passes_count];
	bool skip_passes[passes_count];
	unsigned real_passes = 0, pass_number[passes_count];

	// Count statistics. Unless it's 8-bit key it fractional, so no reason to overoptimize.
	for( size_t row = 0; row < length; ++row )
		Counter<Key, digit_width, passes_count>::count( keys[row], stat.begin() );

	// Count real (non-trivial) passes.
	for( unsigned pass = 0; pass < passes_count; ++pass ) {
		stat_ptr[pass] = &stat[pass * stat_length];
		skip_passes[pass] = stat2offs( stat_ptr[pass], stat_length, length );
		pass_number[real_passes] = pass;
		real_passes += !skip_passes[pass];
	}

	// Nothing to sort at all?
	if( real_passes == 0 )
		return;

	// For sorting by constant key we need up to two spare buffers for keys and one for values.
	Container<Data> spare_vals( length );
	Container<Key> spare_key1( ( real_passes > 1 ) ? length : 0 ),
	    spare_key2( ( real_passes > 2 ) ? length : 0 );
	Key *key_source = keys.begin(), *key_target = spare_key1.begin();
	unsigned pass = 0, shift = 0;

	// We have three cases here: single pass (no spare keys array needed), two passes (need one
	// spare keys array) or more passes (need two spare keys arrays). If real_passes equals to
	// zero - no sorting needed at all, already exited above.

	// First "entry" pass - only if we have at least two real passes.
	if( real_passes > 1 ) {
		pass_both<Data, digit_width>( stat_ptr[pass_number[pass]], key_source, key_source + length,
		                              vals.begin(), key_target, spare_vals.begin(),
		                              digit_width * pass_number[pass], digit_mask );

		// Swap main buffers with spare.
		std::swap( vals, spare_vals );
		key_source = key_target;
		key_target = spare_key2.begin();  // It's ok if that's empty.
		++pass;
	}

	// Not first or last "main loop" passes - only if we have more than two real passes.
	while( pass < ( real_passes - 1 ) ) {
		pass_both<Data, digit_width>( stat_ptr[pass_number[pass]], key_source, key_source + length,
		                              vals.begin(), key_target, spare_vals.begin(),
		                              digit_width * pass_number[pass], digit_mask );

		// Swap keys with spare buffer (now contains sorted keys).
		std::swap( key_source, key_target );
		std::swap( vals, spare_vals );
		++pass;
	}

	// Final "exiting" pass.
	size_t *offsets = stat_ptr[pass_number[pass]];
	Key *end = key_source + length, *k0 = key_source;
	Data *d0 = vals.begin(), *d1 = spare_vals.begin();

	shift = digit_width * pass_number[pass];

#ifdef PREFETCH
	const Key *e1 = end - pf_solo_cnt;

	for( ; k0 < e1; ++k0, ++d0 ) {
		__builtin_prefetch( offsets + ( ( k0[pf_solo_cnt] >> shift ) & digit_mask ), 1, 3 );
		__builtin_prefetch( d1 + offsets[( k0[pf_solo] >> shift ) & digit_mask], 1, 0 );
		d1[offsets[( *k0 >> shift ) & digit_mask]++] = *d0;
	}
#endif
	for( ; k0 < end; ++k0, ++d0 )
		d1[offsets[( *k0 >> shift ) & digit_mask]++] = *d0;

	// Swap main buffers with spare.
	std::swap( vals, spare_vals );
}
