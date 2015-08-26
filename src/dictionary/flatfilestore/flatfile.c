/*
 * flatfile.c
 *
 *  Created on: Aug 14, 2014
 *      Author: workstation
 */

#include "flatfile.h"

err_t
ff_initialize(
		ff_file_t			*file,
	    key_type_t			key_type,
		ion_key_size_t		key_size,
		ion_value_size_t	value_size
)
{
#if DEBUG
	io_printf("Initializing file \n");
#endif

	//check to see if file exists and if it does, throw exception

	if ((file->file_ptr = fopen("file.bin", "rb")) != NULL)
	{
		fclose(file->file_ptr);
		return -1;			/** @todo correct error return code*/
	}

	//assume the the file does not exists -> this will come from the upper layers
	file->file_ptr = fopen("file.bin","w+b");

	if (file->file_ptr == NULL)
	{
		return -2;				/** @todo correct error return code */
	}

	/** @todo move to parent? */
	file->write_concern 			= wc_insert_unique;			/* By default allow unique inserts only */
	file->super.record.key_size 	= key_size;
	file->super.record.value_size 	= value_size;
	file->super.key_type 			= key_type;

	//need to write a file header out here
	fwrite(&(file->super), sizeof(file->super) , 1, file->file_ptr);

	//record the start of the data block
	file->start_of_data = ftell(file->file_ptr);

#if DEBUG
	io_printf("Record key size: %i\n", file->super.record.key_size);
	io_printf("Record value size: %i\n", file->super.record.value_size);
#endif

	//and flush contents to disk
	fflush(file->file_ptr);
	return 0;
}

err_t
ff_destroy(
		ff_file_t 	*file
)
{

	file->super.compare = NULL;
	file->super.record.key_size 	= 0;
	file->super.record.value_size	= 0;

	fclose(file->file_ptr);
	if (fremove("file.bin") == 0)			//check to ensure that you are not freeing something already free
	{
		return err_ok;
	}
	else
	{
		return err_colllection_destruction_error;
	}
}


err_t
ff_update(
	ff_file_t	 	*file,
	ion_key_t 		key,
	ion_value_t		value
)
{
	//TODO: lock potentially required
	write_concern_t current_write_concern 	= file->write_concern;
	file->write_concern 					= wc_update;			//change write concern to allow update
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
/** @todo requires massive cleanup for function exit */
	f_file_record_t *record;

	int record_size = file->super.record.key_size + file->super.record.value_size
					+ SIZEOF(STATUS);
#if DEBUG
	printf("inserting record of size %i \n",record_size);
#endif

	if ((record = (f_file_record_t *)malloc(record_size)) == NULL)
	{
		return err_out_of_memory;
	}

	//set to start of data block
	fseek(file->file_ptr, file->start_of_data, SEEK_SET);

#if DEBUG
	printf("resetting to start of data\n");
	printf("start of data %i\n",(int)file->start_of_data);
	printf("reading record %i bytes long\n",record_size);
#endif

	do
	{
		fread(record, record_size, 1, file->file_ptr);

		if (feof(file->file_ptr) /*== EOF*//*|| (record->status == DELETED)*/ )
		{
			//problem is here with base types as it is just an array of data.  Need better way
#if DEBUG
			printf("inserting record\n");
#endif
			record->status = IN_USE;
			memcpy(record->data, key, (file->super.record.key_size));
			memcpy(record->data + file->super.record.key_size, value,
					(file->super.record.value_size));
			if (1 	== fwrite(record,record_size,1,file->file_ptr))
			{
				fflush(file->file_ptr);
				free(record);
				return err_ok;
			}
			else
			{
				fflush(file->file_ptr);
				free(record);
				return err_file_write_error;
			}
		}

		if (record->status == IN_USE) 	//if a cell is in use, need to key to
		{
#if DEBUG
			printf("encountered location in use\n");
#endif
			/*if (memcmp(item->data, key, hash_map->record.key_size) == IS_EQUAL)*/
			if (file->super.compare((ion_key_t)record->data, key, file->super.record.key_size) == IS_EQUAL)
			{
				if (file->write_concern == wc_insert_unique)//allow unique entries only
				{
					free(record);
					return err_duplicate_key;
				}
				else if (file->write_concern == wc_update)//allows for values to be updated											//
				{
					//rewind pointer to value
					fseek(file->file_ptr, -file->super.record.value_size/*sizeof(record->status) + file->super.record.key_size*/, SEEK_CUR);
					if (1 	== fwrite(value, file->super.record.value_size, 1, file->file_ptr))
					{
						fflush(file->file_ptr);
						free(record);
						return err_ok;
					}
					else
					{
						free(record);
						return err_file_write_error;
					}
				}
				else
				{
					free(record);
					return err_write_concern;// there is a configuration issue with write concern
				}
			}
		}
		//There is no other condition now

	}while (!feof(file->file_ptr)/* != EOF*/ );		//loop until a deleted location or EOF

	free(record);
	return err_ok;		//this needs to be corrected
}

err_t
ff_find_item_loc(
	ff_file_t 			*file,
	ion_key_t			key,
	ion_fpos_t			*location
)
{

	if (*location == -1) 	//then the position has not been initialized
	{
		fseek(file->file_ptr, file->start_of_data, SEEK_SET);
	}//otherwise continue on from the position
	else
	{
		fseek(file->file_ptr, *location, SEEK_SET);
	}
	int record_size = SIZEOF(STATUS) + file->super.record.key_size+file->super.record.value_size;

	f_file_record_t * record;
	record = (f_file_record_t *)malloc(record_size);

	ion_fpos_t cur_pos;

	//while we have not reached the end of the the datafile
	while (!feof(file->file_ptr)/* != EOF*/)
	{

		cur_pos = ftell(file->file_ptr);	//get current position in file

		/** @todo depending on how much memory available, could minimize reads through buffering*/
		fread(record,record_size,1,file->file_ptr);

		if (record->status != DELETED)
		{
			/** @todo correct compare to use proper return type*/
			int key_is_equal 	= file->super.compare((ion_key_t)record->data, key, file->super.record.key_size);
			
			if (IS_EQUAL == key_is_equal)
			{

				*location = cur_pos;
				free(record);
				return err_ok;
			}
		}
	}
	free(record);
	return err_item_not_found; 				//key have not been found
}

ion_status_t
ff_delete(
	ff_file_t 		*file,
	ion_key_t 		key
)
{
	ion_fpos_t loc 		= UNINITIALISED;		// position to delete
	ion_status_t status;					// return status
	status.err 		= err_item_not_found;	// init such that record will not be found
	status.count 	= 0;					// number of items deleted

	while (ff_find_item_loc(file, key, &loc) != err_item_not_found)
	{
		f_file_record_t record;
		record.status = DELETED;

		fseek(file->file_ptr, loc, SEEK_SET);
		fwrite(&record,sizeof(record.status),1,file->file_ptr);
		status.count ++;
#if DEBUG
		io_printf("Item deleted at location %d\n", loc);
#endif
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
	ion_fpos_t loc = -1;		//initialize

	if (ff_find_item_loc(file, key, &loc) == err_ok)
	{
#if DEBUG
		io_printf("Item found at location %d\n", loc);
#endif
		//isolate value from record
		fseek(file->file_ptr,loc + SIZEOF(STATUS) + file->super.record.key_size,SEEK_SET);
		//copy the value from file to memory
		fread(value, file->super.record.value_size, 1, file->file_ptr);
		return err_ok;
	}
	else
	{
#if DEBUG
		io_printf("Item not found in file.\n");
#endif
		value = NULL;				/**set the number of bytes to 0 */
		return err_item_not_found;
	}
}
