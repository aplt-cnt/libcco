/**
 * full_language.cpp — 完整 CCO 语言: $typedef, $enum, $temp, 继承, 序列化
 *
 * 编译: g++ -std=c++11 -I../include full_language.cpp -L../target/Debug -lcco
 */
#include <cnt/cco.hpp>
#include <iostream>

int main() {
    using namespace cnt;

    const char *src =
        "$typedef.Url: String,\n"
        "$enum.HttpMethod = (GET, POST, PUT, DELETE),\n"
        "$temp.Endpoint: (\n"
        "    url: Url,\n"
        "    method: HttpMethod\n"
        "),\n"
        "$temp.SecureEndpoint + Endpoint: (\n"
        "    port: Integer = 443\n"
        "),\n"
        "endpoints: (\n"
        "    api:   #Endpoint(\"https://api.example.com\", GET),\n"
        "    admin: #SecureEndpoint(\"https://admin.example.com\", POST, 8443)\n"
        ")\n";

    auto pr = parse(src);
    std::cout << "Defined types: Url, HttpMethod, Endpoint, SecureEndpoint\n\n";

    /* ValueProxy 隐式转换读取嵌套值 */
    Value eps   = pr.root()["endpoints"];
    Value api   = eps["api"];
    Value admin = eps["admin"];

    std::cout << "api.url      = " << static_cast<const char *>(api["url"])
              << "\napi.method   = " << static_cast<long long>(api["method"])
              << " (GET=0)" << std::endl;

    std::cout << "admin.url    = " << static_cast<const char *>(admin["url"])
              << "\nadmin.port   = " << static_cast<long long>(admin["port"])
              << "\nadmin.method = " << static_cast<long long>(admin["method"])
              << std::endl;

    /* 迭代 Map */
    std::cout << "\nendpoints keys:";
    for (auto it = eps.begin(); it != eps.end(); ++it)
        std::cout << " " << it->first;
    std::cout << std::endl;

    /* 完整序列化 */
    std::cout << "\nSerialized:\n" << pr.serialize_pretty(2) << std::endl;

    return 0;
}
