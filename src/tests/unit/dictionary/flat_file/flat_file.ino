#include <SPI.h>
#include <SD.h>

void
runalltests_flat_file();

void
runalltests_flat_file_handler();

void
setup(
)
{
    Serial.begin(9600);
    runalltests_flat_file();
    runalltests_flat_file_handler();
    Serial.end();
}

void
loop(
)
{

}