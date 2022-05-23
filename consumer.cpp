#include <iostream>//Thomas Kestler : file for consumer thread function
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
#include <vector>
#include <semaphore.h>
#include "BufferADT.h"
#include "consumer.h"
#include "producer.h"
#include "pc_unique.h"
#include <time.h>
#include "io.h"
#include "ridesharing.h"


#include <cctype>
using namespace std;

extern "C" void * consumer_thread(void * temp){//takes a pc_unique object as input which contains a pointer to the shared data structure and a flag for the kind of consumer: fast matching or cost saving

    
    pc_unique * consumer_unique = (pc_unique *)temp;
    
    int consumerID=consumer_unique->consumer;//set an ID from the pc_unique object for ease of use
   
    
    while(true){//this loop will run until manually exited when the production limit has been exceeded and the request queue is empty
   

    /*I do not completely separate the logic for fast matching/cost saving consumers in this function similarly to the producer thread
    this is because those two consumers must be able to deal with both human and robot requests, and because there will be more common logic in this function for both consumer types*/
    if(consumer_unique->brokerPtr->requestQueue.size()==0 && consumer_unique->brokerPtr->curRequestsProduced>=consumer_unique->brokerPtr->productionLimit  ){
        //exit the thread and update the precedence constraint when the production limit is exceeded and the queue is empty
        //checking both of these because the producer threads should stop before the consumer threads
        //sem_post(&consumer_unique->brokerPtr->unconsumed);   
        sem_post(&consumer_unique->brokerPtr->precedenceConstraint);
        pthread_exit(NULL);//make sure thread and function exit properly
        //return NULL;
        break;
        }
    //wait until there is an unconsumed item, and if there is not, block until the producer puts a new one on the buffer queue
    sem_wait(&consumer_unique->brokerPtr->unconsumed);

    

    //access buffer exclusively: start critical section
    sem_wait(&consumer_unique->brokerPtr->bufferMutex);

    if(consumer_unique->brokerPtr->requestQueue.size()==0 && consumer_unique->brokerPtr->curRequestsProduced>=consumer_unique->brokerPtr->productionLimit  ){
            
            sem_post(&consumer_unique->brokerPtr->precedenceConstraint);
            sem_post(&consumer_unique->brokerPtr->bufferMutex);
            //it is important to update the mutex since this is inside the critical section. We want to unblock the other consumer thread when one exits
            
            pthread_exit(NULL);//make sure thread and function exit properly
            //return NULL;
            break;
    }
    
    //remove item from buffer queue, but make sure to save the item before removing to determine if it was a human or autonomous car request
    int lastItem = consumer_unique->brokerPtr->requestQueue.back();
    consumer_unique->brokerPtr->requestQueue.pop_back();
    
    
    //check if the item removed is a human or auto driver request
    if(lastItem==HDR_REQUEST){
        //signal producer of human driver requests, and update the arrays for the output function: increase total consumed of human requests for the appropriate consumer ID
        //decrement the number of HDR requests currently in the queue for the output function as well
        consumer_unique->brokerPtr->curTotalConsumed[consumerID][HDR_REQUEST]+=1;
        consumer_unique->brokerPtr->curInRequestQueue[HDR_REQUEST]-=1;
        io_remove_type((Consumers)consumer_unique->consumer,(RequestType)HDR_REQUEST,(int *)consumer_unique->brokerPtr->curInRequestQueue,consumer_unique->brokerPtr->curTotalConsumed[consumerID]);
        
    }
    else if(lastItem==RDR_REQUEST){
        //do the same here as above , except make sure to modify the correct members of the arrays corresponding to the number of robot requests
        consumer_unique->brokerPtr->curTotalConsumed[consumerID][RDR_REQUEST]+=1;
        consumer_unique->brokerPtr->curInRequestQueue[RDR_REQUEST]-=1;
        io_remove_type((Consumers)consumer_unique->consumer,(RequestType)RDR_REQUEST,(int *)consumer_unique->brokerPtr->curInRequestQueue,consumer_unique->brokerPtr->curTotalConsumed[consumerID]);
        
    }


   
    if(consumer_unique->brokerPtr->requestQueue.size()==0 && consumer_unique->brokerPtr->curRequestsProduced>=consumer_unique->brokerPtr->productionLimit  ){
            
            sem_post(&consumer_unique->brokerPtr->precedenceConstraint);
            sem_post(&consumer_unique->brokerPtr->bufferMutex);
            //it is important to update the mutex since this is inside the critical section. We want to unblock the other consumer thread when one exits
            
            pthread_exit(NULL);//make sure thread and function exit properly
            //return NULL;
            break;
    }

    sem_post(&consumer_unique->brokerPtr->bufferMutex);//end the critical section and release the lock on the shared data


    //signal producer of available slot. if the last item removed was an HDR, then also signal the producer of a new available slot for an HDR
    if(lastItem==HDR_REQUEST){
        sem_post(&consumer_unique->brokerPtr->humanDrivers);
    }
    sem_post(&consumer_unique->brokerPtr->availableSlots);
    //simulate time it takes to consume the item
    
    usleep(MS_PERSEC*consumer_unique->sleepValue);

    if(consumer_unique->brokerPtr->requestQueue.size()==0 && consumer_unique->brokerPtr->curRequestsProduced>=consumer_unique->brokerPtr->productionLimit  ){
        //exit the thread and update the precedence constraint when the production limit is exceeded and the queue is empty
        //checking both of these because the producer threads should stop before the consumer threads
            
        sem_post(&consumer_unique->brokerPtr->precedenceConstraint);
        pthread_exit(NULL);//make sure thread and function exit properly
        //return NULL;
        break;
        }
    
    }
    //the end of the while loop here should never be reached because the if statements should ensure that the thread is exited before reaching here. 
    //however, I will keep these exit statements here just in case
    pthread_exit(NULL);
    return NULL;
}