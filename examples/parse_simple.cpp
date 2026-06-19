/**
 * parse_simple.cpp — C++ 方式解析 .cco 字符串并访问嵌套值
 *
 * 编译: g++ -std=c++11 -I../include parse_simple.cpp -L../target/Debug -lcco
 */
#include <cnt/cco.hpp>
#include <iostream>
#include <string>

int main() {
    using namespace cnt;

    const char *src =
        "server: (\n"
        "    host: \"localhost\",\n"
        "    port: 8080,\n"
        "    ssl: false,\n"
        "    tags: [\"api\", \"v2\"]\n"
        ")\n";

    try {
        auto pr = parse(src);
        Value srv = pr.root()["server"];

        /* ValueProxy → std::string 隐式转换 */
        std::string host = srv["host"];
        std::cout << "host = " << host << std::endl;

        /* ValueProxy → long long */
        long long port = srv["port"];
        std::cout << "port = " << port << std::endl;

        /* ValueProxy → bool */
        bool ssl = srv["ssl"];
        std::cout << "ssl = " << (ssl ? "true" : "false") << std::endl;

        /* ValueProxy → const char* (explicit cast needed) */
        std::cout << "tags = "
                  << static_cast<const char *>(srv["tags"]) << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
