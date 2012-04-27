/*******************************************************
                          main.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/
//Some of this code is reused. Original authors: Chris Barile & Chris Coffey

#include <stdlib.h>
#include <assert.h>
#include <fstream>
using namespace std;

#include "cache.h"

//make a struct for a directory entry
struct ENTRY{
	ulong block_num;
	int inUse;
	uchar blk_state;
	int bitVector[4]; //keeps track of what caches have this block cached
};

//int x = 0; //keeps track of where to insert next entry in directory

int main(int argc, char *argv[])
{
	
	FILE * pFile;

	if(argv[1] == NULL){
		 printf("input format: ");
		 printf("./smp_cache <cache_size> <assoc> <block_size> <trace_file> \n");
		 exit(0);
        }

	/*****uncomment the next five lines*****/
	int cache_size = atoi(argv[1]);
	int cache_assoc= atoi(argv[2]);
	int blk_size   = atoi(argv[3]);
	char *fname =  (char *)malloc(20);
 	fname = argv[4];
 	int num_processors = 4;
	
	//****************************************************//
	//**printf("===== Simulator configuration =====\n");**//
	//*******print out simulator configuration here*******//
	//****************************************************//
 
	//*********************************************//
        //*****create an array of caches here**********//
	//*********************************************//	
    Cache *cachesArray[num_processors];
    
    for(int k = 0; k <num_processors;k++){
        cachesArray[k] = new Cache(cache_size,cache_assoc, blk_size);
    }
    

	pFile = fopen (fname,"r");
	if(pFile == 0)
	{   
		printf("Trace file problem\n");
		exit(0);
	}
    
          
	///******************************************************************//
	//**read trace file,line by line,each(processor#,operation,address)**//
	//*****propagate each request down through memory hierarchy**********//
	//*****by calling cachesArray[processor#]->Access(...)***************//
	///******************************************************************//
	char currLine [20];
	int proc_num;	
	int shared = 0;
	int cached = 0;
	uchar op;
	ulong address;
	int  x = 0; //keeps track of where to insert next entry in directory
	int * xp;
	xp = &x;

	int totEntries = (cache_size / blk_size) * num_processors;
	//int totEntries = 2;

	ENTRY dir[totEntries]; //make the directory

	//initialize all parts of entries in the directory to initial values
	for(int i = 0; i < totEntries; i++){
		dir[i].block_num = 0;
		dir[i].inUse = 0;
		dir[i].blk_state = 'U';
		for(int q = 0; q < num_processors; q++){
			dir[i].bitVector[q] = 0;
		}
	}
	//printf("%i %c %i\n", (int)dir[511].block_num, dir[511].blk_state, dir[511].bitVector[3]);
	//printf("%i\n", totEntries);
	//printf("%i\n", *xp);

	int line = 0;
	while(!feof(pFile)) {
		shared = 0;
		cached = 0;
		if(fgets(currLine, 20, pFile) == NULL)
			break;

		sscanf(currLine, "%i %c %lx", &proc_num, &op, &address);
		line++;
		//printf("Line %i: %i %c %lx\n", line, proc_num, op, address);

		if(cachesArray[proc_num]->findLine(address) != NULL) //check to see if the requesting proc already has the block in its cache - if so, directory doesn't need to do anything if it's a read hit
			cached = 1;
		//printf("Cached check\n");
		ulong tag = ( address >> (ulong)log2(blk_size) );
		//printf("Tagged\n");
		//printf("x = %i\n", x);
		for(int i = 0; i < x; i++){
			//printf("Entered for\n");
			if(dir[i].inUse == 1){
				if(dir[i].block_num == tag){
				//printf("Entered if\n");
					for(int q = 0; q < num_processors; q++){
						//printf("Entered second for. q = %i\n", q);
						if(cachesArray[q]->findLine(address) == NULL){
							//printf("Entered second if\n");
							dir[i].bitVector[q] = 0;
						}
					}
					if(dir[i].bitVector[0] == 0 && dir[i].bitVector[1] == 0 && dir[i].bitVector[2] == 0 && dir[i].bitVector[3] == 0){
						//printf("Entered third if\n");
						dir[i].inUse = 0;
					}
					//break;
				}
			}
			
		}
		//printf("After first for\n");

		int tempLine = -1;
		for(int i = 0; i < x; i++){//look for block in directory
		tempLine = -1;

				if(dir[i].inUse == 1){
	//		printf("Inside first if (122)\n");
					if(dir[i].block_num == tag){ //found in directory
	//		printf("Inside second if (124)\n");
						tempLine = i;
						if(dir[i].blk_state == 'U'){
	//		printf("Inside third if (127)\n");
							if(op == 'w'){ //write
	//  	 	printf("Inside fourth if (129)\n");
								dir[i].blk_state = 'E';
								dir[i].bitVector[proc_num] = 1;
							}
							else if(cached == 0){ //read miss
	//		printf("Inside fifth if (134)\n");
								for(int q = 0; q < num_processors; q++){
									if( (q != proc_num) && (dir[i].bitVector[q] == 1) ){ //block is shared
	//									printf("Inside sixth if (137)\n");
										shared = 1;
										for(int q = 0; q < num_processors; q++){
											if( (q != proc_num) && (dir[i].bitVector[q] == 1) ){
	//											printf("Inside twelfth if (168)\n");
												cachesArray[q]->snoopRequest(address, 'R');
											}
										}
									}
								}
								if(shared == 1){
	//								printf("Inside seventh if (143)\n");
									dir[i].blk_state = 'S';
									dir[i].bitVector[proc_num] = 1;
								}
								else{
	//								printf("Inside eighth if (148)\n");
									dir[i].blk_state = 'E';
									dir[i].bitVector[proc_num] = 1;
								}							
							}
						}
						else if(dir[i].blk_state == 'S'){
	//						printf("Inside ninth if (155)\n");
							if(op == 'r' && cached == 0){ //read miss
	//							printf("Inside tenth if (157)\n");
								dir[i].bitVector[proc_num] = 1;
								shared = 1;
								for(int q = 0; q < num_processors; q++){
									if( (q != proc_num) && (dir[i].bitVector[q] == 1) ){
	//									printf("Inside twelfth if (168)\n");
										cachesArray[q]->snoopRequest(address, 'R');
									}
								}
							}
							else if(op == 'w'){ //write
	//							printf("Inside eleventh if (162)\n");
								dir[i].blk_state = 'E';
								dir[i].bitVector[proc_num] = 1;
								shared = 1;
								for(int q = 0; q < num_processors; q++){
									if( (q != proc_num) && (dir[i].bitVector[q] == 1) ){
	//									printf("Inside twelfth if (168)\n");
										cachesArray[q]->snoopRequest(address, 'W');
									}
								}
							}
						}
						else if(dir[i].blk_state == 'E'){
	//						printf("Inside thirteenth if (175)\n");
							if(op == 'r' && cached == 0){ //read miss
	//							printf("Inside fourteenth if (177)\n");
								dir[i].blk_state = 'S';
								dir[i].bitVector[proc_num] = 1;
								shared = 1;
								for(int q = 0; q < num_processors; q++){
									if( (q != proc_num) && (dir[i].bitVector[q] == 1) ){
	//									printf("Inside twelfth if (168)\n");
										cachesArray[q]->snoopRequest(address, 'R');
									}
								}
								//printf("Ctc\n");
								cachesArray[proc_num]->cacheTrans();
								//maybe ctcTransfers only happen here? B/c of Interrupt?
							}
							else if(op == 'w' && cached == 0){ //write
	//							printf("Inside fifteenth if (183)\n");
								for(int q = 0; q < num_processors; q++){
									if( (q != proc_num) && (dir[i].bitVector[q] == 1) ){
	//									printf("Inside sixteenth if (186)\n");
										cachesArray[q]->snoopRequest(address, 'W');
									}
								}
								dir[i].bitVector[proc_num] = 1;
							}
						}
						break;
					}
				}
			//printf("i = %i\n", i);
		}

		//block is not in directory at all - a read or write op will have same results
	//	printf("%i\n", *xp);
		if(tempLine == -1 && *xp < totEntries){
	//		printf("Inside last if: x = %i\n", x);
			shared = 0;
			dir[x].block_num = tag;
			dir[x].inUse = 1;
			dir[x].blk_state = 'E';
			dir[x].bitVector[proc_num] = 1;
			*xp = *xp + 1;
			//printf("%i\n", *xp);
		}


		uchar busOps = cachesArray[proc_num]->Access(address, op, shared);
		busOps = 'C';
	}
	//printf("Outside of while loop: *xp = %i\n", *xp);
	fclose(pFile);

	//********************************//
	//print out all caches' statistics //
	//********************************//
	printf("===== 506 DSM (MESI with Full-bit Vector Simulator Configuration =====\n");
	printf("L1_SIZE:               %d\n", cache_size);
	printf("L1_ASSOC:              %d\n", cache_assoc);
	printf("L1_BLOCKSIZE:          %d\n", blk_size);
	printf("NUMBER OF PROCESSORS:  %d\n", num_processors);	
	printf("TRACE FILE:            %s\n", fname);
	for(int i = 0; i < num_processors; i++){
	cachesArray[i]->printStats(i);
	}
}
