#include "bpp_tree.h"

unsigned char EMPTY = 0;
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
		for(int i = 1; i<head->depth+1; i++){
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
	}
	newTree->indices = getRowIndices(newTree->head);
	return newTree;
}
static void debugPrintHeader(ion_bpp_header *head){
	if(head==NULL){printf("Header is NULL\n");} else{
	printf("Revision Version:\t%i\nKey Size (bytes):\t%i\nValue Size (bytes):\t%i\nLeaf Count:\t\t%i\nRecord Count:\t\t%lu\nTree Depth:\t\t%lu\n",head->revision,head->keySize,head->valueSize,head->leafCount,head->recordCount,head->depth);
	}
}
static void* searchRecords(ion_bpp *bpp, void* searchKey){ //TODO CLEANUP, FIXES
	int* tmp = (int*) searchKey;
	printf("Searching for %i\n\n",*tmp);
	void* leaf;
	leaf = malloc(bpp->head->keySize * bpp->head->leafCount);
	long leaf_offset=0;
	bool found = 0;
	bool loc = 0;
	//Approach Appropriate Bottom Row.
	for (int i = 0; i<bpp->head->depth; i++){
		printf("[I]%i [index] %lu [offset] %lu\n", i,bpp->indices[i],leaf_offset);
		fseek(bpp->file, bpp->indices[i]+(leaf_offset*bpp->head->keySize),SEEK_SET);
		fread(leaf,bpp->head->keySize, bpp->head->leafCount, bpp->file);
		found = 0;
		for (int j = 0; j<bpp->head->leafCount;j++){
			void* element = leaf + (j * bpp->head->keySize);
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
				if(memcmp(calculateNullNode(0,0),element,bpp->head->keySize) == 0){
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
	//Special case, EMPTY tree
	if(bpp->head->depth==0){
		printf("Begin Special case Insertion!\n");
		if(bpp->file == NULL){printf("Wtf? NULL?\n");}
		fseek(bpp->file, BPP_FIRST_LEAF_OFFSET, SEEK_SET);
		if(fwrite(&key, bpp->head->keySize, 1, bpp->file) == 0) {
			printf("ERR: Insert First Leaf Failed!\n");
		} else {
			fwrite(&EMPTY,1, (bpp->head->leafCount-1)*bpp->head->keySize, bpp->file); //Create entries for other nodes in leaf.
			fwrite(&value,bpp->head->valueSize, 1, bpp->file);
			fwrite(&EMPTY,1, (bpp->head->leafCount-1)*bpp->head->valueSize, bpp->file); //Create entries for other nodes in leaf.
			bpp->head->depth = 1;
			bpp->head->recordCount++;
			writeHeader(bpp->file, bpp->head);
		}
	}//TODO :: After mergeRight and mergeUp are built./* else {
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




static void debugPrintTree(ion_bpp *bpp){
	long records = bpp->head->leafCount;
	if(bpp->file == NULL){printf("Wtf? NULL?\n");}
	fseek(bpp->file, BPP_FIRST_LEAF_OFFSET, SEEK_SET);
	long data = 0;
	for(int z = 0; z<bpp->head->depth;z++){
		for(int i = 0; i<records;i++){
			fread(&data,bpp->head->keySize, 1, bpp->file);
			printf("|%lu|",data);
		}
		printf("\n");
		records *= bpp->head->leafCount + 1;
	}

}

static void* calculateNullNode(unsigned short keySize, unsigned char revision){ //TODO
	int *value = (int*) malloc(sizeof(int));
	*value = 0;
	return value;
}
