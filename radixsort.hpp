#pragma once

#include <type_traits>

#include "radixsort_works.hpp"

/*
 * Top-level functions to use radixsort.
 * Functions works STRICTLY in a structure-of-arrays manner.
 * Probably no support for signed integers, but I may fix that later.
 * No support for float, only double.
 * If you need float it's quite easy to do looking at code.
 * Argument containers will be replaced with another containers of same type with sorted data.
 * So, use only "whole" containers like std::vector.
 * No, it doesn't works on lists. Sort 'em via std::sort().
 * sort_both() and sort_data() expect values to be of trivially copyable types. No check, though.
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
    Sorting<Key, Container>::sort_key( 0, keys );
}

/// Sorts arrays of keys and tied values of trivially copyable types.
/// @param KeyContainer - container type (anything with .size() and ::value_type will do).
/// @param LoadContainer - container type (anything with .size() and ::value_type will do).
/// @param keys - container of keys. WILL BE MODIFIED (will be ANOTHER OBJECT of same type).
/// @param load - container of keys. WILL BE MODIFIED (will be ANOTHER OBJECT of same type).
template <typename Key, typename Data, template <class...> class Container, class... Args>
void radixsort( Container<Key> &keys, Container<Data> &load )
{
	static_assert( std::is_integral<Key>::value || std::is_same<Key, double>::value,
	               "Radix sort works only for doubles and integral types" );
	Sorting<Key, Container>::sort_both( 0, keys, load );
}

/// Sorts only values of trivially copyable types by key array (will remain unsorted).
/// @param KeyContainer - container type (anything with .size() and ::value_type will do).
/// @param LoadContainer - container type (anything with .size() and ::value_type will do).
/// @param keys - container of keys. WILL BE MODIFIED (will be ANOTHER OBJECT of same type).
/// @param load - container of keys. WILL BE MODIFIED (will be ANOTHER OBJECT of same type).
template <typename Key, typename Data, template <class...> class Container, class... Args>
void radixsort_data( Container<const Key> &keys, Container<Data> &load )
{
    static_assert( std::is_integral<Key>::value || std::is_same<Key, double>::value,
                  "Radix sort works only for doubles and integral types" );
    Sorting<Key, Container>::sort_data( 0, keys, load );
}
