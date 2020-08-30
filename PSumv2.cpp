#include <stdio.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h> 
#include <errno.h>
#include <string>
#include <iostream>
#include <fstream>
#include <fcntl.h>

int *sharedArray;
int *tempArray;
int sizeofArray = 0;
int numElements = 0;
int numProcessors = 0;
int iterations = 0;
int internalLoopSize = 0;
int numSwaps = 0;
pthread_barrier_t tBarriers;

using namespace std;

// Function to print an int array (for testing puposes with threads)
void* vprintArray (void* array) 
{
	int* arr = (int *) array;
	cout << "Thread printing:" << endl;
	
	for(int i = 0; i < numElements; i++)
	{
		cout << arr[i] << " ";
	}

	printf("\n");

	return NULL;
}

void* vPrefixSum (void* val)
{
	// Assign each thread a unique Thread id to calculate indexes
	int* threadNum = (int*) val;

	int neighborDistance = 0;

	//Outter iteration loop
	for(int i = 0; i <= iterations; i++) {
		// Inner loop iterates from 0 -> loopSize
		for(int j=0; j < internalLoopSize; j++) {
			int index = *threadNum * internalLoopSize + j;	// Adjusting index for elements
		  // Break if bigger than index in the case of an unused thread
			if(index >= sizeofArray)
			{
				break;
			}

			if(index < pow(2, i)) {
				tempArray[index] = sharedArray[index];
			} else {
				neighborDistance = index - pow(2, i);	// Calculating distance of neighbor from current index
				tempArray[index] =  sharedArray[neighborDistance] + sharedArray[index];
			}
		}

		// Thread barrier to wait for all threads to finish their work
		pthread_barrier_wait(&tBarriers);

		// Thread with ID 0 swapping pointers to shared and temp arrays for next iteration
		if (*threadNum == 0) {
			int* temp = sharedArray;
			sharedArray = tempArray;
			tempArray = temp;
		} 		
		
		// Thread barrier to wait on thread with ID 0 to finish swapping pointers  
		pthread_barrier_wait(&tBarriers);
	}
	return NULL;
}

// Function to test if semaphores are working 
/*
void* testSemaphore(void* arg)
{
	sem_wait(&mutex);
	cout << "Entered " << endl;
	sleep(4);
	cout << "Exiting " << endl;
	sem_post(&mutex);
}
*/
void printArray(int *arr, int size)
{
	for(int i = 0; i < size; i++)
	{
		cout << arr[i] << " ";
	}
}

void printUsage(){
  cout << "Usage:" << endl << "./a.out [number of elements] [input.txt] [output.txt] [number of threads]" << endl; 
}

int main (int argc, char* argv[])
{
	numElements = atoi(argv[1]);
  ifstream input(argv[2]);
	ofstream output(argv[3]);
  numProcessors = atoi(argv[4]);
	
  // Make sure the number of elements and number of threads is greater than 0
  if(numElements <= 0 || numProcessors <= 0){
    printUsage();
    return 1;
  }
	
	int numInput = 0, i = 0, numRead = 0, k;

	internalLoopSize = (int) ceil((float) numElements / numProcessors);
	sizeofArray = numElements + (internalLoopSize - (numElements % internalLoopSize));
	
	pthread_t prefixSumThreads[numProcessors];		// Initiliaze an array of threads
	int ret;

  // Check command line numofelements with actual numofelements
	if (input.is_open()) 
	{
		while (!input.eof())
		{
			input >> i;
			numInput++;
		}
	} else {
		cout << "Unable to open input file." << endl;
    printUsage();
		return 1;
	}

	cout << "Numinput: " << numInput << endl;
	if (numInput != numElements)
	{
		cout << "The file does not contin exactly n integers.\n";
    printUsage();
		return 1;
	}

  // Go back to beginning of input file
	input.clear();
	input.seekg(0);
	
	// Dynamically allocating shaared and temp arrays
	sharedArray = new int[sizeofArray];
	tempArray = new int [sizeofArray];

	// Populating shared and temp arrays
	for(int i = 0; i < sizeofArray; i++)
	{
		if(i < numElements){
			input >> numRead;
		} else {
			numRead = 0;
		}

		sharedArray[i] = numRead;
		tempArray[i] = numRead;
	}

	input.close();

	// Setting prefix sum algorithm loop sizes 
	iterations = ((int) ceil(log(sizeofArray) / log(2))) - 1;	// Calculate the number of iterations 

	int tIDs [numProcessors];

	ret = pthread_barrier_init(&tBarriers, NULL, numProcessors);

	if(ret) {

		cerr << "Error initializing Thread Barrier" << endl;
		exit(1);
	}

	for (int i = 0; i < numProcessors; i++)
	{
		tIDs[i] = i;													
		pthread_create(&prefixSumThreads[i], NULL, vPrefixSum, &tIDs[i]);		// Creating threads t execute prefix sum function and passing unique ID from tIDs array	
	}
	
	// Joining threads/waiting for threads to finish 
	for (int i = 0; i < numProcessors; i++) {
		pthread_join(prefixSumThreads[i], NULL);
	}

  // Output final answers to output file
	if(output.is_open()){
    for(int i = 0; i < numElements - 1; i++){
      output << sharedArray[i] << " ";
    }
    output << sharedArray[numElements - 1] << endl;  
  } else {
    cout << "Unable to open output file" << endl;
    printUsage();
    return 1;
  }
  
  // Close and destroy before ending program
  output.close();
	pthread_barrier_destroy(&tBarriers);

	return 0; 
}

