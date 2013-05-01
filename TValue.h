#ifndef __TSTREAMING_TVALUE_H
#define __TSTREAMING_TVALUE_H

/**
 * Compile time constants, wrapped in a type.
 * Sometimes, a template class has a non-type template argument, but it
 * doesn't really care what type it is. Since there is no "any type"
 * keyword for these, the standard way to implement this is:
 *   template <typename T, T value> class Foo {
 *    ...
 *    print(value);
 *    ...
 *  }
 *
 * And instantiate like:
 *   Foo<int, 10>
 *   Foo<const char *, &x>
 * Or perhaps:
 *   Foo<decltype(&x), &x>
 *
 * However, this is a bit ugly and verbose, especially for the string
 * case. These classes try to (slightly) alleviate this by wrapping the
 * type and value in class (subclass of TValue). Of course, we now still
 * need to explicitely specify the type, inside the TValue subclass
 * name. However, we can pass it to Formatters as a single argument,
 * which makes it a bit easier to read. The above examples would now
 * become:
 *
 *   template <typename V> class Foo {
 *    ...
 *    print(V::value);
 *    ...
 *  }
 *
 * And instantiate like:
 *   Foo<TInt<10>>
 *   Foo<TStr<&x>>
 *
 * These templates are intended to be used as template arguments to
 * other templates, you should never use them to create objects.
 */
template <typename T, T v>
struct TValue {
  static constexpr T value = v;
  typedef T type;
};

/* Wrap a string. Must be the address of a global non-static variable
 * (specifically "an object with external linkage"). For example:
 * const char some_string[] = "foo";
 *
 * TStr<&some_string> ...
 */
template <const char *v>
struct TStr : public TValue<decltype(v), v> { };

/* A NULL string, defined here as a convenience. Note that this is not
 * the empty string, but a NULL pointer. */
typedef TStr<(const char*)0> TNullStr;

template <char v>
struct TChar : public TValue<decltype(v), v> { };

template <int v>
struct TInt : public TValue<decltype(v), v> { };

template <unsigned int v>
struct TUInt : public TValue<decltype(v), v> { };

template <int8_t v>
struct TInt8 : public TValue<decltype(v), v> { };

template <uint8_t v>
struct TUInt8 : public TValue<decltype(v), v> { };

template <int16_t v>
struct TInt16 : public TValue<decltype(v), v> { };

template <uint16_t v>
struct TUInt16 : public TValue<decltype(v), v> { };

#endif // __TSTREAMING_TVALUE_H

/* vim: set sw=2 sts=2 expandtab filetype=cpp: */
