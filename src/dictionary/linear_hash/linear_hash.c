#include "linear_hash.h"
#include "../../key_value/kv_system.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO: OPEN FILE IN INIT METHOD AND CREATE A DESTROY METHOD THAT CLOSES THE FILE */
/* initialization function */
ion_err_t
linear_hash_init(
	ion_dictionary_id_t id,
	ion_key_type_t		key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size,
	int					initial_size,
	int					split_threshold,
	int					records_per_bucket,
	array_list_t		*bucket_map,
	linear_hash_table_t *linear_hash
) {
	linear_hash->super.id					= id;
	linear_hash->super.key_type				= key_type;
	linear_hash->super.record.key_size		= key_size;
	linear_hash->super.record.value_size	= value_size;

	char data_filename[ION_MAX_FILENAME_LENGTH];

	dictionary_get_filename(linear_hash->super.id, "lhd", data_filename);

	char state_filename[ION_MAX_FILENAME_LENGTH];

	dictionary_get_filename(linear_hash->super.id, "lhs", state_filename);

	/* open datafile */
	linear_hash->database			= fopen(data_filename, "w+b");

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
	linear_hash->bucket_map			= bucket_map;

	/* write out initial buckets */
	for (int i = 0; i < linear_hash->initial_size; i++) {
		write_new_bucket(i, linear_hash);
	}

	linear_hash->state = fopen(state_filename, "w+b");

	/* write the state of the linear_hash to disk */
	fwrite(linear_hash, sizeof(linear_hash_table_t), 1, linear_hash->state);

	printf("Linear hash table successfully initialized\n");

	/* return pointer to the linear_hash that is sitting in memory */
	return err_ok;
}

linear_hash_table_t
linear_hash_read_state(
) {
	/* create a pointer to the file */
	FILE *linear_hash_state;

	linear_hash_state = fopen("linear_hash_state.bin", "r+");

	/* create a temporary store for records that are read */
	linear_hash_table_t linear_hash;

	/* check if file is open */
	if (!linear_hash_state) {
		printf("Unable to open file\n");
		exit(-1);
	}

	fseek(linear_hash_state, 0, SEEK_SET);
	/* read state */
	fread(&linear_hash, sizeof(linear_hash_table_t), 1, linear_hash_state);

	fclose(linear_hash_state);
	return linear_hash;
}

int
linear_hash_bucket_is_full(
	linear_hash_bucket_t bucket
) {
	/* TODO CHANGE HARDCODED 4 TO LINEAR HASH FIELD */
	return bucket.record_count == 4;
}

void
linear_hash_increment_num_records(
	linear_hash_table_t *linear_hash
) {
	linear_hash->num_records++;

	if (linear_hash_above_threshold(linear_hash)) {
		write_new_bucket(linear_hash->num_buckets, linear_hash);
		linear_hash_increment_num_buckets(linear_hash);
		split(linear_hash);
	}

	printf("Incremented record count to %d\n", linear_hash->num_records);
}

/* decrement the count of the records stored in the linear hash */
void
linear_hash_decrement_num_records(
	linear_hash_table_t *linear_hash
) {
	linear_hash->num_records--;
	printf("Decremented record count to %d\n", linear_hash->num_records);
}

void
linear_hash_increment_num_buckets(
	linear_hash_table_t *linear_hash
) {
	linear_hash->num_buckets++;

	if (linear_hash->num_buckets == 2 * linear_hash->initial_size + 1) {
		linear_hash->initial_size	= linear_hash->initial_size * 2;
		printf("Size doubled, increasing intial size to %d\n", linear_hash->initial_size);
		linear_hash->next_split		= 0;
	}

	printf("Incremented bucket count to %d\n", linear_hash->num_buckets);
}

void
linear_hash_increment_next_split(
	linear_hash_table_t *linear_hash
) {
	linear_hash->next_split++;
	printf("Incremented next_split to %d\n", linear_hash->next_split);
}

void
linear_hash_update_state(
	linear_hash_table_t *linear_hash
) {
	/* create pointer to file */
	/* FILE *linear_hash_state; */

	/* linear_hash_state = fopen("linear_hash_state.bin", "r+"); */

	/* check the file is open */
/*	if (!linear_hash_state) { */
/*		printf("Unable to open file\n"); */
/*	} */

	/* write to file */
	fseek(linear_hash->state, 0, SEEK_SET);
	fwrite(&linear_hash, sizeof(linear_hash_table_t), 1, linear_hash->state);
	/* fclose(linear_hash_state); */
	printf("Updated linear hash state\n");
}

void
split(
	linear_hash_table_t *linear_hash
) {
	printf("\nPERFORMING SPLIT\n");

	/* get bucket to split */
	ion_fpos_t				bucket_loc	= bucket_idx_to_ion_fpos_t(linear_hash->next_split, linear_hash);
	linear_hash_bucket_t	bucket		= linear_hash_get_bucket(bucket_loc, linear_hash);

	printf("splitting bucket %d\n", bucket.idx);

	ion_fpos_t record_loc;

	/* stores for record data */
	ion_byte_t	*record_key		= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value	= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status;

	/* status to hold amount of records deleted */
	ion_status_t status;

	ion_fpos_t record_total_size = linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(record_status);

	while (bucket.overflow_location != -1) {
		if (bucket.record_count > 0) {
			record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

			for (int i = 0; i < linear_hash->records_per_bucket; i++) {
				linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

				if (record_status == 1) {
					status = linear_hash_delete(record_key, linear_hash);

					for (int i = 0; i < status.count; i++) {
						linear_hash_insert(record_key, record_value, hash_to_bucket(record_key, linear_hash), linear_hash);
					}
				}

				record_loc += record_total_size;
			}
		}

		bucket_loc	= bucket.overflow_location;
		bucket		= linear_hash_get_bucket(bucket_loc, linear_hash);
	}

	if (bucket.record_count > 0) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

		for (int i = 0; i < linear_hash->records_per_bucket; i++) {
			linear_hash_get_record(record_loc, record_key, record_value, &record_status, linear_hash);

			if (record_status == 1) {
				status = linear_hash_delete(record_key, linear_hash);

				for (int i = 0; i < status.count; i++) {
					linear_hash_insert(record_key, record_value, hash_to_bucket(record_key, linear_hash), linear_hash);
				}
			}

			record_loc += record_total_size;
		}
	}

	linear_hash_increment_next_split(linear_hash);
	printf("split complete\n");
}

int
linear_hash_above_threshold(
	linear_hash_table_t *linear_hash
) {
	printf("Checking if %d records is above split threshold of %d\n", linear_hash->num_records, linear_hash->split_threshold);

	double	numerator	= (double) (100 * (linear_hash->num_records));
	double	denominator = (double) (linear_hash->num_buckets * linear_hash->records_per_bucket);

	double load			= numerator / denominator;

	printf("CURRENT LOAD %f\n", load);

	int above_threshold = (load > linear_hash->split_threshold);

	return above_threshold;
}

/* linear hash operations */
ion_status_t
linear_hash_insert(
	ion_key_t			key,
	ion_value_t			value,
	int					hash_bucket_idx,
	linear_hash_table_t *linear_hash
) {
	ion_status_t status			= ION_STATUS_INITIALIZE;

	/* create a linear_hash_record with the desired key, value, and status of full*/
	ion_byte_t	*record_key		= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value	= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status	= 1;

	memcpy(record_key, key, linear_hash->super.record.key_size);
	memcpy(record_value, value, linear_hash->super.record.value_size);

	ion_fpos_t record_total_size	= linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(record_status);

	/* get the appropriate bucket for insertion */
	linear_hash_bucket_t bucket		= linear_hash_get_bucket(bucket_idx_to_ion_fpos_t(hash_bucket_idx, linear_hash), linear_hash);

	/* location of the records in the bucket to be stored in */
	ion_fpos_t	bucket_loc			= bucket_idx_to_ion_fpos_t(hash_bucket_idx, linear_hash);
	ion_fpos_t	bucket_records_loc	= get_bucket_records_location(bucket_loc);
	ion_fpos_t	record_loc;

	/* Case the bucket is empty */
	if (bucket.anchor_record == -1) {
		record_loc				= bucket_records_loc;
		bucket.anchor_record	= record_loc;
	}
	else {
		/* Case that the bucket is full but there is not yet an overflow bucket */
		if (linear_hash_bucket_is_full(bucket)) {
			/* Get location of overflow bucket and update the tail record for the linked list of buckets storing
			 * items that hash to this bucket and update the tail bucket with the overflow's location */
			ion_fpos_t overflow_location = create_overflow_bucket(bucket.idx, linear_hash);

			printf("Bucket full, creating an overflow bucket at %ld\n", overflow_location);
			linear_hash_update_bucket(bucket_loc, bucket, linear_hash);

			/* Set the location of the anchor record on the new overflow bucket and update the record_loc for storing
			 * the new record to be this location */
			ion_fpos_t overflow_anchor_record_loc = get_bucket_records_location(overflow_location);

			bucket					= linear_hash_get_bucket(overflow_location, linear_hash);
			bucket.anchor_record	= overflow_anchor_record_loc;
			record_loc				= bucket.anchor_record;
			bucket_loc				= overflow_location;
			linear_hash_update_bucket(overflow_location, bucket, linear_hash);
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

			while (bucket.overflow_location != -1 && stop != 1) {
				for (int i = 0; i < linear_hash->records_per_bucket; i++) {
					linear_hash_get_record(scanner_loc, scanner_key, scanner_value, &scanner_status, linear_hash);

					if (scanner_status == 0) {
						stop = 1;
						break;
					}

					scanner_loc += record_total_size;
				}

				scanner_bucket_loc	= bucket.overflow_location;
				bucket				= linear_hash_get_bucket(scanner_bucket_loc, linear_hash);
			}

			/* scan last bucket if necesarry */
			if (stop != 1) {
				for (int i = 0; i < linear_hash->records_per_bucket; i++) {
					linear_hash_get_record(scanner_loc, scanner_key, scanner_value, &scanner_status, linear_hash);

					if (scanner_status == 0) {
						stop = 1;
						break;
					}

					scanner_loc += record_total_size;
				}
			}

			if (stop == 1) {
				printf("tombstone found!\n");
				record_loc	= scanner_loc;
				bucket_loc	= scanner_bucket_loc;
			}
			else {
				printf("no tombstones\n");
				bucket		= linear_hash_get_bucket(bucket_loc, linear_hash);

				/* get location to insert new record at */
				record_loc	= bucket.anchor_record + bucket.record_count * record_total_size;
			}
		}
	}

	/* write new record to the db */
	linear_hash_write_record(record_loc, record_key, record_value, &record_status, linear_hash);
	status.error = err_ok;
	status.count++;

	/* update bucket */
	bucket.record_count++;
	linear_hash_update_bucket(bucket_loc, bucket, linear_hash);

	linear_hash_increment_num_records(linear_hash);
	printf("Successfully inserted record at offset %ld\n", record_loc);
	return status;
}

/* linear hash operations */
ion_status_t
linear_hash_get(
	ion_key_t			key,
	ion_value_t			value,
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
	ion_fpos_t				bucket_loc	= bucket_idx_to_ion_fpos_t(bucket_idx, linear_hash);
	linear_hash_bucket_t	bucket		= linear_hash_get_bucket(bucket_loc, linear_hash);

	/* create a linear_hash_record with the desired key, value, and status of full*/
	ion_byte_t	*record_key				= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value			= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status;
	ion_fpos_t	record_total_size		= linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(ion_byte_t);

	linear_hash_get_record(bucket.anchor_record, record_key, record_value, &record_status, linear_hash);

	ion_fpos_t record_loc	= bucket.anchor_record;

	int found				= 0;

	while (bucket.overflow_location != -1 && found == 0) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

		for (int i = 0; i < linear_hash->records_per_bucket; i++) {
			linear_hash_get_record(bucket.anchor_record, record_key, record_value, &record_status, linear_hash);

			if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
				status.count++;
				memcpy(value, record_value, linear_hash->super.record.value_size);
				found = 1;
				break;
			}

			record_loc += record_total_size;
		}

		bucket_loc	= bucket.overflow_location;
		bucket		= linear_hash_get_bucket(bucket_loc, linear_hash);
	}

	record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

	if (found == 0) {
		for (int i = 0; i < linear_hash->records_per_bucket; i++) {
			linear_hash_get_record(bucket.anchor_record, record_key, record_value, &record_status, linear_hash);

			if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
				status.count++;
				memcpy(value, record_value, linear_hash->super.record.value_size);
				found = 1;
				break;
			}

			record_loc += record_total_size;
		}
	}

	if (status.count == 0) {
		status.error = err_item_not_found;
		printf("Record not found\n");
	}
	else {
		status.error = err_ok;
		printf("Record found %d records with id given\n", status.count);
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
	ion_fpos_t				bucket_loc	= bucket_idx_to_ion_fpos_t(bucket_idx, linear_hash);
	linear_hash_bucket_t	bucket		= linear_hash_get_bucket(bucket_loc, linear_hash);

	/* create a temporary store for records that are read */
	ion_byte_t	*record_key				= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value			= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status;

	ion_fpos_t	record_loc;
	ion_fpos_t	record_total_size = linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(ion_byte_t);

	while (bucket.overflow_location != -1) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

		for (int i = 0; i < linear_hash->records_per_bucket; i++) {
			linear_hash_get_record(bucket.anchor_record, record_key, record_value, &record_status, linear_hash);

			if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
				record_status = 1;
				linear_hash_write_record(record_loc, record_key, record_value, &record_status, linear_hash);
				status.count++;
			}

			record_loc += record_total_size;
		}

		bucket_loc	= bucket.overflow_location;
		bucket		= linear_hash_get_bucket(bucket_loc, linear_hash);
	}

	record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

	for (int i = 0; i < linear_hash->records_per_bucket; i++) {
		linear_hash_get_record(bucket.anchor_record, record_key, record_value, &record_status, linear_hash);

		if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
			record_status = 1;
			linear_hash_write_record(record_loc, record_key, record_value, &record_status, linear_hash);
			status.count++;
		}

		record_loc += record_total_size;
	}

	if (status.count == 0) {
		printf("Record not found, performing upsert\n");
		status.error = err_item_not_found;
		return linear_hash_insert(key, value, insert_hash_to_bucket(key, linear_hash), linear_hash);
	}
	else {
		status.error = err_ok;
		printf("Updated %d records with id given\n", status.count);
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
	ion_fpos_t				bucket_loc	= bucket_idx_to_ion_fpos_t(bucket_idx, linear_hash);
	linear_hash_bucket_t	bucket		= linear_hash_get_bucket(bucket_loc, linear_hash);

	/* create a temporary store for record data that are read */
	ion_byte_t	record_key[linear_hash->super.record.key_size];
	ion_byte_t	record_value[linear_hash->super.record.key_size];
	ion_byte_t	record_status;

	ion_fpos_t	record_loc;
	ion_fpos_t	record_total_size = linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(ion_byte_t);

	while (bucket.overflow_location != -1) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

		for (int i = 0; i < linear_hash->records_per_bucket; i++) {
			linear_hash_get_record(record_loc, key, record_value, &record_status, linear_hash);

			if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
				record_status = 0;
				linear_hash_write_record(record_loc, record_key, record_value, &record_status, linear_hash);
				status.count++;
				bucket.record_count--;
				linear_hash_update_bucket(bucket_loc, bucket, linear_hash);
				linear_hash_decrement_num_records(linear_hash);
				printf("TOMBSTONE WRITTEN\n");
			}

			record_loc += record_total_size;
		}

		bucket_loc	= bucket.overflow_location;
		bucket		= linear_hash_get_bucket(bucket_loc, linear_hash);
	}

	record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

	for (int i = 0; i < linear_hash->records_per_bucket; i++) {
		linear_hash_get_record(record_loc, key, record_value, &record_status, linear_hash);

		if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
			record_status = 0;
			linear_hash_write_record(record_loc, record_key, record_value, &record_status, linear_hash);
			status.count++;
			bucket.record_count--;
			linear_hash_update_bucket(bucket_loc, bucket, linear_hash);
			linear_hash_decrement_num_records(linear_hash);
			printf("TOMBSTONE WRITTEN\n");
		}

		record_loc += record_total_size;
	}

	if (status.count == 0) {
		printf("Record not found\n");
		status.error = err_item_not_found;
	}
	else {
		status.error = err_ok;
		printf("Deleted %d records\n", status.count);
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
	fseek(linear_hash->database, loc, SEEK_SET);

	/* read record data elements */
	fread(status, sizeof(ion_byte_t), 1, linear_hash->database);
	fread(key, linear_hash->super.record.key_size, 1, linear_hash->database);
	fread(value, linear_hash->super.record.value_size, 1, linear_hash->database);

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
	/* store current ion_fpos_t in data file to enforce same ion_fpos_t after processing contract */
	ion_fpos_t starting_ion_fpos_t = ftell(linear_hash->database);

	/* check the file is open */
	if (!linear_hash->database) {
		printf("Unable to open file\n");
	}

	/* seek to end of file to append new bucket */
	fseek(linear_hash->database, record_loc, SEEK_SET);
	fwrite(status, sizeof(ion_byte_t), 1, linear_hash->database);
	fwrite(key, linear_hash->super.record.key_size, 1, linear_hash->database);
	fwrite(value, linear_hash->super.record.value_size, 1, linear_hash->database);

	/* restore data pointer to the original location */
	fseek(linear_hash->database, starting_ion_fpos_t, SEEK_SET);
	linear_hash->data_pointer = starting_ion_fpos_t;

	return err_ok;
}

void
write_new_bucket(
	int					idx,
	linear_hash_table_t *linear_hash
) {
	/* store current ion_fpos_t in data file to enforce same ion_fpos_t after processing contract */
	ion_fpos_t starting_ion_fpos_t = ftell(linear_hash->database);

	linear_hash_bucket_t bucket;

	/* initialize bucket fields */
	bucket.idx					= idx;
	bucket.record_count			= 0;
	bucket.overflow_location	= -1;
	bucket.anchor_record		= -1;

	/* check the file is open */
	if (!linear_hash->database) {
		printf("Unable to open file\n");
	}

	/* seek to end of file to append new bucket */
	ion_fpos_t	bucket_loc;
	ion_fpos_t	record_total_size = linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(ion_byte_t);

	if (idx == 0) {
		fseek(linear_hash->database, 0, SEEK_SET);
		bucket_loc = 0;
	}
	else {
		fseek(linear_hash->database, linear_hash->records_per_bucket * record_total_size, SEEK_END);
		bucket_loc = ftell(linear_hash->database);
	}

	/* write bucket data to file */
	fwrite(&bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database);

	/* write bucket data to file */
	ion_byte_t record_status;

	ion_byte_t record_blank[linear_hash->super.record.key_size + linear_hash->super.record.value_size];

	memset(record_blank, 0, linear_hash->super.record.key_size + linear_hash->super.record.value_size);

	for (int i = 0; i < linear_hash->records_per_bucket; i++) {
		fwrite(&record_status, sizeof(ion_byte_t), 1, linear_hash->database);
		fwrite(&record_blank, linear_hash->super.record.key_size + linear_hash->super.record.value_size, 1, linear_hash->database);
	}

	/* restore data pointer to the original location */
	fseek(linear_hash->database, starting_ion_fpos_t, SEEK_SET);
	linear_hash->data_pointer = starting_ion_fpos_t;

	/* write bucket_loc in mapping */
	/* store_bucket_loc_in_map(idx, bucket_loc, linear_hash); */
	array_list_insert(idx, bucket_loc, linear_hash->bucket_map);
	printf("Successfully wrote a new bucket with index %d to the database\n", bucket.idx);
}

/* returns the struct representing the bucket at the specified index */
linear_hash_bucket_t
linear_hash_get_bucket(
	ion_fpos_t			bucket_loc,
	linear_hash_table_t *linear_hash
) {
	/* create a temporary store for records that are read */
	linear_hash_bucket_t bucket;

	/* check if file is open */
	if (!linear_hash->database) {
		printf("Unable to open file\n");
		exit(-1);
	}

	ion_fpos_t starting_ion_fpos_t = ftell(linear_hash->database);

	/* seek to location of record in file */
	fseek(linear_hash->database, bucket_loc, SEEK_SET);

	/* read record */
	fread(&bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database);

	/* restore data pointer to the original location */
	fseek(linear_hash->database, starting_ion_fpos_t, SEEK_SET);
	linear_hash->data_pointer = starting_ion_fpos_t;

	return bucket;
}

void
linear_hash_update_bucket(
	ion_fpos_t				bucket_loc,
	linear_hash_bucket_t	bucket,
	linear_hash_table_t		*linear_hash
) {
/*	// create pointer to file */
/*	FILE *database; */
/*	database = fopen("data.bin", "r+"); */

	/* store current ion_fpos_t in data file to enforce same ion_fpos_t after processing contract */
	ion_fpos_t starting_ion_fpos_t = ftell(linear_hash->database);

	/* check the file is openå */
	if (!linear_hash->database) {
		printf("Unable to open file\n");
	}

	/* seek to end of file to append new bucket */
	fseek(linear_hash->database, bucket_loc, SEEK_SET);

	/* write to file */
	fwrite(&bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database);

	/* restore data pointer to the original location */
	fseek(linear_hash->database, starting_ion_fpos_t, SEEK_SET);
	linear_hash->data_pointer = starting_ion_fpos_t;
	printf("Successfully updated bucket %d\n", bucket.idx);
}

ion_fpos_t
create_overflow_bucket(
	int					bucket_idx,
	linear_hash_table_t *linear_hash
) {
	/* store current ion_fpos_t in data file to enforce same ion_fpos_t after processing contract */
	ion_fpos_t starting_ion_fpos_t = ftell(linear_hash->database);

	/* initialize bucket fields */
	linear_hash_bucket_t bucket;

	bucket.idx					= bucket_idx;
	bucket.record_count			= 0;
	/* bucket.overflow_location = -1; */
	bucket.overflow_location	= array_list_get(bucket_idx, linear_hash->bucket_map);
	bucket.anchor_record		= -1;

	/* check the file is open */
	if (!linear_hash->database) {
		printf("Unable to open file\n");
	}

	/* seek to end of file to append new bucket */
	fseek(linear_hash->database, 0, SEEK_END);

	/* ion_fpos_t overflow_loc = ftell(linear_hash->database); */
	ion_fpos_t overflow_loc = ftell(linear_hash->database);

	array_list_insert(bucket.idx, overflow_loc, linear_hash->bucket_map);

	/* write to file */
	fwrite(&bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database);

	/* write bucket data to file */
	ion_byte_t record_status = 0;

	ion_byte_t record_blank[linear_hash->super.record.key_size + linear_hash->super.record.value_size];

	memset(record_blank, 0, linear_hash->super.record.key_size + linear_hash->super.record.value_size);

	for (int i = 0; i < linear_hash->records_per_bucket; i++) {
		fwrite(&record_status, sizeof(ion_byte_t), 1, linear_hash->database);
		fwrite(&record_blank, linear_hash->super.record.key_size + linear_hash->super.record.value_size, 1, linear_hash->database);
	}

	/* restore data pointer to the original location */
	fseek(linear_hash->database, starting_ion_fpos_t, SEEK_SET);
	linear_hash->data_pointer = starting_ion_fpos_t;

	return overflow_loc;
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
	ion_key_t			key,
	linear_hash_table_t *linear_hash
) {
	/* Case the record we are looking for was in a bucket that has already been split and h1 was used */
/*	if(id > linear_hash->initial_size) { */
	return *((ion_byte_t *) key) % (2 * linear_hash->initial_size);
/*	} */
/*  */
/*	else { */
/*		return id % linear_hash->initial_size; */
/*	} */
}

int
insert_hash_to_bucket(
	ion_key_t			key,
	linear_hash_table_t *linear_hash
) {
	return *((ion_byte_t *) key) % linear_hash->initial_size;
}

/* Write the offset of bucket idx to the map in linear hash state */
void
store_bucket_loc_in_map(
	int					idx,
	ion_fpos_t			bucket_loc,
	linear_hash_table_t *linear_hash
) {
	/* create a pointer to the file */
	/* FILE *linear_hash_state; */

	/* linear_hash->state = fopen("linear_hash_state.bin", "r+"); */

	/* seek to the location of the bucket in the map */
	ion_fpos_t loc_in_map = sizeof(linear_hash_table_t) + idx * sizeof(ion_fpos_t);

	fseek(linear_hash->state, loc_in_map, SEEK_SET);

	/* read ion_fpos_t of bucket from mapping in linear hash */
	fwrite(&bucket_loc, sizeof(ion_fpos_t), 1, linear_hash->state);
	printf("Wrote the location of bucket %d to %ld in map\n", idx, bucket_loc);
}

/* ARRAY LIST METHODS */
array_list_t *
array_list_init(
	int				init_size,
	array_list_t	*array_list
) {
	array_list->current_size	= init_size;
	array_list->data			= malloc(init_size * sizeof(ion_fpos_t));
	return array_list;
}

void
array_list_insert(
	int				bucket_idx,
	ion_fpos_t		bucket_loc,
	array_list_t	*array_list
) {
	printf("INSERTING %ld AS HEAD LOC FOR %d\n", bucket_loc, bucket_idx);

	/* case we need to expand array */
	if (bucket_idx >= array_list->current_size) {
		array_list->current_size = array_list->current_size * 2;
		/* array_list->current_size = array_list->current_size + 10; */

		/* TODO UPDATE THE POINTER TO THE ARRAY LIST ON THE LINEAR HASH AFTER REALLOC? */
		array_list->data = (ion_fpos_t *) realloc(array_list->data, array_list->current_size * sizeof(ion_fpos_t));
		printf("expanded array list currrent size to %d\n", array_list->current_size);
	}

	array_list->data[bucket_idx] = bucket_loc;
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

/*
void
print_array_list_data(
	array_list_t *array_list
) {
	printf("ARRAY LIST\n\tcurrent_size = %d\n", array_list->current_size);

	for (int i = 0; i < array_list->current_size; i++) {
		printf("\t%d: %ld\n", i, array_list->data[i]);
	}
}
*/

void
print_linear_hash_state(
	linear_hash_table_t *linear_hash
) {
	printf("Linear Hash State\n\tinitial size: %d\n\tnum records %d\n\tnum buckets %d\n\tnext split: %d\n\tsplit threshold: %d\n", linear_hash->initial_size, linear_hash->num_records, linear_hash->num_buckets, linear_hash->next_split, linear_hash->split_threshold);
}

void
print_linear_hash_bucket_from_idx(
	int					idx,
	linear_hash_table_t *linear_hash
) {
	ion_fpos_t bucket_loc = bucket_idx_to_ion_fpos_t(idx, linear_hash);

	printf("PRINTING ALL %d IDX BUCKETS\nHEAD BUCKET AT %ld\n", idx, bucket_loc);

	linear_hash_bucket_t	bucket = linear_hash_get_bucket(bucket_loc, linear_hash);
	ion_fpos_t				record_loc;

	printf("head bucket\n");
	print_linear_hash_bucket(bucket);

	ion_byte_t	*record_key			= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*record_value		= alloca(linear_hash->super.record.value_size);
	ion_byte_t	record_status;
	ion_fpos_t	record_total_size	= linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(ion_byte_t);

	while (bucket.overflow_location != -1) {
		record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

		for (int i = 0; i < linear_hash->records_per_bucket; i++) {
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
		bucket_loc	= bucket.overflow_location;
		bucket		= linear_hash_get_bucket(bucket_loc, linear_hash);
	}

	record_loc = bucket_loc + sizeof(linear_hash_bucket_t);

	for (int i = 0; i < linear_hash->records_per_bucket; i++) {
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

/*
void
print_linear_hash_record(
	linear_hash_record_t record
) {
	printf("Record \n\tid %d\n", record.id);
}

void
print_linear_hash_bucket_map(
	linear_hash_table_t *linear_hash
) {
	printf("Bucket Map State:\n");

	for (int i = 0; i < linear_hash->bucket_map->current_size; i++) {
		printf("\tbucket idx: %d, bucket loc in data file %ld\n", i, array_list_get(i, linear_hash->bucket_map));
	}
}

void
linear_hash_bucket_iterator(
	ion_fpos_t			bucket_loc,
	linear_hash_table_t *linear_hash
) {
	linear_hash_bucket_t	bucket		= linear_hash_get_bucket(bucket_loc, linear_hash);
	ion_fpos_t				record_loc	= bucket_loc + sizeof(linear_hash_bucket_t);
	linear_hash_record_t	record;

	while (bucket.overflow_location != -1) {
		for (int i = 0; i < linear_hash->records_per_bucket; i++) {
			record		= linear_hash_get_record(record_loc, linear_hash);
			record_loc	+= sizeof(linear_hash_record_t);
		}

		record_loc	= bucket.overflow_location + sizeof(linear_hash_bucket_t);
		bucket		= linear_hash_get_bucket(bucket.overflow_location, linear_hash);
	}

	for (int i = 0; i < linear_hash->records_per_bucket; i++) {
		record		= linear_hash_get_record(record_loc, linear_hash);
		record_loc	+= sizeof(linear_hash_record_t);
	}
}
*/

/* CLOSE, OPEN, CREATE, DESTROY METHODS */
ion_err_t
linear_hash_close(
	linear_hash_table_t *linear_hash
) {
	free(linear_hash->bucket_map);
	linear_hash->bucket_map = NULL;

	if (0 != fclose(linear_hash->database)) {
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

	linear_hash->database = NULL;
	return err_ok;
}
