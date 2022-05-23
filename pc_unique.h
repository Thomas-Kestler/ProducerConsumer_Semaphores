//Thomas Kestler 
//struct for pc_unique aka producer_consumer_unique
//holds numeric sleep values for the appropriate producer/consumer and a pointer to a BufferADT object

#ifndef pc_unique_H
#define pc_unique_H
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <vector>
#include "BufferADT.h"
#include "io.h"
#include "ridesharing.h"

typedef struct pc_unique {

struct BufferADT *brokerPtr;//pointer to the shared data structure with the queue and other shared attributes
int sleepValue;//appropriate value for sleep for the threads

int request; /*This value will be set to 0 for human driver/HDR, and set to 1 for autonomous car/robot driver/RDR.  */
int consumer; /*This will be 0 for cost saving, and will be 1 for fast matching.  */

}pc_unique;


#endif