#include "../../../../dictionary/linear_hash/linear_hash_handler.c"
int
main(
) {
	ion_dictionary_t			dict;
	ion_dictionary_handler_t	handler;

	linear_hash_dict_init(&handler);
	dictionary_create(&handler, &dict, 1, key_type_numeric_unsigned, sizeof(ion_key_t), sizeof(ion_value_t), sizeof(ion_dictionary_t));
	dict.handler->insert(&dict, 10, 1);
/*	dict.handler->insert(&dict, 10, 1); */
/*	dict.handler->insert(&dict, 10, 1); */
/*	dict.handler->insert(&dict, 10, 1); */
/*	dict.handler->insert(&dict, 10, 1); */
/*	dict.handler->insert(&dict, 10, 1); */
/*	dict.handler->insert(&dict, 10, 1); */
/*	dict.handler->insert(&dict, 10, 1); */
/*	dict.handler->insert(&dict, 5, 5); */
/*	dict.handler->insert(&dict, 5, 5); */
/*	dict.handler->insert(&dict, 5, 5); */
/*	dict.handler->insert(&dict, 5, 5); */
/*	dict.handler->insert(&dict, 15, 15); */
/*	dict.handler->insert(&dict, 15, 15); */
/*	dict.handler->insert(&dict, 15, 15); */
/*	dict.handler->insert(&dict, 20, 20); */
/*	dict.handler->insert(&dict, 20, 20); */
/*	dict.handler->insert(&dict, 20, 20); */
/*	dict.handler->insert(&dict, 20, 20); */
/*	dict.handler->insert(&dict, 20, 20); */
/*	dict.handler->insert(&dict, 25, 25); */
/*	dict.handler->insert(&dict, 25, 25); */
/*	dict.handler->insert(&dict, 25, 25); */
/*	dict.handler->insert(&dict, 25, 25); */
/*	dict.handler->insert(&dict, 50, 50); */
/*	dict.handler->insert(&dict, 50, 50); */
/*	dict.handler->insert(&dict, 50, 50); */
/*	dict.handler->insert(&dict, 50, 50); */
/*	dict.handler->insert(&dict, 1, 1); */
/*	dict.handler->insert(&dict, 1, 1); */
/*	dict.handler->insert(&dict, 1, 1); */
/*	dict.handler->insert(&dict, 1, 1); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 2, 2); */
/*	dict.handler->insert(&dict, 12, 12); */
/*	dict.handler->insert(&dict, 12, 12); */
/*	dict.handler->insert(&dict, 12, 12); */
/*	dict.handler->insert(&dict, 12, 12); */
/*	dict.handler->insert(&dict, 12, 12); */
/*	dict.handler->insert(&dict, 12, 12); */
/*	dict.handler->insert(&dict, 12, 12); */
/*	dict.handler->insert(&dict, 12, 12); */
/*	dict.handler->insert(&dict, 12, 12); */
/*	dict.handler->insert(&dict, 122, 122); */
/*	dict.handler->insert(&dict, 122, 122); */
/*	dict.handler->insert(&dict, 122, 122); */
/*	dict.handler->insert(&dict, 122, 122); */
/*	dict.handler->insert(&dict, 122, 122); */
/*	dict.handler->insert(&dict, 13, 13); */
/*	dict.handler->insert(&dict, 13, 13); */
/*	dict.handler->insert(&dict, 13, 13); */
/*	dict.handler->insert(&dict, 13, 13); */
/*	dict.handler->insert(&dict, 13, 13); */
/*	dict.handler->insert(&dict, 13, 13); */
/*	dict.handler->insert(&dict, 13, 13); */
/*	dict.handler->insert(&dict, 13, 13); */
/*	dict.handler->insert(&dict, 13, 13); */
/*	dict.handler->insert(&dict, 17, 17); */
/*	dict.handler->insert(&dict, 17, 17); */
/*	dict.handler->insert(&dict, 17, 17); */
/*	dict.handler->insert(&dict, 17, 17); */
/*	dict.handler->insert(&dict, 17, 17); */
/*	dict.handler->insert(&dict, 17, 17); */
/*	dict.handler->insert(&dict, 16, 16); */
/*	dict.handler->insert(&dict, 26, 26); */
/*	dict.handler->insert(&dict, 26, 26); */
/*	dict.handler->insert(&dict, 16, 16); */
/*	dict.handler->insert(&dict, 66, 66); */
/*	dict.handler->insert(&dict, 25, 25); */
/*  */
/*	print_linear_hash_state(dict.instance); */
/*  */
/* //	linear_hash_get(10, dict.instance); */
/* //	dict.handler->remove(&dict, 10); */
/* //	printf("\nPERFORMING GET 5\n"); */
/* //	linear_hash_get(10, dict.instance); */
/*  */
/*	print_linear_hash_bucket_from_idx(5, dict.instance); */

	return 0;
}
