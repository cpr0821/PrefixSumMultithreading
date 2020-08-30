#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h> 
#include <errno.h> 
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

// Function to print an int array 
void printArray (int arr[], int size) 
{
	for(int i = 0; i < size; i++)
	{
		cout << array[i] << " ";
	}
	cout << endl;
}

// Prefix sum -> Requires sharedArray, tempArray, number of elements and number of processors to do the work
int prefixSum(int* sharedArray, int* tempArray, int numElements, int numProcessors) {

	int logProc = ((int) ceil(log(numElements) / log(2))) - 1;	// Calculate the number of iterations 
	int loopSize = numElements / numProcessors;			// Calculate the number of internal loops
	int i, j, k, neighborDistance, pid, segment_id, sem_id;		
	
	k = k % numProcessors;	// Divide k to give each a unique ID
	
	// Outter iteration loop
	for(i = 0; i <= logProc; i++) {
		// Inner loop iterates from 0 -> loopSize
		for(j=0; j < loopSize; j++) {
						
			int index = k * loopSize + j;	// Adjusting index for elements

			if(index < pow(2, i)) {
				tempArray[index] = sharedArray[index];
			} else {
				neighborDistance = index - pow(2, i);	// Calculating distance of neighbor from current index
				tempArray[index] =  sharedArray[neighborDistance] + sharedArray[index];
			}
		}
  }
		// Swap temp and shared arrays 
		int* temp = sharedArray;
		sharedArray = tempArray;
		tempArray = temp; 
}

int main (int argc, char* argv[])
{
	int sizeofArray = atoi(argv[1]);
	ifstream input(argv[2]);
	ofstream output(argv[3]);
	int numProcessors = atoi(argv[4]);
	int i, numInput;
		
	int originalArray [sizeofArray];
	int* tempArray;
	int* sharedArray;

	// If the file is open, count number of integers. Else print error statement and end 		program.
   	if (input.is_open()) {
      		while (!input.eof()) {
         		input >> i;
			numInput++;
		}
      	} else {
		cout << "Unable to open file." << endl;
		return -1;	
	}

	// If the number of integers in the file does not equal the number indicated in the 		command line, print error report and end program
   	if (numInput != sizeofArray) {
      		cout << "The file does not contain exactly n integers.";
     		return -1;
   	}

	// Move file pointer back to beginning and clear flags (eof)
	input.seekg(0);
	input.clear();

	// Put the integers from the file into the array
	for (i = 0; i < sizeofArray; i++)
	{
		input >> originalArray[i];
	}

	// Close input file
	input.close();

	//Populating arrays
	for (i = 0; i < sizeofArray; i++)
	{
		sharedArray[i] = originalArray[i];
		tempArray[i] = originalArray[i];
	}

	// Calls the prefix sum function on the array passing the two arrays in shared memory 
	// (one containing original values), the size of the arrays and the number of processors
	// being used in parallel to compute the prefix sum

	//prefixSum(sharedArray, tempArray, sizeofArray, numProcessors);
	
	cout << "Processing complete: Results written to output.txt" << endl;
	
	int logProc = ((int) ceil(log(sizeofArray) / log(2))) -1;	// Calculating the number of iterations

	// Storing the array into an output file
	if (logProc % 2 == 1)
	{	// Case where sharedArray is final array
		for(i = 0; i < sizeofArray; i++)
		{	
			if (i == sizeofArray - 1)
			{
				output << sharedArray[i] << endl;
			} else
			{
				output << sharedArray[i] << " ";
			}
		}
	} else // Case where tempArray is final array
	{
		for(i = 0; i < sizeofArray; i++)
		{
			if(i == sizeofArray -1)
			{
				output << tempArray[i] << endl;
			} else
			{
				output << tempArray[i] << " ";
			}
		}
	}

	// Close the output file
	output.close();

	return 0; 
}

