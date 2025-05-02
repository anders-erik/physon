
#include <iostream>
#include <fstream>
#include <string>

std::string read_file(){
    std::string path = "data/penpaper.json";
    
    std::ifstream file(path); // Open the file
    if (!file.is_open()) {
        std::cerr << "Failed to open the file: " << path << std::endl;
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()); // Read file content
    file.close(); // Close the file
    return content;
}


enum class value_type {
    STRING = 0,
    NUMBER,
    OBJECT,
    ARRAY,
    TRUE,
    FALSE,
    NULL_,
};

struct json_value {
    int id;
    value_type type;

    int str_start_i;
    int str_end_i;
    std::string str_value;
};

struct Physon {
    std::string path;
    std::string content;

    json_value value_root;

    void parse();
};



void Physon::parse() {
    
    //
    for(int i = 0; i < content.size(); i++){
        if(content[i] == '['){
            std::cout << "new array at "  << i << std::endl;
        }
        else if (content[i] == '{'){
            std::cout << "new object at "  << i << std::endl;
        }
        else if (content[i] == '"'){
            std::cout << "string at "  << i << std::endl;
        }
    }

}

bool is_whitespace(char ch) {
    char space = '\u0020';
    char tab = '\u0009';
    char new_line = '\u000A';
    char c_return = '\u000D';
    return ch == space || ch == tab || ch == new_line || ch == c_return;
}

int main (int argc, char **argv) {

    std::cout << "Hello, World!" << std::endl;

    Physon physon;

    physon.path = "data/penpaper.json";
    physon.content = read_file();
    if (physon.content.empty()) {
        std::cerr << "Failed to read the file content." << std::endl;
        return 1;
    }

    std::string json = read_file();
    std::cout << "JSON content: " << json << std::endl;

    // bool space =  is_whitespace(' ');
    // bool tab =  is_whitespace('\t');
    // bool new_line =  is_whitespace('\n');
    // bool c_return =  is_whitespace('\r');


    physon.parse();

    return 0;
}