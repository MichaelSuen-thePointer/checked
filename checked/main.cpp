#include "checked.h"
#include <chrono>
using namespace mq;
void foo(int v)
{
}
void bar(short d)
{
}
int main()
{
    checked<int> a = 10;
    checked<long long> b = 11;
    auto c = a + b;
    auto d = a + 1;

    auto e = 1 + a;
    try
    {   
        auto f = 1u - a;
    } catch(std::overflow_error&)
    {
        
    }
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

    a *= 1;
    a /= 1;
    a %= 1;


    short sh = checked_cast<short>(a);
    int in = a;
    long lo = a;
    long long llo = a;
    foo(a);
    bar(checked_cast<short>(a));

    checked<int> c1 = 10;
    checked<int> c2 = 20;

    c1 > c2;
    c1 >= c2;
    c1 <= c2;
    c1 < c2;
    c1 == c2;
    c1 != c2;

    checked<bool> bo = true;
    if (bo)
    { /*ok*/
    }
}
