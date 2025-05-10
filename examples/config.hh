#include <string>
#include <vector>

#include "../physon.hh"
#include "../physon_types.hh"

/** 
    The primary json config tool. Intended to be used exclusively as a base class.
    A new Derived class is created for each confuration type.
  */
struct Config {
    Physon physon;
    std::string physon_string;

    Config(std::string physon_str) : physon {Physon(physon_str) } {};
};

