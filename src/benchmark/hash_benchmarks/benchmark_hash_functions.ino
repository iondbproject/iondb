#include "hash_functions.h"
#include "hash_functions.c"
#include <string.h>

int sampleSize = 1000;
int maxLength = 256;

void test_hashfunction(uint32_t (*hashFunc)(char *, size_t), char *name) {
    Serial.print(F("Testing "));
    Serial.print(name);
    Serial.println();

    char *bytes = (char *) calloc(maxLength, sizeof(char));
    strncpy(bytes, "The Quick Brown Fox Jumps over the Lazy Dog", maxLength);
    uint32_t outArray[4] = {0};
    for (int i = 1; i <= maxLength; i++) {
        long InitTime = micros();
        uint32_t val;
        for (int j = 0; j < sampleSize; j++) {
            val = (*hashFunc)(bytes, i);
        }
        long totalTime = micros() - InitTime;
        Serial.print(i);
        Serial.print(", ");
        Serial.print(totalTime);
        Serial.print(", ");
        Serial.println(val);
    }
    Serial.println("Done");
}

void setup() {
    Serial.begin(BAUD_RATE);
//    test_hashfunction(sdbm, "sdbm");
//    test_hashfunction(jenkins, "jenkins");
//    test_hashfunction(fnv1a, "fnv1a");
//    test_hashfunction(djb2, "djb2");
    test_hashfunction(SuperFastHash, "superfasthash");
}

void loop() {

}