#pragma once

#include <vector>
#include <stack>
#include <iostream>
#include <fstream>
#include <string>

enum class json_value_type {
    STRING = 0,
    NUMBER,
    TRUE,
    FALSE,
    NULL_,

    ARRAY,
    OBJECT,
    KV,

    NONE, /** When no type is valid */
};

struct json_value {
    int store_id = 0; // unique identifier for specific type
    json_value_type type = json_value_type::NONE;

    // equal if identical type and store id
    bool operator==(const json_value& other) const {
        return type == other.type && store_id == other.store_id;
    }

    json_value () : store_id {0},type {json_value_type::NONE} {};
    json_value (json_value_type _type) : type {_type} {};
    json_value (int _store_id, json_value_type _type) : store_id {_store_id}, type {_type} {};
};

typedef std::string     json_string;
typedef bool            json_bool;
typedef std::nullptr_t  json_null;
typedef double          json_float;
typedef long long int   json_int;

typedef std::pair<json_string, json_value> json_kv;
typedef std::vector<json_value> 
                        json_array;
typedef std::vector<json_value> 
                        json_object;




/** 
    JSON data storage. 
    The json object/array vectors store the array/object tree.
 */
struct json_store {

    std::vector<long long int> integers;
    std::vector<double> floats;
    std::vector<std::string> strings;

    std::vector<json_array> arrays;
    std::vector<json_object> objects;
    std::vector<json_kv> kvs;

    int add_integer(long long int value);
    long long int& get_integer(int id);

    int add_float(double value);
    double& get_float(int id);

    int add_string(std::string new_str){
        strings.push_back(new_str);
        return strings.size() - 1;
    }
    std::string& get_string(int id){
        return strings[id];
    }

    json_value new_array(){
        arrays.emplace_back();

        json_value array;
        array.store_id = arrays.size() - 1;
        array.type = json_value_type::ARRAY;

        return array;
    }
    json_array& get_array(int id){
        return arrays[id];
    }

    json_value new_object(){
        objects.emplace_back();

        json_value object;
        object.store_id = objects.size() - 1;
        object.type = json_value_type::OBJECT;

        return object;
    }
    json_object& get_object(int id){
        return objects[id];
    }
    json_value new_kv(json_string key){
        
        // Store key-string
        json_kv& kv = kvs.emplace_back();
        kv.first = key;

        json_value value (kvs.size()-1, json_value_type::KV);

        return value;
    }
    json_kv& get_kv(int id){
        return kvs[id];
    }

    void clear() {
        integers.clear();
        floats.clear();
        strings.clear();
        objects.clear();
        arrays.clear();
    }

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

enum class JSON_PARSE_STATE {

    ROOT_BEFORE_VALUE,      /** Looking for the root value */
    ROOT_END_OF_VALUE,      /** Reached end of root value */
    DONE,                   /** Parsing done. Return primary parse method. */

    ARRAY_ENTERED,
    ARRAY_ENTER,
    ARRAY_CLOSE,

    OBJECT_ENTER,
    OBJECT_PARSE_KEY_COMMA,
    OBJECT_PARSE_VALUE,
    OBJECT_CLOSE,

    NEW_KV,

    VALUE_AT_NEW_VALUE_CHAR,
    VALUE_PARSE_LITERAL,
    VALUE_END_OF_VALUE,

    ERROR,
};


struct ParserCursor {
    size_t index = 0;
    JSON_PARSE_STATE state = JSON_PARSE_STATE::ROOT_BEFORE_VALUE;  // Keeps track of the current parsing state
    // json_element current_element;             // element cursor
    // json_element current_container;           // container cursor
    // json_value_type current_container_type; // current container type (array or object)
    std::stack<json_value> container_trace; // stack of current container level

};

