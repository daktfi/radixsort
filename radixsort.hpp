#pragma once

#include <type_traits>

#include "radixsort_works.hpp"

/*
 * Top-level functions to use radixsort.
 * Functions works STRICTLY in a structure-of-arrays manner.
 * No support for float, only double.
 * If you need float it's quite easy to do looking at code.
 * Argument containers will be replaced with another containers of same type with sorted data.
 * So, use only "whole" containers like std::vector.
 * No, it doesn't works on lists. Don't use lists over thousand items or sort 'em via std::sort().
 */

/// Sorts array of keys.
/// @param KeyContainer - container type (anything with .size() and ::value_type will do).
/// @param keys - container of keys. WILL BE MODIFIED (i.e., it will be ANOTHER OBJECT of same
/// type).
template <typename Key, template <class...> class Container, class... Args>
void radixsort( Container<Key> &keys )
{
	static_assert( std::is_integral<Key>::value || std::is_same<Key, double>::value,
	               "Radix sort works only for doubles and integral types" );
	return radixsort_key( keys );
}

/// Sorts arrays of keys and tied values of FIXED SIZE types.
/// Dunno what will happen to anything other.
/// @param KeyContainer - container type (anything with .size() and ::value_type will do).
/// @param LoadContainer - container type (anything with .size() and ::value_type will do).
/// @param keys - container of keys. WILL BE MODIFIED (will be ANOTHER OBJECT of same type).
/// @param load - container of keys. WILL BE MODIFIED (will be ANOTHER OBJECT of same type).
template <typename Key, typename Data, template <class...> class Container, class... Args>
void radixsort( Container<Key> &keys, Container<Data> &load )
{
	static_assert( std::is_integral<Key>::value || std::is_same<Key, double>::value,
	               "Radix sort works only for doubles and integral types" );
	return radixsort_both( keys, load );
}
