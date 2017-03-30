#include "bpp_tree.h"
static long* getRowIndices(ion_bpp_header *head);
static void writeHeader(FILE *file, ion_bpp_header *new_head);
static ion_bpp* openBpp(char *filename, ion_bpp_header *new_file);
static void* searchRecords(ion_bpp *bpp, void* searchKey);
static ion_bpp_search_record* searchRecordOffset(ion_bpp *bpp, void* searchKey);
static void* readRecords(ion_bpp *bpp, ion_bpp_search_record *rec, bool return_all);
static unsigned char insertRecords(ion_bpp *bpp, void* key, void* value);
static unsigned char updateRecords(ion_bpp *bpp, void* key, void* value);
static unsigned char deleteRecords(ion_bpp *bpp, void* key);
static bool mergeRight(ion_bpp *bpp, int depth, long leaf_offset);
static bool mergeUp(ion_bpp *bpp, int depth, long leaf_offset);
static void move(ion_bpp *bpp, unsigned int depth, long source_offset, long dest_offset, bool clear, bool whole_leaf);
static unsigned char* calculateNullNode(unsigned short keySize, unsigned char revision);

static unsigned short getLeafCount(ion_bpp *bpp, unsigned short elementSize, long absolute_offset);

static void debugPrintHeader(ion_bpp_header *head);
static void debugPrintTree(ion_bpp *tree);

/*
 * Determine where to start probing for each level of the B+ Tree on Disk
 * returns a pointer.
 */
static long* getRowIndices(ion_bpp_header *head){
	if(head->depth > 0) {
		long* indexes;
		indexes = (long *) malloc(sizeof(long) * head->depth+1);
		long records_per_row = head->leafCount;
		indexes[0] = BPP_FIRST_LEAF_OFFSET;
		for(unsigned int i = 1; i<head->depth+1; i++){
			indexes[i] = indexes[i-1] + records_per_row * head->keySize;
			records_per_row *= head->leafCount + 1;	//Links are Leaf+1, times the number of records in the previous row.
		}
		//if(head->depth > 1){ //Root could be last leaf I guess
		//	records_per_row /= head->leafCount + 1;
		//	indexes[head->depth] = indexes[head->depth-1] + records_per_row * head->valueSize;
		//}
		return indexes;
	} else {return NULL;}
}
static void writeHeader(FILE *file, ion_bpp_header *head){
	fseek(file, 0, SEEK_SET);
	if(fwrite(&head->revision, sizeof(unsigned char), 1, file) == 0) {
		printf("ERR: Write of Revision Failed!\n"); //TODO ERROR HANDLING
	}
	if(fwrite(&head->keySize, sizeof(unsigned short), 3, file) == 0) { /*Write all 3 shorts */
		printf("ERR: Write of keySize,valueSize,leafCount Failed!\n");
	}
	if(fwrite(&head->recordCount, sizeof(unsigned long), 2, file) == 0) { /*Write both longs */
		printf("ERR: Write of recordCount, depth Failed!\n");
	}
	
	if(head->revision > 0) {
		if(fwrite(&head->allow_duplicates, sizeof(bool), 1, file) == 0) {
			printf("ERR: Write of allow_duplicates Failed!\n");
		}
	}
	//fwrite(&EMPTY,1,9,file);
}
static ion_bpp* openBpp(char *filename, ion_bpp_header *new_file){
	ion_bpp *newTree;
	newTree = (ion_bpp *) malloc(sizeof(ion_bpp));
	newTree->fileName = filename;

	//Check if exists
	newTree->file = fopen(filename,"r");
	if(newTree->file == NULL){
		newTree->file = fopen(filename, "w+b"); //If it doesnt exist, create.
	} else {
		fclose(newTree->file);
		newTree->file = fopen(filename,"rw+b"); //Otherwise, open for editing.
	}
	if(newTree->file == NULL) {printf("EXCEPTION; File load failure!\n");}


	newTree->head = (ion_bpp_header *) malloc(sizeof(ion_bpp_header));
	if(fread(&newTree->head->revision, 1, 1, newTree->file) == 0) {
		printf("FILE EMPTY\n");
		free(newTree->head);
		if(new_file == NULL){
			printf("FAILURE; FILE EMPTY, AND STARTER HEADER NULL\n");
			fclose(newTree->file);
			free(newTree);
			return NULL;
		}
	//TODO: Add check that leafs/keys/values aren't size 0.
		newTree->head = new_file;
		newTree->head->depth = 0;
		newTree->head->recordCount = 0;
		writeHeader(newTree->file, newTree->head);
	} else {
		if(new_file != NULL) {free(new_file);}
		if(fread(&newTree->head->keySize, sizeof(unsigned short), 3, newTree->file) == 0) {
			printf("ERR: Read of keySize,valueSize,leafCount Failed!\n");
		}
		if(fread(&newTree->head->recordCount, sizeof(unsigned long), 2, newTree->file) == 0) {
			printf("ERR: Read of recordCount, depth Failed!\n");
		}
		if(newTree->head->revision > 0) {
			if(fread(&newTree->head->allow_duplicates, sizeof(bool), 1, newTree->file) == 0) {
				printf("ERR: Read of allow_duplicates Failed!\n");
			}
		} else {
			newTree->head->allow_duplicates = 0;
		}
		
	}
	newTree->indices = getRowIndices(newTree->head);
	return newTree;
}
static void debugPrintHeader(ion_bpp_header *head){
	if(head==NULL){printf("Header is NULL\n");} else{
	printf("Revision Version:\t%i\nKey Size (bytes):\t%i\nValue Size (bytes):\t%i\nLeaf Count:\t\t%i\nRecord Count:\t\t%lu\nTree Depth:\t\t%lu\n",head->revision,head->keySize,head->valueSize,head->leafCount,head->recordCount,head->depth);
	}
}
static void* searchRecords(ion_bpp *bpp, void* searchKey){ //TODO CLEANUP, FIXES //PLZ USE OTHER SEARCH
	int* tmp = (int*) searchKey;
	printf("Searching for %i\n\n",*tmp);
	unsigned char* leaf;
	leaf = malloc(bpp->head->keySize * bpp->head->leafCount);
	long leaf_offset=0;
	bool found = 0;
	bool loc = 0;
	//Approach Appropriate Bottom Row.
	for (unsigned int i = 0; i<bpp->head->depth; i++){
		printf("[I]%i [index] %lu [offset] %lu\n", i,bpp->indices[i],leaf_offset);
		fseek(bpp->file, bpp->indices[i]+(leaf_offset*bpp->head->keySize),SEEK_SET);
		fread(leaf,bpp->head->keySize, bpp->head->leafCount, bpp->file);
		found = 0;
		for (unsigned int j = 0; j<bpp->head->leafCount;j++){
			unsigned char* element = leaf + (j * bpp->head->keySize);
			tmp = (int*) element;
			printf("|%i|",*tmp);
			int comp = memcmp(searchKey,element/*leaf[j]*/,bpp->head->keySize);
			if (comp<0 || comp==0){
				printf("Modifying Offset-\n");
				//if (i<bpp->head->depth) {
				printf("[]");
				//leaf_offset = (leaf_offset * (bpp->head->leafCount+1))+j +1; //On final row, value is directly bellow.
				if(comp == 0){
					leaf_offset+=j;
					if (i==bpp->head->depth-1){ //TODO Remove when bug fixed
						loc = 1;
					} else {leaf_offset = leaf_offset + ((j+1)*(bpp->head->leafCount));}
				} else {
					printf("][");
					leaf_offset += j;
				}
				found = 1;
				break;
			} else {
				unsigned char* nullNode = (unsigned char*) calculateNullNode(0,bpp->head->revision);
				if(memcmp(nullNode,element,bpp->head->keySize) == 0){
					printf("|NULL|[J]%i",j);
					leaf_offset = (leaf_offset * (bpp->head->leafCount+1))+(j+1);
					found = 1;
					break;
				}
			}
		}
		printf("\n");
		if (found == 0){
			printf("Modifying Offset\n");
			if(leaf_offset > 0) {
				leaf_offset = ((leaf_offset * (bpp->head->leafCount+1))-1)*bpp->head->leafCount;
			} else {
				leaf_offset = bpp->head->leafCount;
			}
		}
	}
	//If record was found, return value.
	if(loc != 0){
		printf("[Located] %i [index] %lu [offset] %lu\n", loc,bpp->indices[bpp->head->depth],leaf_offset);
		fseek(bpp->file, bpp->indices[bpp->head->depth]+(leaf_offset*bpp->head->valueSize),SEEK_SET);
		fread(leaf,bpp->head->valueSize, 1, bpp->file);
		return leaf;
	} else {
		return NULL;
	}
}
static unsigned char insertRecords(ion_bpp *bpp, void* key, void* value){
	if(key == NULL){return 255;}/*Invalid Key TODO::Check if Key value == 0*/
	//int depth = 0;
	
	//Special case, EMPTY tree
	if(bpp->head->depth==0){
		printf("Begin Special case Insertion!\n");
		if(bpp->file == NULL){printf("Wtf? NULL?\n");}
		fseek(bpp->file, BPP_FIRST_LEAF_OFFSET, SEEK_SET);
		if(fwrite(&key, bpp->head->keySize, 1, bpp->file) == 0) {
			printf("ERR: Insert First Leaf Failed!\n");
		} else {
			void *EMPTY = calculateNullNode(bpp->head->keySize, bpp->head->revision);
			fwrite(&EMPTY,1, (bpp->head->leafCount-1)*bpp->head->keySize, bpp->file); //Create entries for other nodes in leaf.
			fwrite(&value,bpp->head->valueSize, 1, bpp->file);
			fwrite(&EMPTY,1, (bpp->head->leafCount-1)*bpp->head->valueSize, bpp->file); //Create entries for other nodes in leaf.
			bpp->head->depth = 1;
			bpp->head->recordCount++;
			writeHeader(bpp->file, bpp->head);
		}
	}//TODO :: After mergeRight and mergeUp are built.
/* else {
		void* leaf;
		leaf = malloc(bpp->head->keySize * bpp->head->leafCount);
		long leaf_offset=0;
		//Approach Appropriate Bottom Row.
		for (int i = 0; i<bpp->head->depth-1; i++){
			fseek(bpp->file, bpp->indices[i]+(leaf_offset*bpp->head->keySize),SEEK_SET);
			fread(leaf,bpp->head->keySize, bpp->head->leafCount, bpp->file);
			bool found = 0;
			for (int j = 0; j<=bpp->head->leafCount;j++){
				void* element = leaf + (j * bpp->head->keySize);
				int comp = memcmp(key,element,bpp->head->keySize);
				if(comp<0 || comp==0){
					leaf_offset = (leaf_offset * bpp->head->leafCount+1)+j;
					found = 1;
					break;
				}
			}
			if (found == 0) {
				leaf_offset = (leaf_offset * bpp->head->leafCount+1)+bpp->head->leafCount+1;
			}
		}
		//Insert
		void* bucket = malloc(bpp->head->keySize);
		fseek(bpp->file, bpp->indices[bpp->head->depth-1]+(leaf_offset*bpp->head->keySize),SEEK_SET);
		fread(leaf,bpp->head->keySize, bpp->head->leafCount, bpp->file);
		
	}*/
	return 0;
}



static ion_bpp_search_record* searchRecordOffset(ion_bpp *bpp, void* searchKey){ //SUPERIOR TO OTHER SEARCH
	unsigned char* leaf;
	unsigned char* element;
	leaf = malloc(bpp->head->keySize * bpp->head->leafCount);
	bool found = 0;
	bool next_row = 0;
	unsigned long offset = 0;
	void* nullValue = calculateNullNode(bpp->head->keySize, bpp->head->revision);
	int comp = 0;
	int isNull = 0;
	for (unsigned short depth = 0; depth < bpp->head->depth;depth++){
		found = 0;
		fseek(bpp->file, bpp->indices[depth]+(offset*bpp->head->keySize),SEEK_SET);
		fread(leaf,bpp->head->keySize, bpp->head->leafCount, bpp->file);
		for (unsigned short node = 0; node < bpp->head->leafCount; node++){
			element = leaf + (node * bpp->head->keySize);
			comp = memcmp(searchKey,element,bpp->head->keySize);
			isNull = memcmp(nullValue,element,bpp->head->keySize);
			printf("cmp: %i",comp);
			if(comp<0){
				if(depth == bpp->head->depth-1) {
					offset = offset + node;
				} else {
					offset = offset * (bpp->head->leafCount+1) + node;
				}
				next_row = 1;
				break;
			}
			if(comp==0){
				printf("Found: Depth %i", depth);
				if(depth == bpp->head->depth-1) {
					offset = offset + node;
				} else {
					offset = offset * (bpp->head->leafCount+1) + node+1;
				}
				found=1;
				next_row = 1;
				break;
			}
			if(isNull == 0){
				if(depth == bpp->head->depth-1) {
					offset = offset + node;
				} else {
					offset = offset * (bpp->head->leafCount+1) + node+1;
				}
				found=0;
				next_row = 1;
				break;
			}
			//offset * (bpp->head->leafCount+1) + i
		}
		if(next_row == 0){
			printf("NEXTROW %lu\n", offset);
			//offset = (offset + (bpp->head->leafCount)) * (bpp->head->leafCount+1);
			//offset = (offset * (bpp->head->leafCount+1)) + (bpp->head->leafCount);
			if(depth != bpp->head->depth-1){
				offset = (offset + (bpp->head->leafCount+1)) * (bpp->head->leafCount);
			}
			printf("Off %lu\n",offset);
		}
		next_row = 0;
	}
	ion_bpp_search_record* rec;
	rec = (ion_bpp_search_record*) malloc(sizeof(ion_bpp_search_record));
	rec->row_offset = (unsigned long*) malloc(sizeof(long));
	(*rec->row_offset) = offset;
	rec->found = found;
	rec->has_end_offset = 0;
	if(found && bpp->head->allow_duplicates){
		while(true){
			offset++;
			fseek(bpp->file, bpp->indices[bpp->head->depth-1]+(offset*bpp->head->keySize),SEEK_SET);
			if(fread(element,bpp->head->keySize, 1, bpp->file) == 0){
				break;
			}
			if(memcmp(searchKey,element,bpp->head->keySize) == 0){
				if(rec->has_end_offset == 0){
					rec->end_offset = (unsigned long*)malloc(sizeof(long));
					rec->has_end_offset = 1;
				}
				(*rec->end_offset) = offset;
			} else {break;}
		}
	}
	free(leaf);
	free(nullValue);
	return rec;
}
static void* readRecords(ion_bpp *bpp, ion_bpp_search_record *rec, bool return_all){
	if(return_all){
		void* result = malloc(bpp->head->valueSize * ((*rec->end_offset) - (*rec->row_offset)));
		fseek(bpp->file, bpp->indices[bpp->head->depth]+((*rec->row_offset) * bpp->head->valueSize),SEEK_SET);
		fread(result,bpp->head->valueSize, (*rec->end_offset - *rec->row_offset), bpp->file);
		return result;
	} else {
		void* result = malloc(bpp->head->valueSize);
		fseek(bpp->file, bpp->indices[bpp->head->depth]+((*rec->row_offset) * bpp->head->valueSize),SEEK_SET);
		fread(result,bpp->head->valueSize, 1, bpp->file);
		return result;
	}
}

static unsigned char deleteRecords(ion_bpp *bpp, void* key){ //No support for duplicate keys at this time TODO
	ion_bpp_search_record* rec = searchRecordOffset(bpp, key);
	if (rec->found == 0) {
		return 0;
	} else {
		unsigned short depth_in_leaf = (*rec->row_offset) % bpp->head->leafCount;
		printf("\nDepth in Leaf: %u\n", depth_in_leaf);
		if(depth_in_leaf < (bpp->head->leafCount)) {
			//void* nullValue = calculateNullNode(bpp->head->keySize, bpp->head->revision);
			//void* element = malloc(bpp->head->keySize);
			//fseek(bpp->file, bpp->indices[bpp->head->depth-1]+((*rec->row_offset) * bpp->head->keySize),SEEK_SET);
			//fread(element,bpp->head->keySize, 1, bpp->file);
			for(unsigned short i = 0; i<(bpp->head->leafCount-1);i++){
				printf("HERE! %lu : %lu",*rec->row_offset + i + 1, *rec->row_offset + i);
				//fread(element,bpp->head->keySize, 1, bpp->file);
				//if(memcmp(nullValue,element_b,bpp->head->keySize)==0){
				move(bpp, bpp->head->depth-1, *rec->row_offset + i + 1, *rec->row_offset + i, i==(bpp->head->leafCount-2),false);
				//}
			}
		} else {
			move(bpp, bpp->head->depth-1, *rec->row_offset+1, *rec->row_offset, 1,0);
		}
	}
	return 1;
}


static void move(ion_bpp *bpp, unsigned int depth, long source_offset, long dest_offset, bool clear, bool whole_leaf){
	void* leaf;
	unsigned short* size;
	unsigned short nodes = 1;
	if (whole_leaf) {nodes = bpp->head->leafCount;}
	if(depth != bpp->head->depth){
		size = &(bpp->head->keySize);
		leaf = malloc(bpp->head->keySize * nodes);
	} else {
		size = &(bpp->head->valueSize);
		leaf = malloc(bpp->head->valueSize * nodes);
	}
	//printf("size: %ui\nnodes: %ui\n", *size, nodes);
	fseek(bpp->file, bpp->indices[depth]+(source_offset * (*size)),SEEK_SET);
	fread(leaf,*size, nodes, bpp->file);
	//int *temp = (int*) leaf;
	//printf("%i", *temp);
	fseek(bpp->file, bpp->indices[depth]+(dest_offset * (*size)),SEEK_SET);
	fwrite(leaf,*size, nodes, bpp->file);
	if (clear) {
		//printf("clear\n");
		leaf = calculateNullNode(*size, bpp->head->revision);
		fseek(bpp->file, bpp->indices[depth]+(source_offset * (*size)),SEEK_SET);
		for (unsigned short i = 0; i<nodes;i++){
			fwrite(leaf,*size, 1, bpp->file);
		}
	}
	if(depth != bpp->head->depth){
		if (!whole_leaf) {nodes = 0; /* Only do left children if only one node; whatever called this will have to do cleanup to shift... or, I could add it here TODO Add left_shift */}
		for (unsigned short i = 0; i<=nodes; i++){
			//printf("Recursive\n");
			if (depth != bpp->head->depth -1) {
				move(bpp, depth+1, source_offset * (bpp->head->leafCount+1) + i, dest_offset * (bpp->head->leafCount+1)+i, clear, 1);
			} else{
				move(bpp, depth+1,source_offset,dest_offset,clear,1);
			}
		}
	}
}

static unsigned short getLeafCount(ion_bpp *bpp, unsigned short elementSize, long absolute_offset){
	unsigned char* leaf;
	leaf = malloc(elementSize * bpp->head->leafCount);
	fseek(bpp->file, absolute_offset,SEEK_SET);
	fread(leaf,elementSize, bpp->head->leafCount, bpp->file);
	unsigned char* nullValue = calculateNullNode(elementSize, bpp->head->revision);
	unsigned short i = 0;
	unsigned char* element = leaf + (i * bpp->head->keySize);
	while(i<bpp->head->leafCount && memcmp(nullValue,element,elementSize) != 0){
		i++;
		element = leaf + (i * bpp->head->keySize);
	}
	return i;
}



static void debugPrintTree(ion_bpp *bpp){
	long records = bpp->head->leafCount;
	if(bpp->file == NULL){printf("Wtf? NULL?\n");}
	fseek(bpp->file, BPP_FIRST_LEAF_OFFSET, SEEK_SET);
	long data = 0;
	for(unsigned short z = 0; z<bpp->head->depth;z++){
		for(unsigned short i = 0; i<records;i++){
			fread(&data,bpp->head->keySize, 1, bpp->file);
			printf("|%lu|",data);
		}
		printf("\n");
		records *= bpp->head->leafCount + 1;
	}

}

static unsigned char* calculateNullNode(unsigned short keySize, unsigned char revision){ //TODO
	unsigned char *value = (unsigned char*) malloc(sizeof(int));
	*value = keySize + (unsigned int) revision;
	*value = 0;
	return value;
}
