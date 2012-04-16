/*******************************************************
                          main.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <stdio.h>
#include <iostream>
using namespace std;

#include "cache.h"

int main(int argc, char *argv[])
{
	
	ifstream fin;
//	FILE * pFile;

	if(argv[1] == NULL){
		 printf("input format: ");
		 printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
		 exit(0);
        }

	/*****uncomment the next five lines*****/
	int cache_size = atoi(argv[1]);
	int cache_assoc= atoi(argv[2]);
	int blk_size   = atoi(argv[3]);
	int num_processors = 4;/*1, 2, 4, 8*/
	//int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:MOESI*/
	char *fname =  (char *)malloc(20);
 	fname = argv[4];

	
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
    string line;
    int processorNumber;
	uchar operation;
	ulong address;

//	pFile = fopen (fname,"r");
    	ifstream pFile (fname);
//	if(pFile == 0)
//	{   
//		printf("Trace file problem\n");
//		exit(0);
//	}
    if(pFile.is_open()){
        
		while(!pFile.eof()){
			getline(pFile, line, ' ');
			processorNumber = atoi(line.c_str());
			if(!(processorNumber >=0 && processorNumber <= 8))break;
			getline(pFile, line, ' ');
			operation = line[0];
			getline(pFile, line, '\n');
			address = strtoul(line.c_str(), NULL, 16);   
        }
    }
          
	///******************************************************************//
	//**read trace file,line by line,each(processor#,operation,address)**//
	//*****propagate each request down through memory hierarchy**********//
	//*****by calling cachesArray[processor#]->Access(...)***************//
	///******************************************************************//
	pFile.close();

	//********************************//
	//print out all caches' statistics //
	//********************************//
	
}
