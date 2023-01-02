#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mpi.h"
#include "omp.h"
#include <sys/time.h>

#define FILE_NAME "../resources/input.dat"
#define ROOT 0
#define SLAVE 1
#define WORK_TAG 1
#define STOP_TAG 0
#define NUM_OF_THREADS 2

double *readFromFile(const char *filename, int *N);
double calculateBElement(double elementOfA, int kMaxVal);
void slave(int kMaxVal);
double master(int kMaxVal);

int main(int argc, char *argv[]) {
	int numProcs, rank, MAX, i, iterations;
	double finalResult = 0, startTime, endTime;

	if (argc >= 2)
	{
		MAX = atoi(argv[1]);
		if (MAX < 1)
		{
			printf("Expected a positive number for MAX, recieved %s", argv[1]);
			MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		}
		iterations = atoi(argv[2]);
		if (iterations < 1)
		{
			printf("Expected a positive number for iterations, recieved %s", argv[1]);
			MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		}
	}
	else {
		printf("Please enter a number.\n");
		exit(1);
	}

	omp_set_num_threads(NUM_OF_THREADS);
	MPI_Init(&argc, &argv); //initialize the processes
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (numProcs != 2)
	{
		printf("Number of processes must be 2.\n");
		exit(1);
	}
	startTime = MPI_Wtime();
	for(i = 0; i < iterations; i++)
	{
		if (rank == ROOT)
		{
			finalResult = master(MAX);
		}
		else {
			slave(MAX);
		}
	}
	if (rank == ROOT)
	{
		endTime = MPI_Wtime();
		printf("answer = %e\n", finalResult);
		printf("Average Time measured: %.3f seconds.\n", ((endTime - startTime)/iterations));
	}
	
	MPI_Finalize(); //end process
	return EXIT_SUCCESS;
}

double *readFromFile(const char *filename, int *N) {

	FILE *fp;
	double *numbers;

	if ((fp = fopen(filename, "r")) == NULL)
	{
		printf("cannot open file %s for reading\n", filename);
		return NULL;
	}

	if (fscanf(fp, "%d", N) != 1)
	{
		printf("Problem reading number of variables\n");
		return NULL;
	}

	numbers = (double*) malloc(*N * sizeof(double));
	
	if (numbers == NULL)
	{
		printf("Problem to allocate memory to numbers array \n");
		return NULL;
	}

	for (int i = 0; i < *N; i++)
	{
		if (fscanf(fp, "%lf", &numbers[i]) != 1)
		{
			printf("Problem reading double number: %d\n", i);
			return NULL;
		}
	}

	fclose(fp);
	return numbers;
}

double master(int kMaxVal)
{
	MPI_Status status;
	double result, tempAnswer , *values;
	int amountForSlave, splitOn, i, N;

	values = readFromFile(FILE_NAME, &N);//does it change the pointer?

	amountForSlave = splitOn = (N / 2);
	if((N%2) != 0)// if not an even number the slave will handle one more value
	{
		amountForSlave += 1;
	}

	MPI_Send(&amountForSlave, 1, MPI_INT, SLAVE, WORK_TAG, MPI_COMM_WORLD);//send number of values that will be recived
	MPI_Send(&values[splitOn], amountForSlave, MPI_DOUBLE, SLAVE, WORK_TAG, MPI_COMM_WORLD);//send the values

	#pragma omp parallel shared(values, splitOn, i) reduction(min:tempAnswer)
		{
			double temp;
			#pragma omp for
			for(i = 0; i < splitOn; i++)
			{
				temp = calculateBElement(values[i], kMaxVal);
				if(!tempAnswer)
				{
					tempAnswer = temp;
				}else{
					tempAnswer = tempAnswer <= temp ? tempAnswer : temp; //min(tempAnswer, temp);
				}
			}
		}
	MPI_Reduce(&tempAnswer, &result, 1, MPI_DOUBLE, MPI_MIN, ROOT, MPI_COMM_WORLD);
	free(values);
	return result;
}

void slave(int kMaxVal)
{
	MPI_Status status;
	double result, tempAnswer, *values;
	int N, i;

	MPI_Recv(&N, 1, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);//amount of values to be recived

	values = (double*) malloc(N * sizeof(double));
	MPI_Recv(values, N, MPI_DOUBLE, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);//recive values

	#pragma omp parallel shared(values, N, i) private(tempAnswer) reduction(min:result)
		{
			#pragma omp for
			for(i = 0; i < N; i++)
			{
				tempAnswer = calculateBElement(values[i], kMaxVal);
				if(!result)
				{
					result = tempAnswer;
				}else{
					result = result <= tempAnswer ? result : tempAnswer; //min(result, tempAnswer); //do min between this thread result and then reduction between all threads
				}
			}
		}
	MPI_Reduce(&result, NULL, 1, MPI_DOUBLE, MPI_MIN, ROOT, MPI_COMM_WORLD);
	free(values);
}

double calculateBElement(double elementOfA, int kMaxVal){
	double res;
	for(int k = 0; k < kMaxVal; k++)
	{
		double tempRes = sin(k * exp(sin(elementOfA * k)));
		if(k == 0)
		{
			res = tempRes;
		}else{
		res = res <= tempRes ? res : tempRes; //min(res, tempRes);
		}
	}
	return res;
}
