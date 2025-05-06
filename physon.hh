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
    
    json_value root_value; // the first element encountered in file

    ParserCursor cursor;  // source string cursor
    std::vector<Token> tokens;

    
    Physon(std::string json_str) : content {json_str} {}; 


    // PRINT
    void print_original();

    /** Temporary string for stringification. */
    std::string stringify_string;
    /** returns full json structure as string */
    std::string stringify();            
    /** recursive string builder */    
    void build_string(json_value value);
    

    // QUERYING
    json_value& find(std::string key);      // for json_object - not recursive
    json_object& get_object();              // for json_object
    json_array& get_array();                // for json_array


    // PARSING
    
    void parse();                   // parse the content
    void print_tokens();

    json_value try_parse_element(); /**  parse full literal or starts new container */

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



void Physon::print_original() {
    std::cout << "JSON String: " << std::endl 
    << "------" << std::endl 
    << content << std::endl 
    << "------" << std::endl;
}

std::string Physon::stringify(){
    stringify_string = "";

    build_string(root_value);

    return stringify_string;
}

void Physon::build_string(json_value value){
    
    if(is_literal(value.type)){

        switch (value.type)
        {
        case json_value_type::NULL_:
            stringify_string.append("null");
            break;
        case json_value_type::TRUE:
            stringify_string.append("true");
            break;
        case json_value_type::FALSE:
            stringify_string.append("false");
            break;

        default:
            break;
        }

        return;
    }

    // Print array
    if(value.type == json_value_type::ARRAY){
        stringify_string.append("[");

        json_array array =  store.get_array(value.store_id);

        for(json_value array_entry : array){
            build_string(array_entry);
            stringify_string.append(", ");
        }

        // no comma after last entry
        if(array.size() > 0)
            stringify_string.erase(stringify_string.size()-2);

        stringify_string.append("]");
    }
}


void Physon::parse() {
    
    cursor.index = 0;

    // grab root (top/first/only) element
    json_value new_value = try_parse_element();
    root_value = new_value;

    // if root element is a literal type, then make sure no other elements are present
    if(is_literal(new_value.type)){

        if(cursor.index != content.size())
            throw std::runtime_error("Invalid JSON: Extra characters after root literal at index " + std::to_string(cursor.index));
        
        return;
    }

    // As we now know that the root element is a container
    cursor.container_trace.push(new_value);

    // Main Parsing loop
    while(cursor.index < content.size()){


        if(cursor.container_trace.top().type == json_value_type::ARRAY){

            // Need to make sure that we are at a value-parsable state
            new_value = try_parse_element();

            if(is_literal(new_value.type)){
                json_array & array = store.get_array(cursor.container_trace.top().store_id);
                array.push_back(new_value);
            }
            else if(is_container(new_value.type)){
                // add new array to current container
                json_array & array = store.get_array(cursor.container_trace.top().store_id);
                array.push_back(new_value);

                cursor.container_trace.push(new_value);

            }

            if(content[cursor.index] == ','){
                log("Comma at " << cursor.index);
                cursor.index++;
                continue;
            }
            

            if(content[cursor.index] == ']'){
                log("End of array at " << cursor.index);
                cursor.container_trace.pop();
                cursor.index++;
                gobble_ws();

                if(cursor.container_trace.size() == 1){
                    return;
                }
                else if(cursor.container_trace.size() > 0){

                    // keep popping if end of containers
                    while(content[cursor.index] == ']'){
                        log("End of array at " << cursor.index);
                        cursor.container_trace.pop();
                        cursor.index++;
                        gobble_ws();
                    }

                    // We just closed the root array
                    if(cursor.container_trace.size() == 0)
                        return;

                    // prepare for next value parse
                    if(content[cursor.index] == ','){
                        log("Comma at " << cursor.index);
                        cursor.index++;
                    }
                }
                
            }

        }

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
        std::string error_msg = "Invalid JSON: Unexpected first char '" + content.substr(cursor.index, 1) + "' in new value. Occured at index " + std::to_string(cursor.index);
        throw std::runtime_error(error_msg);
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
