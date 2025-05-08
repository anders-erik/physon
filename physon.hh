#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "physon_types.hh"


#define log(x) std::cout << x << std::endl;

#define QUOTATION_MARK      '\u0022'
#define SOLLIDUS            '\u002F'
#define SOLLIDUS_BACKWARDS  '\u005C'


struct Physon {
    std::string content;

    json_store store; // store for all json data
    
    json_value root_value; // the first element encountered in file

    ParserCursor cursor;  // source string cursor
    std::vector<Token> tokens;

    
    Physon(std::string json_str) : content {json_str} {
        if(content.size() == 0)
            json_error("Error: json content string is empty. ");
    }; 


    // PRINT
    void print_original();

    /** Temporary string for stringification. */
    std::string stringify_string;
    /** returns full json structure as string */
    std::string stringify();            
    /** recursive string builder */    
    void build_string(json_value value);
    /** Converts a std::string to is JSON equivelence. e.g. <I "mean" it..> --> <"I \"mean\" it.."> */
    std::string string_to_json_representation(std::string cpp_string);
    

    // QUERYING
    json_value& find(std::string key);      // for json_object - not recursive
    json_object& get_object();              // for json_object
    json_array& get_array();                // for json_array


    // PARSING

    JSON_PARSE_STATE state;

    void before_root_value();
    void end_of_root_value();

    void array_entered();
    void value_new_value_char();
    void value_end_of_value();

    void json_error(std::string error_msg);

    void add_value_to_current_container(json_value value);
    
    void parse();                   /** Parse the content string */

    void print_tokens();

    void array_enter();
    void array_close();

    void enter_object();
    void close_object();

    json_value value_parse_literal(); /** Parse literal with cursor confirmed pointing at first char in literal */

    json_value parse_string_literal();  /** parse and progress index. */
    json_value parse_true_literal();    /** confirm "true" chars and progress index. */
    json_value parse_false_literal();   /** confirm "false" chars and progress index. */
    json_value parse_null_literal();    /** confirm "null" chars and progress index. */

    char current_char();
    bool is_new_value_char();   /** is_new_literal_char() U is_new_container_char() */
    bool is_new_literal_char();
    bool is_new_container_char();
    bool is_new_array_char();
    bool is_close_array_char();
    bool is_new_object_char();
    bool is_close_object_char();

    bool is_whitespace(char ch);
    void gobble_ws();               /** progress index until cursor not pointing at whitespace */

    bool is_literal(json_value_type type);
    bool is_container(json_value_type type);
    bool current_container_is_array();
    bool current_container_is_object();
    json_value_type current_container_type();
    
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

std::string Physon::string_to_json_representation(std::string cpp_string){

    std::string json_representation = "";

    json_representation += "\"";

    for(const char ch : cpp_string){
        
        if(ch == QUOTATION_MARK){
            json_representation += "\\\"";
        }
        else if (ch == SOLLIDUS){
            json_representation += "/\\";
        }
        else if(ch == SOLLIDUS_BACKWARDS){
            json_representation += "\\\\";
        }
        else if(ch == '\u0008'){
            json_representation += "\\b";
        }
        else if(ch == '\u0009'){
            json_representation += "\\t";
        }
        else if(ch == '\u000A'){
            json_representation += "\\n";
        }
        else if(ch == '\u000C'){
            json_representation += "\\f";
        }
        else if(ch == '\u000D'){
            json_representation += "\\r";
        }
        else if( (ch >= '\u0000' && ch < '\u0020') ){
            json_representation += "\\u00XX";
            // TODO: CONVERT ch to hex-string and append
            json_representation += std::to_string(ch);
        }
        else {
            json_representation += ch;
        }

    }

    json_representation += "\"";

    return json_representation;
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
        case json_value_type::STRING:
            // 1) Grab value from store
            // 2) convert to json representation
            // 3) append to stringify-string
            stringify_string.append(
                string_to_json_representation(
                    store.get_string(value.store_id)
                )
            );
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


void Physon::array_entered(){

    gobble_ws();

    if(content[cursor.index] == ']'){
        state = JSON_PARSE_STATE::ARRAY_CLOSE;
    }
    else if(is_new_value_char()){
        state = JSON_PARSE_STATE::VALUE_NEW_VALUE_CHAR;
    }
    else {
        json_error("Error: not a valid character during state JSON_PARSE_STATE::ARRAY_ENTERED. Content index = " + std::to_string(cursor.index));
    }
}
void Physon::value_new_value_char(){
    gobble_ws();

    if(is_new_literal_char())
        state = JSON_PARSE_STATE::VALUE_PARSE_LITERAL;
    else if(is_new_array_char())
        state = JSON_PARSE_STATE::ARRAY_ENTER;
    else 
        json_error("Error: not a valid character during state JSON_PARSE_STATE::VALUE_NEW_VALUE_CHAR. Content index = " + std::to_string(cursor.index));

}
void Physon::value_end_of_value(){
    gobble_ws();

    if(current_char() == ','){
        cursor.index++;
        gobble_ws();
        state = JSON_PARSE_STATE::VALUE_NEW_VALUE_CHAR;
    }
    else if(is_close_array_char()){

        if(! current_container_is_array())
            json_error("Error: Tried to close an array in a non array container.");

        state = JSON_PARSE_STATE::ARRAY_CLOSE;
    }

}

void Physon::before_root_value(){


    gobble_ws();
    
    // Nothing except whitespace
    if(cursor.index == content.size())
        json_error("Error: No valid JSON values.");
    

    // A single literal value in the json content string
    if(is_new_literal_char()){
        root_value = value_parse_literal();
        gobble_ws();

        if(cursor.index != content.size())
            json_error("Invalid JSON: Extra characters after root literal at index " + std::to_string(cursor.index));
        
        return;
    }

    // Root container
    if(is_new_array_char())
        state = JSON_PARSE_STATE::ARRAY_ENTER;
    else if(is_new_object_char())
        enter_object();
    else
        json_error("Invalid JSON: No valid first character of root element.");

}

void Physon::end_of_root_value(){
    gobble_ws();

    if(cursor.index != content.size())
        json_error("Invalid JSON: Extra characters after root container. Found at index " + std::to_string(cursor.index) + ".");

    state = JSON_PARSE_STATE::DONE;
    return;
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
bool Physon::current_container_is_array(){
    return cursor.container_trace.top().type == json_value_type::ARRAY;
}
bool Physon::current_container_is_object(){
    return cursor.container_trace.top().type == json_value_type::OBJECT;
}
json_value_type Physon::current_container_type(){
    return cursor.container_trace.empty() ? json_value_type::NULL_ : cursor.container_trace.top().type;
}


json_value Physon::value_parse_literal(){

    gobble_ws();

    json_value new_value;

    char current_char = content[cursor.index];


    if     (current_char == 't')
        new_value = parse_true_literal();

    else if(current_char == 'f')
        new_value = parse_false_literal();

    else if(current_char == 'n')
        new_value = parse_null_literal();

    else if(current_char == '"')
        new_value = parse_string_literal();

    else if(current_char == '-' || (current_char >= '0' && current_char <= '9' ))
        json_error("Error: Number parsing not yet implemented.");

    else
        json_error("Invalid JSON: Expected beginning of literal. Instead '" + content.substr(cursor.index, 1) + "' at first literal character. Occured at global index " + std::to_string(cursor.index));
    

    gobble_ws();

    add_value_to_current_container(new_value);

    state = JSON_PARSE_STATE::VALUE_END_OF_VALUE;

    return new_value;

}


void Physon::array_enter(){

    gobble_ws();

    if(content[cursor.index] != '[')
        json_error("Invalid JSON: Unexpected first char '" + content.substr(cursor.index, 1) + "' in new value. Occured at index " + std::to_string(cursor.index));


    cursor.index++;
    gobble_ws();


    json_value new_array;
    int new_array_id = store.add_array();
    new_array.type = json_value_type::ARRAY;
    new_array.store_id = new_array_id;

    size_t current_trace_depth = cursor.container_trace.size();

    if(current_trace_depth == 0){ // ROOT
        root_value = new_array;
    }
    else {
        
        json_value current_container = cursor.container_trace.top();

        // Add new array to tree
        if(current_container.type == json_value_type::ARRAY){
            json_array& current_array = store.get_array(current_container.store_id);
            current_array.push_back(new_array);
        }
        else if(current_container.type == json_value_type::OBJECT){
            json_object& current_object = store.get_object(current_container.store_id);
            current_object.back().second = new_array;
        }
    }

    cursor.container_trace.push(new_array);

    state = JSON_PARSE_STATE::ARRAY_ENTERED;

    return;

};

void Physon::array_close(){

    gobble_ws();

    if(! is_close_array_char())
        json_error("Invalid JSON: Unexpected char when trying to close array. Occured at index " + std::to_string(cursor.index));


    cursor.index++;
    gobble_ws();

    cursor.container_trace.pop();
    if(cursor.container_trace.size() == 0){
        state = JSON_PARSE_STATE::END_OF_ROOT_VALUE;
        return;
    }

    state = JSON_PARSE_STATE::VALUE_END_OF_VALUE;

    return;
};



void Physon::enter_object(){

    gobble_ws();

    if(! is_new_object_char())
        json_error("Invalid JSON: Unexpected first char '" + content.substr(cursor.index, 1) + "' in enter object. Occured at index " + std::to_string(cursor.index));


    cursor.index++;
    gobble_ws();


    json_value new_object;
    int new_object_id = store.add_object();
    new_object.type = json_value_type::OBJECT;
    new_object.store_id = new_object_id;


    if(state == JSON_PARSE_STATE::BEFORE_ROOT_VALUE){
        root_value = new_object;
    }
    else {
        json_value current_container = cursor.container_trace.top();

        // Add new object to tree
        if(current_container.type == json_value_type::ARRAY){
            json_array& current_array = store.get_array(current_container.store_id);
            current_array.push_back(new_object);
        }
        else if(current_container.type == json_value_type::OBJECT){
            json_object& current_object = store.get_object(current_container.store_id);
            current_object.back().second = new_object;
        }
    }

    cursor.container_trace.push(new_object);

    state = JSON_PARSE_STATE::NEW_OBJECT;

    return;

};

void Physon::close_object(){

    gobble_ws();

    if(content[cursor.index] != '}')
        json_error("Invalid JSON: Unexpected char when trying to close object. Occured at index " + std::to_string(cursor.index));


    cursor.index++;
    gobble_ws();

    cursor.container_trace.pop();
    if(cursor.container_trace.size() == 0){
        state = JSON_PARSE_STATE::END_OF_ROOT_VALUE;
        return;
    }

    state = JSON_PARSE_STATE::VALUE_END_OF_VALUE;

    return;
};

json_value Physon::parse_string_literal(){

    cursor.index++;

    std::string new_string = "";

    while(content[cursor.index] != QUOTATION_MARK ){

        // Current char
        char ch = content[cursor.index];
        
        if( ch >= '\u0000' && ch < '\u0020'){
            json_error("Error: unescaped control character in string. Found at index " + std::to_string(content[cursor.index]) );
        }
        else if(ch == SOLLIDUS_BACKWARDS){

            // skip backwards sollidus
            cursor.index++;
            ch = content[cursor.index];

            switch (ch)
            {

            case QUOTATION_MARK:
                new_string += QUOTATION_MARK;
                break;
            case SOLLIDUS:
                new_string += SOLLIDUS;
                break;
            case SOLLIDUS_BACKWARDS:
                new_string += SOLLIDUS_BACKWARDS;
                break;
            
            case 'b':
                new_string += '\u0008';
                break;
            case 'f':
                new_string += '\u000C';
                break;
            case 'n':
                new_string += '\u000A';
                break;
            case 'r':
                new_string += '\u000D';
                break;
            case 't':
                new_string += '\u0009';
                break;

            case 'u':
                // Parse unicode : '\uXXXX'
                // Currently only supports ASCII
                {
                    std::string unicode_digits = content.substr(cursor.index+1, 4);

                    unsigned int unicode_value_decimal;
                    std::stringstream _stringstream;
                    _stringstream << std::hex << unicode_digits;
                    _stringstream >> unicode_value_decimal;

                    // log(unicode_digits);
                    // log(unicode_value_decimal);

                    // ASCII
                    if(unicode_value_decimal < 0x7F){
                        new_string += static_cast<char>(unicode_value_decimal);
                    }
                    else {
                        json_error("ERROR: non-ASCII unicode values in string are not yet supported.");
                    }
                }
                // move to last unicode digit
                cursor.index += 4;
                break;
            
            default:
                break;
            }

        }
        else {
            new_string += content[cursor.index];
        }
    

        // Next char
        cursor.index++;

        if(cursor.index >= content.size())
            json_error("Error: Unclosed string literal. Expected closing quotation mark before end of content string.");
    }

    // Move past closing quotation mark
    cursor.index++;

    gobble_ws();

    // Add string to store
    json_value new_value;
    new_value.type = json_value_type::STRING;
    new_value.store_id = store.add_string(new_string);


    return new_value;
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


char Physon::current_char(){
    return content[cursor.index];
}
bool Physon::is_new_container_char(){
    char c = content[cursor.index];
    bool is_container_char = c == '{' || c == '[';
    return is_container_char;
}
bool Physon::is_new_array_char(){
    return content[cursor.index] == '[';
}
bool Physon::is_close_array_char(){
    return content[cursor.index] == ']';
}
bool Physon::is_new_object_char(){
    return content[cursor.index] == '{';
}
bool Physon::is_close_object_char(){
    return content[cursor.index] == '}';
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

void Physon::add_value_to_current_container(json_value value){

    switch (current_container_type()){

    case json_value_type::NULL_:

        break;

    case json_value_type::ARRAY:
        {
            json_array& trace_top = store.get_array(cursor.container_trace.top().store_id);
            trace_top.push_back(value);
        }
        break;
    
    default:
        break;
    }
}


void Physon::parse() {

    cursor.index = 0;
    store.clear();
    state = JSON_PARSE_STATE::BEFORE_ROOT_VALUE;
    
    // Main Parsing loop
    while(cursor.index < content.size()){

        switch (state){

        case JSON_PARSE_STATE::BEFORE_ROOT_VALUE:
            before_root_value();
            break;

        case JSON_PARSE_STATE::ARRAY_ENTER:
            array_enter();
            break;
        case JSON_PARSE_STATE::ARRAY_ENTERED:
            array_entered();
            break;
        case JSON_PARSE_STATE::ARRAY_CLOSE:
            array_close();
            break;

        case JSON_PARSE_STATE::VALUE_NEW_VALUE_CHAR:
            value_new_value_char();
            break;
        case JSON_PARSE_STATE::VALUE_PARSE_LITERAL:
            value_parse_literal();
            break;
        case JSON_PARSE_STATE::VALUE_END_OF_VALUE:
            value_end_of_value();
            break;
            

        case JSON_PARSE_STATE::END_OF_ROOT_VALUE:
            end_of_root_value();
            break;
        case JSON_PARSE_STATE::DONE:
            return;
            break;

        default:
            json_error("Error: Unknown parsing state.");
            break;
        }


    }

}