#include <cnt/cco.hpp>
#include <iostream>
#include <string>

int main() {
    std::string config = R"(
        window: (
            title: "CCO App",
            width: 1920,
            height: 1080,
            fullscreen: false
        )
    )";

    try {
        std::cout << "Parsing with C++ wrapper...\n";
        cnt::cco::Object obj = cnt::cco::parse(config);
        
        std::cout << "Successfully parsed object of type " << obj.type() << "\n";
        
        std::string json = obj.serialize(true);
        std::cout << "Serialized:\n" << json << "\n";
    } catch (const cnt::cco::ParseError& e) {
        std::cerr << "Caught ParseError: " << e.what() << " (Code: " << e.code() << ")\n";
        return 1;
    }

    return 0;
}
