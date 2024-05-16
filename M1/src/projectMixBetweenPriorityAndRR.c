#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>

struct timeval start_program_time , end_program_time;
double total_exec_time;

void delay(int seconds) {
    time_t start_time = time(NULL);
    time_t current_time;
    do {
        current_time = time(NULL);
    } while (difftime(current_time, start_time) < seconds);
}

void * thread1(){
    struct timeval start_thread_time , end_thread_time;


    struct tm *start_tm, *end_tm;
    char start_str[20], end_str[20];
    //Mark thread start time
    gettimeofday(&start_thread_time,NULL);

    //Converting the timneval to the format YY:MM:DD HH:MM:SS.ss
    start_tm = localtime(&start_thread_time.tv_sec);
    strftime(start_str, sizeof(start_str), "%Y-%m-%d %H:%M:%S", start_tm);

    printf("\nInside FIRST thread. \n");
    printf("Thread Start Time: %s.%06ld\n\n", start_str, start_thread_time.tv_usec);

    printf("\n--------------------------------\n");
    printf("Thread 1 is sleeping for 8 seconds\n");
    // wait(80);
    delay(8);
    printf("Thread 1 woke up\n");
    printf("\n--------------------------------\n");

    //Printing the thread ID
    pthread_t id = pthread_self();
    printf("First Thread ID is %lu \n",id);

    printf("Thread 1 is running on CPU %d\n", sched_getcpu());
    printf("End of THREAD 1\n\n");

    //Mark thread end time
    gettimeofday(&end_thread_time,NULL);

    //Converting the timneval to the format YY:MM:DD HH:MM:SS.ss
    end_tm = localtime(&end_thread_time.tv_sec);
    strftime(end_str, sizeof(end_str), "%Y-%m-%d %H:%M:%S", end_tm);

    printf("Thread End Time: %s.%06ld\n", end_str, end_thread_time.tv_usec);

    //Calculating the time taken by the thread
    double response_time = (end_thread_time.tv_sec - start_thread_time.tv_sec) * 1e6 + (end_thread_time.tv_usec - start_thread_time.tv_usec) ;
    double wait_time = (start_thread_time.tv_sec-start_program_time.tv_sec) * 1e6 + start_thread_time.tv_usec - start_program_time.tv_usec;
    total_exec_time+=response_time;

    printf("\nResponse Time for Thread 1: %f microseconds\n", response_time);
    printf("Wait Time for Thread 1: %f microseconds\n", wait_time);
    printf("\n---------------------------------------\n");

}

void * thread2(){
    struct timeval start_thread_time , end_thread_time;

    struct tm *start_tm, *end_tm;
    char start_str[20], end_str[20];
    //Mark thread start time
    gettimeofday(&start_thread_time,NULL);

    //Converting the timneval to the format YY:MM:DD HH:MM:SS.ss
    start_tm = localtime(&start_thread_time.tv_sec);
    strftime(start_str, sizeof(start_str), "%Y-%m-%d %H:%M:%S", start_tm);

    printf("\nInside SECOND thread. \n");
    printf("Thread Start Time: %s.%06ld\n\n", start_str, start_thread_time.tv_usec);

    printf("\n--------------------------------\n");
    printf("Thread 2 is sleeping for 5 seconds\n");
    // wait(5);
    delay(5);
    printf("Thread 2 woke up\n");
    printf("\n--------------------------------\n");

    //Printing the thread ID
    pthread_t id = pthread_self();
    printf("Second Thread ID is %lu \n",id);

    printf("Thread 2 is running on CPU %d\n", sched_getcpu());
    printf("End of THREAD 2\n\n");

    //Mark thread end time
    gettimeofday(&end_thread_time,NULL);

    //Converting the timneval to the format YY:MM:DD HH:MM:SS.ss
    end_tm = localtime(&end_thread_time.tv_sec);
    strftime(end_str, sizeof(end_str), "%Y-%m-%d %H:%M:%S", end_tm);

    printf("Thread End Time: %s.%06ld\n", end_str, end_thread_time.tv_usec);

    //Calculating the time taken by the thread
    double response_time = (end_thread_time.tv_sec - start_thread_time.tv_sec) * 1e6 + (end_thread_time.tv_usec - start_thread_time.tv_usec);
    double wait_time = (start_thread_time.tv_sec-start_program_time.tv_sec) * 1e6 + start_thread_time.tv_usec - start_program_time.tv_usec;
    total_exec_time+=response_time;

    printf("\nResponse Time for Thread 2: %f microseconds\n", response_time);
    printf("Wait Time for Thread 2: %f microseconds\n", wait_time);
    printf("\n---------------------------------------\n");

}

void * thread3(){
    struct timeval start_thread_time , end_thread_time;

    struct tm *start_tm, *end_tm;
    char start_str[20], end_str[20];
    //Mark thread start time
    gettimeofday(&start_thread_time,NULL);

    //Converting the timneval to the format YY:MM:DD HH:MM:SS.ss
    start_tm = localtime(&start_thread_time.tv_sec);
    strftime(start_str, sizeof(start_str), "%Y-%m-%d %H:%M:%S", start_tm);

    printf("\nInside THIRD thread. \n");
    printf("Thread Start Time: %s.%06ld\n\n", start_str, start_thread_time.tv_usec);

    printf("\n--------------------------------\n");
    printf("Thread 3 is sleeping for 3 seconds\n");
    // wait(3);
    delay(3);
    printf("Thread 3 woke up\n");
    printf("\n--------------------------------\n");


    //Printing the thread ID
    pthread_t id = pthread_self();
    printf("Third Thread ID is %lu \n",id);

    printf("Thread 3 is running on CPU %d\n", sched_getcpu());
    printf("End of THREAD 3\n\n");

    //Mark thread end time
    gettimeofday(&end_thread_time,NULL);

    //Converting the timneval to the format YY:MM:DD HH:MM:SS.ss
    end_tm = localtime(&end_thread_time.tv_sec);
    strftime(end_str, sizeof(end_str), "%Y-%m-%d %H:%M:%S", end_tm);

    printf("Thread End Time: %s.%06ld\n", end_str, end_thread_time.tv_usec);

    //Calculating the time taken by the thread
    double response_time = (end_thread_time.tv_sec - start_thread_time.tv_sec) * 1e6 + (end_thread_time.tv_usec - start_thread_time.tv_usec);
    double wait_time = (start_thread_time.tv_sec-start_program_time.tv_sec) * 1e6 + start_thread_time.tv_usec - start_program_time.tv_usec;
    total_exec_time+=response_time;

    printf("\nResponse Time for Thread 3: %f microseconds\n", response_time);
    printf("Wait Time for Thread 3: %f microseconds\n", wait_time);
    printf("\n---------------------------------------\n");

}

void * thread4(){
    struct timeval start_thread_time , end_thread_time;

    struct tm *start_tm, *end_tm;
    char start_str[20], end_str[20];
    //Mark thread start time
    gettimeofday(&start_thread_time,NULL);

    //Converting the timneval to the format YY:MM:DD HH:MM:SS.ss
    start_tm = localtime(&start_thread_time.tv_sec);
    strftime(start_str, sizeof(start_str), "%Y-%m-%d %H:%M:%S", start_tm);

    printf("\nInside FOURTH thread. \n");
    printf("Thread Start Time: %s.%06ld\n\n", start_str, start_thread_time.tv_usec);


    printf("\n--------------------------------\n");
    printf("Thread 4 is sleeping for 6 seconds\n");
    // wait(6);
    delay(6);
    printf("Thread 4 woke up\n");
    printf("\n--------------------------------\n");


    //Printing the thread ID
    pthread_t id = pthread_self();
    printf("Fourth Thread ID is %lu \n",id);

    printf("Thread 4 is running on CPU %d\n", sched_getcpu());
    printf("End of THREAD 4\n\n");

    //Mark thread end time
    gettimeofday(&end_thread_time,NULL);

    //Converting the timneval to the format YY:MM:DD HH:MM:SS.ss
    end_tm = localtime(&end_thread_time.tv_sec);
    strftime(end_str, sizeof(end_str), "%Y-%m-%d %H:%M:%S", end_tm);

    printf("Thread End Time: %s.%06ld\n", end_str, end_thread_time.tv_usec);

    //Calculating the time taken by the thread
    double response_time = (end_thread_time.tv_sec - start_thread_time.tv_sec) * 1e6 + (end_thread_time.tv_usec - start_thread_time.tv_usec);
    double wait_time = (start_thread_time.tv_sec-start_program_time.tv_sec) * 1e6 + start_thread_time.tv_usec - start_program_time.tv_usec;
    total_exec_time+=response_time;


    printf("\nResponse Time for Thread 4: %f microseconds\n", response_time);
    printf("Wait Time for Thread 4: %f microseconds\n", wait_time);
    printf("\n---------------------------------------\n");

}

int main(){
    
    pthread_attr_t attr;
    pthread_t thread;

    // Initialize thread attributes object
    pthread_attr_init(&attr);

    // Set scheduling policy and scope
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    // Set scheduling priority
    struct sched_param param;
    param.sched_priority = 99; // Set priority to maximum value
    pthread_attr_setschedparam(&attr, &param);

    // Set CPU affinity to one core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);

    //Declaring threads
    pthread_t ptid1, ptid2, ptid3, ptid4;

    //Marling the start of our program
    gettimeofday(&start_program_time,NULL);

    //Creating Threads
    pthread_create(&ptid1, &attr, &thread1, NULL);
    pthread_create(&ptid2, &attr, &thread2, NULL);
    pthread_create(&ptid3, &attr, &thread3, NULL);
    pthread_create(&ptid4, &attr, &thread4, NULL);

    //We join here just to assure that all threads finished execution before terminating the program
    pthread_join(ptid1, NULL);
    pthread_join(ptid2, NULL);
    pthread_join(ptid3, NULL);
    pthread_join(ptid4, NULL);

    //Marking the end of our program
    gettimeofday(&end_program_time,NULL);

    //Calculating the time taken by the program
    double response_time = (end_program_time.tv_sec - start_program_time.tv_sec) * 1e6 + (end_program_time.tv_usec - start_program_time.tv_usec);
    printf("\nTime for the program to execute: %f microseconds\n", response_time);

    //Calculate useful work done by cpu
    printf("\nUsefull work is %f\n", total_exec_time/response_time);
    




}