/**
 * array_ops.cpp — C++ 数组操作与 ArrayProxy
 *
 * 编译: g++ -std=c++11 -I../include array_ops.cpp -L../target/Debug -lcco
 */
#include <cnt/cco.hpp>
#include <iostream>
#include <cassert>

int main() {
    using namespace cnt;

    /* 构建 Array → 包装为 Value */
    Array a;
    a.add(Value::make_int(10));
    a.add(Value::make_int(20));
    a.add(Value::make_int(30));

    Value v = Value::make_array(std::move(a));
    assert(v.is_array());
    assert(v.array_size() == 3);

    /* ArrayProxy 隐式转换 */
    long long e0 = v[static_cast<size_t>(0)];
    long long e1 = v[static_cast<size_t>(1)];
    long long e2 = v[static_cast<size_t>(2)];
    std::cout << e0 << " " << e1 << " " << e2 << std::endl;

    /* ArrayProxy operator= 替换元素 */
    v[static_cast<size_t>(1)] = Value::make_int(99);
    std::cout << "after replace [1] = 99: " << v << std::endl;

    /* operator<< */
    std::cout << "array = " << v.serialize() << std::endl;

    return 0;
}
