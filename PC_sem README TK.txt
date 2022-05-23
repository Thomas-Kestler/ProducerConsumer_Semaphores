pc_semaphores readme by Thomas Kestler
file description
BufferADT - implementation of a shared data structure to be shared among the producer and consumer threads
consumer - implements the consumer thread function to be shared by two consumers, one fast matching and one cost saving
io - files provided by SDSU to help format the output, not created by me
main - handles main thread and creation of consumer/producer threads, as well as the synchronization of the order of execution and ensures the producer threads stop before the consumer threads, and prints the final output
makefile
pc_unique - struct for unique consumers and producers with attributes telling the thread functions which request type they are handling and which consumer/producer type called the function
producer - implements producer thread to be shared with both producers, human and robot request producers
ridesharing - file provided by SDSU with flags to assist pc_unique and shared data structure