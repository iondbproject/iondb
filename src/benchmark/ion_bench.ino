#include "dictionary.h"
#include "benchmark.h"

/* NOTICE: Due to changes to IONIZE, this benchmarking suite will no longer compile. */

/* #include "slhandler.h" */
/* #include "oadictionaryhandler.h" */
/* #include "oafdictionaryhandler.h" */
/* #include "ffdictionaryhandler.h" */
/* #include "dictionary/bpp_tree/bpptreehandler.h" */

#include <SD.h>
#include "sd_stdio_c_iface.h"

#include "lfsr.h"

#define SHOW()				printf("%s: ", __func__)
/**< MAKE_ION_KEY :: int -> ion_key_t (byte*) */
#define MAKE_ION_KEY(x)		(_keyswap = x, &_keyswap)
/**< ION_KEY_TO_INT :: ion_key_t (byte*) -> int */
#define ION_KEY_TO_INT(key) *((int *) key)

static int _keyswap;

/* CONFIG PARAMS */

/**< Handler of dict to test. */
/* void			  (*handler_fptr)(ion_dictionary_handler_t*)  = sldict_init; */
/* void			  (*handler_fptr)(ion_dictionary_handler_t*)  = oadict_init; */
/* void			  (*handler_fptr)(ion_dictionary_handler_t*)  = oafdict_init; */
/* void			  (*handler_fptr)(ion_dictionary_handler_t*)  = ffdict_init; */
/* void			  (*handler_fptr)(ion_dictionary_handler_t*)  = bpptree_init; */

/**< Type of key to test. */
ion_key_type_t key_type			= key_type_numeric_signed;
/**< Size of key to test. (Default = 2) */
ion_key_size_t key_size		= 2;
/**< Size of value to test. (Default = 8) */
ion_value_size_t value_size = 8;
/**< Size of dict to test. */
int dict_size				= 10;

/**< Dictionary instances. */
ion_dictionary_t			dict;
ion_dictionary_handler_t	handler;
/**< Value payload. */
ion_value_t test_value = (char *) {
	"IonDB Test String"
};
/**< Number sequence */
lfsr_t keygen;

/**< LFSR Seed */
#define ION_KEY_SEED 0xACE1u

void
setup(
) {
	/* Arduino stuff */
	Serial.begin(9600);
	Serial.println("ready!");

	/* File stuff */
	/* pinMode(10, OUTPUT); */
	pinMode(53, OUTPUT);

	if (!SD.begin(4)) {
		Serial.println("sd init failed!");
		Serial.flush();
		return;
	}

	/* FILE * file; */
	/* //remove the file before starting */
	/* fremove("FILE.BIN"); */

	/* LFSR stuff */
	lfsr_init_start_state(ION_KEY_SEED, &keygen);

	fremove("FIXME.val");
	fremove("FIXME.bpt");
}

void
bench_dict_initialize(
	void
) {
	handler_fptr(&handler);
	dictionary_create(&handler, &dict, 1, key_type, key_size, value_size, dict_size);
	lfsr_reset(&keygen);
}

void
bench_dict_cleanup(
	void
) {
	dictionary_delete_dictionary(&dict);
}

void
bench_dict_memusage(
	void
) {
	SHOW();
	benchmark_start();
	bench_dict_initialize();
	benchmark_stop();
	bench_dict_cleanup();
}

void
bench_insert_memusage(
	void
) {
	SHOW();
	bench_dict_initialize();

	benchmark_start();
	dictionary_insert(&dict, IONIZE(0, int), test_value);
	benchmark_stop();

	bench_dict_cleanup();
}

void
bench_insert(
	int count
) {
	SHOW();
	bench_dict_initialize();

	int i;

	benchmark_start();

	for (i = 0; i < count; i++) {
		ion_key_t key = MAKE_ION_KEY(lfsr_get_next(&keygen));

		dictionary_insert(&dict, key, test_value);
	}

	benchmark_stop();

	bench_dict_cleanup();
}

void
bench_get(
	int max,
	int count
) {
	SHOW();
	bench_dict_initialize();

	int i;

	for (i = 0; i < max; i++) {
		ion_key_t key = MAKE_ION_KEY(lfsr_get_next(&keygen));

		dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	benchmark_start();

	for (i = 0; i < count; i++) {
		ion_key_t	key = MAKE_ION_KEY(lfsr_get_next(&keygen));
		char		value[value_size];

		dictionary_get(&dict, key, value);
	}

	benchmark_stop();

	bench_dict_cleanup();
}

void
bench_delete(
	int max,
	int count
) {
	SHOW();
	bench_dict_initialize();

	int i;

	for (i = 0; i < max; i++) {
		ion_key_t key = MAKE_ION_KEY(lfsr_get_next(&keygen));

		dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	benchmark_start();

	for (i = 0; i < count; i++) {
		ion_key_t key = MAKE_ION_KEY(lfsr_get_next(&keygen));

		dictionary_delete(&dict, key);
	}

	benchmark_stop();

	bench_dict_cleanup();
}

void
bench_equality(
	int max,
	int count
) {
	SHOW();
	bench_dict_initialize();

	int i;

	for (i = 0; i < max; i++) {
		ion_key_t key = MAKE_ION_KEY(lfsr_get_next(&keygen));

		dictionary_insert(&dict, key, test_value);
	}

	lfsr_reset(&keygen);
	benchmark_start();

	for (i = 0; i < count; i++) {
		ion_key_t		key		= MAKE_ION_KEY(lfsr_get_next(&keygen));
		ion_dict_cursor_t	*cursor = NULL;
		ion_predicate_t		predicate;

		dictionary_build_predicate(&predicate, predicate_equality, key);

		ion_err_t			status = dictionary_find(&dict, &predicate, &cursor);
		ion_record_t	record;

		record.key		= malloc(dict.instance->record.key_size);
		record.value	= malloc(dict.instance->record.value_size);

		while (cursor->next(cursor, &record) != cs_end_of_results) {
			;
		}

		cursor->destroy(&cursor);
		free(record.key);
		free(record.value);
	}

	benchmark_stop();

	bench_dict_cleanup();
}

void
bench_range(
	int num_inserts,
	int whichhalf
) {
	SHOW();
	bench_dict_initialize();

	int i;
	int min;
	int max;

	for (i = 0; i < num_inserts; i++) {
		ion_key_t key = MAKE_ION_KEY(lfsr_get_next(&keygen));

		if (i == 0) {
			min = ION_KEY_TO_INT(key);
			max = ION_KEY_TO_INT(key);
		}
		else {
			int curkey = ION_KEY_TO_INT(key);

			if (curkey > max) {
				max = curkey;
			}
			else if (curkey < min) {
				min = curkey;
			}
		}

		dictionary_insert(&dict, key, test_value);
	}

	int int_lower_bound;
	int int_upper_bound;

	if (whichhalf == 0) {
		/* Pick lower half */
		int_lower_bound = min;
		int_upper_bound = min + (max - min) / 2;
	}
	else if (whichhalf == 1) {
		int_lower_bound = min + (max - min) / 2;
		int_upper_bound = max;
	}

	ion_key_t	lower_bound = &int_lower_bound;
	ion_key_t	upper_bound = &int_upper_bound;

	benchmark_start();

	ion_dict_cursor_t	*cursor = NULL;
	ion_predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_range, lower_bound, upper_bound);

	ion_err_t			status = dictionary_find(&dict, &predicate, &cursor);
	ion_record_t	record;

	record.key		= malloc(dict.instance->record.key_size);
	record.value	= malloc(dict.instance->record.value_size);

	while (cursor->next(cursor, &record) != cs_end_of_results) {
		;
	}

	cursor->destroy(&cursor);
	free(record.key);
	free(record.value);
	benchmark_stop();

	bench_dict_cleanup();
}

void
loop(
) {
	while (!Serial.available()) {}	/* Wait */

	char testcase = Serial.read();

	Serial.print("(");
	Serial.print(testcase);
	Serial.print(") ");

	switch (testcase) {
		case 'a': {
			bench_insert(50);
			break;
		}

		case 'b': {
			bench_insert(100);
			break;
		}

		case 'c': {
			bench_insert(150);
			break;
		}

		case 'd': {
			bench_insert(200);
			break;
		}

		case 'e': {
			bench_get(200, 50);
			break;
		}

		case 'f': {
			bench_get(200, 100);
			break;
		}

		case 'g': {
			bench_get(200, 150);
			break;
		}

		case 'h': {
			bench_get(200, 200);
			break;
		}

		case 'i': {
			bench_delete(200, 50);
			break;
		}

		case 'j': {
			bench_delete(200, 100);
			break;
		}

		case 'k': {
			bench_delete(200, 150);
			break;
		}

		case 'l': {
			bench_delete(200, 200);
			break;
		}

		case 'm': {
			bench_equality(150, 50);
			break;
		}

		case 'n': {
			bench_equality(150, 100);
			break;
		}

		case 'o': {
			bench_equality(150, 150);
			break;
		}

		case 'p': {
			bench_range(150, 0);
			break;
		}

		case 'q': {
			bench_range(150, 1);
			break;
		}

		case 'r': {
			bench_dict_memusage();
			break;
		}

		case 's': {
			bench_insert_memusage();
			break;
		}

		default: {
			Serial.println("Invalid test case");
		}
	}
}
