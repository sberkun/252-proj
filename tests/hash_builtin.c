#include "locks_queues.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

#define NUM_THREADS 1
#define NUM_BUCKETS 8

typedef struct Node {
    int key;
    struct Node *next;
} Node;

typedef struct HashMap {
    Node *buckets[NUM_BUCKETS];
    atomic_flag locks[NUM_BUCKETS];  // Using atomic flags for locks
} HashMap;

volatile int done_flags[NUM_THREADS] = {0};

void initHashMap(HashMap *map) {
    for (int i = 0; i < NUM_BUCKETS; i++) {
        map->buckets[i] = NULL;
        atomic_flag_clear(&map->locks[i]);  // Initialize lock as unlocked
    }
}

// Just use last 3 bits as "hash"
int hash(int key) {
    return key & 0x7;
}

// Simple lock acquire using atomic flag
void lock_acquire_d(atomic_flag *lock) {
    while (atomic_flag_test_and_set(lock)) {
        // Spin wait, replace with yield or sleep as necessary
    }
}

// Simple lock release using atomic flag
void lock_release_d(atomic_flag *lock) {
    atomic_flag_clear(lock);
}

// Insert a key into the hash map
void insert(HashMap *map, int key) {
    int index = hash(key);
    lock_acquire_d(&map->locks[index]);

    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->key = key;
    newNode->next = map->buckets[index];
    map->buckets[index] = newNode;

    lock_release_d(&map->locks[index]);
}

// Get a key from the hash map
int get(HashMap *map, int key) {
    int index = hash(key);
    lock_acquire_d(&map->locks[index]);

    Node *current = map->buckets[index];
    while (current != NULL) {
        if (current->key == key) {
            lock_release_d(&map->locks[index]);
            return 1;  // Key found
        }
        current = current->next;
    }

    lock_release_d(&map->locks[index]);
    return 0;  // Key not found
}

int main() {
    HashMap map;
    initHashMap(&map);
    srand(42);
    printf("Initialized\n");

    int found = 0;
    int n_elements = 100;

    int t1 = rdcycle();

    // Inserting 100 random ints
    for (int i = 0; i < n_elements; i++) {
        int key = rand() % n_elements;
        insert(&map, key);
    }
    printf("Done inserting\n");

    // Barrier
    done_flags[mhartid()] = 1;
    for (int i = 1; i < NUM_THREADS; i++) {
        while (!done_flags[i]);
    }
    printf("Passed barrier\n");

    // Fetch 100 random ints
    for (int i = 0; i < n_elements; i++) {
        int key = rand() % n_elements;
        if (get(&map, key)) {
            found++;
        }
    }

    int t2 = rdcycle();
    
    printf("done! %d %d\n", t1, t2);
    printf("Found %d/%d keys.\n", found, n_elements);

    return 0;
}
