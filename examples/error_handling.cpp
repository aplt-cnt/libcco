/**
 * error_handling.cpp — 异常与诊断信息
 *
 * 编译: g++ -std=c++11 -I../include error_handling.cpp -L../target/Debug -lcco
 */
#include <cnt/cco.hpp>
#include <iostream>

int main() {
    using namespace cnt;

    /* 解析错误 → 异常 */
    try {
        parse("invalid syntax {{{");
    } catch (const std::runtime_error &e) {
        std::cout << "Caught: " << e.what() << std::endl;
    }

    /* 获取诊断信息 */
    auto diags = get_diagnostics();
    for (const auto &d : diags) {
        std::cout << (d.is_error ? "error" : "warning") << ": "
                  << d.message << " (" << d.line << ":" << d.col << ")"
                  << std::endl;
    }
    clear_diagnostics();

    /* 类型不匹配 → logic_error */
    try {
        Value v = Value::make_int(42);
        bool b = v;  // Int 不可隐式转 bool
        (void)b;
    } catch (const std::logic_error &e) {
        std::cout << "Type mismatch: " << e.what() << std::endl;
    }

    /* key 不存在 → out_of_range */
    try {
        Value m = Value::make_map();
        Value x = m["nonexistent"];
        (void)x;
    } catch (const std::out_of_range &e) {
        std::cout << "Key not found: " << e.what() << std::endl;
    }

    return 0;
}
