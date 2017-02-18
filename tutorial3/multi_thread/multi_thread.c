#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define BUF_SIZE 4096
#define THREAD_NUM 3

// The thread argument struct
struct thread_argv{
	/**
	 * TO-DO Task 2
	 * Finish the definition of this thread argument struct
	 */
    int row;
    int start;
    int end;
    double** inputdata;
    double* outputmean;
};

void *calc_mean(void *_tmp_argv){
	/**
	 * TO-DO Task 3
	 * Complete this thread function
	 */
    int newstart=((struct thread_argv*)_tmp_argv)->start;
    int newend=((struct thread_argv*)_tmp_argv)->end;
    int newrow=((struct thread_argv*)_tmp_argv)->row;
    double** newinputdata=((struct thread_argv*)_tmp_argv)->inputdata;
    double* newoutputmean=((struct thread_argv*)_tmp_argv)->outputmean;
    int i=0,j=0;
    for (j=newstart; j<=newend; j++) {
        for (i=0; i<newrow; i++) {
            newoutputmean[j]+=newinputdata[i][j]/newrow;
        }
    }
    return;
}

int main(){
	/**
	 * Code for reading the dataset, Please do not modify
	 */

    FILE *fp;
    
    int sample_num = 0;
    int attribute_num = 0;
        
	char buffer[BUF_SIZE];

	// Open the file and create a file descriptor
    fp = fopen("dataset.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
        
    // Get the number of samples    
	fgets(buffer, sizeof(buffer), fp);
	sample_num = atoi(buffer);

	// Get the number of attributes
	fgets(buffer, sizeof(buffer), fp);
	attribute_num = atoi(buffer);
	
	// A 1D array for storing if the mean of the attribute is calculated
	int *evaluated = (int *) malloc(attribute_num * sizeof(int));
	
	// A 1D array for storing the mean of every attribute
	double *result = (double *) malloc(attribute_num * sizeof(double));

	// A 2D array for storing the dataset
	double** data = (double **) malloc(sample_num * sizeof(double *));

	// Read the data from file
	for (int s_idx = 0; s_idx < sample_num; s_idx++){
		data[s_idx] = (double *) malloc(sample_num * sizeof(double));
		fgets(buffer, sizeof(buffer), fp);
		
		char *buf_ptr = buffer;
		char *data_ptr = NULL;
		for (int a_idx = 0; a_idx < attribute_num; a_idx++, buf_ptr = NULL){
			char* token = strtok_r(buf_ptr, ",\n", &data_ptr);
			
			data[s_idx][a_idx] = atof(token);
	    }
	}
	
	// Variable Initialization
	for (int a_idx = 0; a_idx < attribute_num; a_idx++){
		evaluated[a_idx] = 0;
		result[a_idx] = 0;	
	}
	/**
	 * End of code for reading the dataset
	 */

    
    
	/**
	 * TO-DO Task 1
	 * Add a pthread mutex lock for variable “evaluated”
	 */
    pthread_mutex_t for_evaluated;

	// Thread ID array
	pthread_t calc_thread_id[THREAD_NUM];

	/**
	 * TO-DO Task 4
	 * Add statement here to creating the thread
	 */
    struct thread_argv* thread_argument1=(struct thread_argv*)malloc(sizeof(struct thread_argv));;
    thread_argument1->start=0;
    thread_argument1->end=1;
    thread_argument1->row=sample_num;
    thread_argument1->inputdata=data;
    thread_argument1->outputmean=result;
    struct thread_argv* thread_argument2=(struct thread_argv*)malloc(sizeof(struct thread_argv));;
    thread_argument2->start=2;
    thread_argument2->end=4;
    thread_argument1->row=sample_num;
    thread_argument1->inputdata=data;
    thread_argument1->outputmean=result;
    struct thread_argv* thread_argument3=(struct thread_argv*)malloc(sizeof(struct thread_argv));
    thread_argument3->start=5;
    thread_argument3->end=7;
    thread_argument1->row=sample_num;
    thread_argument1->inputdata=data;
    thread_argument1->outputmean=result;
    pthread_create(&calc_thread_id[0],NULL,calc_mean,(void*)thread_argument1);
    pthread_create(&calc_thread_id[1],NULL,calc_mean,(void*)thread_argument2);
    pthread_create(&calc_thread_id[2],NULL,calc_mean,(void*)thread_argument3);
    
    

	/**
	 * TO-DO Task 5
	 * Add statement here to join thread in the main thread, to ensure that every thread is terminated before printing the results
	 */
    pthread_join(calc_thread_id[0],NULL);
    pthread_join(calc_thread_id[1],NULL);
    pthread_join(calc_thread_id[2],NULL);
    

	// Print the output
	for (int a_idx = 0; a_idx < attribute_num; a_idx++)
		printf("Mean of attribute %d is: %f\n", a_idx, result[a_idx]);

    return 0;
}