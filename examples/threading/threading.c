#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...) printf("[DEBUG]: " msg "\n", ##__VA_ARGS__)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    // Obtain the threadfunc args in the thread_data type structure
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    DEBUG_LOG("Wait for wait_to_obtain_ms duration seconds");
    sleep( (int)( (thread_func_args -> wait_to_obtain_ms) / 1000) ); //Since sleep takes arguments in seconds
    DEBUG_LOG("Obtain mutex");
    int rc = pthread_mutex_lock(thread_func_args -> mutex);
    if (rc == -1){
    	ERROR_LOG("Mutex lock not achieved!");
    	thread_func_args -> thread_complete_success = false;
    }
    else{
    	DEBUG_LOG("Mutex lock successful!");
    	DEBUG_LOG("Wait to release");
    	sleep( (int)( (thread_func_args -> wait_to_release_ms) / 1000) );
    	DEBUG_LOG("Release mutex");
    	int rc1 = pthread_mutex_unlock(thread_func_args -> mutex);
    	if (rc1 == -1){
    		ERROR_LOG("Mutex unlock not achieved!");
    		thread_func_args -> thread_complete_success = false;
    	}
    	else{
    		DEBUG_LOG("Mutex released successfully!");
    		thread_func_args -> thread_complete_success = true;
    	}
    }
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    DEBUG_LOG("Allocating dynamically memory for thread_data fields, using malloc");
    struct thread_data * thread_args = (struct thread_data *)malloc(sizeof(struct thread_data)); // Pointer to thread_data structure
    
    DEBUG_LOG("Checking threads data memory allocated");
    if (thread_args == NULL){
    	ERROR_LOG("Memory could not be allocated for thread args!");
    	return false;
    }
   
    DEBUG_LOG("Setup mutex and wait arguments");
    thread_args -> mutex = mutex;
    thread_args -> wait_to_obtain_ms = wait_to_obtain_ms;
    thread_args -> wait_to_release_ms = wait_to_release_ms;
    
    DEBUG_LOG("Create thread");
    int thread_rc = pthread_create(thread,
                          NULL,
                          threadfunc,
                          thread_args);
    if (thread_rc == -1){
    	ERROR_LOG("Thread could not be created!");
    	return false;
    }
    else{
    	DEBUG_LOG("Thread created successfully!");
    	return true;
    }
}

