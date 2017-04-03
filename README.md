# checked
checked is a safe integer library in C++17 providing overflow and truncation check, and of course natural usage.

If there is an overflow, `std::overflow_error` will be thrown.

# When to use it
* When you need overflow check in your code, and you don't want to place `if` statements everywhere.
* When you are dealing with libraries which do arithmetic operation, but do not support overflow check in their code.(But you want it.) e.g.: `chrono::duration`, `Eigen::Matrix`.

# Usage
This library is header only, and requires C++17 support for it uses variable template feature in C++17.
The library passes compilation under MSVC19.10(VS2017), clang 3.9 and gcc 7.0.1

## Defining Checked Integer
`checked<T>` template object will help you do all the checking things in arithmetic operations. Simply it support all kinds of integral types, as well as `bool`.

There are additional constraints on `checked<bool>` and signed type `T` for safety, see below.

### Implicit(value) construct are allowed when there is no potential overflow
```c++
checked<int> a = 1; //allowed, for converting 1 to `int` will not overflow
checked<int64_t> b = 1u; //allowed, for `unsigned` can convert to `int64_t` without overflow
/*checked<int> a = 1u;*/ //not allowed, there may be an overflow error when converting `unsigned` to `int`
checked<int> a{1u}; //must use explicit construct, throws exception when there is overflow
```

## Do arithmetic operations as usual
* If there is overflow, `overflow_error` will be thrown
* Operation on `checked` yields result of type `checked`, so the overflow check is performed on the rest of the expression.
* Integer promotion rule and usual arithmetic conversion rule are followed by `checked`
```c++
checked<int> a = 10;
checked<long long> b = 11;
auto c = a + b;
auto d = a + 1;

auto e = 1 + a;
auto f = 1u - a;
auto g = 1ll * a;
auto h = 1ull / a;

auto i = a + 1;
auto j = a - 1u;
auto k = a * 1ll;
auto l = a / 1ull;

a += 1;
a -= 1u;
a *= 1l;
a /= 1ul;
a %= 1ull;
```

## But bitwise operation on signed type is disabled
Use unsigned type for `&`, `|`, `~`, and `^` insead.

## Do implicit conversion when it is safe
You can use `checked` object to initialize plain integral type only when it is safety.
```c++
void foo(uint64_t v) {}
int main() {
    checked<int> a = 10;
    int i32 = a;
    long li = a;
    long long lli = a;
    foo(a);
    // both allowed, there is no overflow on these operations
}
```

## If you must do potentially-overflow conversion
Use `checked_cast`
```c++
void foo(int8_t v) {}
int main() {
    checked<int> a = 10;
    short sh = checked_cast<short>(a);
    bar(checked_cast<int8_t>(a));
}
```
by the way, you cannot use `checked_cast` on safe conversion, go use implicit conversion instead
## Of course, comparasion is supported
which yields `bool` rather than `checked<bool>`
```c++
checked<int> c1 = 10;
checked<int> c2 = 20;

c1 > c2; c1 > 20;
c1 >= c2; c1 >= 20;
c1 <= c2; c1 <= 20;
c1 < c2; c1 < 20;
c1 == c2; c1 == 20;
c1 != c2; c1 != 20;
```

## But checked integers can no longer implicit convert to `bool`
```c++
checked<int> a = 10;
/* if (a) */ //yields compile time error
if (a != 0) //ok
/* bool boo = a; */ //yields compile time error
```

## `checked<bool>` can be used in `if` statements directly
```c++
checked<bool> b = true;
if (b) { /*ok*/ }
```

## Check the `noexcept`-ness
When the operation will never cause overflow, such as bitwise operation, casting to a bigger type or implicit construction. The operation is `noexcept`

# Details
## Object Model
* All `checked<T>` is guaranteed to be POD type, which means `sizeof(T) == sizeof(checked<T>)`, and you can manipulate its binary representation via `reinterpret_cast` or whatever something.
* Default construction, copy/move construction/assignment between two object with same type is trivial and `noexcept`.

# Note
* Since C++14, 1 << 31 is specially allowed, whichs yields INT_MIN, but in this library, it will cause an overflow excption.
* Left hand side operation are not checked, for example:
```c++
  1 + 1 + 3 + checked<int>{3};
//^^^^^^^^^
//This part of the operation is not checked if they overflows.
```