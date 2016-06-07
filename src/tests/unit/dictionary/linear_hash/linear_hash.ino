#include <SPI.h>
#include <SD.h>
#include "test_linear_hash.h"
#include "test_linear_hash_dictionary.h"
#include "test_linear_hash_dictionary_handler.h"
#include "test_file_linked_list.h"

void
setup(
)
{
    SPI.begin();
    SD.begin(SD_CS_PIN);
    Serial.begin(BAUD_RATE);
    runalltests_file_linked_list();
    runalltests_linear_hash();
    runalltests_linear_hash_handler();
    run_all_tests_linearHashDictionary();
}

void
loop(
)
{

}