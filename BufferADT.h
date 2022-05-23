
//BufferADT shared data structure declaration - Thomas Kestler 
#ifndef BufferADT_H
#define BufferADT_H
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <vector>
#include <cstdlib>
#include "io.h"
#include "ridesharing.h"
//BufferADT
using namespace std;


/**
* Shared constants and data structures among threads
**/
#define NUMCONSUMERPRODUCERS 2

#define DEFAULT_PRODUCTIONLIMIT 120
//default number for production limit/number of requests
#define DEFAULT_COSTSAVINGDISPATCHER_TIME 0
/* default time it takes for the cost saving dispatcher to dispatch a request */
#define DEFAULT_FASTMATCHINGDISPATCHER_TIME 0
/* default time for fast matching dispatcher to dispatch a request */
#define DEFAULT_HDRREQUEST_TIME 0
/* default time to produce a human driver request */
#define DEFAULT_RDRREQUEST_TIME 0
/* default time to produce an automated car request */

#define MS_PERSEC 1000

#define HDR_REQUEST 0//flags to inform the thread functions of which producer/consumer called it
#define RDR_REQUEST 1
#define FASTMATCHING_REQUEST 1
#define COSTSAVING_REQUEST 0
#define AVAILABLE_SLOTS 12



/* Common data shared between threads */
typedef struct BufferADT {

 //vector for the queue of requests, I chose this structure since it has functions similar to a queue which makes it easy to remove an item from the front and insert one to the back
 vector<int> requestQueue;
    
 
    //setting default values here in case some optional command line arguments are not included
    int productionLimit=DEFAULT_PRODUCTIONLIMIT;

    int timeForCostSaving=DEFAULT_COSTSAVINGDISPATCHER_TIME;
    int timeForFastMatching=DEFAULT_FASTMATCHINGDISPATCHER_TIME;
    int timeForHDR=DEFAULT_HDRREQUEST_TIME;
    int timeForRDR = DEFAULT_RDRREQUEST_TIME;
    /* 
    */
    int curRequestsProduced;//this will be updated by the producer threads each time a new request is produced

    sem_t bufferMutex;//used to create a critical section to exclusively access the shared data - -the queue should only be accessed by one producer/consumer at a time
    sem_t availableSlots;//tracks the total # of empty slots in the queue. this should never go above 12
    sem_t unconsumed;//tracks the total # of unconsumed requests in the queue
    sem_t precedenceConstraint;//used to ensure the order of execution: the producer/consumer threads must all finish before the main thread finishes and prints the final report
    sem_t humanDrivers;//tracks the current # of slots for human requests in the queue. The maximum is 4
    

    /*arrays for i/o functions  */
    int curInRequestQueue[NUMCONSUMERPRODUCERS];
    int curTotalProduced[NUMCONSUMERPRODUCERS];
    int **curTotalConsumed;//initialized as a double pointer and a 2D array since the number of HDRs and RDRs must be tracked independently for the fast matching and cost saving consumers
    
    BufferADT(){
        
        curTotalConsumed = (int **)malloc(sizeof(int*)*NUMCONSUMERPRODUCERS);//allocate appropriate memory for the totalConsumed array
        curTotalConsumed[HDR_REQUEST] = (int *)malloc(sizeof(int)*NUMCONSUMERPRODUCERS);
        curTotalConsumed[RDR_REQUEST] = (int *)malloc(sizeof(int)*NUMCONSUMERPRODUCERS);
        curTotalConsumed[HDR_REQUEST][HDR_REQUEST]=0;
        curTotalConsumed[HDR_REQUEST][RDR_REQUEST]=0;
        curTotalConsumed[RDR_REQUEST][HDR_REQUEST]=0;
        curTotalConsumed[RDR_REQUEST][RDR_REQUEST]=0;
        
        curTotalProduced[HDR_REQUEST]=0;
        curTotalProduced[RDR_REQUEST]=0;
        curInRequestQueue[HDR_REQUEST]=0;
        curInRequestQueue[RDR_REQUEST]=0;
        //manually initialize values for the arrays to prevent abnormalities in the values when being printed in the output functions
        requestQueue.reserve(AVAILABLE_SLOTS);
        //reserve 12 empty slots in the queue. filling it will 12 zeros is not desirable because the size should initially be zero
        
    }


} BufferADT;
#endif