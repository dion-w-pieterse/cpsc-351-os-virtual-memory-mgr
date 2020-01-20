/*
 Name: Dion W. Pieterse
 Class: CPSC 351
 Description:
 This application is part of a virtual memory manager. The logical addresses are read in from a .txt file line by line into a buffer. The logical addresses
 are translated to physical addresses. The program first checks the TLB table for the physical frame, then the page table and if it is not in the page table then a page
 fault is recorded and the information is fetched from the backing store binary file which is also provided as an argument in the CLI upon initiation of the application.
 The page is copied from the backing store into main memory, the page table and the TLB table.
 Each line that is read from the input file outputs a status of whether the page was found in the TLB, page table or taken from the backing store. The location of the hit type (tlb, page table or backing store), # of page faults, logical address, physical address, value and total tlb hits are printed to console. There is also a print out of a general summary including: total addresses translated, page faults that occurred, page fault rate, TLB hits and TLB hit rate.

 */

#include <iostream>
#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
using namespace std;

#define OFFSET_BITS 8
#define PAGE_MASK 255
#define PAGE_SIZE 256
#define TLB_TABLE_SIZE 16
#define NUM_OF_PAGES 256
#define FILE_ERROR 1
#define MAIN_MEM_SIZE NUM_OF_PAGES * PAGE_SIZE
#define OFFSET_MASK 255
//set the buffer size for the # of chars read in per line.
#define SIZE_OF_BUFFER 10

//define the struct to store the logical page and physical frame
typedef struct {
    unsigned char logical_Page;
    unsigned char physical_Frame;
} tlbElement;

//This variable keeps track of how many TLB elements have been inserted into the table.
int tlbInsertIndex = 0;

//define the TLB table
tlbElement tlbTable[TLB_TABLE_SIZE];

//define the page table which holds the physical frame as the value for each element.
int pageTable[NUM_OF_PAGES];

//define the main memory array.
char mainMemory[MAIN_MEM_SIZE];

//this is the pointer variable to the backing store file.
void* backingPtr;

//define the minimum function to determine the min value of 2 parameters.
int min(int num1, int num2) {
    if(num1 < num2) { return num1; }
    return num2;
}

/***********************************
 *** search_TLB_Table
 ***********************************/
int search_TLB_Table(unsigned char logical_page) {

    size_t max = std::min(tlbInsertIndex, TLB_TABLE_SIZE);
    for(size_t counter = 0; counter < max; ++counter) {
        if(tlbTable[counter].logical_Page == logical_page) { return counter; }
    }
    return -1;
}

/***********************************
 *** add_to_TLB_Table
 ***********************************/
void add_to_TLB_Table(unsigned char lglPage, unsigned char physFrame) {

    tlbElement& entry = tlbTable[tlbInsertIndex++ % TLB_TABLE_SIZE];
    entry.logical_Page = lglPage;
    entry.physical_Frame = physFrame;
}

/**************************************************
 *** translate logical address to physical address
 *************************************************/
void translate_Lcl_To_Phys(FILE* inputFilePtr) {

    printf("************************************************* VIRTUAL MEMORY MANAGER ************************************************************\n\n");
    //For holding hits
    struct {
        int tlbHit;
        int ptHit;
        int backingHit;
    } typedef stats;

    //instantiate the stats struct
    stats hitTypes;

    // Data we need to keep track of to compute stats at end.
    int totalTLBHits = 0;
    int totalPageFaults = 0;
    int total_addr = 0;
    int logical_page;
    int theOffset;
    int logiAddress;
    int physAddress;
    // Character buffer for reading lines of input file.
    char lineReadBuffer[SIZE_OF_BUFFER];

    //initialize all page table elements to -1
    memset(pageTable, -1, sizeof(pageTable) * sizeof(int));

    //# of next unallocated free frame in main memory
    unsigned char freeFrame = 0;
    char valueInMem;
    int physicalFrame;

    //Loop: read each line from the input file until the end of the file
    while (fgets(lineReadBuffer, SIZE_OF_BUFFER, inputFilePtr) != NULL) {

        //increment the total addresses accumulator
        ++total_addr;
        //convert the buffer value to an integer value
        logiAddress = atoi(lineReadBuffer);
        //calculate the offset
        theOffset = logiAddress & OFFSET_MASK;
        //calculate the logical page value
        logical_page = (logiAddress >> OFFSET_BITS) & PAGE_MASK;

        //attempt to collect the physical frame from the tlb table.
        physicalFrame = search_TLB_Table(logical_page);

        if(physicalFrame != -1) {
            //a TLB hit has occurred.
            //set the line status structure values
            hitTypes.tlbHit = 1;
            hitTypes.ptHit = 0;
            hitTypes.backingHit = 0;

            //increment the tlb hit accumulator
            ++totalTLBHits;

        } else if ((physicalFrame = pageTable[logical_page]) != -1) {
            //There is a page table hit.
            //set the line status structure values
            hitTypes.tlbHit = 0;
            hitTypes.ptHit = 1;
            hitTypes.backingHit = 0;

            //add the logical page and physical frame to the tlb table.
            add_to_TLB_Table(logical_page, physicalFrame);
        } else {

            //increment page fault accumulator
            ++totalPageFaults;
            //set the physical frame to the free frame
            physicalFrame = freeFrame;
            ++freeFrame;//increment the free frame

            // Copy page from backing file into physical memory
            memcpy(mainMemory + physicalFrame * PAGE_SIZE, (char*)backingPtr + logical_page * PAGE_SIZE, PAGE_SIZE);
            pageTable[logical_page] = physicalFrame;
            add_to_TLB_Table(logical_page, physicalFrame);

            //set the line status structure values
            hitTypes.tlbHit = 0;
            hitTypes.ptHit = 0;
            hitTypes.backingHit = 1;
        }
        //Calculate the physical address and value variables.
        physAddress = (physicalFrame << OFFSET_BITS) | theOffset;
        valueInMem = mainMemory[physicalFrame * PAGE_SIZE + theOffset];

        //Print out statistics for each line that is read from the input file.
        printf("TLB_Hit: %-1d  -  PT_Hit: %-1d  -  BS_Hit: %-1d  - PF:%-4d Logical address: 0x%-6x Physical address: 0x%-6x Value: %-4d Total TLB Hits: %d \n", hitTypes.tlbHit, hitTypes.ptHit, hitTypes.backingHit, totalPageFaults, logiAddress, physAddress, valueInMem, totalTLBHits);
        printf("-------------------------------------------------------------------------------------------------------------------------------------\n");
    }

    //Print summary of statistics
    printf("\n*************************************************** SUMMARY STATS *******************************************************************\n");
    printf("Total addresses translated: %d\n", total_addr);
    printf("Page faults: %d\n", totalPageFaults);
    printf("Page fault rate: %.3f\n", totalPageFaults / (double)total_addr);
    printf("TLB hits: %d\n", totalTLBHits);
    printf("TLB hit rate: %.3f\n", totalTLBHits / (double)total_addr);


    printf("*************************************************************************************************************************************\n");
}

/***************************
 *** MAIN ***
 *************************/
int main(int argc, const char* argv[]) {

    //Check there are 3 arguments provided when program runs.
    if (argc != 3) {
        fprintf(stderr, "There must be 3 arguments provided in the CLI.\n");
        exit(FILE_ERROR);
    }

    //Open backing store file
    const char* backingFilename = argv[1];
    int backingFileInt = open(backingFilename, O_RDONLY);
    backingPtr = mmap(0, MAIN_MEM_SIZE, PROT_READ, MAP_PRIVATE, backingFileInt, 0);

    //Check if backing store file is opened correctly.
    if(backingPtr == nullptr) {fprintf(stderr, "Error: The backing store file could not be opened. %s\n", backingFilename); exit(FILE_ERROR);}

    //read in file with logical addresses
    const char* inputFilename = argv[2];
    FILE* inputFilePtr = fopen(inputFilename, "r");

    //check if the input file has been opened correctly.
    if(inputFilePtr == nullptr) {
        fprintf(stderr, "The input file could not be opened correctly: %s\n", inputFilename);
        exit(FILE_ERROR);
    }

    //Translate all logical addresses to physical address
    translate_Lcl_To_Phys(inputFilePtr);

    return 0;
}
