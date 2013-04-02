#include <stddef.h>

/**
 * minimal Tuple class.
 *
 * Based on examples from
 * http://www.jot.fm/issues/issue_2008_02/article2.pdf
 */
template<typename... Elements> class tuple;
template<> class tuple<> { };
template<typename Head, typename... Tail>
class tuple<Head, Tail...> : private tuple<Tail...> {
public:
  // Construct tuple from separate arguments.
  tuple(const Head& v, const Tail&... vtail)
  : m_head(v), tuple<Tail...>(vtail...) { }

  Head& head() { return m_head; }
  const Head& head() const { return m_head; }
  tuple<Tail...>& tail() { return *this; }
  const tuple<Tail...>& tail() const { return *this; }

  typedef Head head_type;
  typedef tuple<Tail...> tail_tuple_type;
protected:
Head m_head;
};

template<typename... Ts>
tuple<Ts...> make_tuple(const Ts&... a) {
	return tuple<Ts...>(a...);
}
