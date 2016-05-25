//#include <SPI.h>
#include <SD.h>
#include "test_bpp_tree_handler.h"

void
setup(
)
{
    Serial.begin(9600);
    run_all_tests_bpptreehandler();
    Serial.end();
}

void
loop(
)
{

}