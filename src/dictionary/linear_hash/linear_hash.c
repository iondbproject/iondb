#include "linear_hash.h"
#include "../../key_value/kv_system.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	/* parameter not used */
	linear_hash->super.id					= id;
	linear_hash->dictionary_size			= dictionary_size;
	linear_hash->super.key_type				= key_type;
	linear_hash->super.record.key_size		= key_size;
	linear_hash->super.record.value_size	= value_size;

	char data_filename[ION_MAX_FILENAME_LENGTH];

	dictionary_get_filename(linear_hash->super.id, "lhd", data_filename);

	char state_filename[ION_MAX_FILENAME_LENGTH];

	dictionary_get_filename(linear_hash->super.id, "lhs", state_filename);

	/* open datafile */
	linear_hash->database = fopen(data_filename, "r+b");

	if (NULL == linear_hash->database) {
		/* The file did not exist - lets open to write */
		linear_hash->database = fopen(data_filename, "w+b");

		if (NULL == linear_hash->database) {
			/* Failed to open, even to create */
			return err_file_open_error;
		}
	}

	linear_hash->state = fopen(state_filename, "r+b");

	if (NULL == linear_hash->state) {
		/* The file did not exist - lets open to write */
		linear_hash->state = fopen(state_filename, "w+b");

		if (NULL == linear_hash->state) {
			/* Failed to open, even to create */
			return err_file_open_error;
		}
	}

	/* initialize linear_hash fields */
	linear_hash->initial_size		= initial_size;
	linear_hash->num_buckets		= initial_size;
	linear_hash->num_records		= 0;
	linear_hash->next_split			= 0;
	linear_hash->split_threshold	= split_threshold;
	linear_hash->records_per_bucket = records_per_bucket;

	/* current offset pointed to in the datafile */
	linear_hash->data_pointer		= ftell(linear_hash->database);

	/* mapping of buckets to file offsets */
	array_list_t *bucket_map;

	bucket_map = malloc(sizeof(array_list_t));

	if (NULL == bucket_map) {
		return err_out_of_memory;
	}

	array_list_init(5, bucket_map);
	linear_hash->bucket_map = bucket_map;

	int i;

	ion_err_t err;

	/* write out initial buckets */
	for (i = 0; i < linear_hash->initial_size; i++) {
		err = write_new_bucket(i, linear_hash);

		if (err != err_ok) {
			return err;
		}
	}

	/* write the state of the linear_hash to disk */
	if (1 != fwrite(linear_hash, sizeof(linear_hash_table_t), 1, linear_hash->state)) {
		return err_file_write_error;
	}

	/* return pointer to the linear_hash that is sitting in memory */
	return err_ok;
}

int
linear_hash_bucket_is_full(
	linear_hash_bucket_t	bucket,
	linear_hash_table_t		*linear_hash
) {
	return bucket.record_count == linear_hash->records_per_bucket;
}

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

/* decrement the count of the records stored in the linear hash */
ion_err_t
linear_hash_decrement_num_records(
	linear_hash_table_t *linear_hash
) {
	linear_hash->num_records--;
	return err_ok;
}

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

ion_err_t
linear_hash_increment_next_split(
	linear_hash_table_t *linear_hash
) {
	linear_hash->next_split++;
	return err_ok;
}

ion_err_t
linear_hash_update_state(
	linear_hash_table_t *linear_hash
) {
	/* write to file */
	if (0 != fseek(linear_hash->state, 0, SEEK_SET)) {
		return err_file_bad_seek;
	}

	if (1 != fwrite(&linear_hash, sizeof(linear_hash_table_t), 1, linear_hash->state)) {
		return err_file_write_error;
	}

	return err_ok;
}

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

	ion_fpos_t record_loc;

	/* stores for record data */
	ion_byte_t	*record_key = alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value = alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status;

	ion_fpos_t	record_total_size = linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(ion_byte_t);
	int			split_hash_key;

	int i, j;

	while (bucket.overflow_location != -1) {
		if (bucket.record_count > 0) {
			record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

			for (i = 0; i < linear_hash->records_per_bucket; i++) {
				linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

				int insert_hash_key = insert_hash_to_bucket(record_key, linear_hash);

				split_hash_key = hash_to_bucket(record_key, linear_hash);

				if ((record_status == 1) && (insert_hash_key != split_hash_key)) {
					status = linear_hash_delete(record_key, linear_hash);

					if (status.error != err_ok) {
						return status.error;
					}

					for (j = 0; j < status.count; j++) {
						status = linear_hash_insert(record_key, record_value, hash_to_bucket(record_key, linear_hash), linear_hash);

						if (status.error != err_ok) {
							return status.error;
						}
					}
				}

				record_loc += record_total_size;
			}
		}

		bucket_loc		= bucket.overflow_location;
		status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

		if (status.error != err_ok) {
			return status.error;
		}
	}

	if (bucket.record_count > 0) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

		for (i = 0; i < linear_hash->records_per_bucket; i++) {
			linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

			int insert_hash_key = insert_hash_to_bucket(record_key, linear_hash);

			split_hash_key = hash_to_bucket(record_key, linear_hash);

			if ((record_status == 1) && (insert_hash_key != split_hash_key)) {
				status = linear_hash_delete(record_key, linear_hash);

				if (status.error != err_ok) {
					return status.error;
				}

				for (j = 0; j < status.count; j++) {
					status = linear_hash_insert(record_key, record_value, hash_to_bucket(record_key, linear_hash), linear_hash);

					if (status.error != err_ok) {
						return status.error;
					}
				}
			}

			record_loc += record_total_size;
		}
	}

	return linear_hash_increment_next_split(linear_hash);
}

int
linear_hash_above_threshold(
	linear_hash_table_t *linear_hash
) {
	double	numerator	= (double) (100 * (linear_hash->num_records));
	double	denominator = (double) (linear_hash->num_buckets * linear_hash->records_per_bucket);

	double load			= numerator / denominator;

	int above_threshold = (load > linear_hash->split_threshold);

	return above_threshold;
}

/* TODO it may not be necesarry to copy the key to a location */
/* linear hash operations */
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
	ion_byte_t	record_status	= 1;

	memcpy(record_key, key, linear_hash->super.record.key_size);
	memcpy(record_value, value, linear_hash->super.record.value_size);

	ion_fpos_t record_total_size		= linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(ion_byte_t);

	/* get the appropriate bucket for insertion */
	ion_fpos_t				bucket_loc	= bucket_idx_to_ion_fpos_t(hash_bucket_idx, linear_hash);
	linear_hash_bucket_t	bucket;

	status.error = linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

	if (status.error != err_ok) {
		return status;
	}

	/* location of the records in the bucket to be stored in */
	ion_fpos_t	bucket_records_loc = get_bucket_records_location(bucket_loc);
	ion_fpos_t	record_loc;

	/* Case the bucket is empty */
	if (bucket.anchor_record == -1) {
		record_loc				= bucket_records_loc;
		bucket.anchor_record	= record_loc;
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

			/* Set the location of the anchor record on the new overflow bucket and update the record_loc for storing
			 * the new record to be this location */
			ion_fpos_t overflow_anchor_record_loc = get_bucket_records_location(*overflow_location);

			status.error = linear_hash_get_bucket(*overflow_location, &bucket, linear_hash);

			if (status.error != err_ok) {
				return status;
			}

			bucket.anchor_record	= overflow_anchor_record_loc;
			record_loc				= bucket.anchor_record;
			bucket_loc				= *overflow_location;
			status.error			= linear_hash_update_bucket(*overflow_location, bucket, linear_hash);

			if (status.error != err_ok) {
				return status;
			}
		}
		/* case there is >= 1 record in the bucket but it is not full */
		else {
			/* scan for tombstones and use if available */
			ion_fpos_t scanner_loc			= bucket.anchor_record;
			/* create a linear_hash_record with the desired key, value, and status of full*/
			ion_byte_t	*scanner_key		= alloca(linear_hash->super.record.key_size);
			ion_byte_t	*scanner_value		= alloca(linear_hash->super.record.value_size);
			ion_byte_t	scanner_status;

			ion_fpos_t	scanner_bucket_loc	= bucket_loc;
			int			stop				= 0;

			int i;

			while (bucket.overflow_location != -1 && stop != 1) {
				for (i = 0; i < linear_hash->records_per_bucket; i++) {
					status.error = linear_hash_get_record(scanner_loc, scanner_key, scanner_value, &scanner_status, linear_hash);

					if (status.error != err_ok) {
						return status;
					}

					if (scanner_status == 0) {
						stop = 1;
						break;
					}

					scanner_loc += record_total_size;
				}

				if (stop == 0) {
					scanner_bucket_loc	= bucket.overflow_location;
					status.error		= linear_hash_get_bucket(scanner_bucket_loc, &bucket, linear_hash);

					if (status.error != err_ok) {
						return status;
					}
				}
			}

			/* scan last bucket if necesarry */
			if (stop != 1) {
				for (i = 0; i < linear_hash->records_per_bucket; i++) {
					status.error = linear_hash_get_record(scanner_loc, scanner_key, scanner_value, &scanner_status, linear_hash);

					if (status.error != err_ok) {
						return status;
					}

					if (scanner_status == 0) {
						stop = 1;
						break;
					}

					scanner_loc += record_total_size;
				}
			}

			if (stop == 1) {
				record_loc	= scanner_loc;
				bucket_loc	= scanner_bucket_loc;
			}
			/* TODO is this step necesarry? */
			else {
				status.error = linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

				if (status.error != err_ok) {
					return status;
				}

				/* get location to insert new record at */
				record_loc = bucket.anchor_record + bucket.record_count * record_total_size;
			}
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
	status.error = linear_hash_update_bucket(bucket_loc, bucket, linear_hash);

	if (status.error != err_ok) {
		return status;
	}

	status.error = err_ok;
	linear_hash_increment_num_records(linear_hash);
	return status;
}

/* linear hash operations */
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
	ion_byte_t	*record_key			= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value		= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status;
	ion_fpos_t	record_total_size	= linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(ion_byte_t);
	ion_fpos_t	record_loc;

	int found						= 0;

	int i;

	while (bucket.overflow_location != -1 && found == 0) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

		for (i = 0; i < linear_hash->records_per_bucket; i++) {
			status.error = linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

			if (status.error != err_ok) {
				return status;
			}

			if (record_status != 0) {
				if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
					status.count++;
					memcpy(value, record_value, linear_hash->super.record.value_size);
					found = 1;
					break;
				}
			}

			record_loc += record_total_size;
		}

		if (found == 0) {
			bucket_loc		= bucket.overflow_location;
			status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

			if (status.error != err_ok) {
				return status;
			}
		}
	}

	record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

	if (found == 0) {
		for (i = 0; i < linear_hash->records_per_bucket; i++) {
			status.error = linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

			if (status.error != err_ok) {
				return status;
			}

			if (record_status != 0) {
				if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
					status.count++;
					memcpy(value, record_value, linear_hash->super.record.value_size);
					break;
				}
			}

			record_loc += record_total_size;
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
	ion_byte_t	record_status;

	ion_fpos_t	record_loc;
	ion_fpos_t	record_total_size = linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(ion_byte_t);

	int i;

	while (bucket.overflow_location != -1) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

		for (i = 0; i < linear_hash->records_per_bucket; i++) {
			status.error = linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

			if (status.error != err_ok) {
				return status;
			}

			if (record_status == 1) {
				if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
					status.error = linear_hash_write_record(record_loc, record_key, value, &record_status, linear_hash);

					if (status.error != err_ok) {
						return status;
					}

					status.count++;
				}
			}

			record_loc += record_total_size;
		}

		bucket_loc		= bucket.overflow_location;
		status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

		if (status.error != err_ok) {
			return status;
		}
	}

	record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

	for (i = 0; i < linear_hash->records_per_bucket; i++) {
		status.error = linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

		if (status.error != err_ok) {
			return status;
		}

		if (record_status == 1) {
			if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
				status.error = linear_hash_write_record(record_loc, record_key, value, &record_status, linear_hash);

				if (status.error != err_ok) {
					return status;
				}

				status.count++;
			}
		}

		record_loc += record_total_size;
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
ion_status_t
linear_hash_delete(
	ion_key_t			key,
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

	/* create a temporary store for record data that are read */
	ion_byte_t	*record_key		= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value	= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status;

	ion_fpos_t	record_loc;
	ion_fpos_t	record_total_size = linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(ion_byte_t);

	int i;

	while (bucket.overflow_location != -1) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

		for (i = 0; i < linear_hash->records_per_bucket; i++) {
			status.error = linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

			if (status.error != err_ok) {
				return status;
			}

			if (record_status != 0) {
				if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
					record_status	= 0;
					status.error	= linear_hash_write_record(record_loc, record_key, record_value, &record_status, linear_hash);

					if (status.error != err_ok) {
						return status;
					}

					status.count++;
					bucket.record_count--;
					status.error = linear_hash_update_bucket(bucket_loc, bucket, linear_hash);

					if (status.error != err_ok) {
						return status;
					}

					linear_hash_decrement_num_records(linear_hash);
				}
			}

			record_loc += record_total_size;
		}

		bucket_loc		= bucket.overflow_location;
		status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

		if (status.error != err_ok) {
			return status;
		}
	}

	record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

	for (i = 0; i < linear_hash->records_per_bucket; i++) {
		status.error = linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

		if (status.error != err_ok) {
			return status;
		}

		if (record_status != 0) {
			if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
				record_status	= 0;
				status.error	= linear_hash_write_record(record_loc, record_key, record_value, &record_status, linear_hash);

				if (status.error != err_ok) {
					return status;
				}

				status.count++;
				bucket.record_count--;
				status.error = linear_hash_update_bucket(bucket_loc, bucket, linear_hash);

				if (status.error != err_ok) {
					return status;
				}

				linear_hash_decrement_num_records(linear_hash);
			}
		}

		record_loc += record_total_size;
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

	/* read record data elements */
	if (1 != fread(status, sizeof(ion_byte_t), 1, linear_hash->database)) {
		return err_file_read_error;
	}

	if (1 != fread(key, linear_hash->super.record.key_size, 1, linear_hash->database)) {
		return err_file_read_error;
	}

	if (1 != fread(value, linear_hash->super.record.value_size, 1, linear_hash->database)) {
		return err_file_read_error;
	}

	return err_ok;
}

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

	if (1 != fwrite(status, sizeof(ion_byte_t), 1, linear_hash->database)) {
		return err_file_write_error;
	}

	if (1 != fwrite(key, linear_hash->super.record.key_size, 1, linear_hash->database)) {
		return err_file_write_error;
	}

	if (1 != fwrite(value, linear_hash->super.record.value_size, 1, linear_hash->database)) {
		return err_file_write_error;
	}

	return err_ok;
}

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
	bucket.overflow_location	= -1;
	bucket.anchor_record		= -1;

	/* seek to end of file to append new bucket */
	ion_fpos_t bucket_loc;

	if (idx == 0) {
		if (0 != fseek(linear_hash->database, 0, SEEK_SET)) {
			return err_file_bad_seek;
		}

		bucket_loc = 0;
	}
	else {
		if (0 != fseek(linear_hash->database, 0, SEEK_END)) {
			return err_file_bad_seek;
		}

		bucket_loc = ftell(linear_hash->database);
	}

	/* write bucket data to file */
	if (1 != fwrite(&bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	/* write bucket data to file */
	ion_byte_t	record_status = 0;
	ion_byte_t	record_blank[linear_hash->super.record.key_size + linear_hash->super.record.value_size];

	memset(record_blank, 0, linear_hash->super.record.key_size + linear_hash->super.record.value_size);

	int i;

	for (i = 0; i < linear_hash->records_per_bucket; i++) {
		if (1 != fwrite(&record_status, sizeof(ion_byte_t), 1, linear_hash->database)) {
			return err_file_incomplete_write;
		}

		if (1 != fwrite(record_blank, linear_hash->super.record.key_size + linear_hash->super.record.value_size, 1, linear_hash->database)) {
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

/* writes the struct representing the bucket at the location to the bucket parameter*/
ion_err_t
linear_hash_get_bucket(
	ion_fpos_t				bucket_loc,
	linear_hash_bucket_t	*bucket,
	linear_hash_table_t		*linear_hash
) {
	/* check if file is open */
	if (!linear_hash->database) {
		return err_file_close_error;
	}

	/* seek to location of record in file */
	if (0 != fseek(linear_hash->database, bucket_loc, SEEK_SET)) {
		return err_file_bad_seek;
	}

	/* read record */
	if (1 != fread(bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database)) {
		return err_file_bad_seek;
	}

	return err_ok;
}

ion_err_t
linear_hash_update_bucket(
	ion_fpos_t				bucket_loc,
	linear_hash_bucket_t	bucket,
	linear_hash_table_t		*linear_hash
) {
	/* check the file is open */
	if (!linear_hash->database) {}

	/* seek to end of file to append new bucket */
	if (0 != fseek(linear_hash->database, bucket_loc, SEEK_SET)) {
		return err_file_bad_seek;
	}

	/* write to file */
	if (1 != fwrite(&bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database)) {
		return err_file_write_error;
	}

	return err_ok;
}

ion_fpos_t
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
	/* bucket.overflow_location = -1; */
	bucket.overflow_location	= array_list_get(bucket_idx, linear_hash->bucket_map);
	bucket.anchor_record		= -1;

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
		return err_file_write_error;
	}

	/* write bucket data to file */
	ion_byte_t record_status = 0;

	ion_byte_t record_blank[linear_hash->super.record.key_size + linear_hash->super.record.value_size];

	memset(record_blank, 0, linear_hash->super.record.key_size + linear_hash->super.record.value_size);

	int i;

	for (i = 0; i < linear_hash->records_per_bucket; i++) {
		if (1 != fwrite(&record_status, sizeof(ion_byte_t), 1, linear_hash->database)) {
			return err_file_write_error;
		}

		if (1 != fwrite(record_blank, linear_hash->super.record.key_size + linear_hash->super.record.value_size, 1, linear_hash->database)) {
			return err_file_write_error;
		}
	}

	return err_ok;
}

ion_fpos_t
get_bucket_records_location(
	ion_fpos_t bucket_loc
) {
	return bucket_loc + sizeof(linear_hash_bucket_t);
}

/* Returns the file offset where bucket with index idx begins */
ion_fpos_t
bucket_idx_to_ion_fpos_t(
	int					idx,
	linear_hash_table_t *linear_hash
) {
/*	if(idx == 0) { */
/*		return 0; */
/*	} */
/*	else { */
/*  */
/*		// create a pointer to the file */
/*		FILE *linear_hash_state; */
/*		linear_hash_state = fopen("linear_hash_state.bin", "r+"); */
/*  */
/*		// seek to the location of the bucket in the map */
/*		ion_fpos_t loc_in_map = sizeof(linear_hash_table_t) + idx * sizeof(ion_fpos_t); */
/*		fseek(linear_hash_state, loc_in_map, SEEK_SET); */
/*  */
/*		// read ion_fpos_t of bucket from mapping in linear hash */
/*		ion_fpos_t bucket_loc; */
/*		fread(&bucket_loc, sizeof(ion_fpos_t), 1, linear_hash_state); */
/*		fclose(linear_hash_state); */
/*		return bucket_loc; */
/*	} */

	return array_list_get(idx, linear_hash->bucket_map);
}

int
hash_to_bucket(
	ion_byte_t			*key,
	linear_hash_table_t *linear_hash
) {
	/* Case the record we are looking for was in a bucket that has already been split and h1 was used */
	int key_bytes_as_int = *((ion_byte_t *) key);

	return key_bytes_as_int % (2 * linear_hash->initial_size);	/*	} */
}

/* TODO change back to ion_key_t */
int
insert_hash_to_bucket(
	ion_byte_t			*key,
	linear_hash_table_t *linear_hash
) {
	int key_bytes_as_int = *((ion_byte_t *) key);

	return key_bytes_as_int % linear_hash->initial_size;
}

/* Write the offset of bucket idx to the map in linear hash state */
ion_err_t
store_bucket_loc_in_map(
	int					idx,
	ion_fpos_t			bucket_loc,
	linear_hash_table_t *linear_hash
) {
	/* seek to the location of the bucket in the map */
	ion_fpos_t loc_in_map = sizeof(linear_hash_table_t) + idx * sizeof(ion_fpos_t);

	if (0 != fseek(linear_hash->state, loc_in_map, SEEK_SET)) {
		return err_file_bad_seek;
	}

	/* read ion_fpos_t of bucket from mapping in linear hash */
	if (1 != fwrite(&bucket_loc, sizeof(ion_fpos_t), 1, linear_hash->state)) {
		return err_file_write_error;
	}

	return err_ok;
}

/* ARRAY LIST METHODS */
ion_err_t
array_list_init(
	int				init_size,
	array_list_t	*array_list
) {
	array_list->current_size	= init_size;
	array_list->data			= malloc(init_size * sizeof(ion_fpos_t));

	if (NULL == array_list->data) {
		return err_out_of_memory;
	}

	return err_ok;
}

ion_err_t
array_list_insert(
	int				bucket_idx,
	ion_fpos_t		bucket_loc,
	array_list_t	*array_list
) {
	/* case we need to expand array */
	if (bucket_idx >= array_list->current_size) {
		array_list->current_size	= array_list->current_size * 2;

		array_list->data			= (ion_fpos_t *) realloc(array_list->data, array_list->current_size * sizeof(ion_fpos_t));

		if (NULL == array_list->data) {
			free(array_list->data);
			return err_out_of_memory;
		}
	}

	array_list->data[bucket_idx] = bucket_loc;

	return err_ok;
}

ion_fpos_t
array_list_get(
	int				bucket_idx,
	array_list_t	*array_list
) {
	/* case bucket_idx is outside of current size of array */
	if (bucket_idx >= array_list->current_size) {
		return -1;
	}
	/* case bucket_idx is inside array */
	else {
		return array_list->data[bucket_idx];
	}
}

void
print_array_list_data(
	array_list_t *array_list
) {
	int i;

	for (i = 0; i < array_list->current_size; i++) {
		printf("\t%d: %ld\n", i, array_list->data[i]);
	}
}

void
print_linear_hash_state(
	linear_hash_table_t *linear_hash
) {
	printf("Linear Hash State\n\tinitial size: %d\n\tnum records %d\n\tnum buckets %d\n\tnext split: %d\n\tsplit threshold: %d\n\tbucket_map size: %d", linear_hash->initial_size, linear_hash->num_records, linear_hash->num_buckets, linear_hash->next_split, linear_hash->split_threshold, linear_hash->bucket_map->current_size);
}

void
print_linear_hash_bucket_from_idx(
	int					idx,
	linear_hash_table_t *linear_hash
) {
	ion_fpos_t bucket_loc = bucket_idx_to_ion_fpos_t(idx, linear_hash);

	printf("PRINTING ALL %d IDX BUCKETS\nHEAD BUCKET AT %ld\n", idx, bucket_loc);

	linear_hash_bucket_t bucket;

	linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

	ion_fpos_t record_loc;

	printf("head bucket\n");
	print_linear_hash_bucket(bucket);

	ion_byte_t	*record_key			= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value		= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status;
	ion_fpos_t	record_total_size	= linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(ion_byte_t);

	int i;

	while (bucket.overflow_location != -1) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

		for (i = 0; i < linear_hash->records_per_bucket; i++) {
			linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

			record_loc += record_total_size;

			if ((linear_hash->super.key_type == key_type_numeric_unsigned) || (linear_hash->super.key_type == key_type_numeric_signed)) {
				printf("record\n\tkey: %d\n\tvalue: %d\n\tstatus: %d\n", NEUTRALIZE(record_key, int), NEUTRALIZE(record_value, int), record_status);
			}
			else {
				printf("record\n\tkey: %d\n\tvalue: %d\n\tstatus: %d\n", NEUTRALIZE(record_key, char), NEUTRALIZE(record_value, int), record_status);
			}
		}

		printf("current bucket\n");
		print_linear_hash_bucket(bucket);
		printf("getting next overflow!!\n");
		bucket_loc = bucket.overflow_location;
		linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);
	}

	record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

	for (i = 0; i < linear_hash->records_per_bucket; i++) {
		linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

		record_loc += record_total_size;

		if ((linear_hash->super.key_type == key_type_numeric_unsigned) || (linear_hash->super.key_type == key_type_numeric_signed)) {
			printf("record\n\tkey: %d\n\tvalue: %d\n\tstatus: %d\n", NEUTRALIZE(record_key, int), NEUTRALIZE(record_value, int), record_status);
		}
		else {
			printf("record\n\tkey: %d\n\tvalue: %d\n\tstatus: %d\n", NEUTRALIZE(record_key, char), NEUTRALIZE(record_value, int), record_status);
		}
	}
}

void
print_linear_hash_bucket(
	linear_hash_bucket_t bucket
) {
	printf("\nBucket\n\tindex %d\n\tanchor record location %ld\n\toverflow location %ld\n\trecord count: %d\n", bucket.idx, bucket.anchor_record, bucket.overflow_location, bucket.record_count);
}

void
print_linear_hash_bucket_map(
	linear_hash_table_t *linear_hash
) {
	printf("Bucket Map State:\n");

	int i;

	for (i = 0; i < linear_hash->bucket_map->current_size; i++) {
		printf("\tbucket idx: %d, bucket loc in data file %ld\n", i, array_list_get(i, linear_hash->bucket_map));
	}
}

/* CLOSE, OPEN, CREATE, DESTROY METHODS */
ion_err_t
linear_hash_close(
	linear_hash_table_t *linear_hash
) {
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

	if (0 != fclose(linear_hash->state)) {
		return err_file_close_error;
	}

	return err_ok;
}

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

	linear_hash->state = NULL;

	dictionary_get_filename(linear_hash->super.id, "lhd", filename);

	if (0 != fremove(filename)) {
		return err_file_delete_error;
	}

	linear_hash->database = NULL;

	return err_ok;
}
