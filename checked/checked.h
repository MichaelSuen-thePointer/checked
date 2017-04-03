#pragma once

#include <limits>
#include <type_traits>
#include <stdexcept>
#include <sstream>
#include <climits>

namespace mq
{
namespace detail
{
constexpr auto char_bit = CHAR_BIT;

struct unsigned_type
{
};

struct signed_type
{
};

struct unspecified_sign
{
};

template <class T>
struct promoted_type
{
    using type = decltype(+std::declval<T>());
};

template <class T>
using promoted_type_t = typename promoted_type<T>::type;

template <class T>
using signness_t = std::conditional_t<std::is_unsigned_v<T>,
    unsigned_type,
    std::conditional_t<std::is_signed_v<T>,
    signed_type,
    unspecified_sign>>;

template <class T>
struct always_false
{
    constexpr static bool value = false;
};

template<class T>
constexpr bool false_v = always_false<T>::value;

template <class T, class Signness = signness_t<T>>
struct arith_impl;

template <class T>
struct arith_impl<T, unsigned_type>
{
    using limit = std::numeric_limits<T>;
    static constexpr auto max = limit::max();
    static constexpr auto min = 0;
    using self = arith_impl<T, unsigned_type>;

    static T shift_left(T l, size_t r)
    {
        if (sizeof(T) * char_bit <= r)
        {
            overflow();
        }
        return l << r;
    }

    static T shift_right(T l, size_t r)
    {
        if (sizeof(T) * char_bit <= r)
        {
            overflow();
        }
        return l >> r;
    }

    static T plus(T l, T r)
    {
        if (max - l < r)
        {
            overflow();
        }
        return l + r;
    }

    static T minus(T l, T r)
    {
        if (l < r)
        {
            overflow();
        }
        return l - r;
    }

    static T multiply(T l, T r)
    {
        if (r != 0 && max / r < l)
        {
            return overflow();
        }
        return l * r;
    }

    static T divide(T l, T r)
    {
        if (r == 0)
        {
            overflow();
        }
        return l / r;
    }

    static T modulo(T l, T r)
    {
        if (r == 0)
        {
            overflow();
        }
        return l % r;
    }

    static T bitwise_and(T l, T r) noexcept
    {
        return l & r;
    }

    static T bitwise_or(T l, T r) noexcept
    {
        return l | r;
    }

    static T inverse(T l) noexcept
    {
        return ~l;
    }

    static T exclusive_or(T l, T r) noexcept
    {
        return l ^ r;
    }

    template <class U>
    static U cast_to(T r, unsigned_type) noexcept(sizeof(U) >= sizeof(T))
    { //目标如果比源的bit数多，则绝不会抛异常
        if (r > arith_impl<U>::max)
        {
            overflow();
        }
        return static_cast<U>(r);
    }

    template <class U>
    static U cast_to(T r, signed_type) noexcept(sizeof(U) * char_bit >= sizeof(T) * char_bit + 1)
    { //目标如果比源的bit数多1以上（即不考虑符号位），则绝不会抛异常，比如8位无符号整数，需要>=9位有符号整数来表示
        if (r > arith_impl<U>::max)
        {
            overflow();
        }
        return static_cast<U>(r);
    }

    template <class U>
    static U cast_to(T r) noexcept(noexcept(self::template cast_to<U>(r, signness_t<U>{})))
    {
        return self::template cast_to<U>(r, signness_t<U>{});
    }

    [[noreturn]]
    static void overflow()
    {
        throw std::overflow_error{ "Calculation overflow." };
    }
};

template <class T>
struct arith_impl<T, signed_type>
{
    using limit = std::numeric_limits<T>;
    static constexpr auto max = limit::max();
    static constexpr auto min = limit::min();
    using self = arith_impl<T, signed_type>;

    static T shift_left(T l, size_t r)
    {
        if (l < 0 || // l < 0 is UB
            sizeof(T) * char_bit >= r || // left shift overflow
            (max >> r) < l) // MSB overrides sign bit is UB
        { // Note: in C++14, 1 << 31 (assume int is 32bit) is valid, which yields INT_MIN,
          // but here it will raise overflow exception
            overflow();
        }
        return l << r;
    }

    static T shift_right(T l, size_t r)
    {
        if (sizeof(T) * char_bit >= r)
        {
            overflow();
        }
        return l >> r;
    }

    static T plus(T l, T r)
    {
        if ((l > 0 && max - l < r) || (l < 0 && min - l > r))
        {
            overflow();
        }
        return l + r;
    }

    static T minus(T l, T r)
    {
        if ((l < 0 && r > 0 && l < min + r) || (l > 0 && r < 0 && max + r < l))
        {
            overflow();
        }
        return l - r;
    }

    static T multiply(T l, T r)
    {
        if ((l > 0 && r > 0 && max / l < r) || (l < 0 && r < 0 && max / r < l) ||
            (l > 0 && r < 0 && min / l > r) || (l < 0 && r > 0 && min / r > l))
        {
            overflow();
        }
        return l * r;
    }

    static T divide(T l, T r)
    {
        if (r == 0 || (r == -1 && l == min))
        {
            overflow();
        }
        return l / r;
    }

    static T modulo(T l, T r)
    {
        if (r == 0 || (r == -1 && l == min))
        {
            overflow();
        }
        return l % r;
    }

    template <class U>
    static U cast_to(T r, unsigned_type)
    {
        if (r < 0 || static_cast<std::make_unsigned_t<T>>(r) > arith_impl<U>::max)
        {
            overflow();
        }
        return static_cast<U>(r);
    }

    template <class U>
    static U cast_to(T r, signed_type) noexcept(sizeof(U) >= sizeof(T))
    {
        if (r > arith_impl<U>::max || r < arith_impl<U>::min)
        {
            overflow();
        }
        return static_cast<U>(r);
    }

    template <class U>
    static U cast_to(T r) noexcept(noexcept(self::template cast_to<U>(r, signness_t<U>{})))
    {
        return self::template cast_to<U>(r, signness_t<U>{});
    }

    template <class U = void>
    static T inverse(T l) noexcept
    {
        static_assert(always_false<U>::value, "~ not supported on signed type.");
        return 0;
    }

    template <class U = void>
    static T bitwise_and(T l, T r) noexcept
    {
        static_assert(always_false<U>::value, "& not supported on signed type.");
        return 0;
    }

    template <class U = void>
    static T bitwise_or(T l, T r) noexcept
    {
        static_assert(always_false<U>::value, "| not supported on signed type.");
        return 0;
    }

    template <class U = void>
    static T exclusive_or(T l, T r) noexcept
    {
        static_assert(always_false<U>::value, "^ not supported on signed type.");
        return 0;
    }

    [[noreturn]]
    static void overflow()
    {
        throw std::overflow_error{ "Calculation overflow." };
    }
};

template <class T, class U>
struct is_noexcept_convertible
{
    constexpr static bool value = noexcept(arith_impl<T>::template cast_to<U>(T{}));
};

template<class T, class U>
constexpr bool is_noexcept_convertible_v = is_noexcept_convertible<T, U>::value;

template <class T, class U>
struct is_no_overflow_convertible
    : std::conditional_t<std::is_integral_v<T> && std::is_integral_v<U>,
    is_noexcept_convertible<T, U>,
    std::false_type>
{
};

template <class T, class U>
constexpr bool is_no_overflow_convertible_v = is_no_overflow_convertible<T, U>::value;

#define MAKE_RETURN_(expr) noexcept(noexcept(expr)) -> decltype(expr) { return expr; }

#define MAKE_RETURN(expr) MAKE_RETURN_ expr

template <class T, class U>
struct arith
{
    //static_assert(std::is_integral<T>::value && std::is_integral<U>::value, "Integer required.");
    static_assert(is_no_overflow_convertible<T, promoted_type_t<T>>::value, "for debug, this cannot happen");

    static auto cast(T t)
        MAKE_RETURN((arith_impl<promoted_type_t<T>>::template cast_to<U>(t)))

        static_assert(std::is_same<decltype(cast(T())), U>::value, "for debug, this cannot happen");

    using result_type = std::common_type_t<T, U>;
    using inverse_type = promoted_type_t<T>;

    using op = arith_impl<result_type>;
    using invop = arith_impl<inverse_type>;
    using shiftop = arith_impl<promoted_type_t<T>>;

    using arithT = arith<T, result_type>;
    using arithU = arith<U, result_type>;

    static auto add(T l, U r)
        MAKE_RETURN((op::plus(arithT::cast(l), arithU::cast(r))))

        static auto sub(T l, U r)
        MAKE_RETURN((op::minus(arithT::cast(l), arithU::cast(r))))

        static auto mul(T l, U r)
        MAKE_RETURN((op::multiply(arithT::cast(l), arithU::cast(r))))

        static auto div(T l, U r)
        MAKE_RETURN((op::divide(arithT::cast(l), arithU::cast(r))))

        static auto mod(T l, U r)
        MAKE_RETURN((op::modulo(arithT::cast(l), arithU::cast(r))))

        static auto shl(T l, U r)
        MAKE_RETURN((shiftop::shift_left(l, arith<U, size_t>::cast(r))))

        static auto shr(T l, U r)
        MAKE_RETURN((shiftop::shift_right(l, arith<U, size_t>::cast(r))))

        static auto bit_and(T l, U r)
        MAKE_RETURN((op::bitwise_and(arithT::cast(l), arithU::cast(r))))

        static auto bit_or(T l, U r)
        MAKE_RETURN((op::bitwise_or(arithT::cast(l), arithU::cast(r))))

        static auto bit_xor(T l, U r)
        MAKE_RETURN((op::exclusive_or(arithT::cast(l), arithU::cast(r))))

        static auto inv(T l)
        MAKE_RETURN((invop::inverse(arith<T, inverse_type>::cast(l))))

        static auto eq(T l, U r)
        MAKE_RETURN((arithT::cast(l) == arithU::cast(r)))

        static auto ne(T l, U r)
        MAKE_RETURN((!eq(l, r)))

        static auto gt(T l, U r)
        MAKE_RETURN((arithT::cast(l) > arithU::cast(r)))

        static auto le(T l, U r)
        MAKE_RETURN((!gt(l, r)))

        static auto lt(T l, U r)
        MAKE_RETURN((arithT::cast(l) < arithU::cast(r)))

        static auto ge(T l, U r)
        MAKE_RETURN((!lt(l, r)))
};

//return whether T and U are both `bool` or both not
template <class T, class U>
using all_bool_or_all_not = std::integral_constant<bool, ((std::is_same<T, U>::value && std::is_same<T, bool>::value) || (!std::is_same<T, bool>::value && !std::is_same<U, bool>::value))>;

template <class T, class U>
constexpr bool all_bool_or_all_not_v = all_bool_or_all_not<T, U>::value;
}

//simple SFINAE
#define MQ_REQUIRES(...) std::enable_if_t<__VA_ARGS__, int> = 0
#define MQ_REQUIRED(...) std::enable_if_t<__VA_ARGS__, int>
template <class T>
class checked;

template <class T, MQ_REQUIRES(std::is_integral_v<T>)>
auto make_checked(T t) noexcept->checked<T>;

template <class T>
class checked
{
private:
    T _val;
public:
    constexpr checked() noexcept = default;
    checked(const checked&) noexcept = default;
    checked(checked&&) noexcept = default;
    checked& operator=(const checked&) noexcept = default;
    checked& operator=(checked&&) noexcept = default;

    constexpr checked(T v) noexcept
        : _val(v)
    {
    }

    template <class U, MQ_REQUIRES(std::is_integral_v<U> && detail::is_no_overflow_convertible_v<U, T>)>
    checked(U u) noexcept
        : _val(u)
    {
    }

    template <class U, MQ_REQUIRES(!detail::is_no_overflow_convertible_v<U, T>)>
    explicit checked(U u) noexcept(detail::is_noexcept_convertible_v<U, T>)
        : _val(detail::arith<U, T>::cast(u))
    {
    }

    template <class U, MQ_REQUIRES(!std::is_same_v<T, U> && detail::is_no_overflow_convertible_v<U, T>)>
    checked(checked<U> u) noexcept
        : _val(static_cast<T>(u))
    {
    }

    template <class U, MQ_REQUIRES(!detail::is_no_overflow_convertible_v<U, T>)>
    explicit checked(checked<U> u) noexcept(detail::is_noexcept_convertible_v<U, T>)
        : checked(u.template cast_to<T>()) //redirect to the ctor above
    {
    }

    template <class U, MQ_REQUIRES(std::is_integral_v<U>)>
    checked& operator=(U u) noexcept(detail::is_noexcept_convertible_v<U, T>)
    {
        _val = detail::arith<U, T>::cast(u);
        return *this;
    }

    template <class U, MQ_REQUIRES(std::is_integral_v<U>)>
    checked& operator=(checked<U> u) noexcept(detail::is_noexcept_convertible_v<U, T>)
    {
        _val = u.template cast_to<T>();
        return *this;
    }

    template <class U, MQ_REQUIRES(detail::is_no_overflow_convertible_v<T, U> && detail::all_bool_or_all_not_v<T, U>)>
    operator U() const noexcept
    {
        return _val;
    }

    template <class U, MQ_REQUIRES(!std::is_same_v<T, U> && std::is_integral_v<U> && detail::all_bool_or_all_not_v<T, U>)>
    explicit operator U() const noexcept(detail::is_noexcept_convertible_v<U, T>)
    {
        static_assert(detail::always_false<U>::value, "please use `static_check_cast` for protentially overflow cast");
        return 0;
    }

    template <class U, class Ty = T, MQ_REQUIRES(std::is_same_v<U, bool> && !std::is_same_v<Ty, bool>)>
    explicit operator U() const
    {
        static_assert(detail::always_false<U>::value, "non-bool types cannot be convert to bool, please use `operator==`");
        return false;
    }

    template <class U>
    checked<U> cast_to() const noexcept(detail::is_noexcept_convertible_v<U, T>)
    {
        return checked<U>{detail::arith<T, U>::cast(_val)};
    }
};

template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>
auto operator+(checked<T> t, checked<U> u)
MAKE_RETURN((make_checked(detail::arith<T, U>::add(static_cast<T>(t), static_cast<U>(u)))))

template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>
auto operator+(T t, checked<U> u)
MAKE_RETURN((make_checked(detail::arith<T, U>::add(t, static_cast<U>(u)))))

template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>
auto operator+(checked<T> t, U u)
MAKE_RETURN((make_checked(detail::arith<T, U>::add(static_cast<T>(t), u))))

template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>
auto operator+=(checked<T>& t, checked<U> u)
MAKE_RETURN((t = t + u))

template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>
auto operator+=(checked<T>& t, U u)
MAKE_RETURN((t = t + u))

//the macro simply generates the code above
#define MAKE_ARITH_OPERATOR(OP, FUNC)                                                           \
template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>        \
auto operator OP(checked<T> t, checked<U> u)                                                    \
MAKE_RETURN((make_checked(detail::arith<T, U>::FUNC(static_cast<T>(t), static_cast<U>(u)))))    \
                                                                                                \
template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>        \
auto operator OP(T t, checked<U> u)                                                             \
MAKE_RETURN((make_checked(detail::arith<T, U>::FUNC(t, static_cast<U>(u)))))                    \
                                                                                                \
template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>        \
auto operator OP(checked<T> t, U u)                                                             \
MAKE_RETURN((make_checked(detail::arith<T, U>::FUNC(static_cast<T>(t), u))))                    \
                                                                                                \
template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>        \
auto operator OP=(checked<T>& t, checked<U> u)                                                  \
MAKE_RETURN((t = t OP u))                                                                       \
                                                                                                \
template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>        \
auto operator OP=(checked<T>& t, U u)                                                           \
MAKE_RETURN((t = t OP u))

MAKE_ARITH_OPERATOR(-, sub)
MAKE_ARITH_OPERATOR(*, mul)
MAKE_ARITH_OPERATOR(/ , div)
MAKE_ARITH_OPERATOR(%, mod)
MAKE_ARITH_OPERATOR(<< , shl)
MAKE_ARITH_OPERATOR(>> , shr)
MAKE_ARITH_OPERATOR(^, bit_xor)
MAKE_ARITH_OPERATOR(&, bit_and)
MAKE_ARITH_OPERATOR(| , bit_or)

#undef MAKE_ARITH_OPERATOR

template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>
auto operator==(checked<T> t, checked<U> u)
MAKE_RETURN((make_checked(detail::arith<T, U>::eq(static_cast<T>(t), static_cast<U>(u)))))

template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>
auto operator==(T t, checked<U> u)
MAKE_RETURN((make_checked(detail::arith<T, U>::eq(t, static_cast<U>(u)))))

template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>
auto operator==(checked<T> t, U u)
MAKE_RETURN((make_checked(detail::arith<T, U>::eq(static_cast<T>(t), u))))

#define MAKE_COMPARASON_OPERATOR(OP, FUNC)                                                      \
template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>        \
auto operator OP(checked<T> t, checked<U> u)                                                    \
MAKE_RETURN((make_checked(detail::arith<T, U>::FUNC(static_cast<T>(t), static_cast<U>(u)))))    \
                                                                                                \
template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>        \
auto operator OP(T t, checked<U> u)                                                             \
MAKE_RETURN((make_checked(detail::arith<T, U>::FUNC(t, static_cast<U>(u)))))                    \
                                                                                                \
template <class T, class U, MQ_REQUIRES(std::is_integral_v<T> && std::is_integral_v<U>)>        \
auto operator OP(checked<T> t, U u)                                                             \
MAKE_RETURN((make_checked(detail::arith<T, U>::FUNC(static_cast<T>(t), u))))

MAKE_COMPARASON_OPERATOR(!= , ne)
MAKE_COMPARASON_OPERATOR(>= , ge)
MAKE_COMPARASON_OPERATOR(> , gt)
MAKE_COMPARASON_OPERATOR(<= , le)
MAKE_COMPARASON_OPERATOR(< , lt)

#undef MAKE_COMPARASON_OPERATOR

    template <class T, MQ_REQUIRED(std::is_integral_v<T>)>
auto make_checked(T t) noexcept -> checked<T>
{
    return checked<T>(t);
}

template <class T>
std::ostream& operator<<(std::ostream& os, checked<T> t)
{
    return os << t.get();
}

template <class T, class U, MQ_REQUIRES(!std::is_same_v<T, U> && std::is_integral_v<U> && detail::all_bool_or_all_not_v<T, U>)>
auto checked_cast(checked<U> u)
MAKE_RETURN((checked<T>{static_cast<U>(u)}))

template <class T, class U, MQ_REQUIRES(!(detail::is_no_overflow_convertible_v<T, U> && detail::all_bool_or_all_not_v<T, U>))>
checked<T> checked_cast(checked<U>)
{
    static_assert(detail::always_false<T>::value, "please use implicit cast for conversion with no overflow");
    return 0;
}

template <class T, class U, MQ_REQUIRES(std::is_same_v<U, bool> && !std::is_same_v<U, bool>)>
checked<T> checked_cast(checked<U>)
{
    static_assert(detail::always_false<T>::value, "non-bool cannot convert to bool");
    return 0;
}

#undef MAKE_RETURN
#undef MAKE_RETURN_
#undef MQ_REQUIRES
#undef MQ_REQUIRED
}
