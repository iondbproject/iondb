#include <stdio.h>
#include "bpp_tree.c"
int main( int argc, const char* argv[] ){
	ion_bpp_header *head = NULL;
	/*
	if(argc > 1){
		head = (ion_bpp_header *) malloc(sizeof(ion_bpp_header));
		head->revision = 0;
		head->keySize = 1;
		head->valueSize = 1;
		head->leafCount = 3;
		head->recordCount = 0;
		head->depth = 0;
	}
	ion_bpp *tree = openBpp("test_bpp.bin", head);
	if(tree!=NULL){
		debugPrintHeader(tree->head);
		long k = 9;
		long v = 5;
		long *key = &k;
		long *value = &v;
		insertRecords(tree,key,value);
		debugPrintHeader(tree->head);
	} else {
		printf("B+ TREE NOT INITIALIZED!\n");
	}*/
	ion_bpp *tree = openBpp("404_slides_pg79.bin", head);
	
		debugPrintHeader(tree->head);
		printf("------------------------------------------------\n\n");
		debugPrintTree(tree);
		printf("------------------------------------------------\n\n");
		int v = 99;
		int *value = &v;
		value = searchRecords(tree, value);
		if(value == NULL) {printf("Record [99] NOT FOUND");} else{printf("%i",*value);}
		printf("------------------------------------------------\n\n");

		v = 95;
		value = &v;
		value = searchRecords(tree, value);
		if(value == NULL) {printf("Record [95] NOT FOUND");} else{printf("%i",*value);}
		printf("------------------------------------------------\n\n");

		v=4;
		value = &v;
		value = searchRecords(tree, value);
		if(value == NULL) {printf("Record [4] NOT FOUND");} else{printf("%i",*value);}
		printf("------------------------------------------------\n\n");

	
	



/*
	char* INPUT;
	INPUT = (char *)malloc(sizeof(char)*255);
	ion_bpp *tree;
	while(true){
		printf("Input> ");
		scanf("%s", INPUT);
		if((&INPUT[0])=="u" && INPUT[1] == "s" && INPUT[2] == "e" && INPUT[3]== " "){
			int end;
			for (end = 3; INPUT[end] != NULL && INPUT[end] != " "; end++);
			char * Filename;
			Filename = (char *) malloc(sizeof(char) * (end - 3));
			for (int i = 4; i<end; i++){
				Filename[i-4] = INPUT[i];
			}
			//TODO :: Initialize Head.
			if(tree != NULL){
				free(tree->head);
				free(tree); //TODO
			}
			tree = openBpp(Filename, NULL);
			if(tree != NULL) {printf("Successfully Opened BPP '%s'", Filename);} else {printf("Failed opening file: %s",Filename);}
		}
	}
*/
}

