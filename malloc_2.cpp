#include <iostream>
#include <unistd.h>
#include <cmath>
#include <stdio.h>
#include <string.h>
#include "malloc_2.h"
#include <stddef.h>




struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;

    MallocMetadata(size_t newSize,MallocMetadata** prevData):size(newSize),is_free(false),next(NULL),prev(*prevData){}
};

struct ListOfMallocMetadata{
    size_t totalAllocatedBlocks;
    size_t totalAllocatedBytes;
    size_t numberOfBlocksInUse;
    size_t numberOfBytesInUse;
    MallocMetadata* firstBlock;
    MallocMetadata* lastBlock;

    ListOfMallocMetadata():totalAllocatedBlocks(0),totalAllocatedBytes(0),numberOfBlocksInUse(0),numberOfBytesInUse(0){}
};

static ListOfMallocMetadata listOfBlocks;


void* smalloc(size_t size){
    if(size==0 || size>pow(10,8))
        return NULL;

//first block allocation
    if(!listOfBlocks.totalAllocatedBlocks){
       void* firstBlockAdress=sbrk(sizeof(MallocMetadata)+size);
       if(firstBlockAdress==(void*)(-1))
           return NULL;

        MallocMetadata* firstMeta=(MallocMetadata*)firstBlockAdress;
        firstMeta->size=size;
        firstMeta->is_free=false;
        listOfBlocks.totalAllocatedBlocks++;
        listOfBlocks.totalAllocatedBytes=size;
        listOfBlocks.numberOfBlocksInUse++;
        listOfBlocks.numberOfBytesInUse=size;
        listOfBlocks.firstBlock=firstMeta;
        listOfBlocks.lastBlock=firstMeta;
        return (void*)(firstMeta+1);
    }else{
    MallocMetadata* currBlock=listOfBlocks.firstBlock;
    MallocMetadata* finalLinkedBlock;

    while(currBlock!=NULL){
        if(currBlock->size>=size && currBlock->is_free)
            return (void*)(currBlock+1);
        if(!(currBlock->next))
            finalLinkedBlock=currBlock;

        currBlock=currBlock->next;
    }

    //didn't find proper block,need to allocate
        void* newBlockAdress=sbrk(sizeof(MallocMetadata)+size);
        if(newBlockAdress==(void*)(-1))
            return NULL;
        MallocMetadata* newMeta=(MallocMetadata*)newBlockAdress;
        newMeta->size=size;
        newMeta->is_free=false;
        newMeta->prev=finalLinkedBlock;
        finalLinkedBlock->next=newMeta;
        listOfBlocks.totalAllocatedBlocks++;
        listOfBlocks.totalAllocatedBytes+=size;
        listOfBlocks.numberOfBlocksInUse++;
        listOfBlocks.numberOfBytesInUse+=size;
        listOfBlocks.lastBlock=newMeta;

        return (void*)(newMeta+1);
    }


}

void* scalloc(size_t num,size_t size){
    if(size==0 || size*num>pow(10,8))
        return NULL;

    void* newBlockAdress=smalloc(num*size);
    if(!newBlockAdress)
        return NULL;
    memset(newBlockAdress,0,num*size);
    return newBlockAdress;

}

void sfree(void* p){
    if(!p)
        return;
    else if(((MallocMetadata*)p-1)->is_free)
        return;

    ((MallocMetadata*)p-1)->is_free=true;
    listOfBlocks.numberOfBlocksInUse--;
    listOfBlocks.numberOfBytesInUse-=((MallocMetadata*)p-1)->size;
}

void* srealloc(void* oldp,size_t size){
    if(size==0 || size>pow(10,8))
        return NULL;
    if(!oldp)
        return smalloc(size);
    if(((MallocMetadata*)oldp-1)->size>=size)
        return oldp;
    else{
        sfree(oldp);
        void* newBlockAdress=smalloc(size);
        if(!newBlockAdress)
            return NULL;

        memcpy(newBlockAdress,oldp,((MallocMetadata*)oldp-1)->size);

        return newBlockAdress;
    }
}

size_t _num_free_blocks(){
    return listOfBlocks.totalAllocatedBlocks-listOfBlocks.numberOfBlocksInUse;
}

size_t _num_free_bytes(){
    return listOfBlocks.totalAllocatedBytes-listOfBlocks.numberOfBytesInUse;
}

size_t _num_allocated_blocks(){
    return listOfBlocks.totalAllocatedBlocks;
}

size_t _num_allocated_bytes(){
    return listOfBlocks.totalAllocatedBytes;
}

size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}

size_t _num_meta_data_bytes(){
    return (listOfBlocks.numberOfBlocksInUse)*(_size_meta_data());
}