#include <string>

#include "examples/config_shape.hh"

#include "physon.hh"
#include "physon_types.hh"


std::string load_file(std::string path) {

    std::ifstream file(path); // Open the file
    if (!file.is_open()) {
        std::cerr << "Failed to open the file: " << path << std::endl;
        throw std::runtime_error("Failed to open the file");
    }

    // Read file content into a string
    std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();


    return file_contents;
}



int main (int argc, char **argv) {

    std::cout << "Hello, Physon!" << std::endl;

    // std::string json_string = load_file("data/empty.json");
    // std::string json_string = load_file("data/ws.json");
    // std::string json_string = load_file("data/null.json");
    // std::string json_string = load_file("data/true.json");
    // std::string json_string = load_file("data/false.json");
    // std::string json_string = load_file("data/name_literals_array.json");
    // std::string json_string = load_file("data/name_literals_nested_array.json");
    // std::string json_string = load_file("data/string_array.json");
    // std::string json_string = load_file("data/object.json");
    // std::string json_string = load_file("data/object_nested.json");

    // std::string json_string = load_file("data/integer.json");
    // std::string json_string = load_file("data/integers.json");
    // std::string json_string = load_file("data/numbers.json");
    // std::string json_string = load_file("data/numbers_2.json");

    // std::string json_string = load_file("data/penpaper.json");

    // Non-valid
    // std::string json_string = load_file("data/unclosed_string.json");

    // Shapes
    std::string _json_string = load_file("data/shapes.json");


    Physon physon (_json_string);

    physon.parse();

    physon.print_original();
    physon.print_tokens();
    // print_type_sizes();
    
    // physon.build_string(physon.root_wrapper);

    std::cout << std::endl << physon.stringify() << std::endl;
    


    // Shape config
    ConfigShape shape_config {_json_string};
    std::vector<Shape>& shapes = shape_config.load_shapes();
    for(Shape shape : shapes){
        shape.print();
    }

    // bool space =  is_whitespace(' ');
    // bool tab =  is_whitespace('\t');
    // bool new_line =  is_whitespace('\n');
    // bool c_return =  is_whitespace('\r');

    return 0;
}