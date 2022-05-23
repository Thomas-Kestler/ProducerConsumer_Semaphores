#include <iostream>//Thomas Kestler : file for producer thread function
#include <fstream> 
#include <string.h>
#include <cstring>
#include <string>
#include <exception>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <vector>
#include <semaphore.h>
#include "BufferADT.h"
#include "consumer.h"
#include "producer.h"
#include "pc_unique.h"
#include "io.h"
#include "ridesharing.h"


#include <cctype>
using namespace std;

extern "C" void * producer_thread(void * temp){//takes a pc_unique object as input which contains a pointer to the shared data structure and a flag for the kind of producer: human or robot
    
    pc_unique * producer_unique = (pc_unique *)temp;
    
    while(true){//this loop will run until manually exited when the production limit has been exceeded 

    //simulate time it takes to produce a request at beginning of loop
    usleep(MS_PERSEC*producer_unique->sleepValue);
    
    //this thread function has to deal w/ two producers, one human and one autonomous
    if(producer_unique->brokerPtr->curRequestsProduced>=producer_unique->brokerPtr->productionLimit){//exit the thread and update the precedence constraint if the production limit is exceeded
            sem_post(&producer_unique->brokerPtr->precedenceConstraint);
            pthread_exit(NULL);//make sure thread and function exit properly
            return NULL;
            break;
        }
        else{
            
        }
    
    
    if(producer_unique->request==HDR_REQUEST){//I have separated the logic depending on if the HDR or RDR producer called this thread function
        
        sem_wait(&producer_unique->brokerPtr->humanDrivers);//if there are no available human requests, then wait until consumer opens one up
        sem_wait(&producer_unique->brokerPtr->availableSlots);//waiting on available slots after checking for a human driver to prevent unnecessary blocking of the other producer thread
        sem_wait(&producer_unique->brokerPtr->bufferMutex);//attempt to access the buffer: begin critical section

        if(producer_unique->brokerPtr->curRequestsProduced>=producer_unique->brokerPtr->productionLimit){
            //if the production limit is exceeded inside the critical section, update the precedence constraint
            sem_post(&producer_unique->brokerPtr->precedenceConstraint);
            sem_post(&producer_unique->brokerPtr->unconsumed);//I update unconsumed and humanDrivers here in an attempt to prevent the consumer threads from getting blocked at the end
            sem_post(&producer_unique->brokerPtr->humanDrivers);
            sem_post(&producer_unique->brokerPtr->bufferMutex);//release the lock on the buffer so the other threads don't get blocked indefinitely
            pthread_exit(NULL);//make sure thread and function exit properly
            return NULL;
            break;
        }
        else{
            
        }
        
        producer_unique->brokerPtr->curRequestsProduced+=1;//increment the current number of requests produced
        //mutual exclusion, insert item to buffer
        //only add a human request to the queue if there is space
        producer_unique->brokerPtr->requestQueue.push_back(HDR_REQUEST);
        producer_unique->brokerPtr->curTotalProduced[HDR_REQUEST]+=1;//update the arrays in the shared data appropriately for the output functions
        producer_unique->brokerPtr->curInRequestQueue[HDR_REQUEST]+=1;
        io_add_type((RequestType)HDR_REQUEST,(int *)producer_unique->brokerPtr->curInRequestQueue,(int *)producer_unique->brokerPtr->curTotalProduced);
        //add a human request to the queue        
        //end critical section: release the lock on the buffer
        sem_post(&producer_unique->brokerPtr->bufferMutex);

    }
else if(producer_unique->request==RDR_REQUEST){//logic for the RDR producer thread

        sem_wait(&producer_unique->brokerPtr->availableSlots);//check for available slots in the buffer queue before accessing the mutex
        
        sem_wait(&producer_unique->brokerPtr->bufferMutex);//attempt to access the buffer: begin critical section

        if(producer_unique->brokerPtr->curRequestsProduced>=producer_unique->brokerPtr->productionLimit){
             //if the production limit is exceeded inside the critical section, update the precedence constraint
            sem_post(&producer_unique->brokerPtr->precedenceConstraint);
            sem_post(&producer_unique->brokerPtr->unconsumed);//I update unconsumed here in an attempt to prevent the consumer threads from getting blocked at the end
            
            sem_post(&producer_unique->brokerPtr->bufferMutex);//release the lock to unblock other threads, since this thread will exit before releasing the lock in the while loop
            pthread_exit(NULL);//make sure thread and function exit properly
            return NULL;
            break;
        }
        else{
            
        }
        
        
        producer_unique->brokerPtr->curRequestsProduced+=1;
        //mutual exclusion, insert item to buffer
        
        //add a robot request to the queue, and update the arrays for the output function
        producer_unique->brokerPtr->requestQueue.push_back(RDR_REQUEST);
        
        producer_unique->brokerPtr->curTotalProduced[RDR_REQUEST]+=1;
        producer_unique->brokerPtr->curInRequestQueue[RDR_REQUEST]+=1;
        io_add_type((RequestType)RDR_REQUEST,(int *)producer_unique->brokerPtr->curInRequestQueue,(int *)producer_unique->brokerPtr->curTotalProduced);
        
        //end critical section, release lock for other threads
        sem_post(&producer_unique->brokerPtr->bufferMutex);


    }
    else{
        //if request value not 0 or 1 , something is wrong
       
    }

    sem_post(&producer_unique->brokerPtr->unconsumed);//update the unconsumed semaphore to signal the consumer threads. this should be done regardless of whether this is the HDR or RDR thread

   


    }
    
    //the end of the while loop here should never be reached because the if statements should ensure that the thread is exited before reaching here. 
    //however, I will keep these exit statements here just in case
    pthread_exit(NULL);
    return NULL;
    
}