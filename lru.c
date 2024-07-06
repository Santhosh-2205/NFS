#include "headers.h"

#define NUM_CACHES 10
cache lru[NUM_CACHES];

int lru_index = 0;

void initializeCache() 
{
    for (int i = 0; i < NUM_CACHES; i++) 
    {
        memset(lru[i].path, 0, sizeof(lru[i].path));
        lru[i].ss_id = -1;
    }
    lru_index = 0;
}

void addEntry(const char *path, int ss_id) 
{
    int i = lru_index;

    strncpy(lru[i].path, path, sizeof(lru[i].path) - 1);
    lru[i].ss_id = ss_id;

    lru_index = (lru_index + 1) % NUM_CACHES;
}

int retrieve_ssid(const char *path) 
{
    for (int i = 0; i < NUM_CACHES; i++) 
    {
        if (strcmp(lru[i].path, path) == 0) 
        {
            return lru[i].ss_id;
        }
    }
    return -1; // Cache miss
}
