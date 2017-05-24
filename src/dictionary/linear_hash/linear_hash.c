#include "linear_hash.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GET_BUCKET_RECORDS_LOC(bucket_loc) (bucket_loc) + sizeof(linear_hash_bucket_t)

/* initialization function */
ion_err_t
linear_hash_init(
	ion_dictionary_id_t		id,
	ion_dictionary_size_t	dictionary_size,
	ion_key_type_t			key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	int						initial_size,
	int						split_threshold,
	int						records_per_bucket,
	linear_hash_table_t		*linear_hash
) {
	/* err */
	ion_err_t err;

	/* parameter not used */
	linear_hash->super.id					= id;
	linear_hash->dictionary_size			= dictionary_size;
	linear_hash->super.key_type				= key_type;
	linear_hash->super.record.key_size		= key_size;
	linear_hash->super.record.value_size	= value_size;

	/* initialize linear_hash fields */
	linear_hash->initial_size				= initial_size;
	linear_hash->num_buckets				= initial_size;
	linear_hash->num_records				= 0;
	linear_hash->next_split					= 0;
	linear_hash->split_threshold			= split_threshold;
	linear_hash->records_per_bucket			= records_per_bucket;
	linear_hash->record_total_size			= key_size + value_size + sizeof(ion_byte_t);
	linear_hash->cache						= malloc(128);

	char data_filename[ION_MAX_FILENAME_LENGTH];

	dictionary_get_filename(linear_hash->super.id, "lhd", data_filename);

	char state_filename[ION_MAX_FILENAME_LENGTH];

	dictionary_get_filename(linear_hash->super.id, "lhs", state_filename);

	/* mapping of buckets to file offsets */
	array_list_t *bucket_map;

	bucket_map = malloc(sizeof(array_list_t));

	if (NULL == bucket_map) {
		/* clean up resources before returning if out of memory */
		linear_hash_close(linear_hash);
		return err_out_of_memory;
	}

	err = array_list_init(5, bucket_map);

	if (err != err_ok) {
		/* clean up resources before returning if out of memory */
		linear_hash_close(linear_hash);
		return err;
	}

	linear_hash->bucket_map = bucket_map;
	linear_hash->database	= fopen(data_filename, "r+b");

	if (NULL == linear_hash->database) {
		linear_hash->database = fopen(data_filename, "w+b");

		if (NULL == linear_hash->database) {
			return err_file_open_error;
		}

		int i;

		for (i = 0; i < linear_hash->initial_size; i++) {
			err = write_new_bucket(i, linear_hash);

			if (err != err_ok) {
				linear_hash_close(linear_hash);
				return err;
			}
		}
	}

	linear_hash->state = fopen(state_filename, "r+b");

	if (NULL == linear_hash->state) {
		linear_hash->state = fopen(state_filename, "w+b");

		if (NULL == linear_hash->state) {
			return err_file_open_error;
		}

		err = linear_hash_write_state(linear_hash);

		if (err != err_ok) {
			return err;
		}
	}
	else {
		err = linear_hash_read_state(linear_hash);

		if (err != err_ok) {
			return err;
		}
	}

	err = linear_hash_write_state(linear_hash);

	if (err != err_ok) {
		return err;
	}

	/* return pointer to the linear_hash that is sitting in memory */
	return err_ok;
}

/**
@brief		Writes the current state of the linear hash to a .lhs file.
@details	Each instace of a linear hash has an associated .lhs file which stores its state in non-volatile storage. The name of a linear hash's .lhs file is the id of linear hash in the master table. This is the file the state is written to.
@param[in]	linear_hash
Which linear hash instance to write.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_err_t
linear_hash_write_state(
	linear_hash_table_t *linear_hash
) {
	if (1 != fwrite(&linear_hash->initial_size, sizeof(linear_hash->initial_size), 1, linear_hash->state)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&linear_hash->next_split, sizeof(linear_hash->next_split), 1, linear_hash->state)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&linear_hash->split_threshold, sizeof(linear_hash->split_threshold), 1, linear_hash->state)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&linear_hash->num_buckets, sizeof(linear_hash->num_buckets), 1, linear_hash->state)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&linear_hash->num_records, sizeof(linear_hash->num_records), 1, linear_hash->state)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&linear_hash->records_per_bucket, sizeof(linear_hash->records_per_bucket), 1, linear_hash->state)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&linear_hash->bucket_map->current_size, sizeof(int), 1, linear_hash->state)) {
		return err_file_write_error;
	}

	ion_byte_t *cached_bucket_map = alloca(sizeof(ion_fpos_t) * linear_hash->bucket_map->current_size);

	memset(cached_bucket_map, 0, sizeof(ion_fpos_t) * linear_hash->bucket_map->current_size);
	memcpy(cached_bucket_map, linear_hash->bucket_map->data, linear_hash->num_buckets * sizeof(ion_fpos_t));

	if (1 != fwrite(cached_bucket_map, sizeof(linear_hash->bucket_map->data), 1, linear_hash->state)) {
		return err_file_write_error;
	}

	return err_ok;
}

/**
@brief		Read the state of a linear hash from a .lhs file.
@details	Each instace of a linear hash has an associated .lhs file which stores its state in non-volatile storage. The name of a linear hash's .lhs file is the id of linear hash in the master table. This is the file the state is read from.
@param[in]	linear_hash
				Pointer to a linear hash instance to read the data to.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_err_t
linear_hash_read_state(
	linear_hash_table_t *linear_hash
) {
	if (0 != fseek(linear_hash->state, 0, SEEK_SET)) {
		return err_file_bad_seek;
	}

	if (1 != fread(&(linear_hash->initial_size), sizeof(linear_hash->initial_size), 1, linear_hash->state)) {
		return err_file_read_error;
	}

	if (1 != fread(&linear_hash->next_split, sizeof(linear_hash->next_split), 1, linear_hash->state)) {
		return err_file_read_error;
	}

	if (1 != fread(&linear_hash->split_threshold, sizeof(linear_hash->split_threshold), 1, linear_hash->state)) {
		return err_file_read_error;
	}

	if (1 != fread(&linear_hash->num_buckets, sizeof(linear_hash->num_buckets), 1, linear_hash->state)) {
		return err_file_read_error;
	}

	if (1 != fread(&linear_hash->num_records, sizeof(linear_hash->num_records), 1, linear_hash->state)) {
		return err_file_read_error;
	}

	if (1 != fread(&linear_hash->records_per_bucket, sizeof(linear_hash->records_per_bucket), 1, linear_hash->state)) {
		return err_file_read_error;
	}

	if (1 != fread(&linear_hash->bucket_map->current_size, sizeof(int), 1, linear_hash->state)) {
		return err_file_read_error;
	}

	if (1 != fwrite(&linear_hash->bucket_map->data, sizeof(ion_fpos_t) * linear_hash->num_buckets, 1, linear_hash->state)) {
		return err_file_read_error;
	}

	return err_ok;
}

/**
@brief		Helper method to check if a linear hash bucket is full.
@param[in]	bucket
				Pointer to a bucket.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		True if the bucket is full, false if it is not.
*/
int
linear_hash_bucket_is_full(
	linear_hash_bucket_t	bucket,
	linear_hash_table_t		*linear_hash
) {
	return bucket.record_count == linear_hash->records_per_bucket;
}

/**
@brief		Helper method to increment the number of records in the linear hash.
@details	When a record is inserted into a linear hash, the load of the linear hash increases. If this pushes the load above the split threshold, then a new bucket is created and a split is performed.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		err_ok or the resulting status of the several file operations used to create a new bucket.
*/
ion_err_t
linear_hash_increment_num_records(
	linear_hash_table_t *linear_hash
) {
	linear_hash->num_records++;

	ion_err_t err = err_ok;

	if (linear_hash_above_threshold(linear_hash)) {
		err = write_new_bucket(linear_hash->num_buckets, linear_hash);

		if (err != err_ok) {
			return err;
		}

		linear_hash_increment_num_buckets(linear_hash);
		err = split(linear_hash);
	}

	return err;
}

/**
@brief		Helper method to decrement the number of records in the linear hash.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		err_ok.
*/
ion_err_t
linear_hash_decrement_num_records(
	linear_hash_table_t *linear_hash
) {
	linear_hash->num_records--;
	return err_ok;
}

/**
@brief		Helper method to increment the number of buckets in the linear hash.
@details	The new bucket is written to the end of the .lhd file associated with the linear hash. When a bucket is added to a linear hash, the inital_size parameter used in the hash functions is doubled and the pointer is reset to the first bucket. This helps the linear hash's distribution remains constant over time.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		err_ok.
*/
ion_err_t
linear_hash_increment_num_buckets(
	linear_hash_table_t *linear_hash
) {
	linear_hash->num_buckets++;

	if (linear_hash->num_buckets == 2 * linear_hash->initial_size + 1) {
		linear_hash->initial_size	= linear_hash->initial_size * 2;
		linear_hash->next_split		= 0;
	}

	return err_ok;
}

/**
@brief		Helper method to increment the split linear hash.
@details	The new bucket is written to the end of the .lhd file associated with the linear hash. When a bucket is added to a linear hash, the inital_size parameter used in the hash functions is doubled and the pointer is reset to the first bucket. This helps the linear hash's distribution remains constant over time.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		err_ok.
*/
ion_err_t
linear_hash_increment_next_split(
	linear_hash_table_t *linear_hash
) {
	linear_hash->next_split++;
	return err_ok;
}

/**
@brief		Performs the split operation on a linear hash instance.
@details	A split is triggered when the load of the linear hash surpasses the split_threshold. A new bucket is created and the bucket currently pointed to by the split pointer has all of its records checked against a condition. If this condition passes, the record is deleted from the current bucket and inserted to the new bucket. This operation may require several several calls to linear_hash_insert and linear_hash_delete methods. To avoid data loss caused by the swap-on-delete strategy used in the linear_hash_delete deleting multiple records per call potentially, a cache is used to store the values associated with records which are deleted.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		err_ok.
*/
ion_err_t
split(
	linear_hash_table_t *linear_hash
) {
	/* status to hold amount of records deleted */
	ion_status_t status					= ION_STATUS_INITIALIZE;

	/* get bucket to split */
	ion_fpos_t				bucket_loc	= bucket_idx_to_ion_fpos_t(linear_hash->next_split, linear_hash);
	linear_hash_bucket_t	bucket;

	status.error = linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

	if (status.error != err_ok) {
		return status.error;
	}

	/* stores for record data */
	ion_byte_t	*record_key		= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value	= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status	= linear_hash_record_status_empty;

	int split_hash_key;
	int insert_hash_key;

	int			i, j;
	ion_byte_t	*records = alloca(linear_hash->record_total_size * linear_hash->records_per_bucket);

	memset(records, 0, linear_hash->record_total_size * linear_hash->records_per_bucket);

	ion_fpos_t		record_offset	= 0;
	ion_boolean_t	terminal		= boolean_false;

	while (terminal == boolean_false) {
		/* if the bucket is not empty */
		if (bucket.record_count > 0) {
			/* read all records into memory */
			fseek(linear_hash->database, bucket_loc + sizeof(linear_hash_bucket_t), SEEK_SET);
			fread(records, linear_hash->record_total_size, linear_hash->records_per_bucket, linear_hash->database);

			/* scan records for records that should be placed in the new bucket */
			for (i = 0; i < bucket.record_count; i++) {
				memcpy(&record_status, records + record_offset, sizeof(record_status));
				memcpy(record_key, records + record_offset + sizeof(record_status), linear_hash->super.record.key_size);
				memcpy(record_value, records + record_offset + sizeof(record_status) + linear_hash->super.record.key_size, linear_hash->super.record.value_size);

				insert_hash_key = insert_hash_to_bucket(record_key, linear_hash);

				split_hash_key	= hash_to_bucket(record_key, linear_hash);

				/* if the record is not a tombstone and h0(k) != h1(k) */
				if ((record_status == linear_hash_record_status_full) && (insert_hash_key != split_hash_key)) {
					/* delete all records with this key from the table */
					status = linear_hash_delete(record_key, linear_hash);

					if (status.error != err_ok) {
						return status.error;
					}

					int num_deleted = status.count;

					linear_hash->last_cache_idx = 0;

					/* insert that many records into the table */
					for (j = 0; j < num_deleted; j++) {
						memcpy(record_value, linear_hash->cache + linear_hash->last_cache_idx * linear_hash->super.record.value_size, linear_hash->super.record.value_size);
						linear_hash->last_cache_idx++;

						status = linear_hash_insert(record_key, record_value, hash_to_bucket(record_key, linear_hash), linear_hash);

						if (status.error != err_ok) {
							return status.error;
						}
					}

					/* refresh cached data and restart iteration and offset tracker */
					fseek(linear_hash->database, bucket_loc + sizeof(linear_hash_bucket_t), SEEK_SET);
					fread(records, linear_hash->record_total_size, linear_hash->records_per_bucket, linear_hash->database);
					status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);
					i				= -1;
					record_offset	= -1 * linear_hash->record_total_size;
				}

				/* track offset from locations of records in memory the next record is at */
				record_offset += linear_hash->record_total_size;
			}

			/* reset offset */
			record_offset = 0;
		}

		if (bucket.overflow_location == linear_hash_end_of_list) {
			terminal = boolean_true;
		}
		else {
			bucket_loc		= bucket.overflow_location;
			status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

			if (status.error != err_ok) {
				return status.error;
			}
		}
	}

	return linear_hash_increment_next_split(linear_hash);
}

/**
@brief		Helper method to increment check if a linear hash's load is above its split threshold.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		True if the linear hash's load is above its split threshold.
*/
ion_boolean_t
linear_hash_above_threshold(
	linear_hash_table_t *linear_hash
) {
	double	numerator				= (double) (100 * (linear_hash->num_records));
	double	denominator				= (double) (linear_hash->num_buckets * linear_hash->records_per_bucket);

	double load						= numerator / denominator;

	ion_boolean_t above_threshold	= (load > linear_hash->split_threshold);

	return above_threshold;
}

/**
@brief		Obtains the data associated with the last record the bucket chain associated with bucket_idx and deletes the original copy at its current location.
@param[in]	bucket_idx
				The index of the bucket to obtain a the swap record for.
@param[in]	record_loc
				Pointer which receives the location of the swap_record in the .lhd file.
@param[in]	key
				Write back parameter for the record key.
@param[in]	value
				Write back parameter for the record value.
@param[in]	status
				Write back parameter for the record status.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_err_t
linear_hash_get_bucket_swap_record(
	int					bucket_idx,
	ion_fpos_t			*record_loc,
	ion_byte_t			*key,
	ion_byte_t			*value,
	ion_byte_t			*status,
	linear_hash_table_t *linear_hash
) {
	/* read in bucket currently swapping to obtain the last record */
	ion_fpos_t				bucket_loc = array_list_get(bucket_idx, linear_hash->bucket_map);
	linear_hash_bucket_t	bucket;

	linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

	ion_fpos_t swap_record_loc	= bucket_loc + sizeof(linear_hash_bucket_t) + ((bucket.record_count - 1) * linear_hash->record_total_size);

	/* read in the record to swap with next */
	ion_err_t err				= linear_hash_get_record(swap_record_loc, key, value, status, linear_hash);

	if (err != err_ok) {
		return err;
	}

	ion_byte_t deleted_status = linear_hash_record_status_empty;

	/* TODO JUST NEED TO WRITE STATUS HERE */
	err = linear_hash_write_record(swap_record_loc, key, value, &deleted_status, linear_hash);

	if (err != err_ok) {
		return err;
	}

	*record_loc = swap_record_loc;
	bucket.record_count--;

	/* garuntee the bucket in the bucket map has records in it - THIS LEAVES EMPTY BUCKETS FLOATING ABOUT */
	if ((bucket.record_count == 0) && (bucket.overflow_location != linear_hash_end_of_list)) {
		array_list_insert(bucket.idx, bucket.overflow_location, linear_hash->bucket_map);
	}

	/* only to update bucket if not becoming junk bucket */
	err = linear_hash_update_bucket(bucket_loc, &bucket, linear_hash);
	linear_hash_decrement_num_records(linear_hash);

	if (err != err_ok) {
		return err;
	}

	return err;
}

/* linear hash operations */
/**
@brief		Insert a record into the linear hash.
@details	Insert a new record into the linear hash table. If the bucket at hash_bucket_idx is full, a new bucket is written and the record is inserted there. If inserting the record pushes the load of the linear hash above the split threshold, a split is performed.
@param[in]	key
				Pointer to the key of the record to insert.
@param[in]	value
				Pointer to the value of the record to insert.
@param[in]	hash_bucket_idx
				Index of the bucket to insert the record to.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_status_t
linear_hash_insert(
	ion_key_t			key,
	ion_value_t			value,
	int					hash_bucket_idx,
	linear_hash_table_t *linear_hash
) {
	ion_status_t status = ION_STATUS_INITIALIZE;

	if (hash_bucket_idx < linear_hash->next_split) {
		hash_bucket_idx = hash_to_bucket(key, linear_hash);
	}

	/* create a linear_hash_record with the desired key, value, and status of full*/
	ion_byte_t	*record_key		= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value	= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status	= linear_hash_record_status_full;

	memcpy(record_key, key, linear_hash->super.record.key_size);
	memcpy(record_value, value, linear_hash->super.record.value_size);

	/* get the appropriate bucket for insertion */
	ion_fpos_t				bucket_loc = bucket_idx_to_ion_fpos_t(hash_bucket_idx, linear_hash);
	linear_hash_bucket_t	bucket;

	status.error = linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

	if (status.error != err_ok) {
		return status;
	}

	/* location of the records in the bucket to be stored in */
	ion_fpos_t	bucket_records_loc = GET_BUCKET_RECORDS_LOC(bucket_loc);
	ion_fpos_t	record_loc;

	/* Case the bucket is empty */
	if (bucket.record_count == 0) {
		record_loc = bucket_records_loc;
	}
	else {
		/* Case that the bucket is full but there is not yet an overflow bucket */
		if (linear_hash_bucket_is_full(bucket, linear_hash)) {
			/* Get location of overflow bucket and update the tail record for the linked list of buckets storing
			 * items that hash to this bucket and update the tail bucket with the overflow's location */
			ion_fpos_t *overflow_location = alloca(sizeof(ion_fpos_t));

			status.error = create_overflow_bucket(bucket.idx, overflow_location, linear_hash);

			if (err_ok != status.error) {
				return status;
			}

			/* update parameters of bucket to match that of overflow just created */
			bucket.record_count			= 0;
			bucket.overflow_location	= bucket_loc;

			/* update the locations to write to in the file for the record and bucket */
			record_loc					= GET_BUCKET_RECORDS_LOC(*overflow_location);
			bucket_loc					= *overflow_location;
		}
		/* case there is >= 1 record in the bucket but it is not full */
		else {
			/* create a linear_hash_record with the desired key, value, and status of full*/
			record_loc = bucket_loc + sizeof(linear_hash_bucket_t) + bucket.record_count * linear_hash->record_total_size;
		}
	}

	/* write new record to the db */
	status.error = linear_hash_write_record(record_loc, record_key, record_value, &record_status, linear_hash);

	if (status.error != err_ok) {
		return status;
	}

	status.count++;

	/* update bucket */
	bucket.record_count++;
	status.error = linear_hash_update_bucket(bucket_loc, &bucket, linear_hash);

	if (status.error != err_ok) {
		return status;
	}

	status.error = err_ok;
	linear_hash_increment_num_records(linear_hash);
	return status;
}

/* linear hash operations */
/**
@brief		Retrieve a record from the linear hash. The key and value will be written to the key and value pointers passed in.
@param[in]	key
				Pointer where the key of the record is written back to.
@param[in]	value
				Pointer where the value of the record is written back to.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_status_t
linear_hash_get(
	ion_byte_t			*key,
	ion_byte_t			*value,
	linear_hash_table_t *linear_hash
) {
	/* status for result count */
	ion_status_t status = ION_STATUS_INITIALIZE;
	/* get the index of the bucket to read */
	int bucket_idx		= insert_hash_to_bucket(key, linear_hash);

	if (bucket_idx < linear_hash->next_split) {
		bucket_idx = hash_to_bucket(key, linear_hash);
	}

	/* get the bucket where the record would be located */
	ion_fpos_t				bucket_loc = bucket_idx_to_ion_fpos_t(bucket_idx, linear_hash);
	linear_hash_bucket_t	bucket;

	status.error = linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

	if (status.error != err_ok) {
		return status;
	}

	/* create a linear_hash_record with the desired key, value, and status of full*/
	ion_byte_t	*record_key		= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value	= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status	= linear_hash_record_status_empty;

	int found					= boolean_false;
	int i;

	ion_byte_t *records			= alloca(linear_hash->record_total_size * linear_hash->records_per_bucket);

	memset(records, 0, linear_hash->record_total_size * linear_hash->records_per_bucket);

	ion_fpos_t		record_offset	= 0;
	ion_fpos_t		record_loc;
	ion_boolean_t	terminal		= boolean_false;

	while (terminal == boolean_false && found == boolean_false) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);
		fseek(linear_hash->database, bucket_loc + sizeof(linear_hash_bucket_t), SEEK_SET);
		fread(records, linear_hash->record_total_size, linear_hash->records_per_bucket, linear_hash->database);

		for (i = 0; i < linear_hash->records_per_bucket; i++) {
			memcpy(&record_status, records + record_offset, sizeof(record_status));
			memcpy(record_key, records + record_offset + sizeof(record_status), linear_hash->super.record.key_size);
			memcpy(record_value, records + record_offset + sizeof(record_status) + linear_hash->super.record.key_size, linear_hash->super.record.value_size);

			if (record_status != 0) {
				if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
					status.count++;
					memcpy(value, record_value, linear_hash->super.record.value_size);
					found = 1;
					break;
				}
			}

			record_offset	+= linear_hash->record_total_size;
			record_loc		+= linear_hash->record_total_size;
		}

		if (found == boolean_false) {
			if (bucket.overflow_location == linear_hash_end_of_list) {
				terminal = boolean_true;
			}
			else {
				record_offset	= 0;
				bucket_loc		= bucket.overflow_location;
				status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

				if (status.error != err_ok) {
					return status;
				}
			}
		}
	}

	if (status.count == 0) {
		status.error = err_item_not_found;
	}
	else {
		status.error = err_ok;
	}

	return status;
}

/* linear hash operations */
/**
@brief		Update the value of the first record matching the key specified in the linear hash.
@param[in]	key
				Pointer to the key of the record to update.
@param[in]	value
				Pointer to the value to set this record to.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_status_t
linear_hash_update(
	ion_key_t			key,
	ion_value_t			value,
	linear_hash_table_t *linear_hash
) {
	ion_status_t status = ION_STATUS_INITIALIZE;
	/* get the index of the bucket to read */
	int bucket_idx		= insert_hash_to_bucket(key, linear_hash);

	if (bucket_idx < linear_hash->next_split) {
		bucket_idx = hash_to_bucket(key, linear_hash);
	}

	/* get the bucket where the record would be located */
	ion_fpos_t				bucket_loc = bucket_idx_to_ion_fpos_t(bucket_idx, linear_hash);
	linear_hash_bucket_t	bucket;

	status.error = linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

	if (status.error != err_ok) {
		return status;
	}

	/* create a temporary store for records that are read */
	ion_byte_t	*record_key		= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value	= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status	= linear_hash_record_status_empty;

	ion_fpos_t record_loc;

	int				i;
	ion_boolean_t	terminal = boolean_false;

	while (terminal == boolean_false) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

		for (i = 0; i < linear_hash->records_per_bucket; i++) {
			status.error = linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

			if (status.error != err_ok) {
				return status;
			}

			if (record_status == linear_hash_record_status_full) {
				if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
					status.error = linear_hash_write_record(record_loc, record_key, value, &record_status, linear_hash);

					if (status.error != err_ok) {
						return status;
					}

					status.count++;
				}
			}

			record_loc += linear_hash->record_total_size;
		}

		if (bucket.overflow_location == linear_hash_end_of_list) {
			terminal = boolean_true;
		}
		else {
			bucket_loc		= bucket.overflow_location;
			status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

			if (status.error != err_ok) {
				return status;
			}
		}
	}

	if (status.count == 0) {
		status.error = err_item_not_found;
		return linear_hash_insert(key, value, insert_hash_to_bucket(key, linear_hash), linear_hash);
	}
	else {
		status.error = err_ok;
	}

	return status;
}

/* linear hash operations */
/**
@brief		Delete all records with keys matching the key specified in the linear hash.
@details	The delete operation of this implementation deletes all records with keys matching the key specified. To improve the overall performance of the linear hash, particularly the insert method, a swap-on-delete strategy is used. When a record is deleted, the last record in the terminal bucket of the bucket moved from its current location to the now empty location where the record just deleted was previously. This garuantees that if there is empty space in a bucket chain, it will be in the terminal bucket.
@param[in]	key
				Pointer to the key of the record to insert.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_status_t
linear_hash_delete(
	ion_byte_t			*key,
	linear_hash_table_t *linear_hash
) {
	/* status for result count */
	ion_status_t status = ION_STATUS_INITIALIZE;
	/* get the index of the bucket to read */
	int bucket_idx		= insert_hash_to_bucket(key, linear_hash);

	if (bucket_idx < linear_hash->next_split) {
		bucket_idx = hash_to_bucket(key, linear_hash);
	}

	/* get the bucket where the record would be located */
	ion_fpos_t				bucket_loc = bucket_idx_to_ion_fpos_t(bucket_idx, linear_hash);
	linear_hash_bucket_t	bucket;

	status.error = linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

	if (status.error != err_ok) {
		return status;
	}

	/* create a linear_hash_record with the desired key, value, and status of full*/
	ion_byte_t	*record_key		= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value	= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status	= linear_hash_record_status_empty;

	int i;

	ion_byte_t *records = alloca(linear_hash->record_total_size * linear_hash->records_per_bucket);

	memset(records, 0, linear_hash->record_total_size * linear_hash->records_per_bucket);

	ion_fpos_t	record_offset = 0;
	ion_fpos_t	record_loc;

	/* memory allocated to transfer the terminal records to delete location for swap on delete */
	ion_byte_t	*terminal_record_key	= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*terminal_record_value	= alloca(linear_hash->super.record.value_size);
	ion_byte_t	terminal_record_status	= linear_hash_record_status_empty;

	linear_hash->last_cache_idx = 0;

	ion_boolean_t terminal = boolean_false;

	while (terminal == boolean_false) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);
		fseek(linear_hash->database, bucket_loc + sizeof(linear_hash_bucket_t), SEEK_SET);
		fread(records, linear_hash->record_total_size, linear_hash->records_per_bucket, linear_hash->database);

		for (i = 0; i < bucket.record_count; i++) {
			/* read in record */
			memcpy(&record_status, records + record_offset, sizeof(record_status));
			memcpy(record_key, records + record_offset + sizeof(record_status), linear_hash->super.record.key_size);
			memcpy(record_value, records + record_offset + sizeof(record_status) + linear_hash->super.record.key_size, linear_hash->super.record.value_size);

			if (record_status != 0) {
				if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
					/* TODO Create wrapper methods and implement proper error propagation */
					/* cache the record being deleted's value */
					memcpy(linear_hash->cache + linear_hash->last_cache_idx * linear_hash->super.record.value_size, record_value, linear_hash->super.record.value_size);
					linear_hash->last_cache_idx++;

					/* store the record_loc in a write back paramter that is used before being overwritten */
					ion_fpos_t swap_record_loc = record_loc;

					/* obtain the swap record */
					linear_hash_get_bucket_swap_record(bucket_idx, &swap_record_loc, terminal_record_key, terminal_record_value, &terminal_record_status, linear_hash);

					/* delete all swap records which are going to be deleted anyways */
					while (linear_hash->super.compare(terminal_record_key, key, linear_hash->super.record.key_size) == 0) {
						memcpy(linear_hash->cache + linear_hash->last_cache_idx * linear_hash->super.record.value_size, terminal_record_value, linear_hash->super.record.value_size);
						linear_hash->last_cache_idx++;

						/* if we are not trying to swap a record with itself */
						if (record_loc == swap_record_loc) {
							/* write the swapped record to record_loc */
							break;
						}

						linear_hash_get_bucket_swap_record(bucket_idx, &swap_record_loc, terminal_record_key, terminal_record_value, &terminal_record_status, linear_hash);

						if (terminal_record_status == linear_hash_record_status_empty) {
							break;
						}

						status.count++;
					}

					/* if we are not trying to swap a record with itself */
					if (record_loc != swap_record_loc) {
						/* write the swapped record to record_loc */
						linear_hash_write_record(record_loc, terminal_record_key, terminal_record_value, &terminal_record_status, linear_hash);
					}

					status.count++;

					if (status.error != err_ok) {
						return status;
					}

					linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);
				}
			}

			record_loc		+= linear_hash->record_total_size;
			record_offset	+= linear_hash->record_total_size;
		}

		if (bucket.overflow_location == linear_hash_end_of_list) {
			terminal = boolean_true;
		}
		else {
			record_offset	= 0;
			bucket_loc		= bucket.overflow_location;
			status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

			if (status.error != err_ok) {
				return status;
			}
		}
	}

	if (status.count == 0) {
		status.error = err_item_not_found;
	}
	else {
		status.error = err_ok;
	}

	return status;
}

/* returns the struct representing the bucket at the specified index */
/**
@brief		Read the record data at the location specified from the linear hash's .lhd file.
@param[in]	loc
				Location of the record in the linear hash's data file.
@param[in]	key
				Pointer where the key of the record is written back to.
@param[in]	value
				Pointer where the value of the record is written back to.
@param[in]	status
				Pointer where the status of the record is written back to.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_err_t
linear_hash_get_record(
	ion_fpos_t			loc,
	ion_byte_t			*key,
	ion_byte_t			*value,
	ion_byte_t			*status,
	linear_hash_table_t *linear_hash
) {
	/* seek to location of record in file */
	if (0 != fseek(linear_hash->database, loc, SEEK_SET)) {
		return err_file_bad_seek;
	}

	/* cache record data from file */
	ion_byte_t *record = alloca(linear_hash->record_total_size);

	if (1 != fread(record, linear_hash->record_total_size, 1, linear_hash->database)) {
		return err_file_read_error;
	}

	/* read record data elements */
	memcpy(status, record, sizeof(*status));
	memcpy(key, record + sizeof(*status), linear_hash->super.record.key_size);
	memcpy(value, record + sizeof(*status) + linear_hash->super.record.key_size, linear_hash->super.record.value_size);

	return err_ok;
}

/**
@brief		Write record data to the location specified from the linear hash's .lhd file.
@param[in]	record_loc
				Location to write the record data in the linear hash's data file.
@param[in]	key
				Pointer to the key to be written.
@param[in]	value
				Pointer to the value to be written.
@param[in]	status
				Pointer to the status to be written.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_err_t
linear_hash_write_record(
	ion_fpos_t			record_loc,
	ion_byte_t			*key,
	ion_byte_t			*value,
	ion_byte_t			*status,
	linear_hash_table_t *linear_hash
) {
	/* check the file is open */
	if (!linear_hash->database) {
		return err_file_close_error;
	}

	/* seek to end of file to append new bucket */
	if (0 != fseek(linear_hash->database, record_loc, SEEK_SET)) {
		return err_file_bad_seek;
	}

	ion_byte_t *record = alloca(linear_hash->record_total_size);

	memcpy(record, status, sizeof(*status));
	memcpy(record + sizeof(*status), key, linear_hash->super.record.key_size);
	memcpy(record + linear_hash->super.record.key_size + sizeof(*status), value, linear_hash->super.record.value_size);

	if (1 != fwrite(record, linear_hash->record_total_size, 1, linear_hash->database)) {
		return err_file_write_error;
	}

	return err_ok;
}

/**
@brief		Write a new bucket to the linear hash's .lhd file.
@details	The new bucket is intialized with empty memory, including space for all its record. The bucket is appended to the end of the .lhd file.
@param[in]	idx
				Index of the new bucket to be written
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_err_t
write_new_bucket(
	int					idx,
	linear_hash_table_t *linear_hash
) {
	if (linear_hash->database == NULL) {
		return err_file_open_error;
	}

	linear_hash_bucket_t bucket;

	/* initialize bucket fields */
	bucket.idx					= idx;
	bucket.record_count			= 0;
	bucket.overflow_location	= linear_hash_end_of_list;

	/* seek to end of file to append new bucket */
	ion_fpos_t bucket_loc;

	if (0 != fseek(linear_hash->database, 0, SEEK_END)) {
		return err_file_bad_seek;
	}

	bucket_loc = ftell(linear_hash->database);

	/* write bucket data to file */
	if (1 != fwrite(&bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	/* write bucket data to file */
	ion_byte_t record_blank[linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(linear_hash_record_status_empty)];

	memset(record_blank, 0, linear_hash->super.record.key_size + linear_hash->super.record.value_size);

	int i;

	for (i = 0; i < linear_hash->records_per_bucket; i++) {
		if (1 != fwrite(record_blank, linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(linear_hash_record_status_empty), 1, linear_hash->database)) {
			return err_file_incomplete_write;
		}
	}

	/* write bucket_loc in mapping */
	/* store_bucket_loc_in_map(idx, bucket_loc, linear_hash); */
	ion_err_t err = array_list_insert(idx, bucket_loc, linear_hash->bucket_map);

	if (err != err_ok) {
		return err;
	}

	return err_ok;
}

/**
@brief		Read the bucket at the location specified from the linear hash's .lhd file.
@param[in]	bucket_loc
				Location to read the bucket from
@param[in]	bucket
				Pointer to the location the bucket is written back to
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_err_t
linear_hash_get_bucket(
	ion_fpos_t				bucket_loc,
	linear_hash_bucket_t	*bucket,
	linear_hash_table_t		*linear_hash
) {
	if (bucket_loc == -1) {}

	/* check if file is open */
	if (!linear_hash->database) {
		return err_file_close_error;
	}

	/* seek to location of record in file */
	if (0 != fseek(linear_hash->database, bucket_loc, SEEK_SET)) {
		return err_file_bad_seek;
	}

	ion_byte_t *bucket_cache = alloca(sizeof(linear_hash_bucket_t));

	if (1 != fread(bucket_cache, sizeof(linear_hash_bucket_t), 1, linear_hash->database)) {
		return err_file_read_error;
	}

	/* read record data elements */
	memcpy(&bucket->idx, bucket_cache, sizeof(int));
	memcpy(&bucket->record_count, bucket_cache + sizeof(int), sizeof(int));
	memcpy(&bucket->overflow_location, bucket_cache + 2 * sizeof(int), sizeof(ion_fpos_t));

	return err_ok;
}

/**
@brief		Write the bucket provided at the location specified from in linear hash's .lhd file.
@param[in]	bucket_loc
				Location to write the bucket to
@param[in]	bucket
				Pointer to the bucket data to write to the file
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_err_t
linear_hash_update_bucket(
	ion_fpos_t				bucket_loc,
	linear_hash_bucket_t	*bucket,
	linear_hash_table_t		*linear_hash
) {
	/* check the file is open */
	if (NULL == linear_hash->database) {
		return err_file_open_error;
	}

	/* seek to end of file to append new bucket */
	if (0 != fseek(linear_hash->database, bucket_loc, SEEK_SET)) {
		return err_file_bad_seek;
	}

	/* write bucket data to file */
	if (1 != fwrite(bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	return err_ok;
}

/**
@brief		Create an overflow bucket and write it to the linear hash's .lhd file.
@details	Create a new overflow bucket and add it to the end of the bucket chain. The location of the overflow bucket is created at is saved in a write back parameter so that the bucket map of the linear hash points to the end of the linked list of overflow buckets. This saves on disk writes as previous tail does not need to be updated. As with write_new_bucket, the new bucket is intialized with empty memory, including space for all its record, and is appended to the end of the .lhd file.
@param[in]	bucket_idx
				Index of the new bucket to be written
@param[in]	overflow_loc
				Pointer to the location the location of the new overflow bucket is written back to.
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_err_t
create_overflow_bucket(
	int					bucket_idx,
	ion_fpos_t			*overflow_loc,
	linear_hash_table_t *linear_hash
) {
	ion_err_t err = err_uninitialized;

	/* initialize bucket fields */
	linear_hash_bucket_t bucket;

	bucket.idx					= bucket_idx;
	bucket.record_count			= 0;
	bucket.overflow_location	= array_list_get(bucket_idx, linear_hash->bucket_map);

	/* seek to end of file to append new bucket */
	if (0 != fseek(linear_hash->database, 0, SEEK_END)) {
		return err_file_bad_seek;
	}

	/* get overflow location for new overflow bucket */
	*overflow_loc	= ftell(linear_hash->database);

	err				= array_list_insert(bucket.idx, *overflow_loc, linear_hash->bucket_map);

	if (err != err_ok) {
		return err;
	}

	/* write to file */
	if (1 != fwrite(&bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	/* write bucket data to file */
	ion_byte_t record_blank[linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(linear_hash_record_status_empty)];

	memset(record_blank, 0, linear_hash->super.record.key_size + linear_hash->super.record.value_size);

	int i;

	for (i = 0; i < linear_hash->records_per_bucket; i++) {
		if (1 != fwrite(record_blank, linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(linear_hash_record_status_empty), 1, linear_hash->database)) {
			return err_file_incomplete_write;
		}
	}

	return err_ok;
}

/**
@brief		Helper method to get the location of a bucket chain from the linear hash's bucket map.
@param[in]	idx
				Index of the bucket chain to retrieve the location of
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		An ion_fpos_t file position where the bucket chain specified begins.
*/
ion_fpos_t
bucket_idx_to_ion_fpos_t(
	int					idx,
	linear_hash_table_t *linear_hash
) {
	return array_list_get(idx, linear_hash->bucket_map);
}

/**
@brief		Transform a key to an integer.
@details	Applies a polynomial hash to the byte-string representation of the key to transform the key to an integer.
@param[in]	key
				Pointer to the key to hash
@param[in]	linear_hash
				Pointer to a linear hash instance.
@return		The result of applying the polynomial hash to the key as an integer.
*/
int
key_bytes_to_int(
	ion_byte_t			*key,
	linear_hash_table_t *linear_hash
) {
	int			i;
	int			key_bytes_as_int	= 0;
	static int	coefficients[]		= { 3, 5, 7, 11, 13, 17, 19 };

	for (i = 0; i < linear_hash->super.record.key_size - 1; i++) {
		key_bytes_as_int += *(key + i) * coefficients[i + 1] - *(key + i) * coefficients[i];
	}

	return key_bytes_as_int;
}

/**
@brief		Map a key to the address space of the linear hash. Used to map records to buckets with an index greater than or equal to the split pointer.
@param[in]	key
				Pointer to the key to hash
@param[in]	linear_hash
				Pointer to the linear hash instance (required for knowledge of the key-size)
@return		An integer in the address space of the linear hash.
*/
int
hash_to_bucket(
	ion_byte_t			*key,
	linear_hash_table_t *linear_hash
) {
	/* Case the record we are looking for was in a bucket that has already been split and h1 was used */
	int key_bytes_as_int = key_bytes_to_int(key, linear_hash);

	return key_bytes_as_int & ((2 * linear_hash->initial_size) - 1);
}

/**
@brief		Map a key to the address space of the linear hash. Used to map records to buckets with an index less than the split pointer.
@param[in]	key
				Pointer to the key to hash
@param[in]	linear_hash
				Pointer to the linear hash instance (required for knowledge of the key-size)
@return		An integer in the address space of the linear hash.
*/
int
insert_hash_to_bucket(
	ion_byte_t			*key,
	linear_hash_table_t *linear_hash
) {
	int key_bytes_as_int = key_bytes_to_int(key, linear_hash);

	return key_bytes_as_int & (linear_hash->initial_size - 1);
}

/* ARRAY LIST METHODS */
/**
@brief		Initialize an array list
@param[in]	init_size
				The number of indexes to initialize the array list with
@param[in]	array_list
				Pointer to the array list location in memory
@return		err_ok if successful, err_out_of_memory if array list cannot be created with init_size.
*/
ion_err_t
array_list_init(
	int				init_size,
	array_list_t	*array_list
) {
	array_list->current_size	= init_size;
	array_list->data			= malloc(init_size * sizeof(ion_fpos_t));
	memset(array_list->data, 0, sizeof(ion_fpos_t) * init_size);

	if (NULL == array_list->data) {
		return err_out_of_memory;
	}

	return err_ok;
}

/**
@brief		Insert a value into an array list.
@details	If the index specified for insertion is larger than the size of the table, the array list size is doubled.
@param[in]	bucket_idx
				The index in the array list to insert the value at.
@param[in]	bucket_loc
				The value to insert.
@param[in]	array_list
				The array list to insert the value into.
@return		err_ok if successful, err_out_of_memory if array list cannot be created doubled in size.
*/
ion_err_t
array_list_insert(
	int				bucket_idx,
	ion_fpos_t		bucket_loc,
	array_list_t	*array_list
) {
	/* case we need to expand array */
	if (bucket_idx >= array_list->current_size) {
		int old_size = array_list->current_size;

		array_list->current_size = array_list->current_size * 2;

		ion_byte_t *bucket_map_cache = alloca(old_size * sizeof(ion_fpos_t));

		memcpy(bucket_map_cache, array_list->data, old_size * sizeof(ion_fpos_t));
        free(array_list->data);
		array_list->data	= NULL;
		array_list->data	= malloc(2 * old_size * sizeof(ion_fpos_t));
		memset(array_list->data, 0, array_list->current_size * sizeof(ion_fpos_t));
		memcpy(array_list->data, bucket_map_cache, old_size * sizeof(ion_fpos_t));

		if (NULL == array_list->data) {
			free(array_list->data);
			return err_out_of_memory;
		}
	}

	array_list->data[bucket_idx] = bucket_loc;

	return err_ok;
}

/**
@brief		Retreive a value from an array list.
@param[in]	bucket_idx
				The index in the array list to retireve the value from.
@param[in]	array_list
				The array list to retrieve the value from.
@return		An ion_fpos_t if successful, linear_hash_end_of_list if array bucket idx is outside of array list bounds.
*/
ion_fpos_t
array_list_get(
	int				bucket_idx,
	array_list_t	*array_list
) {
	/* case bucket_idx is outside of current size of array */
	if (bucket_idx >= array_list->current_size) {
		return linear_hash_end_of_list;
	}
	/* case bucket_idx is inside array */
	else {
		return array_list->data[bucket_idx];
	}
}

/**
@brief		Close a linear hash instance with proper resource clean-up.
@brief		This will free all the references related to the linear hash in memory and writes it state to its associated .lhs file.
@param[in]	linear_hash
				The linear hash instance to close.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_err_t
linear_hash_close(
	linear_hash_table_t *linear_hash
) {
	if (0 != fclose(linear_hash->state)) {
		linear_hash_write_state(linear_hash);
		return err_file_close_error;
	}

	if (linear_hash->bucket_map->data != NULL) {
		free(linear_hash->bucket_map->data);
		linear_hash->bucket_map->data = NULL;
	}

	if (linear_hash->bucket_map != NULL) {
		free(linear_hash->bucket_map);
		linear_hash->bucket_map = NULL;
	}

	if (0 != fclose(linear_hash->database)) {
		return err_file_close_error;
	}

	if (linear_hash->cache != NULL) {
		free(linear_hash->cache);
		linear_hash->bucket_map = NULL;
	}

	linear_hash->database	= NULL;

	linear_hash->state		= NULL;

	return err_ok;
}

/**
@brief		Close a linear hash instance and delete its associated .lhs and .lhd files.
@param[in]	linear_hash
				The linear hash instance to close.
@return		Resulting status of the several file operations used to commit the write.
*/
ion_err_t
linear_hash_destroy(
	linear_hash_table_t *linear_hash
) {
	ion_err_t err = linear_hash_close(linear_hash);

	if (err_ok != err) {
		return err;
	}

	char filename[ION_MAX_FILENAME_LENGTH];

	dictionary_get_filename(linear_hash->super.id, "lhs", filename);

	if (0 != fremove(filename)) {
		return err_file_delete_error;
	}

	dictionary_get_filename(linear_hash->super.id, "lhd", filename);

	if (0 != fremove(filename)) {
		return err_file_delete_error;
	}

	return err_ok;
}
