#ifndef __TSTREAMING_TYPES_H
#define __TSTREAMING_TYPES_H

/*
 * Various type traits and conditions.
 *
 * These are basic implementations of some type traits as defined in
 * C++11 in <type_traits>.
 */

// Note that __is_base_of might not be available on all compilers...
template<typename B, typename D>
struct is_base_of
{ static const bool value = __is_base_of(B, D); };

template<bool, typename T = void>
struct enable_if
{ };

template<typename T>
struct enable_if<true, T>
{ typedef T type; };

#endif // __TSTREAMING_TYPES_H
