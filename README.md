# CPSC 351 - Virtual Memory Manager

## Overview
This application is part of a virtual memory manager. The logical addresses are read in from a .txt file line by line into a buffer. The logical addresses are translated to physical addresses.

The program first checks the TLB table for the physical frame, then the page table and if it is not in the page table then a page
fault is recorded and the information is fetched from the backing store binary file which is also provided as an argument in the CLI upon initiation of the application.

The page is copied from the backing store into main memory, the page table and the TLB table.
Each line that is read from the input file, outputs a status of whether the page was found in the TLB, page table, or taken from the backing store.

The location of the hit type (tlb, page table or backing store), # of page faults, logical address, physical address, value and total tlb hits are printed to console. There is also a print out of a general summary including: total addresses translated, page faults that occurred, page fault rate, TLB hits and TLB hit rate.

#### Compilation Instructions
g++ -Wall -std=c++11 main.cpp -o Exec

#### Run Application
./Exec <backing-store-filename>\.bin address-bank.txt

**Note:** The address-bank.txt and <backing-store-filename.bin file have not been provided. You must use your own files.
