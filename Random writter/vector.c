#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
	if(initialAllocation==0){
		initialAllocation=4;
	}
	assert(initialAllocation>0 && "No mas memoria para ti");
	v->elems = malloc(elemSize*initialAllocation);
	assert(v->elems != NULL && "No hay memoria");
	v->elemSize = elemSize;
	v->initialSize = initialAllocation;
	v->currentPosition = 0;
	v->freeFn = freeFn;
}

void VectorDispose(vector *v)
{
	if(v->freeFn!=NULL){
		int i;
		void * elemAddress;
		elemAddress = v->elems;
		for(i=0; i<(v->currentPosition); i++){
			elemAddress = (char *)v->elems + i * v->elemSize;
			v->freeFn(elemAddress);
		}
	}
	free(v->elems);
}

int VectorLength(const vector *v)
{
	return v->currentPosition;
}

void *VectorNth(const vector *v, int position)
{
	assert(position < v->currentPosition && "Posicion no valida");
	return (char *)v->elems + position * v->elemSize;
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
	assert(position < v->currentPosition && "Posicion no valida");
	void * elemDir;
	elemDir=(char *)v->elems + position * v->elemSize;
	memcpy(elemDir, elemAddr, v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
	assert(position <= v->currentPosition && "Posicion no valida");
	if(v->currentPosition == v->initialSize){
		v->initialSize *= 2;
		v->elems = realloc(v->elems, v->initialSize * v->elemSize);
		assert(v->elems!=NULL && "No mas memoria para ti");
	}

	void * addr1 = (char *)v->elems + position * v->elemSize;
	void * addr2 = (char *)addr1 + v->elemSize;
	int size = (v->currentPosition - position) * v->elemSize;

	memmove(addr2, addr1, size);

	memcpy(addr1, elemAddr, v->elemSize);

	v->currentPosition++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
	void * elemDir;
	if(v->currentPosition == v->initialSize){
		v->initialSize *= 2;
		v->elems = realloc(v->elems, v->initialSize * v->elemSize);
		assert(v->elems!=NULL && "No mas memoria para ti");
	}
	elemDir = (char *)v->elems + v->currentPosition * v->elemSize;
	memcpy(elemDir, elemAddr, v->elemSize);
	v->currentPosition++;
}

void VectorDelete(vector *v, int position)
{
	assert(position < v->currentPosition && "Posicion no valida");
	void * addr1 = (char *)v->elems + position * v->elemSize;
	void * addr2 = (char *)addr1 + v->elemSize;
	int size = (v->currentPosition - (position + 1)) * v->elemSize;

	memmove(addr1, addr2, size);

	v->currentPosition--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
	qsort(v->elems, v->currentPosition, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
	int i;
	void * elemAddr;
	for(i=0; i < v->currentPosition; i++){
		elemAddr = (char *)v->elems + i * v->elemSize;
		mapFn(elemAddr, auxData);
	}
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
	void *elemAddr;
  	void *base = (char *) v->elems + startIndex * v->elemSize;
  	size_t nmemb = v->currentPosition - startIndex;
  	
	if(isSorted){
    	elemAddr = bsearch(key,base,nmemb,v->elemSize,searchFn);
    }
  	else{
    	elemAddr = lfind(key,base,&nmemb,v->elemSize,searchFn);
    }
  
  	if( elemAddr != NULL ){
    	return ((char *)elemAddr - (char *)v->elems)/v->elemSize;
    }

  	return kNotFound;
} 

//Compilar
//gcc -c vector.c
//gcc -o vectortest vectortest.c vector.c
//./vectortest