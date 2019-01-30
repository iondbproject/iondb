#include <SPI.h>
#include <SD.h>
#include <utility/Sd2Card.h>

uint8_t buffer[512];

Sd2Card card;

void
setup(
) {
    randomSeed(analogRead(0));
    SPI.begin();
    if (!SD.begin(SD_CS_PIN)) {
        Serial.print(F("Unable to initialize card\n"));
        return;
    }
    Serial.begin(BAUD_RATE);
    while (!Serial) { ;
    }
    Serial.print(F("Starting File based Benchmarks\n"));

    Serial.print(F("Benchmarking writing 1000 file blocks\n"));
    for (int i = 0; i < 5; i++) {
        benchmark_writes();
        benchmark_read_sequential();
        benchmark_read_random();
    }

    Serial.print(F("SD File Benchmarks complete"));

    SD.end();

    Serial.print(F("Starting raw benchmarks\n"));
    if (1 == card.init(SPI_HALF_SPEED, SD_CS_PIN)) {
        Serial.print(F("Opened raw card\n"));
        for (int i = 0; i < 5; i++) {
            benchmark_raw_sequential_reads();
            benchmark_raw_random_reads();
            benchmark_raw_sequential_writes();
            benchmark_raw_random_writes();
            benchmark_raw_overwrites();
        }
    } else {
        Serial.print(F("Failed to open raw card\n"));
    }
    Serial.print(F("Completed Raw benchmarks"));
}

void
loop(
) {}


void randomize_buffer() {
    uint64_t it;
    for (unsigned int i = 0; i < 512; i += 64) {
        it = *(buffer + i);
        it = ~(it >> 16);
    }
    buffer[0] = random(1000);
}

//region File Reading

void
benchmark_read_random() {
    Serial.print(F("Reading 1000 random access blocks from the file.\n"));
    File file = SD.open("block.dat", O_READ);

    if (NULL == file) {
        Serial.print(F("Unable to open file\n"));
        return;
    }

    unsigned long start = millis();
    size_t result;
    for (uint32_t i = 0; i < 1000; i++) {
        uint32_t block = random(1000);
        file.seek(block * 512);
        if (512 != file.read(buffer, 512)) {
            Serial.print(F("Unable to read the file block\n"));
            break;
        }
    }
    unsigned long time = millis() - start;
    file.close();
    Serial.print(F("Completed reading in "));
    Serial.print(time);
    Serial.print(F(" ms\n"));
}

void
benchmark_read_sequential() {
    Serial.print(F("Reading 1000 sequential blocks from the file.\n"));
    File file = SD.open("block.dat", O_READ);

    if (NULL == file) {
        Serial.print(F("Unable to open file\n"));
        return;
    }

    unsigned long start = millis();
    size_t result;
    for (uint32_t i = 0; i < 1000; i++) {
        if (512 != file.read(buffer, 512)) {
            Serial.print(F("Unable to read the file block\n"));
            break;
        }
    }
    unsigned long time = millis() - start;
    file.close();
    Serial.print(F("Completed reading in "));
    Serial.print(time);
    Serial.print(F(" ms\n"));
}

//endregion

void
benchmark_sequential_writes() {
    Serial.print(F("Writing a file with 1000 blocks\n"));
    File file = SD.open("block.dat", O_RDWR | O_CREAT);

    unsigned long start = millis();
    size_t result;
    for (int i = 0; i < 1000; i++) {
        randomize_buffer();
        if (512 != file.write(buffer, 512)) {
            Serial.print(F("Unable to write file block\n"));
            break;
        }
    }
    unsigned long time = millis() - start;
    file.close();
    Serial.print(F("Completed writing in "));
    Serial.print(time);
    Serial.print(F(" ms\n"));
}

/**
 * Benchmarks first writing a file with 1000 blocks and then times how long it takes to over write block 0 and 1 with random
 * data 1000 times.
 */
void
benchmark_writes() {
    Serial.print(F("Writing a file with 1000 blocks\n"));
    File file = SD.open("block.dat", O_RDWR | O_CREAT);

    unsigned long start = millis();
    size_t result;
    for (int i = 0; i < 1000; i++) {
        randomize_buffer();
        if (512 != file.write(buffer, 512)) {
            Serial.print(F("Unable to write file block\n"));
            break;
        }
    }
    unsigned long time = millis() - start;

    Serial.print(F("Completed writing a file with 1000 serial blocks in "));
    Serial.print(time);
    Serial.print(F(" ms\n"));

    Serial.print(F("Writing 1000 blocks ontop of block 0 and 1 in the file\n"));
    if (!file.seek(0)) {
        Serial.print(F("Unable to seek to start of file\n"));
    } else {
        start = millis();
        unsigned int block = 0;
        unsigned long offset = 0;
        for (int i = 0; i < 1000; i++) {
            offset = (i % 2) * 512;
            if (!file.seek(offset)) {
                Serial.print(F("Unable to seek to file offset\n"));
                break;
            }
            if (offset == 0) {
                randomize_buffer();
            }
            if (512 != file.write(buffer, 512)) {
                Serial.print(F("Unable to write file block\n"));
                break;
            }
        }
        time = millis() - start;
    }
    Serial.print(F("Completed overwrites in "));
    Serial.print(time);
    Serial.print(F(" ms\n"));


    Serial.print(F("Writing 1000 random blocks in a file of 1000 blocks\n"));
    uint32_t block;
    start = millis();
    for (int i = 0; i < 1000; i++) {
        unsigned long block = random(1000);
        if (!file.seek(block * 512)) {
            Serial.print(F("Unable to write seek to block"));
            Serial.print(block);
            Serial.print(F("\n"));
            break;
        }
        randomize_buffer();
        if (512 != file.write(buffer, 512)) {
            Serial.print(F("Unable to write file block\n"));
            break;
        }
    }
    time = millis() - start;
    Serial.print(F("Completed random writes in "));
    Serial.print(time);
    Serial.print(F(" ms\n"));

    file.close();
}

//region raw reads
/**
 * Benchmarks reading 1000 sequential blocks using the raw file access. Global card must be initialized.
 */
void
benchmark_raw_sequential_reads() {
    Serial.print(F("Starting to read 1000 raw blocks\n"));
    unsigned long start = millis();
    for (uint32_t i = 0; i < 1000; i++) {
        if (0 == card.readBlock(i, buffer)) {
            Serial.print(F("Failed to read block "));
            Serial.print(i);
            Serial.print(F("\n"));
            return;
        }
    }
    unsigned long time = millis() - start;
    Serial.print(F("Read 1000 (raw) blocks in "));
    Serial.print(time);
    Serial.print(F(" ms\n"));
}

/**
 * Benchmarks reading 1000 sequential blocks using the raw file access. Global card must be initialized.
 */
void
benchmark_raw_random_reads() {
    Serial.print(F("Starting to read 1000 raw blocks\n"));
    unsigned long start = millis();
    for (uint32_t i = 0; i < 1000; i++) {
        uint32_t block = random(1000);
        if (0 == card.readBlock(block, buffer)) {
            Serial.print(F("Failed to read block "));
            Serial.print(i);
            Serial.print(F("\n"));
            return;
        }
    }
    unsigned long time = millis() - start;
    Serial.print(F("Read 1000 (raw) random blocks in "));
    Serial.print(time);
    Serial.print(F(" ms\n"));
}
//endregion

//region Raw Writes
/**
 * Benchmarks writing 1000 sequential blocks in raw mode.
 */
void
benchmark_raw_sequential_writes() {
    Serial.print(F("Starting to write 1000 sequential raw blocks\n"));
    unsigned long start = millis();
    for (uint32_t i = 1; i < 1001; i++) {
        randomize_buffer();
        if (0 == card.writeBlock(i, buffer)) {
            Serial.print(F("Failed to write block "));
            Serial.print(i);
            Serial.print(F("\n"));
            return;
        }
    }
    unsigned long time = millis() - start;
    Serial.print(F("Wrote 1000 (raw) blocks in "));
    Serial.print(time);
    Serial.print(F(" ms\n"));
}

/**
 * Benchmarks writing 1000 sequential blocks in raw mode.
 */
void
benchmark_raw_random_writes() {
    Serial.print(F("Starting to write 1000 random raw blocks\n"));
    unsigned long start = millis();
    uint32_t block;
    for (uint32_t i = 1; i < 1001; i++) {
        block = random(1, 1001);
        randomize_buffer();
        if (0 == card.writeBlock(block, buffer)) {
            Serial.print(F("Failed to write block "));
            Serial.print(block);
            Serial.print(F("\n"));
            return;
        }
    }
    unsigned long time = millis() - start;
    Serial.print(F("Wrote 1000 (raw) random blocks in "));
    Serial.print(time);
    Serial.print(F(" ms\n"));
}

/**
 * Benchmarks writing to block 1 and 2 1000 times
 */
void benchmark_raw_overwrites() {
    Serial.print(F("Starting to write 1000 raw blocks on block 1\n"));
    unsigned long start = millis();
    uint32_t block = 1;
    for (uint32_t i = 1; i < 1001; i++) {
        block = (i % 2) + 1;
        if (block == 1) {
            randomize_buffer();
        }
        if (0 == card.writeBlock(block, buffer)) {
            Serial.print(F("Failed to write block "));
            Serial.print(i);
            Serial.print(F("\n"));
            return;
        }
    }
    unsigned long time = millis() - start;
    Serial.print(F("Wrote 1000 (raw) blocks in "));
    Serial.print(time);
    Serial.print(F(" ms\n"));
}

//endregion