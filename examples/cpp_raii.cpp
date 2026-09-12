#include <cnt/cco.hpp>
#include <iostream>
#include <string>

/*
 * Demonstrates the C++ wrapper API which provides
 * exception-based error handling and RAII memory management.
 */

int main() {
    std::string config = R"(
        window: {
            title: "CCO App",
            width: 1920,
            height: 1080,
            fullscreen: false
        }
    )";

    try {
        std::cout << "Parsing with C++ wrapper...\n";
        cnt::cco::Object obj = cnt::cco::parse(config);
        
        std::cout << "Successfully parsed object of type " << obj.type() << "\n";
        
        // In C++, serialization takes care of releasing the temporary string
        std::string json = obj.serialize(true);
        std::cout << "Serialized:\n" << json << "\n";
        
        // Memory is automatically released when 'obj' goes out of scope!
    } catch (const cnt::cco::ParseError& e) {
        std::cerr << "Caught ParseError: " << e.what() << " (Code: " << e.code() << ")\n";
        return 1;
    }

    return 0;
}
