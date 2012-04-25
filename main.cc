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

	//make the directory
	//int totEntries = (cache_size / blk_size) * num_processors;
	int totEntries = 2;
	ulong * block_num = new ulong[totEntries];
	for(int i = 0; i < totEntries; i++){
		block_num[i] = 0;
	}
	//ulong block_num[totEntries];
	//memset(block_num, 0, sizeof(block_num));
	int * inUse = new int[totEntries];
	for(int i = 0; i < totEntries; i ++){
		inUse[i] = 0;
	}
	//int inUse[totEntries];
	//memset(inUse, 0, sizeof(inUse));
	uchar * blk_state = new uchar[totEntries];
	for(int i = 0; i < totEntries; i++){
		blk_state[i] = 'U';
	}
	//uchar blk_state[totEntries]; // 'E' = EM state
	//memset(blk_state, 'U', sizeof(blk_state));
	int ** bitVector = new int*[totEntries];
	for(int i = 0; i < totEntries; i++){
		bitVector[i] = new int[4];
	}
	for(int i = 0; i < totEntries; i++){
		for(int j = 0; j < 4; j++){
			bitVector[i][j] = 0;
		}
	}
	
	//int bitVector[totEntries][num_processors]; //1 = in this cache, 0 = not in this cache
	//memset(bitVector, 0, sizeof(bitVector));
	int x = 0; //keeps track of where to insert next entry in directory
	x = 0;

	while(!feof(pFile)) {
		shared = 0;
		cached = 0;
		if(fgets(currLine, 20, pFile) == NULL)
			break;

		sscanf(currLine, "%i %c %lx", &proc_num, &op, &address);

		/*for (int i = 0; i < num_processors; i++) {
			if (i != proc_num) {
				if(cachesArray[i]->findLine(address) != NULL){
					shared = 1;
					break;
				}
			}
		}*/

		if(cachesArray[proc_num]->findLine(address) != NULL) //check to see if the requesting proc already has the block in its cache - if so, directory doesn't need to do anything if it's a read hit
			cached = 1;

		ulong tag = ( address >> (ulong)log2(blk_size) );
		int tempLine = -1;
		for(int i = 0; i < totEntries; i++){//look for block in directory

			if(inUse[i] == 1){
				if(block_num[i] == tag){ //found in directory
					tempLine = i;
					if(blk_state[i] == 'U'){
						if(op == 'w'){ //write
							blk_state[i] = 'E';
							//bitVector[i][proc_num] = 1;
						}
						else if(cached == 0){ //read
							for(int q = 0; q < num_processors; q++){
								//if( (q != proc_num) && (bitVector[i][q] == 1) ){ //block is shared
									shared = 1;
									break;
								}
							if(shared == 1){
								blk_state[i] = 'S';
								//bitVector[i][proc_num] = 1;
							}
							else{
								blk_state[i] = 'E';
								//bitVector[i][proc_num] = 1;
							}							
						}
					}
					else if(blk_state[i] == 'S'){
						if(op == 'r' && cached == 0){ //read
							//bitVector[i][proc_num] = 1;
							shared = 1;
						}
						else{ //write
							blk_state[i] = 'E';
							//bitVector[i][proc_num] = 1;
							shared = 1;
							for(int q = 0; q < num_processors; q++){
								//if( (q != proc_num) && (bitVector[i][q] == 1) ){
									cachesArray[i]->snoopRequest(address, 'W');
								}
							}
						}
					else if(blk_state[i] == 'E'){
						if(op == 'r' && cached == 0){ //read
							blk_state[i] = 'S';
							//bitVector[i][proc_num] = 1;
							//maybe ctcTransfers only happen here? B/c of Interrupt?
						}
						else if(cached == 0){ //write
							for(int q = 0; q < num_processors; q++){
								//if( (q != proc_num) && (bitVector[i][q] == 1) ){
									cachesArray[i]->snoopRequest(address, 'W');
								}
							}
							//bitVector[i][proc_num] = 1;
						}
					}
					break;
				}
			}

		//block is not in directory at all - a read or write op will have same results
		if(tempLine == -1){
			shared = 0;
			block_num[x] = tag;
			inUse[x] = 1;
			blk_state[x] = 'E';
			//bitVector[x][proc_num] = 1;
			x++;
		}


		uchar busOps = cachesArray[proc_num]->Access(address, op, shared);
		busOps = 'C';

		/*for (int i = 0; i < num_processors; i++) {
			if (i != proc_num) {
				cachesArray[i]->snoopRequest(address, busOps);
			}
		}*/
	}
	fclose(pFile);

	//********************************//
	//print out all caches' statistics //
	//********************************//
	printf("===== 506 SMP Simulator Configuration =====\n");
	printf("L1_SIZE:               %d\n", cache_size);
	printf("L1_ASSOC:              %d\n", cache_assoc);
	printf("L1_BLOCKSIZE:          %d\n", blk_size);
	printf("NUMBER OF PROCESSORS:  %d\n", num_processors);	
	printf("TRACE FILE:            %s\n", fname);
	for(int i = 0; i < num_processors; i++){
	cachesArray[i]->printStats(i);
	}
}
