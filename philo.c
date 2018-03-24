/*
   John Karasev
   CS 360 Systems Programming
   WSUV Spring 2018
   -----------------------------------------------------
   Assignment #7:
   Simulate philosphers diner using threads.
*/


#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
// for eating times
#define STDDEV_EAT 3
#define MEAN_EAT 9

// for thinking times
#define STDDEV_THNK 7
#define MEAN_THNK 11

// keeps track of philosphers info
typedef struct infoPhil {
	unsigned int id;
	int chopsticks[2];
	int thinktime;
	int eattime;
	int cycles;
	bool isAlive;
} pinfo;

// will be passed to the 5 threads
static int t_id[5];

static pinfo threadPhil[5]; // will be used in the 5 threads to keep track of info

static pthread_mutex_t chopsticks[5]; // chopstick mutex

//prints results from the philosphers diner at the end
void printinfo() {
	printf( "\n\n" );
	for ( int i = 0; i < 5; i++ )
		printf( "philospher %d thinktime=%d eattime=%d cycles=%d\n",
		        threadPhil[i].id,  threadPhil[i].thinktime, threadPhil[i].eattime, threadPhil[i].cycles );
	printf("\n");
}

int randomGaussian_r(int mean, int stddev, unsigned int* state) {
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand_r(state) / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand_r(state) / (double) RAND_MAX;
	if (rand_r(state) & (1 << 5))
		return (int) floor(mu + sigma * cos(f2) * f1);
	else
		return (int) floor(mu + sigma * sin(f2) * f1);
}

void lifeOfPi( void* ptr ) {

		unsigned int id = *( (unsigned int *)ptr ); //cast back to int* to find philosher id

		pinfo *info = &threadPhil[id]; //get the struct that is associated with the philospher id

		info->id = id;

		id++; //for seed on gaussian (cannot be 0 so increment by 1 to avoid that )


		//assign the correct chopsticks to each philospher.
		info->chopsticks[0] = ( info->id + 4 ) % 5,
		info->chopsticks[1] = info->id;

		//loop until philospher ate for 100 seconds.
		for ( info->eattime = info->thinktime = info->cycles =  0; info->eattime < 100; info->cycles++ ) {
			int eatTime;
			int thinkTime;

			// wait for the two chopsticks.
			//try to get the two chopsticks at the same time otherwise wait and try again.
			while( true ) {
				pthread_mutex_lock( &chopsticks[info->chopsticks[0]] );
				int result = pthread_mutex_trylock( &chopsticks[info->chopsticks[1]] );
				if ( result == 0 ) break;
				if ( result != EBUSY ) {
					fprintf( stderr, "Error on trylock \n");
					exit( 1 );
				}
				pthread_mutex_unlock( &chopsticks[info->chopsticks[0]] );
				sleep(1); // wait for 1 sec to avoid busy loops.
			}


			// get randomGaussian number for eating.
			eatTime = abs( randomGaussian_r(  MEAN_EAT, STDDEV_EAT, &id ) );
			printf( "philID: %d eattime: %d, (total eat:%d)\n", info->id, eatTime, info->eattime );
			sleep( eatTime );
			info->eattime += eatTime;

			//put chopsticks back on the table.
			pthread_mutex_unlock( &chopsticks[info->chopsticks[0]] );
			pthread_mutex_unlock( &chopsticks[info->chopsticks[1]] );

			// After philospher is satisfied, he thinks for a random time.
			thinkTime = abs( randomGaussian_r( MEAN_THNK, STDDEV_THNK, &id ) );
			printf( "philID: %d thinktime: %d (total think: %d)\n", info->id, thinkTime, info->thinktime );
			info->thinktime += thinkTime;
			sleep( thinkTime );

		}
		// after philosher has eaten for 100 seconds, he gets killed.
		printf( "philospher %d is dead\n", info->id );

		return;
}

int main () {

	pthread_t threads[5]; // allocate threads for 5 philosphers

	for ( int i = 0; i < 5; i++ ) // initialize the mutex (chopsticks)
		pthread_mutex_init( &chopsticks[i], NULL );

	for ( int i = 0; i < 5; i++ ) { // set id to the pointer that will be passed and create threads
		t_id[i] = i;
		pthread_create( &threads[i], NULL, ( void* ) lifeOfPi, &t_id[i] );
	}

	for ( int i = 0; i < 5; i++ )  // start the threads
		pthread_join( threads[i], NULL);

	printinfo(); // print philosphers info

	return 0;
}
