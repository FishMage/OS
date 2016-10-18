
/*ONLINE SOURCE:*/
/*http://elliottback.com/wp/hashmap-implementation-in-c

/*
 * Generic map implementation. This class is thread-safe.
 * free() must be invoked when only one thread has access to the hashmap.
 */
#include < stdlib.h >
#include < stdio.h >
#include "hashmap.h"

#define INITIAL_SIZE 1024

// We need to keep keys and values
typedef struct _hashmap_element{
	char* key;
	int in_use;
	char* data;
} hashmap_element;

// A hashmap has some maximum size and current size,
// as well as the data to hold.
typedef struct _hashmap_map{
	int table_size;
	int size;
	hashmap_element *data;
	semaphore_t lock;
} hashmap_map;

/*
 * Return an empty hashmap, or NULL on failure.
 */
map_t hashmap_new() {
	hashmap_map* m = (hashmap_map*) malloc(sizeof(hashmap_map));
	if(!m) goto err;

	m->data = (hashmap_element*) calloc(INITIAL_SIZE, sizeof(hashmap_element));
	if(!m->data) goto err;

	m->lock = (semaphore_t) semaphore_create();
	if(!m->lock) goto err;

	m->table_size = INITIAL_SIZE;
	m->size = 0;

	return m;
	err:
		if (m) 
			hashmap_free(m);
		return NULL;		
}

/*
 * Hashing function for an integer
 */
unsigned int hashmap_hash_int(hashmap_map * m, char* key){
	/* Robert Jenkins' 32 bit Mix Function */
	key += (key << 12);
	key ^= (key >> 22);
	key += (key << 4);
	key ^= (key >> 9);
	key += (key << 10);
	key ^= (key >> 2);
	key += (key << 7);
	key ^= (key >> 12);

	/* Knuth's Multiplicative Method */
	key = (key >> 3) * 2654435761;

	return key % m->table_size;
}

/*
 * Return the integer of the location in data
 * to store the point to the item, or MAP_FULL.
 */
int hashmap_hash(map_t in, char* key){
	int curr;
	int i;
	
	/* Cast the hashmap */
	hashmap_map* m = (hashmap_map *) in;

	/* If full, return immediately */
	if(m->size == m->table_size) return MAP_FULL;
	
	/* Find the best index */
	curr = hashmap_hash_int(m, key);

	/* Linear probling */
	for(i = 0; i< m->table_size; i++){
		if(m->data[curr].in_use == 0)
			return curr;

		if(strcmp(m->data[curr].key,key)==0 && m->data[curr].in_use == 1)
			return curr;
		
		curr = (curr + 1) % m->table_size;
	}

	return MAP_FULL;
}

/* 
 * Add a pointer to the hashmap with some key
 */
int hashmap_put(map_t in, char* key, char* value){
	int index;
	hashmap_map* m;

	/* Cast the hashmap */
	m = (hashmap_map *) in;

	/* Find a place to put our value */
	index = hashmap_hash(in, key);
	while(index == MAP_FULL){
		if (hashmap_rehash(in) == MAP_OMEM) {
			semaphore_V(m->lock);
			return MAP_OMEM;
		}
		index = hashmap_hash(in, key);
	}

	/* Set the data */
	m->data[index].data = value;
	m->data[index].key = key;
	m->data[index].in_use = 1;
	m->size++; 

	return MAP_OK;
}

/*
 * Get your pointer out of the hashmap with a key
 */
int hashmap_get(map_t in, char* key, char *arg){
	int curr;
	int i;
	hashmap_map* m;
	
	/* Cast the hashmap */
	m = (hashmap_map *) in;

	/* Lock for concurrency */

	/* Find data location */
	curr = hashmap_hash_int(m, key);

	/* Linear probing, if necessary */
	for(i = 0; i< m->table_size; i++){

		if(strcmp(m->data[curr].key,key)==0 && strcmp(m->data[curr].data,arg)==0){	
			return 0;
		}
		curr = (curr + 1) % m->table_size;
	}
	*arg = NULL;

	/* Not found */
	return MAP_MISSING;
}
