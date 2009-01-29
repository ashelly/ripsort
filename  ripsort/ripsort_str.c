/* Ripsort_str.c

   Radixsort In Place
   A C implementation of inplace MSB radix sort.
   sorts N items with sort keys of length k (bytes).

   worst case performance: O(N*2k) 
   with uniform data, I believe it approaches O(N*2logk)

   Allocates no additional memory on the heap.
   worst case additional memory usage  ~2KB * k'
   (k' is the length of the longest initial substring common to more than 256 keys.)
*/

/*
* Copyright (c) 2009, Adam Shelly 
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice, 
*    this list of conditions and the following disclaimer.
*  - Redistributions in binary form must reproduce the above copyright notice, 
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
* THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/* A Shell sort for small subsets
//!!TODO: test if insertion sort is faster here
*/
static const int gGAPS[] = { 0, 1, 4, 10, 23, 57, 132};
void fastsort_str(char** data, size_t nitems, int base)
{
  int i, j;
  char* temp;
  int *gap = &gGAPS[6];
  while (*gap > 0)
  {
    for (i = *gap; i < nitems; i++) 
    {
      j = i;
      temp = data[i];
      while ((j >= *gap) 
        && strcmp(data[j-*gap]+base,temp+base)>0) 
      {
        data[j] = data[j - *gap];
        j = j - *gap;
      }
      data[j] = temp;
    }
    gap--;
  }
}

#define radix_char(s,b)  ((s)[(b)])
#define CHAR_BUCKETS 256  //max char


void ripsort_str_imp(char** data, size_t nitems, int base)
{
	int rad;
	int i, pt,marker;
	int dest;
	char* flier;
	char* temp;
	size_t pivot[CHAR_BUCKETS];
	size_t count[CHAR_BUCKETS]={0};

	while (1)
	{
		int nbuckets =0;
		pt=0;
		//advance past all the terminated strings at the front of the buffer
		//(this is mostly special case handling for uniform length keys)
		while (radix_char(data[pt],base)==0 && ++pt<nitems)
		{ ;}  
		if (pt==nitems) 
		{ return; 
		}
		//simpler sort for small lists, 
		if (nitems<CHAR_BUCKETS)
		{   
			fastsort_str(data+pt, nitems-pt, base);
			return;
		}
		//now sweep backwards, counting how many of each character.
        for (i=pt=nitems;--i>=0;) 
        {   
			rad = radix_char(data[i],base);
			if (rad>0 && !count[rad]) {nbuckets++;} //count non-empty buckets
			count[rad]++;
			if (rad<pt) {pt=rad;}  //start pt
		}
        //if there is only one non-zero bucket, skip a bunch of work and the recursion.
        if (nbuckets == 1 && pt>0)  
        {   
          	count[pt]=0;  //clear the counter
            base++; //try again at the deeper level;
        }
        else break;
    }   
    
    //set up the bin boundaries ( pivot[x] is the first available place 
    // to put strings with radix 'x')
    pivot[0]=0;
    for (i=1;i<CHAR_BUCKETS;i++)
    {   
        pivot[i]=pivot[i-1]+count[i-1];
    }
    
    //NORMALIZE:  Invariant:  data[pivot][i][base]!=i  
    //  (the char at each pivot does not belong before that pivot)
    for (i=0;i<CHAR_BUCKETS;i++)
    { 
        while ( count[i] && radix_char(data[pivot[i]],base)==i) 
        { 
            pivot[i]++; 
            count[i]--;
        }
    }
    i=pt;  //start with the first known non-full bucket
           //(we may have just filled it in the NORMALIZE step.
    while (1)
    {   //advance to first truly non-full bucket
        for (;count[i]==0&&i<CHAR_BUCKETS;i++)
        {
            if (i>0)
            {
                nitems = pivot[i]-pivot[i-1];
                if (nitems > 2)
                {   
                    ripsort_str_imp(data+pivot[i-1],nitems,base+1);
                }
                else if (nitems ==2 
                     && (strcmp(data[pivot[i-1]]+base, data[pivot[i-1]+1]+base)>0))
                {	
                    temp=data[pivot[i-1]];
                    data[pivot[i-1]]=data[pivot[i-1]+1];
                    data[pivot[i-1]+1]=temp;
                }
            }
        }
        if (i==CHAR_BUCKETS) 
        { break;}  //they're all fulll - done.

        //mark the first key which is out of place
        pt = marker = pivot[i];
        //take the entry out of that slot (the 'flier')
        flier = data[pt];
        do
        {   //find out where it goes
            rad = radix_char(flier,base);
            dest = pivot[rad];
            //place it, get a new flier.
            temp = data[dest];
            data[dest]=flier;
            flier = temp;
            do //NORMALIZE  (the one _after_ the one we just placed may be correct)
            {   
                pivot[rad]++; 
                count[rad]--;
            }
            while (count[rad] && radix_char(data[pivot[rad]],base)==rad) ;
            //keep going until the flier goes in the bucket we are working on
        } while (dest!=marker);
    }
}



void ripsort_str(char** data, size_t nitems)
{
    ripsort_str_imp(data, nitems, 0);
}

