#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "vector.c"

void HashSetNew(hashset *h, int elemSize, int numBuckets, HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert(elemSize > 0 && "Tamaño de elemento menor que uno");
	assert(numBuckets > 0 && "Número de buckets menor que uno");
	assert(hashfn != NULL && "Función hash vacía");
	assert(comparefn != NULL && "Función de comparación vacía");

	int i;
	h->elems = malloc(numBuckets * sizeof(vector));

	assert(h->elems != NULL && "No hay memoria");

	for(i=0; i<numBuckets; i++){
		VectorNew(h->elems + i, elemSize, NULL, 2);
	}

	h->elemSize = elemSize;
	h->numBuckets = numBuckets;
	h->hashFn = hashfn;
	h->compareFn = comparefn;
	h->freeFn = freefn;
}

void HashSetDispose(hashset *h)
{
	if(h->freeFn != NULL){
		int i;
		for(i=0; i< h->numBuckets; i++){
			h->freeFn(h->elems + i);
		}
	}
	free(h->elems);
}

int HashSetCount(const hashset *h)
{
	int i;
	int cont=0;
	for(i=0; i< h->numBuckets; i++){
		cont+=VectorLength(h->elems + i);
	}
	return cont;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert(mapfn!=NULL && "Mapfn vacío");
	int i;
	for(i=0; i < h->numBuckets; i++){
		VectorMap(h->elems + i, mapfn, auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	assert(elemAddr != NULL && "Elemento vacío");

	int bucket = h->hashFn(elemAddr, h->numBuckets);

	assert(bucket >= 0 && "Posicion no valida");
	assert(bucket < h->numBuckets && "Posicion no valida");
	
	VectorAppend(h->elems + bucket, elemAddr);
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
	assert(elemAddr != NULL && "Elemento vacío");

	int bucket = h->hashFn(elemAddr, h->numBuckets);

	assert(bucket >= 0 && "Posicion no valida");
	assert(bucket < h->numBuckets && "Posicion no valida");

	int pos = VectorSearch(h->elems + bucket, elemAddr, h->compareFn, 0, false);
	
	if(pos==-1){
		return NULL;
	}

	return VectorNth(h->elems + bucket, pos);
}