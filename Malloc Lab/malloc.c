/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*Referenced from the textbook (figure 9.43)*/
// Basic constants and macros 
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12) //initial size of free block 

#define MAX(x,y) ((x)>(y) ? (x) : (y)) // calculate max value

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p)=(val)) 

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0X7) 
#define GET_ALLOC(p) (GET(p) & 0X1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) 
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) 


/*newly added */
#define INITCHUNKSIZE (1<<6) 
#define LISTLIMIT 20 

#define SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))
#define NEXT_PTR(ptr) ((char *)(ptr))
#define PREV_PTR(ptr) ((char *)(ptr) + WSIZE)

// Return next and prev pointers within segregated list
#define SEG_NEXT_PTR(ptr) (*(char **)(ptr))
#define SEG_PREV_PTR(ptr) (*(char **)(PREV_PTR(ptr)))

// Global variables 
char *heap = 0;
void *seg_free_lists[LISTLIMIT];

// User Defined Function list
static void *extend_heap(size_t words);
static void insert_free_block(void *ptr, size_t size);
static void delete_free_block(void *ptr);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

/* 
 * extend_heap - extends the heap with a new free block
 * except for the insert_free_block function, referenced from the text book (figure 9.45)
 */
static void *extend_heap(size_t words){
    char *bp;
    size_t size;

    // Align to even number of words (8 bytes) and request for heap space.
    size = (words % 2) ? (words +1) * WSIZE : words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

	// Initialize free block header/footer and the epilogue header.
    PUT(HDRP(bp),PACK(size,0)); // Free block header
    PUT(FTRP(bp),PACK(size,0)); // Free block footer
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1)); // New epilogue header
    insert_free_block(bp,size);  // Add the new free block to the free list

    return coalesce(bp); // Coalesce if the previous block was free.
}

/* 
 * insert_free_block - Add the new free block to the free list
 */
static void insert_free_block(void *ptr, size_t size) {
    int seg_idx = 0;  // index for seg list 
    void *next_ptr = ptr; 
    void *prev_ptr = NULL; // The actual position where the free block is inserted
    
    // Seg list is managed by dividing the index by the power of 2
	// this while loop finds correct index by removing size bitwise 
    while ((seg_idx < LISTLIMIT - 1) && (size > 1)) {
        size >>= 1;
        seg_idx++;
    }
    
    next_ptr = seg_free_lists[seg_idx];   
    
	//if the size of the block you want to create is larger than the existing block size, the loop starts
    while ((next_ptr != NULL) && (size > GET_SIZE(HDRP(next_ptr)))) {
        prev_ptr = next_ptr; // Update the previously existing address value to prev_ptr
        next_ptr = SEG_NEXT_PTR(next_ptr); // Move the position of next_ptr to the next block
    }
    
	// Conside 4 cases (figure 9.40)
    if (next_ptr != NULL) {
        if (prev_ptr != NULL) { // Case 1 prev and next allocated
            SET_PTR(NEXT_PTR(ptr), next_ptr);     
            SET_PTR(PREV_PTR(next_ptr), ptr);     
            SET_PTR(PREV_PTR(ptr), prev_ptr);   
            SET_PTR(NEXT_PTR(prev_ptr), ptr);   
        } else { // Case 3: prev free, next allocated
            SET_PTR(NEXT_PTR(ptr), next_ptr);  
            SET_PTR(PREV_PTR(next_ptr), ptr);   
            SET_PTR(PREV_PTR(ptr), NULL);   
            seg_free_lists[seg_idx] = ptr; 
        }
    } else {
        if (prev_ptr != NULL) { //Case 2: prev allocated, next free.
            SET_PTR(NEXT_PTR(ptr), NULL);        
            SET_PTR(PREV_PTR(ptr), prev_ptr);  
            SET_PTR(NEXT_PTR(prev_ptr), ptr); 
        } else { //Case 4: next and prev free
            SET_PTR(NEXT_PTR(ptr), NULL);   
            SET_PTR(PREV_PTR(ptr), NULL);     
            seg_free_lists[seg_idx] = ptr;   
        }
    }
    
    return;
}

/* 
 * delete_free_block - the deletion of a node from a segregated free list
 */
static void delete_free_block(void *ptr) {
    int seg_idx = 0;
    size_t ptr_size = GET_SIZE(HDRP(ptr));
    
    // Find the index of the appropriate free list according to the size
    while ((seg_idx < LISTLIMIT - 1) && (ptr_size > 1)) {
        ptr_size >>= 1;
        seg_idx++;
    }
    
	// Conside 4 cases (figure 9.40)
    if (SEG_NEXT_PTR(ptr) != NULL) {
        if (SEG_PREV_PTR(ptr) != NULL) { 				// Case 1 prev and next allocated
            SET_PTR(PREV_PTR(SEG_NEXT_PTR(ptr)), SEG_PREV_PTR(ptr));  
            SET_PTR(NEXT_PTR(SEG_PREV_PTR(ptr)), SEG_NEXT_PTR(ptr));  
        } else { 								// Case 3: prev free, next allocated
            SET_PTR(PREV_PTR(SEG_NEXT_PTR(ptr)), NULL);     
            seg_free_lists[seg_idx] = SEG_NEXT_PTR(ptr);  
        }
    } else {
        if (SEG_PREV_PTR(ptr) != NULL) { 				//Case 2: prev allocated, next free.
            SET_PTR(NEXT_PTR(SEG_PREV_PTR(ptr)), NULL);  
        } else { 								//Case 4: next and prev free
            seg_free_lists[seg_idx] = NULL;   
        }
    }
    
    return;
}

/* 
 * coalesce - Coalesces free blocks before and after the current block if they exist
			  Referenced from the textbook (figure 9.46) -> from the textbook code, delete_free_block is added 
 */
static void *coalesce(void *bp){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc){ 				// Case 1 prev and next allocated
        return bp;
    }
    else if (prev_alloc && !next_alloc){ 		//Case 2: prev allocated, next free.
        delete_free_block(bp);           
        delete_free_block(NEXT_BLKP(bp));  
        
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }
    else if (!prev_alloc && next_alloc){ 		// Case 3: prev free, next allocated
        delete_free_block(bp);
        delete_free_block(PREV_BLKP(bp));
        
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    else{  										//Case 4: next and prev free
        delete_free_block(bp);
        delete_free_block(PREV_BLKP(bp));
        delete_free_block(NEXT_BLKP(bp));
        
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    
    insert_free_block(bp,size); 
    return bp;
}

/* 
 * find_fit - find a suitable block of memory in a segregated list that can fit the requested size (asize).
 */
static void *find_fit(size_t asize){
    char *bp; 
    
    int seg_idx = 0; 
    size_t seg_size = asize; 

    // Search for a free block in the segregated list
    while (seg_idx < LISTLIMIT) {
        if ((seg_idx == LISTLIMIT - 1) || ((seg_size <= 1) && (seg_free_lists[seg_idx] != NULL))) {
            bp = seg_free_lists[seg_idx];    
            
			// Continue while loop if bp block is not empty and until a block that can fit the target size is found
            while ((bp != NULL) && ((asize > GET_SIZE(HDRP(bp)))))  
            {
                bp = SEG_NEXT_PTR(bp);  // Search the next segregated block
            }
            if (bp != NULL) // If a block that can be allocated is found
                return bp;
        }
        
        seg_size >>= 1;
        seg_idx++;       
    }

    return NULL; // Return NULL if no suitable block is found
}

/* 
 * place - place a requested block size (asize) into a free memory block (bp)
 * referenced from the textbook (Practice problem 9.9)
 */
static void place(void *bp, size_t asize){
    size_t csize = GET_SIZE(HDRP(bp));

	// Remove the block from the free list
	// **different from the textbook 
    delete_free_block(bp);

    if ((csize-asize)>=(2*DSIZE)){
        PUT(HDRP(bp),PACK(asize,1));
        PUT(FTRP(bp),PACK(asize,1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp),PACK(csize-asize,0));
        PUT(FTRP(bp),PACK(csize-asize,0));
        insert_free_block(bp,(csize-asize));
    }
    else{
        PUT(HDRP(bp),PACK(csize,1));
        PUT(FTRP(bp),PACK(csize,1));
    }
}

/* 
 * mm_init - initialize the malloc package.
 * Referenced from the textbook (Figure 9.44)
 */
int mm_init(void)
{
    int list;
    
	// Initialize all segregated free lists to NULL.
	// different from the textbook 
    for (list = 0; list < LISTLIMIT; list++) {
        seg_free_lists[list] = NULL;
    }
    
    // from this code to the end, referenced from the book (figure 9.44)
    // Create the initial empty heap.
    if ((heap = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap, 0);                          /* Alignment padding */
    PUT(heap + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
    heap += (2*WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */    
	if (extend_heap(INITCHUNKSIZE) == NULL)
        return -1;  
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 * 	   Referenced from the textbook (figure 9.47)
 */
void *mm_malloc(size_t size){
    size_t asize; // adjusted block size (overhead and padding included)
    size_t extendsize; // amount to extend heap if not fit 
    char *bp;

    /* Ignore spurious*/
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp; 
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 * referenced from the textbook (figure 9.46)
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp),PACK(size,0));
    PUT(FTRP(bp),PACK(size,0));
    
    insert_free_block(bp,size); //different from the textbook 

    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr; // Save the previous pointer
    void *newptr; // Declare a new pointer
    size_t copySize; // Declare the size for copying
    
    newptr = mm_malloc(size); // Allocate memory for the new pointer
    if (newptr == NULL) // If allocation fails, return NULL
      return NULL;

    // Get the size of the previous pointer
    copySize = GET_SIZE(HDRP(oldptr));

	// If the requested size is less than the previous size, adjust copySize to the smaller size
    if (size < copySize)
      copySize = size;

    memcpy(newptr, oldptr, copySize); // Copy the contents from the previous pointer to the new one
    mm_free(oldptr); // Free the previous pointer

    return newptr;
}