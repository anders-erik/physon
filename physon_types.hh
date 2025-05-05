#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

enum class json_value_type {
    STRING = 0,
    NUMBER,
    OBJECT,
    ARRAY,
    TRUE,
    FALSE,
    NULL_,
};

struct json_value {
    int store_id; // unique identifier for specific type
    json_value_type type;
};

typedef std::string     json_string;
typedef bool            json_bool;
typedef std::nullptr_t  json_null;
typedef double          json_float;
typedef long long int   json_int;
typedef std::pair<json_string, json_value> json_kv;

typedef std::vector<json_value> 
                        json_array;
typedef std::vector<json_kv> 
                        json_object;


struct json_element {
    json_value value; // value of the element

    json_value parent;

    // equal if identical type and store id
    bool operator==(const json_element& other) const {
        return value.type == other.value.type && value.store_id == other.value.store_id;
    }

    json_value& find(std::string key);       // for json_object - not recursive
    json_array& all_kv();          // for json_object
    json_object& all_entries();  // for json_array
};


/** Lists with all json data. This is kept separate from the json tree and needs to be queried when a json_value is read. */
struct json_store {

    std::vector<long long int> integers;
    std::vector<double> floats;
    std::vector<std::string> strings;

    std::vector<json_object> objects;
    std::vector<json_array> arrays;

    int add_integer(long long int value);
    long long int& get_integer(int id);

    int add_float(double value);
    double& get_float(int id);

    int add_string(std::string value);
    std::string& get_string(int id);

    int add_array(){
        arrays.emplace_back();
        return arrays.size() - 1;
    } 
    json_array& get_array(int id){
        return arrays[id];
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

enum class parse_state {
    ROOT, // Looking for the root value
    ARRAY_START,
    ARRAY_END,
    ARRAY_NEW_VALUE,
};


struct ParserCursor {
    size_t index = 0;
    parse_state state = parse_state::ROOT;  // Keeps track of the current parsing state
    json_element current_element;             // element cursor
    json_element current_container;           // container cursor
    json_value_type current_container_type; // current container type (array or object)
};

