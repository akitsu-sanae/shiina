#include <iostream>
#include "shiina.hpp"

int main() {
    std::string str = "  \n nytan \tfads\t ";
    shiina_detail::trim(str);
    std::cout << str << std::endl;
    auto s = shiina::parse(R"({"hoge": 3.141592})");
    std::cout << s << std::endl;
    std::cout << s["hoge"] << std::endl;
}
