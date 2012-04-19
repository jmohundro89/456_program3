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
		 printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
		 exit(0);
        }

	/*****uncomment the next five lines*****/
	int cache_size = atoi(argv[1]);
	int cache_assoc= atoi(argv[2]);
	int blk_size   = atoi(argv[3]);
	int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
	//int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:MOESI*/
	char *fname =  (char *)malloc(20);
 	fname = argv[5];

	
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
	uchar op;
	ulong address;
	
	while(!feof(pFile)) {
		shared = 0;
		if(fgets(currLine, 20, pFile) == NULL)
			break;

		sscanf(currLine, "%i %c %lx", &proc_num, &op, &address);
		for(int i = 0; i < num_processors; i++){
			if(i != proc_num){
				if(cachesArray[i]->findLine(address) != NULL){
					shared = 1;
					break;
				}
			}
		}

		uchar busOps = cachesArray[proc_num]->Access(address, op, shared);

		for (int i = 0; i < num_processors; i++) {
			if (i != proc_num) {
				cachesArray[i]->snoopRequest(address, busOps);
			}
		}
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
