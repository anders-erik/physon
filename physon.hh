#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include "physon_types.hh"


#define log(x) std::cout << x << std::endl;



struct Physon {
    std::string content;

    json_store store; // store for all json data
    
    json_element root_container; // the first element encountered in file

    ParserCursor cursor;  // source string cursor
    std::vector<Token> tokens;

    
    Physon(std::string json_str) : content {json_str} {}; 

    void print_string();
    
    void parse();                   // parse the content
    void print_tokens();

    json_value try_parse_element();

    json_value parse_true_literal();
    json_value parse_false_literal();
    json_value parse_null_literal();

    bool is_new_literal_char();
    bool is_new_container_char();
    bool is_whitespace(char ch);
    bool is_literal(json_value_type type);
    bool is_container(json_value_type type);
    void gobble_ws();               // progress index while pointing to whitespace
};



void Physon::print_string() {
    std::cout << "JSON String: " << std::endl 
    << "------" << std::endl 
    << content << std::endl 
    << "------" << std::endl;
}


void Physon::parse() {
    
    cursor.index = 0;
    // Indicates that the root value is a literal
    root_container.value.type = json_value_type::NULL_;

    // grab root element
    json_value new_value = try_parse_element();

    // if root element is a literal type, then make sure no other elements are present
    if(is_literal(new_value.type)){

        if(cursor.index != content.size())
            throw std::runtime_error("Invalid JSON: Extra characters after root literal at index " + std::to_string(cursor.index));

        return;
    }

    // As we now know that the root element is a container
    root_container.value = new_value;
    cursor.current_container.value = new_value;


    while(cursor.index < content.size()){

        // Need to make sure that we are at a value-parsable state
        new_value = try_parse_element();


        if(cursor.current_container.value.type == json_value_type::ARRAY){

            if(is_literal(new_value.type)){
                json_array & array = store.get_array(cursor.current_container.value.store_id);
                array.push_back(new_value);
                cursor.current_element.value = new_value;
            }
            else if(is_container(new_value.type)){

            }

            if(content[cursor.index] == ','){
                log("Comma at " << cursor.index);
                cursor.index++;
                continue;
            }
            

            if(content[cursor.index] == ']'){
                log("End of array at " << cursor.index);
                cursor.index++;

                if(cursor.current_container == root_container){
                    return; 
                }
                
                // go to parent

            }

        }

        if(new_value.type == json_value_type::NULL_){
            
        }
        
        // Parse containers


        // gobble_ws();
    }

}


void Physon::print_tokens() {
    for (const auto& token : tokens) {
        std::cout << "Token Type: " << static_cast<int>(token.type) 
                  << ", Start Index: " << token.str_start_i
                  << ", Length: " << token.str_length << std::endl;
    }
}

bool Physon::is_literal(json_value_type type){
    return  type == json_value_type::NULL_ ||
            type == json_value_type::TRUE ||
            type == json_value_type::FALSE ||
            type == json_value_type::NUMBER ||
            type == json_value_type::STRING;
}
bool Physon::is_container(json_value_type type){
    return  type == json_value_type::ARRAY ||
            type == json_value_type::OBJECT;
}

json_value Physon::try_parse_element(){

    gobble_ws();

    json_value new_value;

    // LITERALS
    if(content[cursor.index] == 't'){
        new_value = parse_true_literal();
    }
    else if(content[cursor.index] == 'f'){
        new_value = parse_false_literal();
    }
    else if(content[cursor.index] == 'n'){
        new_value = parse_null_literal();
    }
    
    // CONTAINERS
    else if(content[cursor.index] == '['){
        int new_array_id = store.add_array();
        new_value.type = json_value_type::ARRAY;
        new_value.store_id = new_array_id;
        cursor.index++;
    }
    else if(content[cursor.index] == '{'){
        // new_value.type = json_value_type::OBJECT;
        // new_value.type_id = 0;
        // cursor.index++;
    }
    else {
        throw std::runtime_error("Invalid JSON: Unexpected first char in new value. Occured at index " + std::to_string(cursor.index));
    }

    gobble_ws();

    return new_value;
};

json_value Physon::parse_true_literal(){

    bool is_true_literal = (content[cursor.index + 1] == 'r' && content[cursor.index + 2] == 'u' && content[cursor.index + 3] == 'e');
    if(!is_true_literal)
        throw std::runtime_error("Invalid true-literal at index " + std::to_string(cursor.index));
    
    std::cout << "true at "  << cursor.index << std::endl;

    tokens.emplace_back(token_type::TRUE, cursor.index, 4);
    cursor.index += 4;

    json_value new_value;
    new_value.type = json_value_type::TRUE;
    new_value.store_id = 0;

    return new_value;
};

json_value Physon::parse_false_literal(){

    bool is_false_literal = (content[cursor.index + 1] == 'a' && content[cursor.index + 2] == 'l' && content[cursor.index + 3] == 's' && content[cursor.index + 4] == 'e');
    if(!is_false_literal)
        throw std::runtime_error("Invalid false-literal at index " + std::to_string(cursor.index));
    
    std::cout << "false at "  << cursor.index << std::endl;

    tokens.emplace_back(token_type::FALSE, cursor.index, 5);
    cursor.index += 5;

    json_value new_value;
    new_value.type = json_value_type::FALSE;
    new_value.store_id = 0;
    return new_value;
};

json_value Physon::parse_null_literal(){

    bool is_null_literal = (content[cursor.index + 1] == 'u' && content[cursor.index + 2] == 'l' && content[cursor.index + 3] == 'l');
    if(!is_null_literal)
        throw std::runtime_error("Invalid null-literal at index " + std::to_string(cursor.index));
    
    std::cout << "null at "  << cursor.index << std::endl;

    tokens.emplace_back(token_type::NULL_, cursor.index, 4);
    cursor.index += 4;

    json_value new_value;
    new_value.type = json_value_type::NULL_;
    new_value.store_id = 0;
    return new_value;
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
