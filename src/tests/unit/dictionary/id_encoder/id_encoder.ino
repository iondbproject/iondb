#include <SPI.h>
#include <SD.h>

void
runalltests_file_encode_id(
);

void
setup(
)
{
    Serial.begin(9600);
    runalltests_file_encode_id();
    Serial.end();
}

void
loop(
)
{

}