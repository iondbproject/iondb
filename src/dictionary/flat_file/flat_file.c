/*
 * flat_file.c
 *
 *  Created on: Aug 14, 2014
 *	  Author: workstation
 */

#include "flat_file.h"
#include "../../key_value/kv_system.h"

ion_err_t
ff_initialize(
	ion_ff_file_t			*file,
	ion_key_type_t			key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size
) {
#if DEBUG
	io_printf("Initializing file \n");
#endif

	/* check to see if file exists and if it does, throw exception */

	if (NULL != (file->file_ptr = fopen("file.bin", "rb"))) {
		fclose(file->file_ptr);
		return err_file_open_error;
	}

	/* assume the the file does not exists -> this will come from the upper layers */
	file->file_ptr = fopen("file.bin", "w+b");

	if (NULL == file->file_ptr) {
		return err_file_open_error;
	}

	/*@todo move to parent? */
	file->write_concern				= wc_insert_unique;			/* By default allow unique inserts only */
	file->super.record.key_size		= key_size;
	file->super.record.value_size	= value_size;
	file->super.key_type			= key_type;
	file->super.id					= 0;

	/* need to write a file header out here */
	if (0 == fwrite(&(file->super.key_type), sizeof(file->super.key_type), 1, file->file_ptr)) {
		return err_file_write_error;
	}

	if (0 == fwrite(&(file->super.record.key_size), sizeof(file->super.record.key_size), 1, file->file_ptr)) {
		return err_file_write_error;
	}

	if (0 == fwrite(&(file->super.record.value_size), sizeof(file->super.record.value_size), 1, file->file_ptr)) {
		return err_file_write_error;
	}

	if (0 == fwrite(&(file->super.compare), sizeof(file->super.compare), 1, file->file_ptr)) {
		return err_file_write_error;
	}

	/* and flush contents to disk */
	if (0 != fflush(file->file_ptr)) {
		return err_file_write_error;
	}

	/* record the start of the data block */
	if (-1 == (file->start_of_data = ftell(file->file_ptr))) {
		return err_file_bad_seek;
	}

#if DEBUG
	io_printf("Record key size: %i\n", file->super.record.key_size);
	io_printf("Record value size: %i\n", file->super.record.value_size);
#endif

	return err_ok;
}

ion_err_t
ff_destroy(
	ion_ff_file_t *file
) {
	file->super.compare				= NULL;
	file->super.record.key_size		= 0;
	file->super.record.value_size	= 0;

	if (0 != fclose(file->file_ptr)) {
		return err_file_close_error;
	}

	if (0 != fremove("file.bin")) {
		return err_dictionary_destruction_error;
	}

	return err_ok;
}

ion_status_t
ff_update(
	ion_ff_file_t	*file,
	ion_key_t	key,
	ion_value_t value
) {
	/* TODO: lock potentially required */
	ion_write_concern_t current_write_concern = file->write_concern;

	file->write_concern = wc_update;/* change write concern to allow update */

	ion_status_t status = ff_insert(file, key, value);

	file->write_concern = current_write_concern;
	return status;
}

ion_status_t
ff_insert(
	ion_ff_file_t	*file,
	ion_key_t	key,
	ion_value_t value
) {
/*@todo requires massive cleanup for function exit */
	ion_f_file_record_t *record;

	int record_size = file->super.record.key_size + file->super.record.value_size + SIZEOF(STATUS);

#if DEBUG
	printf("inserting record of size %i \n", record_size);
#endif

	if ((record = malloc(record_size)) == NULL) {
		return ION_STATUS_ERROR(err_out_of_memory);
	}

	/* set to start of data block */
	if (0 != fseek(file->file_ptr, file->start_of_data, SEEK_SET)) {
		return ION_STATUS_ERROR(err_file_bad_seek);
	}

#if DEBUG
	printf("resetting to start of data\n");
	printf("start of data %i\n", (int) file->start_of_data);
	printf("reading record %i bytes long\n", record_size);
#endif

	do {
		fread(record, record_size, 1, file->file_ptr);

		if (feof(file->file_ptr)) {
			/* problem is here with base types as it is just an array of data.  Need better way */
#if DEBUG
			printf("inserting record\n");
#endif
			record->status = IN_USE;
			memcpy(record->data, key, (file->super.record.key_size));
			memcpy(record->data + file->super.record.key_size, value, (file->super.record.value_size));

			if (1 == fwrite(record, record_size, 1, file->file_ptr)) {
				free(record);

				if (0 != fflush(file->file_ptr)) {
					return ION_STATUS_ERROR(err_file_write_error);
				}

				return ION_STATUS_OK(1);
			}
			else {
				free(record);
				return ION_STATUS_ERROR(err_file_write_error);
			}
		}

		if (IN_USE == record->status) {
			/* if a cell is in use, need to key to */
#if DEBUG
			printf("encountered location in use\n");
#endif

			/*if (memcmp(item->data, key, hash_map->record.key_size) == IS_EQUAL)*/
			if (IS_EQUAL == file->super.compare(record->data, key, file->super.record.key_size)) {
				if (wc_insert_unique == file->write_concern) {
					/* allow unique entries only */
					free(record);
					return ION_STATUS_ERROR(err_duplicate_key);
				}
				else if (file->write_concern == wc_update) {
					/* allows for values to be updated */
					/* rewind pointer to value */
					if (0 != fseek(file->file_ptr, -file->super.record.value_size, SEEK_CUR)) {
						free(record);
						return ION_STATUS_ERROR(err_file_bad_seek);
					}

					if (1 == fwrite(value, file->super.record.value_size, 1, file->file_ptr)) {
						fflush(file->file_ptr);
						free(record);
						return ION_STATUS_OK(1);
					}
					else {
						free(record);
						return ION_STATUS_ERROR(err_file_write_error);
					}
				}
				else {
					free(record);
					return ION_STATUS_ERROR(err_write_concern);	/* there is a configuration issue with write concern */
				}
			}
		}

		/* There is no other condition now */
	} while (!feof(file->file_ptr));/* loop until a deleted location or EOF */

	free(record);
	return ION_STATUS_OK(1);/* this needs to be corrected */
}

ion_err_t
ff_find_item_loc(
	ion_ff_file_t	*file,
	ion_key_t	key,
	ion_fpos_t	*location
) {
	if (*location == -1) {
		/* then the position has not been initialized */
		if (0 != fseek(file->file_ptr, file->start_of_data, SEEK_SET)) {
			return err_file_bad_seek;
		}
	}	/* otherwise continue on from the position */
	else {
		if (0 != fseek(file->file_ptr, *location, SEEK_SET)) {
			return err_file_bad_seek;
		}
	}

	int record_size = SIZEOF(STATUS) + file->super.record.key_size + file->super.record.value_size;

	ion_f_file_record_t *record;

	if ((record = malloc(record_size)) == NULL) {
		return err_out_of_memory;
	}

	ion_fpos_t cur_pos;

	/* while we have not reached the end of the the datafile */
	while (!feof(file->file_ptr) /* != EOF*/) {
		/* get current position in file */
		if (-1 == (cur_pos = ftell(file->file_ptr))) {
			return err_file_bad_seek;
		}

		/* @todo depending on how much memory available, could minimize reads through buffering*/
		/* @todo FIXME: This read will fail when reading at end of file */
		if (0 == fread(record, record_size, 1, file->file_ptr)) {
			free(record);

			if (feof(file->file_ptr)) {
				return err_item_not_found;
			}
			else {
				return err_file_read_error;
			}
		}

		if (!feof(file->file_ptr) && (DELETED != record->status)) {
			/*@todo correct compare to use proper return type*/
			int key_is_equal = file->super.compare(record->data, key, file->super.record.key_size);

			if (IS_EQUAL == key_is_equal) {
				*location = cur_pos;
				free(record);
				return err_ok;
			}
		}
	}

	free(record);
	return err_item_not_found;	/* key have not been found */
}

ion_status_t
ff_delete(
	ion_ff_file_t	*file,
	ion_key_t	key
) {
	ion_fpos_t		loc = UNINITIALISED;		/* position to delete */
	ion_status_t	status;					/* return status */

	status = ION_STATUS_CREATE(err_item_not_found, 0);	/* init such that record will not be found */

	while (err_item_not_found != ff_find_item_loc(file, key, &loc)) {
		ion_f_file_record_t record;

		record.status = DELETED;

		if (0 != fseek(file->file_ptr, loc, SEEK_SET)) {
			status.error = err_file_bad_seek;
			return status;
		}

		if (0 == fwrite(&record, sizeof(record.status), 1, file->file_ptr)) {
			status.error = err_file_write_error;
			return status;
		}

		status.count++;
#if DEBUG
		io_printf("Item deleted at location %d\n", loc);
#endif
	}

	if (status.count > 0) {
		status.error = err_ok;
	}

	return status;
}

ion_status_t
ff_query(
	ion_ff_file_t	*file,
	ion_key_t	key,
	ion_value_t value
) {
	ion_fpos_t loc = -1;/* initialize */

	if (err_ok == ff_find_item_loc(file, key, &loc)) {
#if DEBUG
		io_printf("Item found at location %d\n", loc);
#endif

		/* isolate value from record */
		if (0 != fseek(file->file_ptr, loc + SIZEOF(STATUS) + file->super.record.key_size, SEEK_SET)) {
			return ION_STATUS_ERROR(err_file_bad_seek);
		}

		/* copy the value from file to memory */
		if (0 == fread(value, file->super.record.value_size, 1, file->file_ptr)) {
			return ION_STATUS_ERROR(err_file_read_error);
		}

		return ION_STATUS_OK(1);
	}
	else {
#if DEBUG
		io_printf("Item not found in file.\n");
#endif
		return ION_STATUS_ERROR(err_item_not_found);
	}
}
