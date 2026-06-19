/**
 * comparison.cpp — 深层比较与拷贝语义
 *
 * 编译: g++ -std=c++11 -I../include comparison.cpp -L../target/Debug -lcco
 */
#include <cnt/cco.hpp>
#include <iostream>
#include <cassert>

int main() {
    using namespace cnt;

    /* == 深层比较 */
    Value a = Value::make_map();
    a.set("x", Value::make_int(1));
    a.set("y", Value::make_string("hello"));

    Value b = Value::make_map();
    b.set("x", Value::make_int(1));
    b.set("y", Value::make_string("hello"));

    assert(a == b);
    std::cout << "a == b: true" << std::endl;

    b.set("x", Value::make_int(2));
    assert(a != b);
    std::cout << "a != b: true" << std::endl;

    /* 深拷贝独立 */
    Value c = a;
    c.set("z", Value::make_bool(true));
    assert(a.map_size() == 2);
    assert(c.map_size() == 3);
    std::cout << "deep copy OK" << std::endl;

    return 0;
}
