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