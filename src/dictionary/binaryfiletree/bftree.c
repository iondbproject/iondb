/*
 * bftree.c
 *
 *  Created on: Sep 10, 2014
 *      Author: workstation
 */

#include "bftree.h"

err_t
bft_initialize(
		bft_file_tree_t		*binary_tree,
	    key_type_t			key_type,
		ion_key_size_t		key_size,
		ion_value_size_t	value_size
)
{
#if DEBUG
	io_printf("Initializing file \n");
#endif

	//check to see if file exists and if it does, throw exception
	if ((binary_tree->key_file_ptr = fopen("btree_key.bin", "r")) != 0)
	{
			fclose(binary_tree->key_file_ptr);
			return -1;			/** @todo correct error return code*/
	}

	if ((binary_tree->value_file_ptr = fopen("btree_value.bin", "r")) != 0)
	{
			fclose(binary_tree->value_file_ptr);
			return -1;			/** @todo correct error return code*/
	}

	//assume the the file does not exists -> this will come from the upper layers
	binary_tree->key_file_ptr = fopen("btree_key.bin","w+");
	if (!binary_tree->key_file_ptr)
	{
		printf("Unable to open file!");
		return -2;				/** @todo correct error return code */
	}

	binary_tree->value_file_ptr = fopen("btree_value.bin","w+");
	if (!binary_tree->value_file_ptr)
	{
		fclose(binary_tree->key_file_ptr);
		printf("Unable to open file!");
		return -2;				/** @todo correct error return code */
	}

	/** @todo move to parent? */
	binary_tree->write_concern 			= wc_insert_unique;			/* By default allow unique inserts only */
	binary_tree->super.record.key_size 	= key_size;
	binary_tree->super.record.value_size 	= value_size;
	binary_tree->super.key_type 			= key_type;

	/** @todo Configuration information needs to be stored */
	//need to write a file header out here
	//fwrite(&(file->super), sizeof(file->super) , 1, file->file_ptr);

	//record the start of the data block
	//fgetpos(file->file_ptr, &file->start_of_data);

#if DEBUG
	io_printf("Record key size: %i\n", file->super.record.key_size);
	io_printf("Record value size: %i\n", file->super.record.value_size);
#endif

	//and flush contents to disk
	//fflush(file->file_ptr);

	return 0;
}

err_t
bft_insert(
	FILE 		*key_file,
	FILE 		*value_file,
	int 		key,
	ion_value_t value)
{

	//go back to the starting positin
	rewind(key_file);
	node_t node;
	fpos_t	parent_pos;
	printf("inserting %i\n",key);
	do
	{
		fgetpos( key_file, &parent_pos);
		//int size = fread(&node,sizeof(node_t), 1, file);
		if (0 != fread(&node,sizeof(node_t), 1, key_file))
		{
#if DEBUG
			DUMP(node.key,"%i");
#endif
			int * child;
			if( key < node.key)
			{
				child = &(node.left);
			}else if (key > node.key)
			{
				child = &(node.right);
			}
			else
			{
				//printf("No Duplicates allowed");
				return -1;
			}
			if (*child == -1)		//attach to the left and be done with it
			{
				fpos_t	child_pos;
				fseek(key_file,0,SEEK_END);
				fgetpos(key_file, &child_pos);
				node_t child_node;
				child_node.key = key;
				child_node.left = -1;
				child_node.right = -1;
				child_node.parent = parent_pos;
				child_node.status = IN_USE;

				fseek(value_file,0,SEEK_END);
				child_node.value = ftell(value_file);
				printf("location in file %i\n", child_node.value);
				f_file_record_t * record;
				int record_size = SIZEOF(STATUS) + size_of_key + size_of_value;
				record = (f_file_record_t *)malloc(record_size);
				record->status = child_node.status;
				memcpy(record->data,&key,size_of_key);
				memcpy(record->data+size_of_key,value,size_of_value);
				{
					int i;
					for (i = 0; i< record_size;i++)
						printf("%c", *(char *)(record+i));
					printf("\n");
				}

				/*fwrite(&key,sizeof(key),1,value_file);*/
				fwrite(record,record_size,1,value_file);
				fflush(value_file);
				fwrite(&child_node,sizeof(child_node),1,key_file);
				//fflush(file);
				*child = child_pos;
				fsetpos(key_file, &parent_pos);
#if DEBUG
				printf("moving to parent position %x\n",parent_pos);
				printf("parent key %i\n",node.key);
#endif
				fwrite(&node,sizeof(node),1,key_file);
				fflush(key_file);
				return err_ok;
			}
			else 		//duplicate
			{
				fseek(key_file,*child,SEEK_SET);			//good to know
				//fsetpos(file,(fpos_t *)(&node.right));	<----this is a problem with fsetpos needs to be calculated relative to current position
				//fgetpos(file, &parent_pos);		//update parent pos
				parent_pos = ftell(key_file);
			}
		}
		else
		{
			//this is the root node
			fseek(key_file,0,SEEK_END);
			node_t child_node;
			child_node.key = key;
			child_node.left = -1;
			child_node.right = -1;
			child_node.parent = -1;
			child_node.status = 1;

			fseek(value_file,0,SEEK_END);
			child_node.value = ftell(value_file);

			printf("location in file %i\n", child_node.value);
			f_file_record_t * record;
			int record_size = SIZEOF(STATUS) + size_of_key + size_of_value;
			record = (f_file_record_t *)malloc(record_size);
			record->status = child_node.status;
			memcpy(record->data,&key,size_of_key);
			memcpy(record->data+size_of_key,value,size_of_value);
			{
				int i;
				for (i = 0; i< record_size;i++)
					printf("%c", *(char *)(record+i));
				printf("\n");
			}

			/*fwrite(&key,sizeof(key),1,value_file);*/
			fwrite(record,record_size,1,value_file);
			fflush(value_file);
			fwrite(&child_node,sizeof(child_node),1,key_file);
			fflush(key_file);
			return err_ok;
		}
	}while (1);
}

err_t
bft_find(
	FILE 		*key_file,
	FILE 		*value_file,
	int 		key,
	ion_value_t value
)
{
	//go back to the starting positin
	rewind(key_file);
	node_t node;
	fpos_t	parent_pos;

	int iops;
	int seeks;

	seeks = 0;
	iops = 0;

	do
	{
		iops++;
		fgetpos( key_file, &parent_pos);
		if (0 != fread(&node,sizeof(node_t), 1, key_file))
		{
#if DEBUG
			DUMP(node.key,"%i");
#endif
			int * child;
			if( key < node.key)
			{
				child = &(node.left);
			}else if (key > node.key)
			{
				child = &(node.right);
			}
			else
			{
				if (node.status == IN_USE)
				{
					printf("Found.  IOPS %i seeks %i\n",iops, seeks);
					printf("position %i\n",node.value);
					int cur_pos = ftell(value_file);
					DUMP(cur_pos,"%i");
					int offset;
					if (cur_pos >= node.value)
					{
						offset = node.value - cur_pos;
					}
					else
					{
						offset = cur_pos - node.value;
					}
					DUMP(offset,"%i");
					fseek(value_file,offset+SIZEOF(STATUS) + size_of_key,SEEK_CUR);
					fread(value,size_of_value,1,value_file);
					return err_ok;
				}
				else
				{
					printf("Not found.  IOPS %i seeks %i\n",iops, seeks);
					return err_item_not_found;
				}
			}

			if (*child == -1)		//attach to the left and be done with it
			{
				return err_item_not_found;
			}
			else 		//duplicate
			{
				seeks++;
				fseek(key_file,*child,SEEK_SET);			//good to know
				//fsetpos(file,(fpos_t *)(&node.right));	<----this is a problem with fsetpos needs to be calculated relative to current position
				//fgetpos(file, &parent_pos);		//update parent pos
				parent_pos = ftell(key_file);
			}
		}
		else
		{
			return err_item_not_found;
		}
	}while (1);
}


/**
 * @brief		Deletes a value from the binary tree
 * @details		Only requires the key file as the status will be used to register the fact
 * 				the that item has been deleted.
 *
 * @param key_file
 * @param key
 * @return
 */
err_t
bft_delete(
	FILE 		*key_file,
	int 		key
)
{
	//go back to the starting position
	rewind(key_file);
	node_t node;
	fpos_t	parent_pos;

	int iops;
	int seeks;

	seeks = 0;
	iops = 0;

	do
	{
		iops++;
		fgetpos( key_file, &parent_pos);
		if (0 != fread(&node,sizeof(node_t), 1, key_file))
		{
#if DEBUG
			DUMP(node.key,"%i");
#endif
			int * child;
			if( key < node.key)
			{
				child = &(node.left);
			}else if (key > node.key)
			{
				child = &(node.right);
			}
			else
			{
				if (node.status == IN_USE)
				{
					printf("Found->deleting.  IOPS %i seeks %i\n",iops, seeks);
					node.status = DELETED;
					seeks++;
					fseek(key_file, -sizeof(node_t), SEEK_CUR);
					iops++;
					fwrite(&node,sizeof(node_t),1, key_file);
					fflush(key_file);
					return err_ok;
				}
				else
				{
					printf("Not found.  IOPS %i seeks %i\n",iops, seeks);
					return err_item_not_found;
				}
			}

			if (*child == -1)		//attach to the left and be done with it
			{
				return err_item_not_found;
			}
			else 		//duplicate
			{
				seeks++;
				fseek(key_file,*child,SEEK_SET);			//good to know
				//fsetpos(key_file,(fpos_t *)(&node.right));	<----this is a problem with fsetpos needs to be calculated relative to current position
				//fgetpos(key_file, &parent_pos);		//update parent pos
				parent_pos = ftell(key_file);
			}
		}
		else
		{
			return err_item_not_found;
		}
	}while (1);
}


