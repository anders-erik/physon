#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip> // setprecision
#include <climits> // LLONG_MAX

#include "physon_types.hh"


#define log(x) std::cout << x << std::endl;

#define QUOTATION_MARK      '\u0022'
#define SOLLIDUS            '\u002F'
#define SOLLIDUS_BACKWARDS  '\u005C'


struct Physon {
    std::string content;

    json_store store; // store for all json data
    
    JsonWrapper root_value; // the first element encountered in file

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
    void build_string(JsonWrapper value);
    std::string float_to_json_representation(json_float float_);
    /** Converts a std::string to is JSON equivelence. e.g. <I "mean" it..> --> <"I \"mean\" it.."> */
    std::string string_to_json_representation(std::string cpp_string);
    

    // QUERYING
    JsonWrapper& find(std::string key);      // for json_object_wrap - not recursive
    json_object_wrap& get_object();              // for json_object_wrap
    json_array_wrap& get_array();                // for json_array

    // UNWRAPPING
    json_bool& unwrap_bool(JsonWrapper wrapper);

    // PARSING

    JSON_PARSE_STATE state;

    void before_root_value();
    void end_of_root_value();
    bool root_no_container();

    void array_entered();
    
    void index_advance();
    void comma_skip(); /** Branching based on the container. */
    void colon_skip();
    void value_at_new_value_char();
    void value_end_of_value();

    std::string state_to_string();
    void json_error(std::string error_msg);

    void add_value_to_current_container(JsonWrapper value);
    
    void parse();                   /** Parse the content string */

    void print_tokens();

    void array_enter();
    void array_close();

    void object_enter();
    void object_parse_key_comma();
    void object_close();

    void value_parse_literal(); /** Parse literal with cursor confirmed pointing at first char in literal */
    JsonWrapper parse_string_literal();  /** parse and progress index. */
    std::string parse_digits();
    bool integer_within_limits(std::string int_);
    JsonWrapper parse_number_literal();  /** parse and progress index. */
    JsonWrapper parse_true_literal();    /** confirm "true" chars and progress index. */
    JsonWrapper parse_false_literal();   /** confirm "false" chars and progress index. */
    JsonWrapper parse_null_literal();    /** confirm "null" chars and progress index. */

    char current_char();
    bool is_new_value_char();   /** is_new_literal_char() U is_new_container_char() */
    bool is_quotation_mark();
    bool is_new_literal_char();
    bool is_new_container_char();
    bool is_new_array_char();
    bool is_close_array_char();
    bool is_new_object_char();
    bool is_close_object_char();
    bool is_valid_number_char(){
        char c = current_char();
        bool is_number_char = (c >= '0' && c <= '9') || c == '-' || c == '+' || c == 'e' || c == 'E' || c == '.';
        return is_number_char;
    };
    bool is_digit(char c){
        return (c >= '0' && c <= '9') ? true : false;
    };
    bool is_non_zero_digit(char c){
        return (c >= '1' && c <= '9') ? true : false;
    };

    bool is_whitespace(char ch);
    void gobble_ws();               /** progress index until cursor not pointing at whitespace */

    bool is_literal(JSON_TYPE type);
    bool is_container(JSON_TYPE type);

    JsonWrapper current_container();
    bool current_container_is_array();
    bool current_container_is_object();
    JSON_TYPE current_container_type();
    
};


// json_bool& Physon::unwrap_bool(JsonWrapper wrapper){
//     bool is_false = wrapper.type == JSON_TYPE::FALSE;
//     bool is_true = wrapper.type == JSON_TYPE::TRUE;
//     if(wrapper.type == JSON_TYPE::TRUE)
//         return store.get_bool(wrapper.store_id);
//     if(wrapper.type == JSON_TYPE::FALSE)
//         return store.get_bool(wrapper.store_id);
//     else
//         json_error("Tried to unwrap a non-boolean wrapper as boolan.");
// }


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



std::string Physon::float_to_json_representation(json_float float_){

    int precision = 7;

    /** Examples : precision == 7 */
    enum class REPRESENTATION {
        FIXED,              // 0.0 -> 0.0000000
        FIXED_TRIMMED,      // 0.0 -> 0.0 : precision == 1
        SCIENTIFIC,         // 0.0 -> 0.0000000e+00
    } representation = REPRESENTATION::SCIENTIFIC;


    std::string str_return = "";

    std::ostringstream ss;

    switch (representation){

    case REPRESENTATION::FIXED:
    case REPRESENTATION::FIXED_TRIMMED:
        ss << std::fixed << std::setprecision(precision) << float_;
        break;

    case REPRESENTATION::SCIENTIFIC:
        ss << std::scientific << std::setprecision(precision) << float_;
        break;

    }

    str_return = ss.str();


    // TODO:  THIS WILL BE REMOVED WITH THE PRECISION SETTING
    if(representation == REPRESENTATION::FIXED_TRIMMED){

         // TRIM TRAILING ZEROS
        while( str_return[str_return.length()-2] == '0')
            str_return.pop_back();

    }

    return str_return;
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

void Physon::build_string(JsonWrapper value){
    
    if(is_literal(value.type)){

        switch (value.type)
        {
        case JSON_TYPE::NULL_:
            stringify_string.append("null");
            break;
        case JSON_TYPE::TRUE:
            stringify_string.append("true");
            break;
        case JSON_TYPE::FALSE:
            stringify_string.append("false");
            break;
        case JSON_TYPE::FLOAT:
            {
                json_float float_ = store.get_float(value.store_id);
                std::string float_str = float_to_json_representation(float_);

                stringify_string.append( float_str );
            }
            break;
        case JSON_TYPE::INTEGER:
            stringify_string.append( std::to_string(store.get_integer(value.store_id) ) );
            break;

        case JSON_TYPE::STRING:
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
    if(value.type == JSON_TYPE::ARRAY){
        stringify_string.append("[");

        json_array_wrap array =  store.get_array(value.store_id);

        for(JsonWrapper array_entry : array){
            build_string(array_entry);
            stringify_string.append(", ");
        }

        // no comma after last entry
        if(array.size() > 0)
            stringify_string.erase(stringify_string.size()-2);

        stringify_string.append("]");
    }
    else if(value.type == JSON_TYPE::KV){

        json_kv_wrap& kv = store.get_kv(value.store_id);

        stringify_string.append("\"");
        stringify_string.append(kv.first);
        stringify_string.append("\": ");

        build_string(kv.second);
    }
    else if(value.type == JSON_TYPE::OBJECT){
        stringify_string.append("{");

        json_object_wrap object =  store.get_object(value.store_id);

        for(JsonWrapper kv : object){
            
            build_string(kv);

            stringify_string.append(", ");
        }

        // no comma after last entry
        if(object.size() > 0)
            stringify_string.erase(stringify_string.size()-2);

        stringify_string.append("}");
    }
}


void Physon::array_entered(){

    gobble_ws();

    if(is_close_array_char()){
        state = JSON_PARSE_STATE::ARRAY_CLOSE;
    }
    else if(is_new_value_char()){
        state = JSON_PARSE_STATE::VALUE_AT_NEW_VALUE_CHAR;
    }
    else {
        json_error("Error: not a valid character during state JSON_PARSE_STATE::ARRAY_ENTERED. Content index = " + std::to_string(cursor.index));
    }
}

void Physon::index_advance(){
    cursor.index++;
    gobble_ws();
}

void Physon::comma_skip(){

    if(current_char() != ',')
        json_error("Unexpected char instead of comma. Index = " + std::to_string(cursor.index));

    index_advance();

    if(current_container_is_object())
        state = JSON_PARSE_STATE::OBJECT_PARSE_KEY_COMMA;
    else
        state = JSON_PARSE_STATE::VALUE_AT_NEW_VALUE_CHAR;

}

void Physon::colon_skip(){
    if(current_char() != ':')
        json_error("Unexpected char during colon skip. Index = " + std::to_string(cursor.index));

    index_advance();
}

void Physon::value_at_new_value_char(){
    gobble_ws();

    // if in an object, we only accept string-keys!
    // if(current_container_is_object() && is_quotation_mark()){
    //     state = JSON_PARSE_STATE::OBJECT_PARSE_KEY_COMMA;
    // }

    if(is_new_literal_char())
        state = JSON_PARSE_STATE::VALUE_PARSE_LITERAL;
    else if(is_new_array_char())
        state = JSON_PARSE_STATE::ARRAY_ENTER;
    else if(is_new_object_char())
        state = JSON_PARSE_STATE::OBJECT_ENTER;
    else 
        json_error("Error: not a valid character during state JSON_PARSE_STATE::VALUE_AT_NEW_VALUE_CHAR. Content index = " + std::to_string(cursor.index));

}


void Physon::value_end_of_value(){
    gobble_ws();

    if (root_no_container()){
        state = JSON_PARSE_STATE::ROOT_END_OF_VALUE;
    }
    else if(current_char() == ','){
        comma_skip();
    }
    else if(is_close_array_char()){
        state = JSON_PARSE_STATE::ARRAY_CLOSE;
    }
    else if(is_close_object_char()){
        state = JSON_PARSE_STATE::OBJECT_CLOSE;
    }
    else {
        json_error("Invalid character encountered after end of value.");
    }

}

void Physon::before_root_value(){


    gobble_ws();
    
    // Nothing except whitespace
    if(cursor.index == content.size())
        json_error("Error: No valid JSON values.");
    

    // A single literal value in the json content string
    if(is_new_literal_char()){
        value_parse_literal();
        gobble_ws();

        if(cursor.index != content.size())
            json_error("Invalid JSON: Extra characters after root literal at index " + std::to_string(cursor.index));
        
        return;
    }

    // Root container
    if(is_new_array_char())
        state = JSON_PARSE_STATE::ARRAY_ENTER;
    else if(is_new_object_char())
        state = JSON_PARSE_STATE::OBJECT_ENTER;
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


bool Physon::root_no_container(){
    return cursor.container_trace.empty();
}

void Physon::print_tokens() {
    for (const auto& token : tokens) {
        std::cout << "Token Type: " << static_cast<int>(token.type) 
                  << ", Start Index: " << token.str_start_i
                  << ", Length: " << token.str_length << std::endl;
    }
}

bool Physon::is_literal(JSON_TYPE type){
    return  type == JSON_TYPE::NULL_ ||
            type == JSON_TYPE::TRUE ||
            type == JSON_TYPE::FALSE ||
            type == JSON_TYPE::NUMBER ||
            type == JSON_TYPE::FLOAT ||
            type == JSON_TYPE::INTEGER ||
            type == JSON_TYPE::STRING;
}
bool Physon::is_container(JSON_TYPE type){
    return  type == JSON_TYPE::ARRAY ||
            type == JSON_TYPE::OBJECT;
}

JsonWrapper Physon::current_container(){
    JsonWrapper empty_value;
    return cursor.container_trace.empty() ? empty_value : cursor.container_trace.top();
}
bool Physon::current_container_is_array(){
    return cursor.container_trace.top().type == JSON_TYPE::ARRAY;
}
bool Physon::current_container_is_object(){
    return cursor.container_trace.top().type == JSON_TYPE::OBJECT;
}
JSON_TYPE Physon::current_container_type(){
    return cursor.container_trace.empty() ? JSON_TYPE::NONE : cursor.container_trace.top().type;
}


void Physon::value_parse_literal(){

    gobble_ws();

    JsonWrapper new_value;

    char current_char = content[cursor.index];


    if     (current_char == 't')
        new_value = parse_true_literal();

    else if(current_char == 'f')
        new_value = parse_false_literal();

    else if(current_char == 'n')
        new_value = parse_null_literal();

    else if(current_char == '"')
        new_value = parse_string_literal();

    else if(current_char == '-' || is_digit(current_char) )
        new_value = parse_number_literal();
        // json_error("Number parsing not yet implemented.");

    else
        json_error("Unexpected first literal character.");
    

    gobble_ws();

    // if(current_container_is_object()){
    //     if(new_value.type != JSON_TYPE::STRING)
    //         json_error("Parsed a non-string literal as new value inside an object container.");
        

    // }
    add_value_to_current_container(new_value);

    state = JSON_PARSE_STATE::VALUE_END_OF_VALUE;

}


void Physon::array_enter(){

    index_advance();

    JsonWrapper new_array = store.new_array();

    add_value_to_current_container(new_array);

    cursor.container_trace.push(new_array);

    state = JSON_PARSE_STATE::ARRAY_ENTERED;

};

void Physon::array_close(){

    if(! current_container_is_array())
        json_error("Error: Tried to close an array when currently not in an array container.");

    index_advance();

    cursor.container_trace.pop();
    state = JSON_PARSE_STATE::VALUE_END_OF_VALUE;

    // Closed root array
    if(cursor.container_trace.size() == 0)
        state = JSON_PARSE_STATE::ROOT_END_OF_VALUE;

};



void Physon::object_enter(){

    if(! is_new_object_char())
        json_error("Invalid JSON: Unexpected first char '" + content.substr(cursor.index, 1) + "' in enter object. Occured at index " + std::to_string(cursor.index));

    // move past start_object token
    index_advance();

    JsonWrapper new_object = store.new_object();
    add_value_to_current_container(new_object);


    if(current_char() == '}'){
        index_advance();
        state = JSON_PARSE_STATE::VALUE_END_OF_VALUE;
        return;
    }
    else if (current_char() == '"'){
        cursor.container_trace.push(new_object);
        state = JSON_PARSE_STATE::OBJECT_PARSE_KEY_COMMA;
    }
    else {
        json_error("Invalid char encountered when entering object.");
    }


    return;

};

void Physon::object_parse_key_comma(){

    if(content[cursor.index] != '"')
        json_error("Invalid JSON: Unexpected char '" + content.substr(cursor.index, 1) + "' when entered object. Occured at index " + std::to_string(cursor.index));
    

    JsonWrapper key = parse_string_literal();
    colon_skip();


    JsonWrapper kv = store.new_kv(store.get_string(key.store_id));

    add_value_to_current_container(kv);

    
    state = JSON_PARSE_STATE::VALUE_AT_NEW_VALUE_CHAR;
}

void Physon::object_close(){

    if(content[cursor.index] != '}')
        json_error("Invalid JSON: Unexpected char when trying to close object. Occured at index " + std::to_string(cursor.index));

    // Skip close curly brace
    index_advance();

    cursor.container_trace.pop();
    state = JSON_PARSE_STATE::VALUE_END_OF_VALUE;

    if(cursor.container_trace.size() == 0)
        state = JSON_PARSE_STATE::ROOT_END_OF_VALUE;

};

std::string Physon::parse_digits(){
    std::string digit_string = "";

    while(is_digit(current_char())){
        digit_string += current_char();
        cursor.index++;
    }

    return digit_string;
}

bool Physon::integer_within_limits(std::string str){

    // TODO: Catch edge cases of integers.  LLONG_MAX < int < e19

    if(str[0] == '-')
        str = str.substr(1, str.size()-1);

    if(str.length() > std::to_string(LLONG_MAX).length() )
        return false;
    
    return true;
}

JsonWrapper Physon::parse_number_literal(){

    enum class NUMBER_STATE {
        NEGATIVE,
        LEADING_ZERO,
        INTEGRAL_DIGITS,
        FRACTION,
        FRACTIONAL_DIGITS,
        EXPONENT_E,
        EXPONENT_SIGN,
        EXPONENT_DIGITS,
        END,
    } number_state = NUMBER_STATE::NEGATIVE;

    bool is_fractional = false;
    
    std::string string_to_parse = "";

    // FIRST CHECKS
    if(current_char() == '-'){
        string_to_parse += "-";
        cursor.index++;
    }
    number_state = NUMBER_STATE::LEADING_ZERO;
    
    if (current_char() == '0'){
        string_to_parse += "0";
        cursor.index++;

        if(is_digit(current_char()))
            json_error("Additional leading zeros.");

    }
    else if (is_non_zero_digit(current_char())) {
        string_to_parse += parse_digits();
    }
    else {
        json_error("First digit in number not valid. ");
    }
    number_state = NUMBER_STATE::FRACTION;

    if(current_char() == '.'){
        is_fractional = true;
        string_to_parse += ".";
        cursor.index++;

        if(! is_digit(current_char()))
            json_error("Fraction delimiter must be followed by digit.");

        string_to_parse += parse_digits();
    }
    number_state = NUMBER_STATE::EXPONENT_E;

    if(current_char() == 'e' || current_char() == 'E'){

        string_to_parse += current_char();
        cursor.index++;
        number_state = NUMBER_STATE::EXPONENT_SIGN;

        bool e_trail_ok = is_digit(current_char()) || current_char() == '+' || current_char() == '-';
        if(! e_trail_ok)
            json_error("Exponent char not trailed by sign nor digit.");
        
        if(current_char() == '+' || current_char() == '-'){
            string_to_parse += current_char();
            cursor.index++;
        }
        number_state = NUMBER_STATE::EXPONENT_DIGITS;

        if(! is_digit(current_char()))
            json_error("No exponent digits detected during number parsing.");

        string_to_parse += parse_digits();
    }

    number_state = NUMBER_STATE::END;

    JsonWrapper wrapper;
    if(is_fractional){
        json_float float_ = std::stod(string_to_parse);
        wrapper = store.new_float(float_);
    }
    else {

        if(! integer_within_limits(string_to_parse))
            json_error("Integer too large for internal representation.");

        json_int int_ = std::stol(string_to_parse);
        wrapper = store.new_integer(int_);
    }

    return wrapper;

    // else if (is_digit(current_char())){
    //     number_state = NUMBER_STATE::INTEGRAL_DIGITS;
    // }
    // else {
        
    // }

    // while (!end_of_number)
    // {
        
    //     switch (number_state){
        
    //     case NUMBER_STATE::LEADING_ZERO:
    //         {
    //             if(current_char() == '0'){
    //                 is_leading_zero = true;
    //                 string_to_parse += "0";
    //                 cursor.index++;
    //                 number_state = NUMBER_STATE::FRACTION;
    //             }
    //             else if ( is_digit(current_char()) ){
    //                 number_state = NUMBER_STATE::INTEGRAL_DIGITS;
    //             }
    //             else {
    //                 json_error("Invalid char when checking for leading zero in number literal.");
    //             }
    //         }
    //         break;
        
    //     case NUMBER_STATE::INTEGRAL_DIGITS:
    //         {
    //             if(is_digit(current_char())){
    //                 string_to_parse += current_char();
    //                 cursor.index++;
    //             }
    //             else {
    //                 number_state = NUMBER_STATE::FRACTION;
    //             }

    //         }
    //         break;

    //     case NUMBER_STATE::FRACTION:
    //         {
    //             if(current_char() == '.'){
    //                 is_fractional = true;
    //                 string_to_parse += '.';
    //                 cursor.index++;
    //                 number_state = NUMBER_STATE::FRACTIONAL_DIGITS;
    //             }
    //             else if (current_char() == 'e' || current_char() == 'E') {
    //                 number_state = NUMBER_STATE::EXPONENT_E;
    //             }
    //             else {
    //                 number_state = NUMBER_STATE::END;
    //             }
                
    //         }
    //         break;

    //     case NUMBER_STATE::FRACTIONAL_DIGITS:
    //         {
    //             if(is_digit(current_char())){
    //                 string_to_parse += current_char();
    //                 cursor.index++;
    //             }
    //             else {
    //                 number_state = NUMBER_STATE::EXPONENT_E;
    //             }
                
    //         }
    //         break;

    //     case NUMBER_STATE::EXPONENT_E:
    //         {
    //             enum class EXPONENT_STATE {
    //                 E,
    //                 SIGN,
    //                 DIGITS,
    //                 DONE,
    //             };

    //             EXPONENT_STATE exponent_state = EXPONENT_STATE::E;

    //             while(exponent_state != EXPONENT_STATE::DONE){

    //                 switch (exponent_state){

    //                 case EXPONENT_STATE::E :
    //                     {
    //                         if(current_char() == 'e' || current_char() == 'E'){
    //                             string_to_parse += current_char();
    //                             cursor.index++;
    //                             exponent_state = EXPONENT_STATE::SIGN;
    //                         }
    //                         else {
    //                             json_error("Failed to identify 'e'/'E' character while in exponent state during number parse. ");
    //                         }
    //                     }
    //                     break;

    //                 case EXPONENT_STATE::SIGN :
    //                     {
    //                         if(current_char() == '+' || current_char() == '-'){
    //                             string_to_parse += current_char();
    //                             cursor.index++;
    //                         }
    //                         exponent_state = EXPONENT_STATE::DIGITS;

    //                     }
    //                     break;

    //                 case EXPONENT_STATE::DIGITS :
    //                     {
    //                         if( is_digit(current_char()) ){
    //                             string_to_parse += current_char();
    //                             cursor.index++;
    //                         }
    //                         else {
    //                             exponent_state = EXPONENT_STATE::DONE;
    //                         }
                            
    //                     }
    //                     break;
                    
    //                 default:
    //                     break;
    //                 }
                    
    //             }
                
    //             number_state = NUMBER_STATE::END;
    //         }
    //         break;
        
    //     default:
    //         break;
    //     }
    // }

    
    
    // type = JSON_TYPE::FLOAT;
    // if(type == JSON_TYPE::FLOAT){
    //     const char* str_cnst = string_to_parse.c_str();
    //     char * c_str = (char*)str_cnst;
    //     char * c_str_end = c_str;
    //     char ** str_end = &c_str_end;
    //     json_float fl = strtod(str_cnst, str_end);
    //     std::cout << fl << std::endl;
        
    // }
    // else if (type == JSON_TYPE::INTEGER){
    //     // json_int integer = std::to_integer(string_to_parse);
    // }
    // else {
    //     json_error("Failed to determine number type during parsing of literal.");
    // }

    // return store.new_integer(1);
}

JsonWrapper Physon::parse_string_literal(){

    // Skip quotation mark, but no gobbling in string literal
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
    index_advance();

    // Add string to store
    JsonWrapper new_value;
    new_value.type = JSON_TYPE::STRING;
    new_value.store_id = store.add_string(new_string);


    return new_value;
};

JsonWrapper Physon::parse_true_literal(){

    bool is_true_literal = (content[cursor.index + 1] == 'r' && content[cursor.index + 2] == 'u' && content[cursor.index + 3] == 'e');
    if(!is_true_literal)
        json_error("Invalid true-literal at index " + std::to_string(cursor.index));
    
    std::cout << "true at "  << cursor.index << std::endl;

    tokens.emplace_back(token_type::TRUE, cursor.index, 4);
    cursor.index += 4;

    JsonWrapper new_value;
    new_value.type = JSON_TYPE::TRUE;
    new_value.store_id = 0;

    return new_value;
};

JsonWrapper Physon::parse_false_literal(){

    bool is_false_literal = (content[cursor.index + 1] == 'a' && content[cursor.index + 2] == 'l' && content[cursor.index + 3] == 's' && content[cursor.index + 4] == 'e');
    if(!is_false_literal)
        json_error("Invalid false-literal at index " + std::to_string(cursor.index));
    
    std::cout << "false at "  << cursor.index << std::endl;

    tokens.emplace_back(token_type::FALSE, cursor.index, 5);
    cursor.index += 5;

    JsonWrapper new_value;
    new_value.type = JSON_TYPE::FALSE;
    new_value.store_id = 0;
    return new_value;
};

JsonWrapper Physon::parse_null_literal(){

    bool is_null_literal = (content[cursor.index + 1] == 'u' && content[cursor.index + 2] == 'l' && content[cursor.index + 3] == 'l');
    if(!is_null_literal)
        json_error("Invalid null-literal at index " + std::to_string(cursor.index));
    
    std::cout << "null at "  << cursor.index << std::endl;

    tokens.emplace_back(token_type::NULL_, cursor.index, 4);
    cursor.index += 4;

    JsonWrapper new_value;
    new_value.type = JSON_TYPE::NULL_;
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

bool Physon::is_quotation_mark(){
    return content[cursor.index] == '"' ? true : false;
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

std::string Physon::state_to_string(){

    switch (state){

    case JSON_PARSE_STATE::ARRAY_CLOSE :
        return std::string("ARRAY_CLOSE");
        break;
    
    case JSON_PARSE_STATE::ARRAY_ENTER :
        return std::string("ARRAY_ENTER");
        break;
    case JSON_PARSE_STATE::ARRAY_ENTERED :
        return std::string("ARRAY_ENTERED");
        break;

    case JSON_PARSE_STATE::DONE :
        return std::string("DONE");
        break;
    case JSON_PARSE_STATE::ERROR :
        return std::string("ERROR");
        break;

    case JSON_PARSE_STATE::ROOT_BEFORE_VALUE :
        return std::string("ROOT_BEFORE_VALUE");
        break;
    case JSON_PARSE_STATE::ROOT_END_OF_VALUE :
        return std::string("ROOT_END_OF_VALUE");
        break;

    case JSON_PARSE_STATE::OBJECT_ENTER :
        return std::string("OBJECT_ENTER");
        break;
    case JSON_PARSE_STATE::OBJECT_PARSE_KEY_COMMA :
        return std::string("OBJECT_PARSE_KEY_COMMA");
        break;
    case JSON_PARSE_STATE::OBJECT_CLOSE :
        return std::string("OBJECT_CLOSE");
        break;

    case JSON_PARSE_STATE::VALUE_AT_NEW_VALUE_CHAR :
        return std::string("VALUE_AT_NEW_VALUE_CHAR");
        break;
    case JSON_PARSE_STATE::VALUE_PARSE_LITERAL :
        return std::string("VALUE_PARSE_LITERAL");
        break;
    case JSON_PARSE_STATE::VALUE_END_OF_VALUE :
        return std::string("VALUE_END_OF_VALUE");
        break;
    

    default:
        return std::string("NO_STRING_REPRESENTATION_FOR_STATE");
        break;
    }

}

void Physon::json_error(std::string error_msg){


    std::string state_string = " ... State: " + state_to_string() + ". ";

    std::string index_msg = "Content index : " + std::to_string(cursor.index) + ", Char: '" + content.substr(cursor.index, 1) + "'.";

    std::string full_error_msg = error_msg + state_string + index_msg;


    state = JSON_PARSE_STATE::ERROR;
    throw std::runtime_error(full_error_msg);

}

void Physon::add_value_to_current_container(JsonWrapper value){

    switch (current_container_type()){

    case JSON_TYPE::NONE: // Not in container ==> new value is root value
        root_value = value;
        break;

    case JSON_TYPE::ARRAY:
        {
            json_array_wrap& real_array = store.get_array(cursor.container_trace.top().store_id);
            real_array.push_back(value);
            
        }
        break;
    case JSON_TYPE::OBJECT:
        {
            json_object_wrap& real_object = store.get_object(cursor.container_trace.top().store_id);
            
            real_object.push_back(value);

            // push kv
            cursor.container_trace.push(value);

        }
        break;
    case JSON_TYPE::KV :
        {
            json_kv_wrap& real_kv = store.get_kv(cursor.container_trace.top().store_id);
            
            real_kv.second = value;

            // pop kv
            cursor.container_trace.pop();
        }
        break;
    
    default:
        break;
    }
}


void Physon::parse() {

    cursor.index = 0;
    store.clear();
    state = JSON_PARSE_STATE::ROOT_BEFORE_VALUE;
    
    // Main Parsing loop
    while(cursor.index < content.size()){

        switch (state){

        case JSON_PARSE_STATE::ROOT_BEFORE_VALUE:
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


        case JSON_PARSE_STATE::OBJECT_ENTER:
            object_enter();
            break;
        case JSON_PARSE_STATE::OBJECT_PARSE_KEY_COMMA:
            object_parse_key_comma();
            break;
        case JSON_PARSE_STATE::OBJECT_CLOSE:
            object_close();
            break;
            

        case JSON_PARSE_STATE::VALUE_AT_NEW_VALUE_CHAR:
            value_at_new_value_char();
            break;
        case JSON_PARSE_STATE::VALUE_PARSE_LITERAL:
            value_parse_literal();
            break;
        case JSON_PARSE_STATE::VALUE_END_OF_VALUE:
            value_end_of_value();
            break;
            

        case JSON_PARSE_STATE::ROOT_END_OF_VALUE:
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