/**
 * basic.cpp — C++ RAII 方式构建 .cco 值并序列化
 *
 * 编译: g++ -std=c++11 -I../include basic.cpp -L../target/Debug -lcco
 */
#include <cnt/cco.hpp>
#include <iostream>

int main() {
    using namespace cnt;

    /* 构建配置: ( name: "CCO", version: 1, active: true ) */
    Value cfg = Value::make_map();
    cfg.set("name",    Value::make_string("CCO"));
    cfg.set("version", Value::make_int(1));
    cfg.set("active",  Value::make_bool(true));

    /* 漂亮打印 */
    std::cout << "Config:\n" << cfg.serialize_pretty(2) << std::endl;

    /* 紧凑格式 (operator<<) */
    std::cout << "Compact: " << cfg << std::endl;

    /* ValueProxy 隐式转换 */
    const char *name   = cfg["name"];
    long long   vers   = cfg["version"];
    bool        active = cfg["active"];
    std::cout << "name = "    << name   << "\n"
              << "version = " << vers   << "\n"
              << "active = "  << active << std::endl;

    return 0;
}
