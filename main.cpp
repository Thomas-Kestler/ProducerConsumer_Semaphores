
//main.cpp Thomas Kestler 
#include <iostream>
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
#define BADFLAG 3;//used for errors in optional arguments
#define MSPERSEC 1000 /*1000 milliseconds per second */
#define FASTMATCHINGFLAG 1;//these flags are used to set attributes in pc_unique to tell threads the kind of producer/consumer that accessed the thread functino
#define COSTSAVINGFLAG 0;
#define HDRFLAG 0;
#define RDRFLAG 1;
#define NOTCONSUMER 99;//this is used for producer pc_unique objects
#define NOTPRODUCER 99;//this is used for consumer pc_unique objects
#define AVAILABLESLOTS 12;//values used to initialize semaphores
#define HUMANDRIVERS 4;
#define MUTEXINITIALIZE 1;


/*void calcSleepTimes(BufferADT *sharedB, int cost, int fast, int HD, int RD){
    struct BufferADT *sharedBuffer = sharedB;
    sharedBuffer->costSleep.tv_sec = cost/MSPERSEC;
    sharedBuffer->fastSleep.tv_sec = fast/MSPERSEC;
    sharedBuffer->HDRSleep.tv_sec = HD/MSPERSEC;
    sharedBuffer->RDRSleep.tv_sec = RD/MSPERSEC;

I ended up not using this function and instead using usleep() since it worked better
}*/




//g++ -std=c++11 -g -o rideshare main.cpp BufferADT.h consumer.h producer.h producer.cpp consumer.cpp pc_unique.h io.cpp io.h ridesharing.h -pthread


// ./rideshare -n 150 -h 5 -a 15 -c 35 -f 20





extern "C" int main(int argc, char **argv){
    try{//simple try catch in case there are errors with optional arguments
    
    BufferADT *sharedBuffer = new BufferADT();//initialize the shared data structure

    int Option=0;//used to hold getopt returned values for opt args
    
    int optionalCount=0;//track number of optional arguments
    int curArg=0;//this will hold integer values entered in optional arguments
    int numRequests=120;//define default values for the optional arguments in case they are not specified
    int costSaving_time=0;
    int fastMatching_time=0;
    int HDR_time=0;
    int RDR_time=0;
    sharedBuffer->productionLimit=120;//set default values in shared data structure
    sharedBuffer->timeForCostSaving=0;
    sharedBuffer->timeForFastMatching=0;
    sharedBuffer->timeForHDR=0;
    sharedBuffer->timeForRDR=0;
    
    //get optional arguments
    while ( (Option = getopt(argc, argv, "n:c:f:h:a:")) != -1) {//handle optional command arguments
        switch (Option) {
        case 'n': /* total number of requests/production limit. default is 120 if not specified */
            optionalCount++;
            for(int i=0;i<argc;i++){//loop through argv to find the associated # of requests. NOTE I am using this method instead of optarg since it caused errors in the past
                if(strcmp(argv[i],"-n")==0){
                    curArg=atoi(argv[i+1]);
                    
                    numRequests=curArg;
                    sharedBuffer->productionLimit=numRequests;
                    if(argc-i<=1){
                        cout<<"optarg found with no value afterwards in argv"<<endl;
                    }
                }
            }
            if(curArg<=0){//prevents number of 0 or less from being used as # of requests
                cout<<"number of requests to process should be 1 or greater"<<endl;
                exit(0);
            }
            break;
        case 'c': /* # of milliseconds for cost saving dispatcher to dispatch a request */
        optionalCount++;
             for(int i=0;i<argc;i++){//loop through argv to find the value
                if(strcmp(argv[i],"-c")==0){
                    curArg=atoi(argv[i+1]);
                   
                    costSaving_time=curArg;
                    sharedBuffer->timeForCostSaving=costSaving_time;
                    if(argc-i<=1){
                        cout<<"optarg found with no value afterwards in argv"<<endl;
                    }
                    
                }
            }
            
            
            if(curArg<0){
                cout<<"# milliseconds for cost saving should be a number and greater than or equal to zero"<<endl;
                exit(0);
            }
            break;
        case 'f': /* # milliseconds for the fast matching dispatcher to dispatch a request */
            optionalCount++;
            for(int i=0;i<argc;i++){
                if(strcmp(argv[i],"-f")==0){//loop through argv to find the associated value
                    
                    if(argc-i<=1){
                        cout<<"optarg found with no value afterwards in argv"<<endl;
                    }
                    else{
                    
                        if(argv[i+1]<0)//handles possibility of a negative argument
                        {cout<<"# milliseconds for fast matching dispatcher must be a number, greater than or equal to 0"<<endl;}
                        else {
                            curArg=atoi(argv[i+1]);
                            fastMatching_time=curArg;
                            sharedBuffer->timeForFastMatching=fastMatching_time;
                            
                        }

                    }
                }
            }
            
            if(curArg<0){
                cout<<"# milliseconds for fast matching dispatcher must be a number, greater than or equal to 0"<<endl;
                exit(0);
            }
            
            break;
        case 'h': /* # milliseconds to produce a ride request for a human driver */
            optionalCount++;
             for(int i=0;i<argc;i++){//loop through argv to find the associated value
                if(strcmp(argv[i],"-h")==0){
                    
                    
                    curArg=atoi(argv[i+1]);
                    HDR_time=curArg;
                    sharedBuffer->timeForHDR=HDR_time;
                    if(argc-i<=1){
                        cout<<"optarg found with no value afterwards in argv"<<endl;
                    }
                    
                }
            }
            if(curArg<0){
                cout<<"# of milliseconds to produce a human driver request should be a number and greater than or equal to zero"<<endl;
                exit(0);
            }

            break;
        case 'a': /* # milliseconds to produce a ride request for an autonomous car */
            optionalCount++;
             for(int i=0;i<argc;i++){//loop through argv to find the associated # of addresses
                if(strcmp(argv[i],"-a")==0){
                    
                    
                    curArg=atoi(argv[i+1]);
                    RDR_time=curArg;
                    sharedBuffer->timeForRDR=RDR_time;
                    //cout<<"a "<<curArg<<" ";
                    if(argc-i<=1){
                        cout<<"optarg found with no value afterwards in argv"<<endl;
                    }
                    
                }
            }
            if(curArg<0){
                cout<<"# of milliseconds to produce a autonomous car request should be a number and greater than or equal to zero"<<endl;
                exit(0);
            }

            break;
        default://
            // if we hit this, an optional argument must have been encountered that doesn't match what should be entered
                    cout<<"b_invalid optional argument detected"<<endl;
                    int badflagerror = BADFLAG;
                    exit(badflagerror); // BADFLAG is an error # defined in a header
                
            }
        }

    //end getting optional arguments

  
    sharedBuffer->curRequestsProduced=0;//initialize current # of requests produced by producers
    //these objects below will be used to start the threads

    pc_unique *costSavingP = new pc_unique;//set up cost saving producer. setting attributes consolidated into one line to minimize clutter
    costSavingP->brokerPtr=sharedBuffer, costSavingP->sleepValue=costSaving_time, costSavingP->consumer=COSTSAVINGFLAG; costSavingP->request=NOTPRODUCER;

    pc_unique *fastMatchingP = new pc_unique;//set up fast matching producer. setting attributes consolidated into one line to minimize clutter
    fastMatchingP->brokerPtr=sharedBuffer, fastMatchingP->sleepValue=fastMatching_time, fastMatchingP->consumer=FASTMATCHINGFLAG; fastMatchingP->request=NOTPRODUCER;

    pc_unique *HDR_C = new pc_unique;//set up human driver request consumer. setting attributes consolidated into one line to minimize clutter
    HDR_C->brokerPtr=sharedBuffer, HDR_C->sleepValue=HDR_time, HDR_C->request=HDRFLAG; HDR_C->consumer=NOTCONSUMER;

    pc_unique *RDR_C = new pc_unique;//set up robot/autonomous driver request consumer. setting attributes consolidated into one line to minimize clutter
    RDR_C->brokerPtr=sharedBuffer, RDR_C->sleepValue=RDR_time, RDR_C->request=RDRFLAG; RDR_C->consumer=NOTCONSUMER;
     
    //initialize semaphores in shared data structure
    int mutex_initialize=MUTEXINITIALIZE;
    int available_slots = AVAILABLESLOTS;
    int human_drivers=HUMANDRIVERS;
    sem_init(&sharedBuffer->bufferMutex,0,mutex_initialize);
    sem_init(&sharedBuffer->availableSlots,0,available_slots);
    sem_init(&sharedBuffer->unconsumed,0,0);
    sem_init(&sharedBuffer->humanDrivers,0,human_drivers);
    sem_init(&sharedBuffer->precedenceConstraint,0,0);
    /*this precedence constraint semaphore ensures the order of execution: the producer threads and consumer threads must finish 
    before the main thread prints the final report*/
    
    pthread_attr_t pthread_attributes;//default pthread attributes
    pthread_t costSavingThread, fastMatchingThread, HDR_Thread , RDR_Thread;
    pthread_attr_init(&pthread_attributes);//populate attributes with defaults
    

    /*create the threads for the producers and consumers with the appropriate objects and names
    
    */

    pthread_create( &HDR_Thread, &pthread_attributes, &producer_thread, (void*) HDR_C);//
    
    pthread_create( &RDR_Thread, &pthread_attributes, &producer_thread, (void*) RDR_C);//
    pthread_create( &costSavingThread, &pthread_attributes, &consumer_thread, (void*) costSavingP);//
    
    pthread_create( &fastMatchingThread, &pthread_attributes, &consumer_thread, (void*) fastMatchingP);//


    sem_wait(&sharedBuffer->precedenceConstraint);
   
    sem_wait(&sharedBuffer->precedenceConstraint);
    
    sem_wait(&sharedBuffer->precedenceConstraint);
    
    //sem_wait(&sharedBuffer->precedenceConstraint);
    //wait until the threads call sem_post on the precedence constraint before continuing
    //if one of the consumer threads consume the last request from the queue, then the main thread will continue and finish, killing the last consumer thread. this solves the issue of the last consumer thread sometimes getting stuck
    
    
    io_production_report((int *)sharedBuffer->curTotalProduced,(int **)sharedBuffer->curTotalConsumed);//print final report of total produced and consumed
    //free up memory to prevent memory leaks
    delete(sharedBuffer);
    delete(HDR_C);
    delete(RDR_C);
    delete(costSavingP);
    delete(fastMatchingP);
    

    
    }
    catch (std::exception const &exc){//handle exceptions caught during running program
        std::cout << "Exception caught "<< exc.what() << "\n";
        std::cerr << "Exception caught "<< exc.what() << "\n";
    }
    catch (...){
        std::cout << "Unknown exception caught\n";
        std::cerr << "Unknown exception caught\n";
    }
}