#include "flat_file_raw.h"

err_t
ff_initialize(
	ff_file_t			*file,
	key_type_t			key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size
)
{
#ifdef DEBUG
	printf("Initializing flat file\n");
#endif /* DEBUG */

	/* By default allow unique inserts only */
	file->write_concern 			= wc_insert_unique;
	file->super.record.key_size 	= key_size;
	file->super.record.value_size 	= value_size;
	file->super.key_type 			= key_type;

#ifdef USE_IONFS
	file->file_ptr = (ionfs_t *) malloc(sizeof(ionfs_t));

	if (ionfs_open(file->file_ptr, 1))
	{
		return -1;
	}
#endif /* USE_IONFS */

	/* Check if flat file already exists. If it does, check to make sure key are
	   value sizes are the same as well as the key type */
	uint8_t status = 0;

#ifdef USE_IONFS
	if (ionfs_read(file->file_ptr, 0, &status, SIZEOF(STATUS), 0))
#else /* USE_IONFS */
	if (device_read(0, &status, SIZEOF(STATUS), 0))
#endif
	{
		return err_file_incomplete_read;
	}

	if (status == INITIALIZED) {
		dictionary_parent_t super;

#ifdef USE_IONFS
		if (ionfs_read(file->file_ptr, 0, &super, sizeof(dictionary_parent_t),
			SIZEOF(STATUS)))
#else
		if (device_read(0, &super, sizeof(dictionary_parent_t), SIZEOF(STATUS)))
#endif
		{
			return err_file_incomplete_read;
		}

		if (file->super.record.key_size != super.record.key_size ||
			file->super.record.value_size != super.record.value_size ||
			file->super.key_type != super.key_type)
		{
			return -1; /* TODO: Need a new error type */
		}
	}
	else
	{
		/* Only allow record size that is <= BLOCK_SIZE bytes */
		if ((file->super.record.key_size + file->super.record.value_size +
			SIZEOF(STATUS)) > BLOCK_SIZE)
		{
			return err_file_write_error;
		}

		status_t init = INITIALIZED;

		/* Write header to block 0 with a status */
#ifdef USE_IONFS
		if (ionfs_write(file->file_ptr, 0, &init, SIZEOF(STATUS), 0) ||
			ionfs_write(file->file_ptr, 0, &(file->super), sizeof(file->super),
						SIZEOF(STATUS)))
#else /* USE_IONFS */
		if (device_write(0, &init, SIZEOF(STATUS), 0) ||
			device_write(0, &(file->super), sizeof(file->super), SIZEOF(STATUS)))
#endif
		{
			return err_file_write_error;
		}

		status_t end_record = END_RECORD;

		/* Write first data block with 0 records and an END_RECORD */
#ifdef USE_IONFS
		if (ionfs_write(file->file_ptr, 1, &end_record, SIZEOF(STATUS), 0))
#else /* USE_IONFS */
		if (device_write(1, &end_record, SIZEOF(STATUS), 0))
#endif
		{
			return err_file_write_error;
		}

		/* Flush data */
#ifdef USE_IONFS
		if (ionfs_flush(file->file_ptr))
#else /* USE_IONFS */
		if (device_flush())
#endif
		{
			return err_file_write_error;
		}
	}

#ifdef DEBUG
	printf("Record key size: %u\n", file->super.record.key_size);
	printf("Record value size: %u\n", file->super.record.value_size);
#endif

	return err_ok;
}

err_t
ff_destroy(
	ff_file_t 	*file
)
{
	status_t uninit = UNINITIALIZED;

	/* Write UNINITIALIZED status */
#ifdef USE_IONFS
	if (ionfs_write(file->file_ptr, 0, &uninit, SIZEOF(STATUS), 0) ||
		ionfs_flush(file->file_ptr))
#else /* USE_IONFS */
	if (device_write(0, &uninit, SIZEOF(STATUS), 0) || device_flush())
#endif
	{
		return err_file_write_error;
	}

#ifdef USE_IONFS
	ionfs_remove(1);
	free(file->file_ptr);
#endif /* USE_IONFS */

	file->super.compare = NULL;
	file->super.record.key_size 	= 0;
	file->super.record.value_size	= 0;

	return err_ok;
}

err_t
ff_update(
	ff_file_t	 	*file,
	ion_key_t 		key,
	ion_value_t		value
)
{
	/* TODO: lock potentially required */
	write_concern_t current_write_concern 	= file->write_concern;
	/* Change write concern to allow update */
	file->write_concern 					= wc_update;
	err_t result 							= ff_insert(file, key, value);
	file->write_concern 					= current_write_concern;

	return result;
}

err_t
ff_insert(
	ff_file_t 		*file,
	ion_key_t 		key,
	ion_value_t	 	value
)
{
	f_file_record_t *record;

	int record_size = file->super.record.key_size +
					  file->super.record.value_size + SIZEOF(STATUS);

#ifdef DEBUG
	printf("Inserting record of size %u\n", record_size);
#endif

	if ((record = (f_file_record_t *) malloc(record_size)) == NULL)
	{
		return err_out_of_memory;
	}

	uint32_t block_address = 1;
	uint16_t block_current_record = 0;

#ifdef USE_SD_RAW
	if (sd_raw_read_continuous_start(1))
	{
		free(record);
		return err_file_incomplete_read;
	}
#endif /* USE_SD_RAW */

	do
	{
		if ((record_size * (block_current_record + 1)) >= BLOCK_SIZE)
		{
			block_address++;
			block_current_record = 0;

#ifdef USE_SD_RAW
			if (sd_raw_read_continuous_next())
			{
				free(record);
				return err_file_incomplete_read;
			}
#endif /* USE_SD_RAW */
		}

#if defined(USE_SD_RAW)
		if (sd_raw_read_continuous(record, record_size,
								   record_size * block_current_record))
#elif defined(USE_IONFS)
		if (ionfs_read(file->file_ptr, block_address, record, record_size,
			record_size * block_current_record))
#else
		if (device_read(block_address, record, record_size,
						record_size * block_current_record))
#endif
		{
			free(record);
			return err_file_incomplete_read;
		}	

		if (record->status == END_RECORD || record->status == DELETED)
		{
#ifdef USE_SD_RAW
			if (sd_raw_read_continuous_stop())
			{
				free(record);
				return err_file_incomplete_read;
			}
#endif /* USE_SD_RAW */

#ifdef DEBUG
			printf("Writing to deleted or end of record location %u in block %u\n",
				   block_current_record, block_address);
#endif
			status_t status = record->status;

			record->status = IN_USE;
			memcpy(record->data, key, (file->super.record.key_size));
			memcpy(record->data + file->super.record.key_size, value,
				   (file->super.record.value_size));

#ifdef USE_IONFS
			if (ionfs_write(file->file_ptr, block_address, record, record_size,
							 record_size * block_current_record))
#else
			if (device_write(block_address, record, record_size,
							 record_size * block_current_record))
#endif
			{
				free(record);
				return err_file_write_error;
			}

			if (status == END_RECORD)
			{
				if ((record_size * (block_current_record + 2)) < BLOCK_SIZE)
				{
#ifdef USE_IONFS
					if (ionfs_write(file->file_ptr, block_address, &status, 
						SIZEOF(STATUS), record_size * (block_current_record + 1)))
#else
					if (device_write(block_address, &status, SIZEOF(STATUS),
						record_size * (block_current_record + 1)))
#endif
					{
						free(record);
						return err_file_write_error;
					}
				}
				else
				{
#ifdef USE_IONFS
					if (ionfs_write(file->file_ptr, block_address + 1, &status, 
						SIZEOF(STATUS), 0))
#else
					if (device_write(block_address + 1, &status, SIZEOF(STATUS), 
									 0))
#endif
					{
						free(record);
						return err_file_write_error;
					}
				}
			}

#ifdef USE_IONFS
			if (ionfs_flush(file->file_ptr))
#else
			if (device_flush())
#endif
			{
				free(record);
				return err_file_write_error;				
			}

#ifdef DEBUG
			printf("Insert Successful\n");
#endif	

			free(record);
			return err_ok;
		} 
		else if (record->status == IN_USE)
		{
			if (file->super.compare((ion_key_t)record->data, key,
									file->super.record.key_size) == IS_EQUAL)
			{
#ifdef USE_SD_RAW
				if (sd_raw_read_continuous_stop())
				{
					free(record);
					return err_file_incomplete_read;
				}
#endif /* USE_SD_RAW */

#ifdef DEBUG
				printf("Writing to location in use\n");
#endif
				if (file->write_concern == wc_insert_unique)	/* Allow unique entries only */
				{
					free(record);
					return err_duplicate_key;
				}
				else if (file->write_concern == wc_update)	/* Allows for values to be updated */
				{
#ifdef USE_IONFS
					if (ionfs_write(file->file_ptr, block_address, value,
									file->super.record.value_size, 
									record_size * block_current_record +
									SIZEOF(STATUS) + file->super.record.key_size))
#else
					if (device_write(block_address, value, file->super.record.value_size,
									 record_size * block_current_record + SIZEOF(STATUS) +
									 file->super.record.key_size))
#endif
					{
						free(record);
						return err_file_write_error;
					}

#ifdef USE_IONFS
					if (ionfs_flush(file->file_ptr))
#else
					if (device_flush())
#endif
					{
						free(record);
						return err_file_write_error;				
					}

#ifdef DEBUG
					printf("Insert Successful\n");
#endif	

					free(record);
					return err_ok;
				}
				else
				{
					free(record);
					return err_write_concern;	/* there is a configuration issue with write concern */
				}
			}
		}
		else	/* Invalid record status */
		{
#ifdef USE_SD_RAW
			if (sd_raw_read_continuous_stop())
			{
				free(record);
				return err_file_incomplete_read;
			}
#endif /* USE_SD_RAW */

			free(record);
			return err_file_incomplete_read;
		}

		block_current_record++;
	} while(record->status != END_RECORD);

	return err_ok;
}

err_t
ff_find_item_loc(
	ff_file_t 		*file,
	ion_key_t		key,
	uint32_t		*block_address,
	uint16_t		*block_current_record
)
{
	f_file_record_t *record;

	int record_size = file->super.record.key_size +
					  file->super.record.value_size + SIZEOF(STATUS);

	if ((record = (f_file_record_t *) malloc(record_size)) == NULL)
	{
		return err_out_of_memory;
	}

#ifdef USE_SD_RAW
	if (sd_raw_read_continuous_start(*block_address))
	{
		free(record);
		return err_file_incomplete_read;
	}
#endif /* USE_SD_RAW */

	do
	{
		if ((record_size * ((*block_current_record) + 1)) >= BLOCK_SIZE)
		{
			(*block_address)++;
			*block_current_record = 0;

#ifdef USE_SD_RAW
			if (sd_raw_read_continuous_next())
			{
				free(record);
				return err_file_incomplete_read;
			}
		}
#endif /* USE_SD_RAW */

#if defined(USE_SD_RAW)
		if (sd_raw_read_continuous(record, record_size,
					   			   record_size * (*block_current_record)))
#elif defined(USE_IONFS)
		if (ionfs_read(file->file_ptr, *block_address, record, record_size,
					   record_size * (*block_current_record)))
#else
		if (device_read(record, record_size, *block_address,
						record_size * (*block_current_record)))
#endif
		{
			free(record);
			return err_file_incomplete_read;
		}

		if (record->status == END_RECORD)
		{
#ifdef USE_SD_RAW
			if (sd_raw_read_continuous_stop())
			{
				free(record);
				return err_file_incomplete_read;
			}
#endif /* USE_SD_RAW */

			free(record);
			return err_item_not_found;
		}

		if (record->status != DELETED)
		{
			if (file->super.compare((ion_key_t) record->data, key,
									file->super.record.key_size) == IS_EQUAL)
			{
#ifdef USE_SD_RAW
				if (sd_raw_read_continuous_stop())
				{
					free(record);
					return err_file_incomplete_read;
				}
#endif /* USE_SD_RAW */

				free(record);
				return err_ok;
			}
		}

		(*block_current_record)++;
	} while(record->status != END_RECORD);

#ifdef USE_SD_RAW
	if (sd_raw_read_continuous_stop())
	{
		free(record);
		return err_file_incomplete_read;
	}
#endif /* USE_SD_RAW */

	free(record);
	return err_item_not_found;
}

return_status_t
ff_delete(
	ff_file_t 		*file,
	ion_key_t 		key
)
{
	/* Position to delete */
	uint32_t block_address = 1;				
	uint16_t block_current_record = 0;
	/* return status */
	return_status_t status;
	/* Initialize such that record will not be found */
	status.err = err_item_not_found;
	/* number of items deleted */
	status.count = 0;

	int record_size = file->super.record.key_size + file->super.record.value_size
					  + SIZEOF(STATUS);

	while (ff_find_item_loc(file, key, &block_address, &block_current_record) !=
							err_item_not_found)
	{
		status_t record_status = DELETED;

#ifdef USE_IONFS
		if (ionfs_write(file->file_ptr, block_address, &record_status, 
			sizeof(record_status), record_size * block_current_record) ||
			ionfs_flush(file->file_ptr))
#else
		if (device_write(block_address, &record_status, sizeof(record_status),
			record_size * block_current_record) ||
			device_flush())
#endif
		{
			status.err = err_file_write_error;
			return status;
		}

		status.count++;
#ifdef DEBUG
		printf("Item deleted at block address %u and it is record %u in the block\n",
			   block_address, block_current_record);
#endif
		block_current_record++;
	}

	if (status.count > 0)
	{
		status.err = err_ok;
	}

	return status;
}

err_t
ff_query(
	ff_file_t 		*file,
	ion_key_t 		key,
	ion_value_t		value)
{
	uint32_t block_address = 1;
	uint16_t block_current_record = 0;

	int record_size = file->super.record.key_size + file->super.record.value_size
					  + SIZEOF(STATUS);

	if (ff_find_item_loc(file, key, &block_address, &block_current_record) == err_ok)
	{
#ifdef DEBUG
		printf("Item found at block address %u and it is record %u in the block\n",
			   block_address, block_current_record);
#endif

#ifdef USE_IONFS
		if (ionfs_read(file->file_ptr, block_address, value,
			file->super.record.value_size, record_size * block_current_record + 
			SIZEOF(STATUS) + file->super.record.key_size))
#else
		if (device_read(block_address, value, file->super.record.value_size,
			record_size * block_current_record + SIZEOF(STATUS) +
			file->super.record.key_size))
#endif
		{
			return err_file_incomplete_read;
		}

		return err_ok;
	}
	else
	{
#ifdef DEBUG
		printf("Item not found in file\n");
#endif
		/* Set the number of bytes to 0 */
		value = NULL;
		return err_item_not_found;
	}
}
