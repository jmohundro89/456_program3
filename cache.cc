/*******************************************************
                          cache.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/
//Some of this code is reused. Original authors: Chris Barile & Chris Coffey

#include <stdlib.h>
#include <assert.h>
#include "cache.h"
using namespace std;

Cache::Cache(int s,int a,int b )
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;
   ctcTransfers = 0;

   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  
   //*******************//
   //initialize your counters here//
   //*******************//
 
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
		tagMask <<= 1;
        tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++) 
      {
	   cache[i][j].invalidate();
      }
   }      
   
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
uchar Cache::Access(ulong addr,uchar op, int shared)
{
	currentCycle++;/*per cache global counter to maintain LRU order 
			among cache ways, updated on every cache access*/
        	
	if(op == 'w') writes++;
	else          reads++;
	
	cacheLine * line = findLine(addr);
	if((line == NULL) || (line.getFlags(INVALID)))/*miss*/
	{
      cacheLine *newline = fillLine(addr);
		if(op == 'w')
      {
         writeMisses++;
         newLine->setFlags(MODIFIED);
         if(shared == 1)
            ctcTransfers++;

         return 'W';
      } 
		else   //op = 'r'
      {
         readMisses++;
         if(shared == 1)
         {
            newLine->setFlags(SHARED);
            ctcTransfers++;
         }
         else
         {
            newLine->setFlags(EXCLUSIVE)
         }

         return 'R';
      }   
		
	}
	else //hit
	{
		/**since it's a hit, update LRU and update dirty flag**/
		updateLRU(line);

      if(op == 'r')
      {
         return '-';
      }

      //Op = 'w' from here on
      ulong flag = line->getFlags();

      if(flag == EXCLUSIVE)
         line->setFlags(MODIFIED);
      else if(flag == SHARED)
      {
         line->setFlags(MODIFIED)
         return 'U';
      }
	}
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache[i][j].isValid())
	        if(cache[i][j].getTag() == tag)
		{
		     pos = j; break; 
		}
   if(pos == assoc)
	return NULL;
   else
	return &(cache[i][pos]); 
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) return &(cache[i][j]);     
   }   
   for(j=0;j<assoc;j++)
   {
	 if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
   } 
   assert(victim != assoc);
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   if(victim->getFlags() == MODIFIED) writeBack(addr);
    	
   tag = calcTag(addr);   
   victim->setTag(tag);
   //victim->setFlags(VALID);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

void Cache::printStats()
{ 
	printf("===== Simulation results      =====\n");
	/****print out the rest of statistics here.****/
	/****follow the ouput file format**************/
}

void Cache::snoopRequest(ulong address, uchar busOp, int protocol){
   cacheLine *line = findLine(address);
   
      if (line != NULL)
      {
         ulong flag = line->getFlags();
         if(busOp == 'R')
         {
            if(flag == EXCLUSIVE)
               line->setFlags(SHARED)
            else if(flag == MODIFIED)
            {
               line->setFlags(SHARED)
               writeBacks++;
            }
         }
         else if(busOp == 'W')
         {
            if(flag == EXCLUSIVE)
               line->setFlags(INVALID)
            else if(flag == MODIFIED)
            {
               line->setFlags(INVALID)
               writeBacks++;
            }
            else if(flag == SHARED)
               line->setFlags(INVALID)
         }
         else if(busOp == 'U')
         {
            
         }
      }