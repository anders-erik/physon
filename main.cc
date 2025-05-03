#include "physon.hh"


int main (int argc, char **argv) {

    std::cout << "Hello, Physon!" << std::endl;

    Physon physon;

    // physon.load_file("data/true.json");
    // physon.load_file("data/false.json");
    physon.load_file("data/null.json");
    // physon.load_file("data/penpaper.json");

    physon.parse();

    physon.print_string();
    physon.print_tokens();


    // bool space =  is_whitespace(' ');
    // bool tab =  is_whitespace('\t');
    // bool new_line =  is_whitespace('\n');
    // bool c_return =  is_whitespace('\r');

    return 0;
}