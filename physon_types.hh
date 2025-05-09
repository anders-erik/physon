#pragma once

#include <vector>
#include <stack>
#include <iostream>
#include <fstream>
#include <string>

void print_type_sizes();


enum class JSON_TYPE {
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

/** JSON value wrapper object.  */
struct JsonWrapper {

    int store_id = 0; // unique identifier for specific type
    JSON_TYPE type = JSON_TYPE::NONE;

    JsonWrapper () : store_id {0},type {JSON_TYPE::NONE} {};
    JsonWrapper (JSON_TYPE _type) : type {_type} {};
    JsonWrapper (int _store_id, JSON_TYPE _type) : store_id {_store_id}, type {_type} {};

    // equal if identical type and store id
    bool operator==(const JsonWrapper& other) const {
        return type == other.type && store_id == other.store_id;
    }

    bool is_bool();
};

typedef std::string     json_string;
typedef bool            json_bool;
typedef std::nullptr_t  json_null;
typedef double          json_float;
typedef long long int   json_int;

// Wrapper containers
typedef std::pair<json_string, JsonWrapper> json_kv_wrap;
typedef std::vector<JsonWrapper>            json_array_wrap;
typedef std::vector<JsonWrapper>            json_object_wrap;

// Value containers
struct JsonValue;
typedef std::vector<JsonValue> json_array_;
typedef std::vector<JsonValue> json_object_;
typedef std::pair<json_string, JsonValue> json_kv_;

union JsonUnion {
    json_string     string_;
    json_bool       bool_;
    json_null       null_;
    json_float      float_;
    json_int        int_;

    json_array_      array_;
    json_object_     object_;
    // json_kv_         kv_;
};


struct JsonValue
{
    int id;             // Global object index in JsonValue storage
    JSON_TYPE type_;

    JsonUnion   union_; 
    
    // Idea: store kv by simply storing key-string and keeping track of which value
    // E.g. is type_ = KV -> return pair<> (but pair is not stored in the union itself)
    // kv_type is the normally store JsonValue-data
    json_string kv_key_;
    JSON_TYPE kv_type;

};


void print_type_sizes(){
    std::cout << " sizeof(json_string)  = "  << sizeof(json_string) << std::endl;
    std::cout << " sizeof(json_bool)    = "  << sizeof(json_bool) << std::endl;
    std::cout << " sizeof(json_null)    = "  << sizeof(json_null) << std::endl;
    std::cout << " sizeof(json_float)   = "  << sizeof(json_float) << std::endl;
    std::cout << " sizeof(json_int)     = "  << sizeof(json_int) << std::endl;
    std::cout << " sizeof(json_kv_wrap)      = "  << sizeof(json_kv_wrap) << std::endl;
    std::cout << " sizeof(json_array_wrap)   = "  << sizeof(json_array_wrap) << std::endl;
    std::cout << " sizeof(json_object_wrap)  = "  << sizeof(json_object_wrap) << std::endl;
}


/** 
    JSON data storage. 
    The json object/array vectors store the array/object tree.
 */
struct json_store {

    std::vector<json_bool>      bools;
    std::vector<json_int>       integers;
    std::vector<json_float>     floats;
    std::vector<std::string>    strings;
    

    std::vector<json_array_wrap>     arrays;
    std::vector<json_object_wrap>    objects;
    std::vector<json_kv_wrap>        kvs;


    // int add_bool(json_bool new_bool){
    //     bools.push_back(new_bool);
    //     return bools.size() - 1;
    // }
    // const json_bool& get_bool(int id){
    //     return bools[id];
    // }

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

    JsonWrapper new_array(){
        arrays.emplace_back();

        JsonWrapper array;
        array.store_id = arrays.size() - 1;
        array.type = JSON_TYPE::ARRAY;

        return array;
    }
    json_array_wrap& get_array(int id){
        return arrays[id];
    }

    JsonWrapper new_object(){
        objects.emplace_back();

        JsonWrapper object;
        object.store_id = objects.size() - 1;
        object.type = JSON_TYPE::OBJECT;

        return object;
    }
    json_object_wrap& get_object(int id){
        return objects[id];
    }
    JsonWrapper new_kv(json_string key){
        
        // Store key-string
        json_kv_wrap& kv = kvs.emplace_back();
        kv.first = key;

        JsonWrapper value (kvs.size()-1, JSON_TYPE::KV);

        return value;
    }
    json_kv_wrap& get_kv(int id){
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
    // JSON_TYPE current_container_type; // current container type (array or object)
    std::stack<JsonWrapper> container_trace; // stack of current container level

};

