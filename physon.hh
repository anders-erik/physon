
#include <vector>
#include <iostream>
#include <fstream>
#include <string>


#define log(x) std::cout << x << std::endl;


enum class value_type {
    STRING = 0,
    NUMBER,
    OBJECT,
    ARRAY,
    TRUE,
    FALSE,
    NULL_,
};

enum class token_type {

    STRING =0,
    NUMBER,
    TRUE,
    FALSE,
    NULL_,

    LEFT_SQUARE,
    RIGHT_SQUARE,
    LEFT_CURLY,
    RIGHT_CURLY,
    COLON,
    COMMA,
};

struct Token {
    // int id;
    token_type type;
    int str_start_i;
    int str_length;

    Token(token_type type, int str_start_i, int str_length) 
        : type(type), str_start_i(str_start_i), str_length(str_length) {}
};

enum class parse_state {
    ROOT, // Looking for the root value
    ARRAY_START,
    ARRAY_END,
    ARRAY_NEW_VALUE,
};

struct json_value {
    int id;
    value_type type;

    int str_start_i;
    int str_end_i;
    std::string str_value;
};

struct Cursor {
    size_t index = 0;
};

struct Physon {
    std::string path;
    std::string content;

    std::vector<Token> tokens;

    Cursor cursor;

    json_value root_value; // the first value encountered in file. Literal or container.

    parse_state state = parse_state::ROOT; // Keeps track of the current parsing state

    void load_file(std::string path); 
    void print_string();
    
    void parse();                   // parse the content
    void print_tokens();

    void parse_true_literal();
    void parse_false_literal();
    void parse_null_literal();

    bool is_new_literal_char();
    bool is_new_container_char();
    bool is_whitespace(char ch);
    void gobble_ws();               // progress index while pointing to whitespace
};

void Physon::load_file(std::string path) {

    this->path = path;

    std::ifstream file(path); // Open the file
    if (!file.is_open()) {
        std::cerr << "Failed to open the file: " << path << std::endl;
        throw std::runtime_error("Failed to open the file");
    }

    // Read file content into a string
    std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    content = file_contents;
}

void Physon::print_string() {
    std::cout << "JSON String: " << std::endl 
    << "------" << std::endl 
    << content << std::endl 
    << "------" << std::endl;
}


void Physon::parse() {
    
    cursor.index = 0;

    while(cursor.index < content.size()){

        // Parse literals
        if(content[cursor.index] == 't')
            parse_true_literal();
        else if(content[cursor.index] == 'f')
            parse_false_literal();
        else if(content[cursor.index] == 'n')
            parse_null_literal();
        
        // Parse containers


        gobble_ws();
    }

}


void Physon::print_tokens() {
    for (const auto& token : tokens) {
        std::cout << "Token Type: " << static_cast<int>(token.type) 
                  << ", Start Index: " << token.str_start_i
                  << ", Length: " << token.str_length << std::endl;
    }
}


void Physon::parse_true_literal(){

    bool is_true_literal = (content[cursor.index + 1] == 'r' && content[cursor.index + 2] == 'u' && content[cursor.index + 3] == 'e');
    if(!is_true_literal)
        throw std::runtime_error("Invalid true-literal at index " + std::to_string(cursor.index));
    
    std::cout << "true at "  << cursor.index << std::endl;

    tokens.emplace_back(token_type::TRUE, cursor.index, 4);
    cursor.index += 4;
};

void Physon::parse_false_literal(){

    bool is_false_literal = (content[cursor.index + 1] == 'a' && content[cursor.index + 2] == 'l' && content[cursor.index + 3] == 's' && content[cursor.index + 4] == 'e');
    if(!is_false_literal)
        throw std::runtime_error("Invalid false-literal at index " + std::to_string(cursor.index));
    
    std::cout << "false at "  << cursor.index << std::endl;

    tokens.emplace_back(token_type::FALSE, cursor.index, 5);
    cursor.index += 5;
};

void Physon::parse_null_literal(){

    bool is_null_literal = (content[cursor.index + 1] == 'u' && content[cursor.index + 2] == 'l' && content[cursor.index + 3] == 'l');
    if(!is_null_literal)
        throw std::runtime_error("Invalid null-literal at index " + std::to_string(cursor.index));
    
    std::cout << "null at "  << cursor.index << std::endl;

    tokens.emplace_back(token_type::NULL_, cursor.index, 4);
    cursor.index += 4;
};


bool Physon::is_whitespace(char ch) {

    char space = '\u0020';
    char tab = '\u0009';
    char new_line = '\u000A';
    char c_return = '\u000D';

    return ch == space || ch == tab || ch == new_line || ch == c_return;

}


bool Physon::is_new_container_char(){
    char c = content[cursor.index];
    bool is_container_char = c == '{' || c == '[';
    return is_container_char;
}

bool Physon::is_new_literal_char(){
    char c = content[cursor.index];
    bool literal_name = c == 't' || c == 'f' || c == 'n';
    bool number = (c >= '0' && c <= '9') || c == '-';
    bool string = c == '"';

    return literal_name || string || number;
}

void Physon::gobble_ws() {

    while (is_whitespace(content[cursor.index]))
        cursor.index++;

}
