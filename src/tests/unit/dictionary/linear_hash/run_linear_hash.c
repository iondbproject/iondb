#include "test_linear_hash.h"
#include "test_linear_hash_dictionary.h"
#include "test_linear_hash_dictionary_handler.h"
#include "test_file_linked_list.h"

#define NO_SPLIT 1

int
main(
) {
	runalltests_file_linked_list();
	runalltests_linear_hash();
	runalltests_linear_hash_handler();
//	run_all_tests_linearHashDictionary();
	return 0;
}
