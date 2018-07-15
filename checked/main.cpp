#include "checked.h"
#include <chrono>
#include <iostream>
#include <memory>
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

    auto _b1 = c1 > c2;
    auto _b2 = c1 >= c2;
    auto _b3 = c1 <= c2;
    auto _b4 = c1 < c2;
    auto _b5 = c1 == c2;
    auto _b6 = c1 != c2;

    auto _t1 = c1++;
    auto _t2 = c1--;
    auto _t3 = ++c1;
    auto _t4 = --c1;

    auto _t5 = +c1;
    auto _t6 = -c1;

    checked<bool> bo = true;
    if (bo)
    { /*ok*/
    }

    checked<unsigned> ii{ 255 };
    auto jj = checked_cast<int>(ii);

    std::chrono::duration<checked<int>> cd1{10};
    std::chrono::duration<checked<long>> cd2{20};

    auto _y = false ? a : b;

    auto _z = false ? checked<int>{1} : checked<long>{ 2 };

    auto _w = false ? 1l : checked<long>{1};
    std::cout << typeid(_w).name();

    checked<long> chkloong = 1l;


    checked<long> chklong{1};
    checked<int> _tt{chklong};

    std::common_type_t<checked<int>, checked<long>> _cmntype;

    std::cout << typeid(_cmntype).name();
    std::common_type_t<checked<long>, char> intval;


    //cd1++;
    //cd1--;
    //--cd1;
    //++cd1;
    //+cd1;
    //-cd1;
    //
    //cd1 += cd2;
    //
    //auto cd3 = cd1 + cd2;
    
    std::cout << std::endl;
}
