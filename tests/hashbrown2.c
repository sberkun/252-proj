#include "locks_queues.h"

#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4
#define NUM_BUCKETS 8

typedef struct Node {
    int key;
    struct Node *next;
} Node;

typedef struct HashMap {
    Node *buckets[NUM_BUCKETS];
} HashMap;

volatile int start_flag = 0;
volatile int done_flags[NUM_THREADS] = {0};

HashMap map;

void initHashMap(HashMap *map) {
    for (int i = 0; i < NUM_BUCKETS; i++) {
        map->buckets[i] = NULL;
        lock_init(i);
    }
}

// Just use last 3 bits as "hash"
int hash(int key) {
    return key & 0x7;
}

// Insert a key into the hash map
void insert(HashMap *map, int key, int hartid) {
    int index = hash(key);
    monitor_acquire(index, hartid);

    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->key = key;
    newNode->next = map->buckets[index];
    map->buckets[index] = newNode;

    monitor_release(hartid);
}


void __main() {
    while(!start_flag);

    int hartid = mhartid();

    int n_elements = 100;
    // Inserting 100 random ints
    for (int i = 0; i < n_elements; i++) {
        int key = rand() % n_elements;
        insert(&map, key, hartid);
    }

    done_flags[hartid] = 1;

    for(;;) {
      asm volatile ("wfi");
    }
}

// Main function to test the hash map
int main() {
    initHashMap(&map);
    srand(42);
    printf("Initialized\n");

    int hartid = mhartid();
    int tt = rdcycle();

    start_flag = 1;
    int n_elements = 100;
    // Inserting 100 random ints
    for (int i = 0; i < n_elements; i++) {
        int key = rand() % n_elements;
        insert(&map, key, hartid);
    }
    printf("Done inserting\n");

    // Barrier
    done_flags[hartid] = 1;
    for (int i = 1; i < NUM_THREADS; i++) {
        while (!done_flags[i]);
    }

    int ttt = rdcycle();
    printf("Passed barrier %d %d %d\n", tt, ttt, ttt-tt);


    return 0;
}