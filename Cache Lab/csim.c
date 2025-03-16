/* 20220302 jihyunk */ 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "cachelab.h"

typedef struct {
  bool valid;
  int tag;
  int frequency;
} lineE;

typedef struct {
  lineE* lines;
} setS;

typedef struct {
  setS* sets;
  size_t setNum;  
  size_t lineNum; 
} Caches;

Caches cache = {};
int set_bits = 0, blockBits = 0; 
size_t hits = 0, misses = 0, evictions = 0; 

void simulate(int addr);
void update(setS *set, size_t line_no);

int main(int argc, char *argv[]) {
  
    FILE* inputfile = 0;

    for (int opt; (opt = getopt(argc, argv, "s:E:b:t:")) != -1;) {
        switch (opt) {
        case 's':
            set_bits = atoi(optarg); 
            cache.setNum = 2 << set_bits;
            break;
        case 'E':
            cache.lineNum = atoi(optarg); 
            break;
        case 'b':
            blockBits = atoi(optarg); 
            break;
        case 't': 
            if (!(inputfile = fopen(optarg, "r"))) { return 1; }
            break;
        default:
            return 1;
        }
    }

    if (!set_bits || !cache.lineNum || !blockBits || !inputfile){
        return 1; 
    }

    cache.sets = malloc(sizeof(setS) * cache.setNum);
    for (int i = 0; i < cache.setNum; i++) {
        cache.sets[i].lines = calloc(sizeof(lineE), cache.lineNum);
    }

    char kind;
    int addr;
  
    while (fscanf(inputfile, " %c %x%*c%*d", &kind, &addr) != EOF) {
        if (kind == 'I') {
            continue;
        }

        simulate(addr);

        if (kind == 'M') {
            simulate(addr); 
        }
    }

  printSummary(hits, misses, evictions);

  fclose(inputfile);

  for (size_t i = 0; i < cache.setNum; ++i){
    free(cache.sets[i].lines);
  }
  free(cache.sets);

  return 0;
}

void simulate(int addr) {
  
    size_t setIndex = (0x7fffffff >> (31 - set_bits)) & (addr >> blockBits);
    int tag = 0xffffffff & (addr >> (set_bits + blockBits));

    setS* set = &cache.sets[setIndex];

    for (size_t i = 0; i < cache.lineNum; i++) {
        lineE* line = &set->lines[i];

        if (!line->valid){ 
            continue; 
        }
        
        if (line->tag != tag){ 
            continue; 
        }

        hits++;
        update(set, i);
        return;
    }

    ++misses;

    for (size_t i = 0; i < cache.lineNum; i++) {
        lineE* line = &set->lines[i];

        if (line->valid) { continue; }

        line->valid = true;
        line->tag = tag;
        update(set, i);
        return;
    }

    evictions++;

    for (size_t i = 0; i < cache.lineNum; ++i) {
        lineE* line = &set->lines[i];

        if (line->frequency) { continue; }

        line->valid = true;
        line->tag = tag;
        update(set, i);
        return;
    }
}

void update(setS *set, size_t L) { 
  lineE *line = &set->lines[L];

  for (size_t i = 0; i < cache.lineNum; i++) {
    lineE *newLine = &set->lines[i];
    if (!newLine->valid){ 
        continue; 
    }

    if (newLine->frequency <= line->frequency){ 
        continue; 
    }

    (newLine -> frequency)--;
  }

  line->frequency = cache.lineNum - 1;
}

