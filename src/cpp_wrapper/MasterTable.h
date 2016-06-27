/*  */
/* Created by Dana Klamut on 2016-06-15. */
/*  */

#if !defined(PLANCK_UNIT_MASTERTABLE_H)
#define PLANCK_UNIT_MASTERTABLE_H

#include "../dictionary/ion_master_table.h"
#include "../dictionary/dictionary_types.h"
#include "../dictionary/dictionary.h"
#include "../kv_system.h"
#include "../dictionary/bpp_tree/bpp_tree_handler.h"
#include "../dictionary/skip_list/skip_list_handler.h"
#include "../dictionary/flat_file/flat_file_dictionary_handler.h"
#include "../dictionary/linear_hash/linear_hash_dictionary_handler.h"
/* #include "../dictionary/open_address_file_hash/open_address_file_hash_dictionary_handler.h" */

class MasterTable {
public:

dictionary_handler_t	handler;
dictionary_t			dict;
int						size_k;
int						size_v;
err_t					last_status;

MasterTable(
	void
) {
	err_t err = ion_init_master_table();
}

err_t
createBppTreeDictionary(
	key_type_t	type_key,
	int			key_size,
	int			value_size
) {
	bpptree_init(&this->handler);

	err_t err = ion_master_table_create_dictionary(&handler, &dict, type_key, key_size, value_size, 0);

	size_k	= key_size;
	size_v	= value_size;

	return err;
}

err_t
createFlatFileDictionary(
	key_type_t	type_key,
	int			key_size,
	int			value_size
) {
	ffdict_init(&this->handler);

	err_t err = ion_master_table_create_dictionary(&handler, &dict, type_key, key_size, value_size, 0);

	size_k	= key_size;
	size_v	= value_size;

	return err;
}

err_t
createLinearHashDictionary(
	key_type_t	type_key,
	int			key_size,
	int			value_size,
	int			dictionary_size
) {
	lhdict_init(&this->handler);

	err_t err = ion_master_table_create_dictionary(&handler, &dict, type_key, key_size, value_size, dictionary_size);

	size_k	= key_size;
	size_v	= value_size;

	return err;
}

/*	err_t */
/*	createOpenAddressFileHashDictionary( */
/*			key_type_t	type_key, */
/*			int			key_size, */
/*			int			value_size, */
/*			int			dictionary_size */
/*	) { */
/*		oafdict_init(&this->handler); */
/*  */
/*		err_t err = ion_master_table_create_dictionary(&handler, &dict, type_key, key_size, value_size, dictionary_size */
/*		); */
/*  */
/*		size_k	= key_size; */
/*		size_v	= value_size; */
/*  */
/*		return err; */
/*	} */

err_t
createSkipListDictionary(
	key_type_t	type_key,
	int			key_size,
	int			value_size,
	int			dictionary_size
) {
	sldict_init(&this->handler);

	err_t err = ion_master_table_create_dictionary(&handler, &dict, type_key, key_size, value_size, dictionary_size);

	size_k	= key_size;
	size_v	= value_size;

	return err;
}

/** Is this to be used by user? */
err_t
masterTableLookup(
	unsigned int					id,
	ion_dictionary_config_info_t	*config
) {
	err_t err = ion_lookup_in_master_table(id, config);

	return err;
}

err_t
masterTableOpenDictionary(
	unsigned int id
) {
	err_t err = ion_open_dictionary(&handler, &dict, id);

	return err;
}

err_t
masterTableCloseDictionary(
) {
	err_t err = ion_close_dictionary(&dict);

	return err;
}

err_t
deleteFromMasterTable(
) {
	err_t err = ion_delete_from_master_table(&dict);

	return err;
}

err_t
closeMasterTable(
) {
	err_t err = ion_close_master_table();

	return err;
}

err_t
deleteMasterTable(
) {
	err_t err = ion_delete_master_table();

	return err;
}
};

#endif /* PLANCK_UNIT_MASTERTABLE_H */
