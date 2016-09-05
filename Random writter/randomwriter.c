#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "hashset.h"

struct frequency {
    char * string;
    char nextLetter;
    double probability;
};

static int Hash(const void * key, int numBuckets){
	int i;
	signed long kHashMultiplier = -1664117991L;
	unsigned long hashcode = 0;
	struct frequency *freq = (struct frequency *)key;
	for(i=0; i<strlen(freq->string); i++){
		hashcode = hashcode * kHashMultiplier + tolower(freq->string[i]);
	}
	return hashcode % numBuckets;
}

static int FindBucket(const void * key, int numBuckets){
	int i;
	signed long kHashMultiplier = -1664117991L;
	unsigned long hashcode = 0;
	char * s = (char *)key;
	for(i=0; i<strlen(s); i++){
		hashcode = hashcode * kHashMultiplier + tolower(s[i]);
	}
	return hashcode % numBuckets;
}

static int CompareString(const void *elem1, const void *elem2){
    struct frequency *freq1 = (struct frequency *)elem1;
    struct frequency *freq2 = (struct frequency *)elem2;
    return strcasecmp(freq1->string, freq2->string);
}

static char * getInitialState(hashset * hashtable, int numberOfBuckets){
	int i;
	vector v;
	VectorNew(&v, sizeof(int*), NULL, 2);
	for(i=0; i<numberOfBuckets; i++){
		vector * temp = (vector *)(&hashtable->elems[i]);
		if(VectorLength(temp) > 0){
			VectorAppend(&v, &i);
		}
	}
	
	int random = rand()%VectorLength(&v);
	int * bucket = (int *)VectorNth(&v, random);
	vector * t = (vector *)(&hashtable->elems[*bucket]);

	random = rand()%VectorLength(t);
	struct frequency * tempStruct = VectorNth(t, random);
	return tempStruct->string;
}

static void write(int order, char * file, int words, int numberOfBuckets){
	// Read sample text
  	FILE *fp;
  	char ch;
  	int i, j;
  	char str[order + 1];

  	hashset hashtable;
	HashSetNew(&hashtable, sizeof(struct frequency), numberOfBuckets, Hash, CompareString, NULL);
	struct frequency localFreq, * found;

  	fp = fopen(file, "r");
  	assert(fp != NULL && "Unable to open file");

  	while( (ch = fgetc(fp)) != EOF){
  		for(i=0; i < order && ch != EOF; i++){
  			str[i] = ch;
  			ch = fgetc(fp);
  		}
  		str[order] = '\0';

  		fseek(fp, -order, SEEK_CUR);

  		if(ch == EOF){
  			break;
  		}

  		// Forming struct
  		localFreq.string = strdup(str);
  		localFreq.nextLetter = ch;

  		// Looking for the key and updating the number of ocurrences
  		found = (struct frequency *) HashSetLookup(&hashtable, &localFreq);
	    if (found != NULL){
	  		localFreq.probability = found->probability + 1;

	  		int bucket = Hash(&localFreq, numberOfBuckets);
	  		vector * temp = (vector *)(&hashtable.elems[bucket]);

	  		for(j=0; j<VectorLength(temp); j++){
	  			struct frequency * t = VectorNth(temp, j);	  			
	  			if(strcasecmp(t->string, localFreq.string) == 0){
	  				t->probability++;
	  			}
	  		}
	    }
	    else{
	  		localFreq.probability = 1;
	    }
  		HashSetEnter(&hashtable, &localFreq);
  	}

  	fclose(fp);
  	

  	// Calculating the probabilities
  	for(i=0; i<numberOfBuckets; i++){
  		vector * temp = (vector *)(&hashtable.elems[i]);
  		for(j=0; j<VectorLength(temp); j++){
  			struct frequency * t = VectorNth(temp, j);
  			t->probability = 1/t->probability;
  		}
  	}


  	// Writing a piece of art
  	FILE *f = fopen("newText.txt", "w");
	assert(f != NULL && "Unable to write file");

	srand(time(NULL));
	double r = ((double) rand() / (RAND_MAX));
	int wordCount = 0;
	
	char prev[order+1];
	strncpy(prev, getInitialState(&hashtable, numberOfBuckets), sizeof(prev));
	prev[order] = '\0';

	char next;
	int flag = 0;

	while(wordCount < words){
		int bucket = FindBucket(&prev, numberOfBuckets);
		vector * temp = (vector *)(&hashtable.elems[bucket]);

		for(i=0; i<VectorLength(temp); i++){
			struct frequency * tempStruct = VectorNth(temp, i);
			if(strcasecmp(tempStruct->string, prev)==0){
				r -= tempStruct->probability;

				if(r<=0){
					next = tempStruct->nextLetter;
					fprintf(f, "%c", next);

					if(next == ' ' || next == '\n'){
						wordCount++;
						if(wordCount == words){
							flag = 1;
						}
					}
					break;
				}
			}
		}

		if(flag){
			break;
		}

		if(r>0){
			strncpy(prev, getInitialState(&hashtable, numberOfBuckets), sizeof(prev));
			prev[strlen(prev)] = '\0';
		}

		//SPECIAL CASE
		if(order > 0){
			memmove(prev, prev+1, strlen(prev));
			prev[strlen(prev)] = next;
			prev[strlen(prev)+1] = '\0';
		}

		r = ((double) rand() / (RAND_MAX));
	}

	fclose(f);
}


int main(int argc, char * argv[]){
	/*
		argv[0] - Name of the executable
		argv[1] - Order or level of text analysis
		argv[2] - Sample text file
		argv[3] - Number of words to write

		Example of usage: ./read 5 testText.txt 100

		Use the Makefile to load the previous example
	*/

	if(argc != 4){
		printf("Not the correct number of arguments!\n");
	    exit(1);
	}
	else{
		if(atoi(argv[1])>10 || atoi(argv[1])<0){
			printf("\"Order\" is not between 0 and 10 (inclusive)\n");
			exit(1);
		}
		else if(atoi(argv[3])<0){
			printf("\"Number of words\" can't be less than 0\n");
			exit(1);
		}
	}

	const int numberOfBuckets = 10000;

	write( atoi(argv[1]), argv[2], atoi(argv[3]), numberOfBuckets );

	return 0;
}