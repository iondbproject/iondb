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

	return err_ok;
}

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

	ion_err_t err = linear_hash_write_state(linear_hash);

	if (err != err_ok) {
		return err;
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

	/* stores for record data */
	ion_byte_t	*record_key		= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value	= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status	= 0;

	int split_hash_key;
	int insert_hash_key;

	int			i, j;
	ion_byte_t	*records = alloca(linear_hash->record_total_size * linear_hash->records_per_bucket);

	memset(records, 0, linear_hash->record_total_size * linear_hash->records_per_bucket);

	ion_fpos_t record_offset = 0;

	while (bucket.overflow_location != linear_hash_end_of_list) {
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

					/* insert that many records into the table */
					for (j = 0; j < num_deleted; j++) {
						/* THIS IS BROKEN -- ALL VALUES OF OTHER KEYS ARE LOST! */
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

		bucket_loc		= bucket.overflow_location;
		status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

		if (status.error != err_ok) {
			return status.error;
		}
	}

	if (bucket.record_count > 0) {
		fseek(linear_hash->database, bucket_loc + sizeof(linear_hash_bucket_t), SEEK_SET);
		fread(records, linear_hash->record_total_size, linear_hash->records_per_bucket, linear_hash->database);

		for (i = 0; i < bucket.record_count; i++) {
			memcpy(&record_status, records + record_offset, sizeof(record_status));
			memcpy(record_key, records + record_offset + sizeof(record_status), linear_hash->super.record.key_size);
			memcpy(record_value, records + record_offset + sizeof(record_status) + linear_hash->super.record.key_size, linear_hash->super.record.value_size);

			insert_hash_key = insert_hash_to_bucket(record_key, linear_hash);

			split_hash_key	= hash_to_bucket(record_key, linear_hash);

			if ((record_status == linear_hash_record_status_full) && (insert_hash_key != split_hash_key)) {
				status = linear_hash_delete(record_key, linear_hash);
				/* tombstone the status of all the records with this key currently in the buffer */
				/*			invalidate_buffer_records(record_key, bucket.record_count, records, linear_hash); */

				if (status.error != err_ok) {
					return status.error;
				}

				int num_deleted = status.count;

				for (j = 0; j < num_deleted; j++) {
					status = linear_hash_insert(record_key, record_value, hash_to_bucket(record_key, linear_hash), linear_hash);

					if (status.error != err_ok) {
						return status.error;
					}
				}

				/* refresh cached data and restart iteration */
				fseek(linear_hash->database, bucket_loc + sizeof(linear_hash_bucket_t), SEEK_SET);
				fread(records, linear_hash->record_total_size, linear_hash->records_per_bucket, linear_hash->database);
				status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);
				i				= -1;
				record_offset	= -1 * linear_hash->record_total_size;
			}

			record_offset += linear_hash->record_total_size;
		}
	}

	return linear_hash_increment_next_split(linear_hash);
}

ion_err_t
invalidate_buffer_records(
	ion_byte_t			*key,
	int					record_count,
	ion_byte_t			*records,
	linear_hash_table_t *linear_hash
) {
	/* TODO IMPLEMENT PROPER ERROR HANDLING */
	/* buffers for reading records */
	ion_byte_t	*record_key			= alloca(linear_hash->super.record.key_size);
	ion_byte_t	record_status;
	ion_byte_t	tombstone_status	= 0;
	ion_fpos_t	record_offset		= 0;

	/* iterate over all records in the buffer */
	int i;

	for (i = 0; i < record_count; i++) {
		memcpy(&record_status, records + record_offset, sizeof(record_status));
		memcpy(record_key, records + record_offset + sizeof(record_status), linear_hash->super.record.key_size);

		/* if record is not tombstoned and has a matching key */
		if ((record_status == linear_hash_record_status_full) && (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0)) {
			/* overwrite its status with a tombstone */
			memcpy(records + record_offset, &tombstone_status, sizeof(record_status));
		}

		record_offset += linear_hash->record_total_size;
	}

	return err_ok;
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

/*	/ * tail bucket may be empty get to the first bucket with records available THIS DOES NOT LEAVE EMPTY BUCKETS FLOATING ABOUT * / */
/*	if (bucket.record_count == 0) { */
/*		while (bucket.overflow_location != linear_hash_end_of_list) { */
/*			bucket_loc = bucket.overflow_location; */
/*			linear_hash_get_bucket(bucket.overflow_location, &bucket, linear_hash); */
/*  */
/*			if (bucket.record_count > 0) { */
/*				break; */
/*			} */
/*		} */
/*	} */

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

	/* TODO FIX YOU BUCKET RECORD TRACKING */
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

			/* Set the location of the anchor record on the new overflow bucket and update the record_loc for storing
			 * the new record to be this location */
			ion_fpos_t overflow_records_loc = GET_BUCKET_RECORDS_LOC(*overflow_location);

			status.error = linear_hash_get_bucket(*overflow_location, &bucket, linear_hash);

			if (status.error != err_ok) {
				return status;
			}

			/* TODO PASS OVERFLOW LOCATION INTO CREATE_OVERFLOW_BUCKET */
			record_loc		= overflow_records_loc;
			bucket_loc		= *overflow_location;
			status.error	= linear_hash_update_bucket(*overflow_location, &bucket, linear_hash);

			if (status.error != err_ok) {
				return status;
			}
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
	ion_byte_t	record_status	= 0;

	int found					= 0;
	int i;

	ion_byte_t *records			= alloca(linear_hash->record_total_size * linear_hash->records_per_bucket);

	memset(records, 0, linear_hash->record_total_size * linear_hash->records_per_bucket);

	ion_fpos_t	record_offset = 0;
	ion_fpos_t	record_loc;

	while (bucket.overflow_location != linear_hash_end_of_list && found == 0) {
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

		if (found == 0) {
			record_offset	= 0;
			bucket_loc		= bucket.overflow_location;
			status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

			if (status.error != err_ok) {
				return status;
			}
		}
	}

	if (found == 0) {
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
					break;
				}
			}

			record_offset	+= linear_hash->record_total_size;
			record_loc		+= linear_hash->record_total_size;
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
	ion_byte_t	record_status	= 0;

	ion_fpos_t record_loc;

	int i;

	while (bucket.overflow_location != linear_hash_end_of_list) {
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
	ion_byte_t	record_status	= 0;

	int i;

	ion_byte_t *records = alloca(linear_hash->record_total_size * linear_hash->records_per_bucket);

	memset(records, 0, linear_hash->record_total_size * linear_hash->records_per_bucket);

	ion_fpos_t	record_offset = 0;
	ion_fpos_t	record_loc;

	/* memory allocated to transfer the terminal records to delete location for swap on delete */
	ion_byte_t	*terminal_record_key	= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*terminal_record_value	= alloca(linear_hash->super.record.value_size);
	ion_byte_t	terminal_record_status	= 0;

	while (bucket.overflow_location != linear_hash_end_of_list) {
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
					/* store the record_loc in a write back paramter that is used before being overwritten */
					ion_fpos_t swap_record_loc = record_loc;

					/* obtain the swap record */
					linear_hash_get_bucket_swap_record(bucket_idx, &swap_record_loc, terminal_record_key, terminal_record_value, &terminal_record_status, linear_hash);

					/* delete all swap records which are going to be deleted anyways */
					while (linear_hash->super.compare(terminal_record_key, key, linear_hash->super.record.key_size) == 0) {
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

		record_offset	= 0;
		bucket_loc		= bucket.overflow_location;
		status.error	= linear_hash_get_bucket(bucket_loc, &bucket, linear_hash);

		if (status.error != err_ok) {
			return status;
		}
	}

	record_loc = bucket_loc + sizeof(linear_hash_bucket_t);
	fseek(linear_hash->database, bucket_loc + sizeof(linear_hash_bucket_t), SEEK_SET);
	fread(records, linear_hash->record_total_size, linear_hash->records_per_bucket, linear_hash->database);

	for (i = 0; i < bucket.record_count; i++) {
		memcpy(&record_status, records + record_offset, sizeof(record_status));
		memcpy(record_key, records + record_offset + sizeof(record_status), linear_hash->super.record.key_size);
		memcpy(record_value, records + record_offset + sizeof(record_status) + linear_hash->super.record.key_size, linear_hash->super.record.value_size);

		if (record_status != 0) {
			if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
				/* TODO Create wrapper methods and implement proper error propagation */
				/* store the record_loc in a write back paramter that is used before being overwritten */
				ion_fpos_t swap_record_loc = record_loc;

				/* obtain the swap record */
				linear_hash_get_bucket_swap_record(bucket_idx, &swap_record_loc, terminal_record_key, terminal_record_value, &terminal_record_status, linear_hash);

				/* delete all swap records which are going to be deleted anyways */
				while (linear_hash->super.compare(terminal_record_key, key, linear_hash->super.record.key_size) == 0) {
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

				if (status.error != err_ok) {
					return status;
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

	if (1 != fwrite(status, sizeof(*status), 1, linear_hash->database)) {
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
	bucket.overflow_location	= linear_hash_end_of_list;

	/* seek to end of file to append new bucket */
	ion_fpos_t bucket_loc;

	if (0 != fseek(linear_hash->database, 0, SEEK_END)) {
		return err_file_bad_seek;
	}

	bucket_loc = ftell(linear_hash->database);

	/* write bucket data to file */
	if (1 != fwrite(&bucket.idx, sizeof(int), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	if (1 != fwrite(&bucket.record_count, sizeof(int), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	if (1 != fwrite(&bucket.overflow_location, sizeof(ion_fpos_t), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	/* write bucket data to file */
	ion_byte_t	record_status = linear_hash_record_status_empty;
	ion_byte_t	record_blank[linear_hash->super.record.key_size + linear_hash->super.record.value_size];

	memset(record_blank, 0, linear_hash->super.record.key_size + linear_hash->super.record.value_size);

	int i;

	for (i = 0; i < linear_hash->records_per_bucket; i++) {
		if (1 != fwrite(&record_status, sizeof(record_status), 1, linear_hash->database)) {
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

	fseek(linear_hash->database, bucket_loc, SEEK_SET);

	/* write bucket data to file */
	if (1 != fwrite(&bucket->idx, sizeof(int), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	if (1 != fwrite(&bucket->record_count, sizeof(int), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	if (1 != fwrite(&bucket->overflow_location, sizeof(ion_fpos_t), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	return err_ok;
}

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
	if (1 != fwrite(&bucket.idx, sizeof(int), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	if (1 != fwrite(&bucket.record_count, sizeof(int), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	if (1 != fwrite(&bucket.overflow_location, sizeof(ion_fpos_t), 1, linear_hash->database)) {
		return err_file_incomplete_write;
	}

	/* write bucket data to file */
	ion_byte_t record_status = linear_hash_record_status_empty;

	ion_byte_t record_blank[linear_hash->super.record.key_size + linear_hash->super.record.value_size];

	memset(record_blank, 0, linear_hash->super.record.key_size + linear_hash->super.record.value_size);

	int i;

	for (i = 0; i < linear_hash->records_per_bucket; i++) {
		if (1 != fwrite(&record_status, sizeof(record_status), 1, linear_hash->database)) {
			return err_file_write_error;
		}

		if (1 != fwrite(record_blank, linear_hash->super.record.key_size + linear_hash->super.record.value_size, 1, linear_hash->database)) {
			return err_file_write_error;
		}
	}

	return err_ok;
}

/* Returns the file offset where bucket with index idx begins */
ion_fpos_t
bucket_idx_to_ion_fpos_t(
	int					idx,
	linear_hash_table_t *linear_hash
) {
	return array_list_get(idx, linear_hash->bucket_map);
}

int
hash(
	int key
) {
	int hash		= 0;
	int i;
	int size_of_int = (int) sizeof(int);

	for (i = 0; i < size_of_int; i++) {
		hash += *(&key + i);
	}

	return key;
}

int
key_bytes_to_int(
	ion_byte_t			*key,
	linear_hash_table_t *linear_hash
) {
	int i;
	int key_bytes_as_int = 0;

	for (i = 0; i < linear_hash->super.record.key_size - 1; i++) {
		key_bytes_as_int += *(key + i);
	}

	return hash(key_bytes_as_int);
}

int
hash_to_bucket(
	ion_byte_t			*key,
	linear_hash_table_t *linear_hash
) {
	/* Case the record we are looking for was in a bucket that has already been split and h1 was used */
	int key_bytes_as_int = key_bytes_to_int(key, linear_hash);

	return hash(key_bytes_as_int & ((2 * linear_hash->initial_size) - 1));
}

int
insert_hash_to_bucket(
	ion_byte_t			*key,
	linear_hash_table_t *linear_hash
) {
	int key_bytes_as_int = key_bytes_to_int(key, linear_hash);

	return key_bytes_as_int & (linear_hash->initial_size - 1);
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
		return linear_hash_end_of_list;
	}
	/* case bucket_idx is inside array */
	else {
		return array_list->data[bucket_idx];
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

	linear_hash->database = NULL;

	if (0 != fclose(linear_hash->state)) {
		linear_hash_write_state(linear_hash);
		return err_file_close_error;
	}

	linear_hash->state = NULL;

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

	dictionary_get_filename(linear_hash->super.id, "lhd", filename);

	if (0 != fremove(filename)) {
		return err_file_delete_error;
	}

	return err_ok;
}

void
print_linear_hash_distribution(
	linear_hash_table_t *linear_hash
) {
	int						num_records		= 0;
	int						num_overflows	= 0;
	int						i;
	linear_hash_bucket_t	*bucket			= alloca(sizeof(linear_hash_bucket_t));
	ion_fpos_t				bucket_loc;

	printf("\nBUCKET_DISTRIBUTION\n");
	printf("\nbucket_idx,num_overflowsn,num_records,global_num_records\n");

	for (i = 0; i < linear_hash->num_buckets; i++) {
		bucket_loc = array_list_get(i, linear_hash->bucket_map);
		linear_hash_get_bucket(bucket_loc, bucket, linear_hash);

		while (bucket->overflow_location != linear_hash_end_of_list) {
			num_overflows++;
			num_records += bucket->record_count;
			linear_hash_get_bucket(bucket->overflow_location, bucket, linear_hash);
		}

		num_records += bucket->record_count;
		printf("%d, %d, %d, %d\n", bucket->idx, num_overflows, num_records, linear_hash->num_records);
		num_records = 0;
	}
}
