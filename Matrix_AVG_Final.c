// Randall Harper and John Boling
// CS4379
// 2/2/2016
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "mpi.h"

#define MAX_ROWS 600
#define MAX_COLUMNS 600

int main (int argc, char *argv[]){
	
	int array_avg[MAX_ROWS][MAX_COLUMNS];
	int source, dest, offset, abort;
	int partition_size;	
	
	float column_holding[MAX_COLUMNS]; // Temporary buffer to average columns together from different processes
	float row_avgs[MAX_ROWS]; // Array of Final averages
	float column_avgs[MAX_COLUMNS]; //Array of Final averages
	
	float row_total;
	float col_total;
	
	MPI_Status status;
	
	//Initialize
	MPI_Init( &argc, &argv);
	
	//Get the number of processes
	int num_processes;
	MPI_Comm_size( MPI_COMM_WORLD, &num_processes);
	
	//Check if num_processes is divisible by 12
	if( 12 % num_processes != 0) {
		printf("Abort. MPI processes must be divisible by 12.\n");
		MPI_Abort( MPI_COMM_WORLD, abort);
		exit(0);
	}

	//Get the rank of processes
	int process_rank;
	MPI_Comm_rank( MPI_COMM_WORLD, &process_rank);
	printf("MPI process %d has started ... \n", process_rank);
	partition_size = ( MAX_ROWS / num_processes); // this determines data how much data is sent to each process
	offset = partition_size;
		
	//Process 0 (master)
	if( process_rank == 0)
	{
		//Populate array with random numbers
		srand(time(NULL));
		for (int i = 0; i < 600; i++)
		{
			for (int j = 0; j < 600; j++)
			{
				int r = rand() % 1000;
				array_avg[i][j] = r;
			}			
		}		
		
		if(num_processes>1)
		{
			//Send data blocks to other processes
			for( dest = 1; dest < num_processes; dest++)
			{
				MPI_Send( &array_avg[offset*dest], partition_size*600, MPI_FLOAT, dest, MPI_ANY_TAG, MPI_COMM_WORLD);
			}				

			//Receive data from other processes
			for( int i = 1; i < num_processes; i++)
			{
				source = i;
				MPI_Recv( &row_avgs[offset*source], partition_size, MPI_FLOAT, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			}
		}
	}	
	
	//Work section
	//Recv allocated partition from master
	if(process_rank>0)
	{
		source = 0;
		MPI_Recv( &array_avg[offset], partition_size*600, MPI_FLOAT, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}
	
	//Row averages
	for (int i = (offset*process_rank); i<(offset*(process_rank + 1)); i++)
	{
		row_total = 0;
		for (int j = 0; j < 600; j++)
		{
			row_total += array_avg[i][j];
		}
		row_avgs[i] = row_total / 600;
	}

	//Column averages
	for(int i=0; i<600;i++)
	{
		col_total=0;
		for(int j=(offset*process_rank);j<(offset(process_rank+1));j++)
		{
			col_total+=array_avg[j][i];
		}
		column_avgs[i]=(col_total/offset);
	}
	
	//Column totalling
	if(process_rank>0)
	{
		source = process_rank-1;
		MPI_Recv( &column_holding[0], 600, MPI_FLOAT, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		for(int i=0;i<600;i++)
		{
			column_avgs[i]=(column_avgs[i]+column_holding[i])/2;
		}
	}
	
	//Send column_avgs to next process to calc averages
	if(process_rank+1!=num_processes)
	{
		dest=process_rank+1;
		MPI_Send( &column_avgs[0], 600, MPI_FLOAT, dest, MPI_ANY_TAG, MPI_COMM_WORLD);
	}
	
	//If last process, send to process rank 0
	if(process_rank+1==num_processes)
	{
		MPI_Send( &column_avgs[0], 600, MPI_FLOAT, 0, MPI_ANY_TAG, MPI_COMM_WORLD);
	}
	
	//At end recieve from last process to have final column averages
	if(process_rank==0)
	{
		MPI_Recv( &column_avgs[0], 600, MPI_FLOAT, num_processes-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}		
	
	//Send row averages back to Process Rank 0
	if(process_rank>0)
	{
		dest = 0;
		MPI_Send( &row_avgs[offset*process_rank], partition_size, MPI_FLOAT, dest, MPI_ANY_TAG, MPI_COMM_WORLD);
	}
	
	MPI_Finalize();
}
	
	
