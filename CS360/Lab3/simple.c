#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <queue>
#include <iostream>
#include <semaphore.h>

sem_t full, empty, mutex;

class myqueue {
	std::queue<int> stlqueue;
	public:
	void push(int sock) {
		sem_wait(&empty);
		sem_wait(&mutex);
		stlqueue.push(sock);
		sem_post(&mutex); //increments and wakes up the waiting processes
		sem_post(&full);
	}
	int pop() {
		sem_wait(&full);
		sem_wait(&mutex);
		int rval = stlqueue.front();
		stlqueue.pop();
		sem_post(&mutex);
		sem_post(&empty);
		return(rval);
	}

} sockqueue;

void *printHello(void *arg) {

// for(;;) {
//   get socket from the queue
//   read request
//   respond }
	for(;;) {
		std::cout <<"Got "<<sockqueue.pop()<<std::endl;
	}
}

main() {

#define NTHREADS 10
#define NQUEUE 20
	pthread_t thread[NTHREADS];

	sem_init(&full, PTHREAD_PROCESS_PRIVATE, 0);
	sem_init(&empty, PTHREAD_PROCESS_PRIVATE, NQUEUE);
	sem_init(&mutex, PTHREAD_PROCESS_PRIVATE, 1);

	for(int i = 0; i < 10; i++) {
		sockqueue.push(i);
	}
	for(long threadid = 0; threadid < NTHREADS; threadid++) {
		pthread_create(&thread[threadid], NULL, printHello, (void*) threadid);
	}
	
	/*for(int threadid = 0; threadid < NTHREADS; threadid++) {
		pthread_create(&thread[threadid], NULL, printHello, (void*)threadid);
	}*/

// Set up socket, bind, listen
// 	for(;;) {
//		socket = accept
//		Put socket in queue }

	pthread_exit(NULL);
}
