
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>



int main(){

    std::ifstream f("data/widget.json");
    nlohmann::json data = nlohmann::json::parse(f);
    bool type_check = data.is_boolean();
    std::cout << "is boolean: " << type_check << std::endl;

    // is object
    type_check = data.is_object();
    std::cout << "is object: " << type_check << std::endl;

    if(type_check){
        std::cout << "true: " << data["widget"]["debug"] << std::endl;
    }
    else{
        std::cout << "not a boolean" << std::endl;
    }
       
}
