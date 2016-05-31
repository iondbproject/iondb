//
// Created by Dana Klamut on 2016-05-31.
//

//#include <iostream>
#include "BppTree.h"

int main() {
    //std::cout << "hello";

    BppTree *bpp_dict = new BppTree(key_type_numeric_unsigned, 4, 8);

    bpp_dict->insert((key_type_t) 6, "Hello");
    bpp_dict->insert((key_type_t) 4, "Goodbye");

    char **val;
    bpp_dict->get((key_type_t) 6, val);

    bpp_dict->deleteKey((key_type_t) 6);

    bpp_dict->update((key_type_t) 6, "Hi");

    bpp_dict->allRecords();

    bpp_dict->range("Hi", "Hey");

    bpp_dict->equality((key_type_t) 4);

    bpp_dict->close();

    bpp_dict->destroy();

    return EXIT_SUCCESS;

}