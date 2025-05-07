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

    JSON_PARSE_STATE state;
    
    void parse();                   /** Parse the content string */

    void print_tokens();

    void enter_array();
    void close_array();

    json_value parse_literal(); /** Parse literal with cursor confirmed pointing at first char in literal */
    json_value parse_true_literal();    /** confirm "true" chars and progress index. */
    json_value parse_false_literal();   /** confirm "false" chars and progress index. */
    json_value parse_null_literal();    /** confirm "null" chars and progress index. */

    bool is_new_value_char();   /** is_new_literal_char() U is_new_container_char() */
    bool is_new_literal_char();
    bool is_new_container_char();

    bool is_whitespace(char ch);
    void gobble_ws();               /** progress index until cursor not pointing at whitespace */

    bool is_literal(json_value_type type);
    bool is_container(json_value_type type);
    
    void json_error(std::string error_msg);
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
    state = JSON_PARSE_STATE::BEFORE_ROOT;

    gobble_ws();
    
    // NO VALUES CHECK
    if(cursor.index == content.size())
        json_error("Error: No valid JSON values.");
    

    // SINGLE LITERAL CHECK
    if(is_new_literal_char()){
        root_value = parse_literal();

        if(cursor.index != content.size())
            json_error("Invalid JSON: Extra characters after root literal at index " + std::to_string(cursor.index));
        
        return;
    }

    // MAKE SURE CONTAINER
    if(!is_new_container_char())
        json_error("Invalid JSON: No valid root element. Detected at index = " + std::to_string(cursor.index));

    
    // CONFIRMED CONTAINER AS ROOT VALUE

    // SET ROOT CONTAINER
    if(content[cursor.index] == '[')
        enter_array();
    

    json_value tmp_value;
    
    // Main Parsing loop
    while(cursor.index < content.size()){


        if(state == JSON_PARSE_STATE::NEW_ARRAY){
            gobble_ws();

            if(content[cursor.index] == ']'){
                close_array();
            }
            else if(is_new_value_char()){
                state = JSON_PARSE_STATE::NEW_VALUE;
            }
            else {
                json_error("Error: not a valid character during state JSON_PARSE_STATE::NEW_ARRAY. Content index = " + std::to_string(cursor.index));
            }
        }
        else if(state == JSON_PARSE_STATE::END_VALUE){
            gobble_ws();

            if(content[cursor.index] == ','){
                cursor.index++;
                gobble_ws();
                state = JSON_PARSE_STATE::NEW_VALUE;
            }
            else if(content[cursor.index] == ']'){
                close_array();
            }
        
        }
        else if(state == JSON_PARSE_STATE::NEW_VALUE){
            gobble_ws();

            if(is_new_literal_char()){
                tmp_value = parse_literal();
                
                // Add literal to current array
                json_array& trace_top = store.get_array(cursor.container_trace.top().store_id);
                trace_top.push_back(tmp_value);
            }
            else if(is_new_container_char()){
                enter_array();
            }
            else {
                json_error("Error: not a valid character during state JSON_PARSE_STATE::NEW_VALUE. Content index = " + std::to_string(cursor.index));
            }
        
        }
        else if (state == JSON_PARSE_STATE::END_OF_ROOT){
            gobble_ws();
            if(cursor.index != content.size())
                json_error("Invalid JSON: Extra characters after root container. Found at index " + std::to_string(cursor.index) + ".");

            return;
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


json_value Physon::parse_literal(){

    gobble_ws();

    json_value new_value;

    char current_char = content[cursor.index];

    if(current_char == 't'){
        new_value = parse_true_literal();
        state = JSON_PARSE_STATE::END_VALUE;
    }
    else if(current_char == 'f'){
        new_value = parse_false_literal();
        state = JSON_PARSE_STATE::END_VALUE;
    }
    else if(current_char == 'n'){
        new_value = parse_null_literal();
        state = JSON_PARSE_STATE::END_VALUE;
    }
    else if(current_char == '"'){
        // STRING
        state = JSON_PARSE_STATE::END_VALUE;
    }
    else if(current_char == '-' || (current_char >= '0' && current_char <= '9' )){
        // NUMBER
        state = JSON_PARSE_STATE::END_VALUE;
    }
    else {
        json_error("Invalid JSON: Expected beginning of literal. Instead '" + content.substr(cursor.index, 1) + "' at first literal character. Occured at global index " + std::to_string(cursor.index));
    }

    gobble_ws();

    return new_value;

}


void Physon::enter_array(){

    gobble_ws();

    if(content[cursor.index] != '[')
        json_error("Invalid JSON: Unexpected first char '" + content.substr(cursor.index, 1) + "' in new value. Occured at index " + std::to_string(cursor.index));


    cursor.index++;
    gobble_ws();


    json_value new_array;
    int new_array_id = store.add_array();
    new_array.type = json_value_type::ARRAY;
    new_array.store_id = new_array_id;

    if(state == JSON_PARSE_STATE::BEFORE_ROOT){
        root_value = new_array;
    }
    else {
        json_array& current_array = store.get_array(cursor.container_trace.top().store_id);
        current_array.push_back(new_array);
    }

    cursor.container_trace.push(new_array);

    state = JSON_PARSE_STATE::NEW_ARRAY;

    return;

};

void Physon::close_array(){

    gobble_ws();

    if(content[cursor.index] != ']')
        json_error("Invalid JSON: Unexpected char when trying to close array. Occured at index " + std::to_string(cursor.index));


    cursor.index++;
    gobble_ws();

    cursor.container_trace.pop();
    if(cursor.container_trace.size() == 0){
        state = JSON_PARSE_STATE::END_OF_ROOT;
        return;
    }

    state = JSON_PARSE_STATE::END_VALUE;

    return;
};


json_value Physon::parse_true_literal(){

    bool is_true_literal = (content[cursor.index + 1] == 'r' && content[cursor.index + 2] == 'u' && content[cursor.index + 3] == 'e');
    if(!is_true_literal)
        json_error("Invalid true-literal at index " + std::to_string(cursor.index));
    
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
        json_error("Invalid false-literal at index " + std::to_string(cursor.index));
    
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
        json_error("Invalid null-literal at index " + std::to_string(cursor.index));
    
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

bool Physon::is_new_value_char(){
    return is_new_literal_char() || is_new_container_char();
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

void Physon::json_error(std::string error_msg){
    state = JSON_PARSE_STATE::ERROR;

    throw std::runtime_error(error_msg);

}