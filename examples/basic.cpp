#include <cnt/cco.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.cco>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    try {
        cnt::cco::Object obj = cnt::cco::parse(content);
        std::cout << "Parsed successfully!\n";
        std::cout << "Type ID: " << obj.type() << "\n";
        std::cout << "Serialized:\n" << obj.serialize(true) << "\n";
    } catch (const cnt::cco::ParseError& e) {
        std::cerr << "Parse error (" << e.code() << "): " << e.what() << "\n";
        return 1;
    }

    return 0;
}
