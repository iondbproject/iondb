#include "dictionary/dictionary.h"
#include "benchmark/benchmark.h"

#include "dictionary/skiplist/slhandler.h"
//#include "dictionary/openaddresshash/oadictionaryhandler.h"
//#include "dictionary/openaddressfilehash/oafdictionaryhandler.h"
//#include "dictionary/flatfilestore/ffdictionaryhandler.h"

//#include <SD.h>
//#include "sd_if/SD_stdio_c_iface.h"

#include "lfsr/lfsr.h"

#define SHOW() printf("%s: ", __func__)
/**< MAKE_ION_KEY :: int -> ion_key_t (unsigned char*) */
#define MAKE_ION_KEY(x) (_keyswap = x, (ion_key_t) &_keyswap)
/**< ION_KEY_TO_INT :: ion_key_t (unsigned char*) -> int */
#define ION_KEY_TO_INT(key) *((int*) key)

static int _keyswap;

/** CONFIG PARAMS **/

/**< Handler of dict to test. */
err_t               (*handler_fptr)(dictionary_handler_t*)  = sldict_init;
//void              (*handler_fptr)(dictionary_handler_t*)  = oadict_init;
//void              (*handler_fptr)(dictionary_handler_t*)  = oafdict_init;
//void              (*handler_fptr)(dictionary_handler_t*)  = ffdict_init;

/**< Type of key to test. */
key_type_t          key_type                                = key_type_numeric_signed;
/**< Size of key to test. (Default = 2) */
ion_key_size_t      key_size                                = 2;
/**< Size of value to test. (Default = 8) */
ion_value_size_t    value_size                              = 8;
/**< Size of dict to test. */
int                 dict_size                               = 10;

/**< Dictionary instances. */
dictionary_t            dict;
dictionary_handler_t    handler;
/**< Value payload. */
ion_value_t     test_value  = (ion_value_t) (char*){"IonDB Test String"};
/**< Number sequence */
lfsr_t keygen;
/**< LFSR Seed */
#define KEY_SEED 0xACE1u

/**< Current test case */
#define TEST 0

void
setup(
)
{
    /* Arduino stuff */
    Serial.begin(9600);
    Serial.println("ready!");

    /* File stuff */
    pinMode(10, OUTPUT);
    //pinMode(53, OUTPUT);
    if (!SD.begin(4))
    {
        Serial.println("sd init failed!");
        Serial.flush();
        return;
    }
    FILE * file;
    //remove the file before starting
    fremove("FILE.BIN"); 

    /* LFSR stuff */
    lfsr_init_start_state(KEY_SEED, &keygen);

    /* Benchmark stuff */
    switch(TEST)
    {
        case 1: {
            bench_insert(25);
            break;
        }
        case 2: {
            bench_insert(50);
            break;
        }
        case 3: {
            bench_insert(75);
            break;
        }
        case 4: {
            bench_insert(100);
            break;
        }
        case 5: {
            bench_get(100, 20);
            break;
        }
        case 6: {
            bench_get(100, 40);
            break;
        }
        case 7: {
            bench_get(100, 60);
            break;
        }
        case 8: {
            bench_get(100, 80);
            break;
        }
        case 9: {
            bench_delete(100, 20);
            break;
        }
        case 10: {
            bench_delete(100, 40);
            break;
        }
        case 11: {
            bench_delete(100, 60);
            break;
        }
        case 12: {
            bench_delete(100, 80);
            break;
        }
        case 13: {
            bench_equality(100, 30);
            break;
        }
        case 14: {
            bench_equality(100, 60);
            break;
        }
        case 15: {
            bench_equality(100, 90);
            break;
        }
        case 16: {
            bench_range(100, 0);
            break;
        }
        case 17: {
            bench_range(100, 1);
            break;
        }
        default: {
            printf("Invalid test case\n");
        }
    }
}

void
bench_dict_initialize(
    void
)
{
    handler_fptr(&handler);
    dictionary_create(&handler, &dict, key_type, key_size, value_size, dict_size);
    lfsr_reset(&keygen);
}

void
bench_dict_cleanup(
    void
)
{
    dictionary_delete_dictionary(&dict);
}

void
bench_insert(
    int count
)
{
    SHOW();
    bench_dict_initialize();

    int i;
    benchmark_start();
    for(i = 0; i < count; i++)
    {
        ion_key_t       key     = MAKE_ION_KEY(lfsr_get_next(&keygen));
        dictionary_insert(&dict, key, test_value);
    }
    benchmark_stop();

    bench_dict_cleanup();
}

void
bench_get(
    int max,
    int count
)
{
    SHOW();
    bench_dict_initialize();

    int i;
    for(i = 0; i < max; i++)
    {
        ion_key_t       key     = MAKE_ION_KEY(lfsr_get_next(&keygen));
        dictionary_insert(&dict, key, test_value);
    }

    lfsr_reset(&keygen);
    benchmark_start();
    for(i = 0; i < count; i++)
    {
        ion_key_t   key     = MAKE_ION_KEY(lfsr_get_next(&keygen));
        char        value[value_size];
        dictionary_get(&dict, key, (ion_value_t) value);
    }
    benchmark_stop();

    bench_dict_cleanup();
}

void
bench_delete(
    int max,
    int count
)
{
    SHOW();
    bench_dict_initialize();

    int i;
    for(i = 0; i < max; i++)
    {
        ion_key_t       key     = MAKE_ION_KEY(lfsr_get_next(&keygen));
        dictionary_insert(&dict, key, test_value);
    }

    lfsr_reset(&keygen);
    benchmark_start();
    for(i = 0; i < count; i++)
    {
        ion_key_t   key     = MAKE_ION_KEY(lfsr_get_next(&keygen));
        dictionary_delete(&dict, key);
    }
    benchmark_stop();

    bench_dict_cleanup();
}

void
bench_equality(
    int max,
    int count
)
{
    SHOW();
    bench_dict_initialize();

    int i;
    for(i = 0; i < max; i++)
    {
        ion_key_t       key     = MAKE_ION_KEY(lfsr_get_next(&keygen));
        dictionary_insert(&dict, key, test_value);
    }

    lfsr_reset(&keygen);
    benchmark_start();
    for(i = 0; i < count; i++)
    {
        ion_key_t   key     = MAKE_ION_KEY(lfsr_get_next(&keygen));

        dict_cursor_t *cursor = NULL;
        predicate_t predicate;
        predicate.type = predicate_equality;
        predicate.statement.equality.equality_value = key;
        err_t status = dict.handler->find(&dict, &predicate, &cursor);
        ion_record_t record;
        record.key      = (ion_key_t) malloc(dict.instance->record.key_size);
        record.value    = (ion_value_t) malloc(dict.instance->record.value_size);
        while(cursor->next(cursor, &record) != cs_end_of_results);
        cursor->destroy(&cursor);
        free(record.key);
        free(record.value);
    }
    benchmark_stop();

    bench_dict_cleanup();
}

void
bench_range(
    int max,
    int whichhalf
)
{
    SHOW();
    bench_dict_initialize();

    int i;
    for(i = 0; i < max; i++)
    {
        ion_key_t key = MAKE_ION_KEY(lfsr_get_next(&keygen));
        if(i == 0)
        {
            min = ION_KEY_TO_INT(key);
            max = ION_KEY_TO_INT(key);
        }
        else
        {
            int curkey = ION_KEY_TO_INT(key);
            if(curkey > max)
            {
                max = curkey;
            }
            else if(curkey < min)
            {
                min = curkey;
            }
        }
        status = dictionary_insert(&dict, key, test_value);
    }

    if(whichhalf == 0) { // Pick lower half
        int int_leq = min;
        int int_geq = min + (max - min) / 2;
    }
    else if(whichhalf == 1) {
        int int_leq = min + (max - min) / 2;
        int int_geq = max;
    }
    ion_key_t   leq     = (ion_key_t) &int_leq;
    ion_key_t   geq     = (ion_key_t) &int_geq;
    benchmark_start();
    dict_cursor_t *cursor = NULL;
    predicate_t predicate;
    predicate.type = predicate_range;
    predicate.statement.range.leq_value = leq;
    predicate.statement.range.geq_value = geq;
    err_t status = dict.handler->find(&dict, &predicate, &cursor);
    ion_record_t record;
    record.key      = (ion_key_t) malloc(dict.instance->record.key_size);
    record.value    = (ion_value_t) malloc(dict.instance->record.value_size);
    while(cursor->next(cursor, &record) != cs_end_of_results);
    cursor->destroy(&cursor);
    free(record.key);
    free(record.value);
    benchmark_stop();

    bench_dict_cleanup();
}

void loop(){} /* Not needed */