#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>

#define TABLESIZE 1267709
#define MAX_WORD_SIZE 100

typedef struct hash {
	char* key;
	unsigned int count;
}hash;
typedef struct {
	hash** hashes;
}hashtable;
typedef struct {
	hash **nodes;
	unsigned int size;
	unsigned int count;
}minHeap;

//function to create hashtable
hashtable *create_hashtable() {
	hashtable* ht = malloc(sizeof(hashtable));
	ht->hashes = malloc(sizeof(hash*) * TABLESIZE);
	for (int i = 0; i < TABLESIZE; ++i) {
		ht->hashes[i] = NULL;
	}
	return ht;
}

//function to initialize min heap
minHeap* create_minHeap(unsigned int heapSize) {
	minHeap* mH = malloc(sizeof(minHeap));
	mH->nodes = malloc(sizeof(hash*) * heapSize);
	mH->size = heapSize;
	mH->count = 0;
	return mH;
}

//helper function to create key/count pair into the hashtable
hash *ht_helper(char *key, unsigned int count) {
	hash* h = malloc(sizeof(hash) * 1);
	h->key = malloc(strlen(key) + 1);

	strcpy(h->key, key);
	h->count = count;
	return h;
}

/*
hash function for the string,
I modified this basic hash function to function properly in a larger hash table
https://cseweb.ucsd.edu/~kube/cls/100/Lectures/lec16/lec16-13.html
1409 is just some random prime number I picked
*/
unsigned int hashCode(char* key) {
	unsigned int keylength = strlen(key);

	unsigned int value = 0;
	for (int i = 0; i < keylength; ++i) {
		value += key[i] * (i + 1) + key[i] * 1409;
	}
	return value % TABLESIZE;
}

/*
swaps 2 nodes in the heap
*/
void swapHeapNodes(hash *a, hash *b) {
	hash temp = *a;
	*a = *b;
	*b = temp;
}

/*
min heap algorithm, basically copied from https://en.wikipedia.org/wiki/Binary_heap#Extract, except I changed it from max to min
*/
void minHeapify(minHeap *mH, int i) {
	int left = 2 * i + 1;
	int right = 2 * i + 2;
	int smallest = i;
	if (left < mH->count && mH->nodes[left]->count < mH->nodes[smallest]->count) {
		smallest = left;
	}
	if (right < mH->count && mH->nodes[right]->count < mH->nodes[smallest]->count) {
		smallest = right;
	}
	if (smallest != i){
		swapHeapNodes(mH->nodes[smallest], mH->nodes[i]);
		minHeapify(mH, smallest);
	}
}

//build the heap, from https://en.wikipedia.org/wiki/Binary_heap#Building_a_heap pseudocode
void build_minheap(minHeap *mh) {
	int n = mh->count - 1;
	for (int i = (n - 1) / 2; i >= 0; --i) {
		minHeapify(mh, i);
	}
}

//insert word to the minheap
void insert_to_minheap(minHeap *mh, hash *word) {
	if (mh->count < mh->size) {
		mh->nodes[mh->count] = word;
		mh->count += 1;
		build_minheap(mh);
	}
	else if(word->count > mh->nodes[0]->count){
		mh->nodes[0] = word;
		minHeapify(mh, 0);
	}
}

//if 2 words have the same hash, this function will handle the collision with linear probing
void handle_collision(hashtable *ht, char *key, unsigned int slot){
	unsigned int new_slot;
	for(int i = 1; i < TABLESIZE - 1; ++i){
		new_slot = (slot + i) % TABLESIZE;
		if (ht->hashes[new_slot] == NULL){
			ht->hashes[new_slot] = ht_helper(key, 1);
			break;
		}else if(strcmp(key, ht->hashes[new_slot]->key) == 0){
			ht->hashes[new_slot]->count += 1;
			break;
		}
	}
}

//check if the word is already in the hashtable, add count if it is, else add the word
void check_word(hashtable *ht, char *key){
	unsigned int slot = hashCode(key);
	hash* h = ht->hashes[slot];
	if (h == NULL) {
		ht->hashes[slot] = ht_helper(key, 1);
	}
	else {
		if (strcmp(key, ht->hashes[slot]->key) != 0){
			handle_collision(ht, key, slot);
	    }else{
			ht->hashes[slot]->count += 1;
		}
	}
}

//insert word to minheap, heapify and finally print the most common words
void display_word_count(hashtable* ht, minHeap* mh) {
	for (int i = 0; i < TABLESIZE; ++i) {
		hash* h = ht->hashes[i];
		if (h != NULL) {
			insert_to_minheap(mh, h);
		}
	}
	int count = mh->count;
	for (int i = count - 1; i >= 0; --i) {
		swapHeapNodes(mh->nodes[0], mh->nodes[i]);
		--mh->count;
		minHeapify(mh, 0);
	}
	for (int i = 0; i < count; ++i) {
		hash* w = mh->nodes[i];
		printf("%i, %s\n", w->count, w->key);
	}
}

//filename inputed as command line argument
int main(int argc, char *argv[]) {
	hashtable* ht = create_hashtable();
	minHeap* mh = create_minHeap(100);
	FILE* fp = fopen(argv[1], "r");
	if (fp == NULL)
		printf("File doesn't exist ");
	else {
		char buffer[MAX_WORD_SIZE];
		while (fscanf(fp, "%99[A-Za-z]%*[^A-Za-z]", buffer) == 1) {
			for (int i = 0; buffer[i]; i++) {
				buffer[i] = tolower(buffer[i]);
			}
			check_word(ht, buffer);
		}
	}
	display_word_count(ht, mh);

	return 0;
}