#include <raylib.h>
#include <unistd.h>
#include <math.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <rlgl.h>
#include <string.h>
#include <limits.h>
#include "main.h"
/////
static int SCREENW; //These contain absolue maxes for W and H
static int SCREENH;

void init_boid(Boid * ptr) {
	// Set a random initial x and y coordinate position
	ptr -> x = ((float)rand() / (float)RAND_MAX) * SCREENW; // << 5; //bit shift left for precision
	ptr -> y = ((float)rand() / (float)RAND_MAX) * SCREENH; // << 5;
	return;
}

///////////////////////////////////////////////////////////////////////////////
// Where the BOID calculations are actually done
// INPUT:
//	a pointer to the current boid being updated
//	a pointer to the heap allocated array of boids
// OUTPUT:
//	None
///////////////////////////////////////////////////////////////////////////////

///**
void update_boid(Boid * ptr, Bucket * buckets, int b) {
//**/
/**
void update_boid(Boid * source, Boid * start, int count) {
**/
	

	// Capture start of bucket this boid exists in:
	Boid * arr = buckets[b].boids;
	int limit = buckets[b].count;
	

	float x = ptr -> x;
	float y = ptr -> y; 
	float xv = ptr -> xv; 
	float yv = ptr -> yv;
	float otherx;
	float othery;
	float otherxv;
	float otheryv;

	float mag_dist;

	int count = 1;
	float sep_force;
	Vector2 distance;
	Vector2 pos = { x, y };
	Vector2 vel = { xv, yv };
	Vector2 sum_pos = pos;
	Vector2 sum_vel = vel;
	Vector2 other_pos;
	Vector2 other_vel;

	for (int i = 0; i < limit; i++) {
		
		if (&arr[i] != ptr) { //Ensure a Boid does not check itself
			otherx = arr[i].x;
			othery = arr[i].y;
			otherxv = arr[i].xv;
			otheryv = arr[i].yv;
			other_pos = (Vector2){ otherx, othery };
			other_vel = (Vector2){ otherxv, otheryv };
	
	

	
			distance = Vector2Subtract(other_pos, pos);
			mag_dist = Vector2Length(distance);

			if (mag_dist < RANGE) {
				count += 1;
				sum_pos = Vector2Add(sum_pos, other_pos);
				sum_vel = Vector2Add(sum_vel, other_vel);

				// Separation Inverse
			
				// If the distance is not acceptable, do not calculate for sep - this check can be removed since mag dist is offset from 0, and max distance is determined by spatial hashing
				if (mag_dist < 90.0f && mag_dist > 0.0f) {
					sep_force = (10.0f / (0.01f * (mag_dist + 10))) - 10;
					vel = Vector2Add(vel, Vector2Scale(distance, (float)(sep_force * -SEPARATION * SIZE)));
				}
			}
		}
	}
	
	// Cohesion
	sum_pos = Vector2Scale(sum_pos, (float)1.0f / count);
	if (!Vector2Equals(sum_pos, pos)) {
		vel = Vector2Add(vel, 
				Vector2Scale(
					Vector2Normalize(
						Vector2Subtract(sum_pos, pos)), COHESION));
	}
	
	// Alignmnet
	sum_vel = Vector2Scale(sum_vel, (float)1.0f / count);
	if (!Vector2Equals(sum_vel, vel)) {
		vel = Vector2Add(vel,
				Vector2Scale(
					Vector2Normalize(
						Vector2Subtract(sum_vel, vel)), ALIGNMENT));
	}
	
	// Rescale Speed
	vel  = Vector2Add(vel, 
				Vector2Scale(
					Vector2Subtract(
						Vector2Scale(
							Vector2Normalize(vel), TARGETSPEED),
					vel),
				RESOLVE)
			);

	// Move the boid	
	pos = Vector2Add(pos, vel);

	//Infinite borders
	if (pos.x >= (float)GetScreenWidth()) {
		pos.x = 1.0f;
	}
	if (pos.x <= 0.0f) {
		pos.x = (float)(GetScreenWidth() - 1);
	}
	if (pos.y >= (float)GetScreenHeight()) {
		pos.y = 1.0f;
	}
	if (pos.y <= 0.0f) {
		pos.y = (float)(GetScreenHeight() - 1);
	}
	
	ptr -> x = pos.x;
	ptr -> y = pos.y;
	ptr -> xv = vel.x;
	ptr -> yv = vel.y;
	
	return;	
}

///////////////////////////////////////////////////////////////////////////////
// Used to print an interface to terminal. Polls boid data from the first
// min(25, MAXBOIDS) boids and displays them to screen
// INPUT
//	a pointer to the boid array
//	a pointer to the number of lines to print computed in main
// OUTPUT
//	None
///////////////////////////////////////////////////////////////////////////////
void print_boid_interface(Boid * a, int * lines) {

	float x;
	float y;
	float xv;
	float yv;
	
	// "\r\033[K" clears the line before overwriting
	fprintf(stdout,
		"\r\033[K******************************************* Interface ****************************************************\n");
	fprintf(stdout, "\r\033[KFPS: %d\n", GetFPS());
	fprintf(stdout, "\rBoid Count: %d\n", MAXBOIDS);	
	fprintf(stdout, 
		"\r\033[K---------------------------------------------------------------------------------------------------------\n");
	fprintf(stdout, "\r\033[KNo.\t\tData Contents\t\tXpos\t\tYpos\t\tXvel\t\tYvel\n");
	fprintf(stdout, 
		"\r\033[K---------------------------------------------------------------------------------------------------------\n");
		
	for (int i = 0; i < *lines; i++) {
			
		x = a[i].x; // * 0.03125f; // val * 1/2^5
		y = a[i].y; // * 0.03125f;
		xv = a[i].xv; // * 0.00390625f; // val * 1/2^8
		yv = a[i].yv; // * 0.00390625f;
		
			
		fprintf(stdout, "\r\033[K%d\t\t\t\t\t%4.2f\t\t%4.2f\t\t%3.3f\t\t%3.3f\n", i+1, x, y, xv, yv); 
	}


	fprintf(stdout, "\033[%dA", *lines + 6);
	return;
}



// Pass in the boid array
void draw(Boid * ptr) {
	
	float x;
	float y;
	float xv;
	float yv;
	float theta;
	Vector2 vel;
	Vector2 pos;
	
	x = (float)(ptr -> x);//  * 0.03125f; // val * 1/2^5
	y = (float)(ptr -> y); // * 0.03125f;
	xv = (float)(ptr -> xv); // * 0.00390625f; // val * 1/2^8
	yv = (float)(ptr -> yv); // * 0.00390625f;

	pos = (Vector2) { x, y };
	vel = (Vector2) { xv, yv };

	
	//Vec

	//DrawTriangle();





	theta = RAD2DEG * (acos(vel.x / (Vector2Length(vel) + 0.1f))); //float div
	DrawPoly(pos, 3, SIZE, theta, WHITE); 		

	return;
}







///////////////////////////////////////////////////////////////////////////////
// MAIN FUNCTON
// Contains Game Loop and Bucketing logic
//
//
//
///////////////////////////////////////////////////////////////////////////////
int main() 
{
	
	double time;
	// Allocate Memory for boids and counting sort
	// a boid array located on the stack called boids - could be 
	// Boid ** boids = (boid*) malloc(2 * sizeof(boid*));
	Boid *boids[2];
	// First row to store Boids
	boids[0] = calloc(MAX_NUM_BOIDS, sizeof(Boid));
	// Second row to store Boids
	boids[1] = calloc(MAX_NUM_BOIDS, sizeof(Boid));
	
	// An array of helper data structures(BucketIndex), which are to contain 
	// two pieces of data for each boid in the previous boid row
	BucketIndex * bucket_indices = calloc(MAX_NUM_BOIDS, sizeof(BucketIndex));

	// An array of bucket counters size = numBuckets. Just an array with a slot
	// for each bucket which contains the number of boids in that bucket
	BucketCounter * bucket_counters = calloc(NUM_BUCKETS, sizeof(BucketCounter));

	
	// An array to store the contents of each buckets. each index of this array
	// contains a bucket object with an array of boids in that bucket and a
	// count of the number iof boids in that bucket
	Bucket * buckets = calloc(MAX_NUM_BOIDS, sizeof(Bucket));

	//WHy are we allocating MAx NUM Boids buckets?
	//Bucket * buckets = calloc(NUM_BUCKETS, sizeof(Bucket));


	// Configure Window
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(500, 500, "Boids");
	MaximizeWindow();
	SCREENW = GetScreenWidth();
	SCREENH = GetScreenHeight();
	SetWindowMaxSize(SCREENW, SCREENH);
	SetWindowSize(500, 500);

	// Initialize boids
	for (int i = 0; i < MAXBOIDS; i++) {
		// Pass in the ADDRESS of the Boid at a[i] so that it can be modified
		// Always initialize for row 0
		init_boid(&boids[0][i]);
	}
	// More Configuration setup
	SetTargetFPS(90); //Caps fps at 60
	int lines = fmin((double)25, (double)MAXBOIDS);
	

	int accum = 0;
	Boid *src_boids; //ptr to the start of the previous frame's boids
	Boid *dest_boids; //ptr to the start of the current frame's boids
	


	//Game Loop
	while(!WindowShouldClose())
	{
		
		time = GetTime();



		// Accumulator. flips the start index for the source and the every update
		
		src_boids = boids[accum & 1]; //if accum = 0, src = 0, if accum = 1, src = 1
		dest_boids = boids[((accum + 1) & 1)];
		accum++; 
		//printf("Accum: %i\n", accum);

		/**
		if (accum) {
			src_boids = boids[0]; //if accum = 0, src = 0, if accum = 1, src = 1
			dest_boids = boids[1];
			accum = 0;
		} else {
			src_boids = boids[1]; //if accum = 0, src = 0, if accum = 1, src = 1
			dest_boids = boids[0];
			accum = 1;
		}
		**/

		// Reset bucket count and bucket frame every frame
		memset(bucket_indices, 0, sizeof(BucketIndex) * MAXBOIDS);
		memset(bucket_counters, 0, sizeof(BucketCounter) * NUM_BUCKETS);
		

		int scalefactor = sqrt(NUM_BUCKETS); //for 16 buckets this is 4
		int bucket; //stores current bucket
/////////////////////////////////Count/////////////////////////////////////////
		
		for (int i = 0; i < MAXBOIDS; i++) {

			// This chunk of code determines the bucket of a boid
			float x = src_boids[i].x;
			float y = src_boids[i].y;
			int bucketx = (int)floorf((x / SCREENW) * scalefactor);
			int buckety = (int)floorf((y / SCREENH) * scalefactor);
			bucket = bucketx + (buckety * scalefactor);

			// The alternate method would be to store Boid position from
			// 1 to UINT_MAX               1 - 4bi;
			//[0000] 0000 0000 0000 0000 0000 0000 0000 min
			//[1111] 1111 1111 1111 1111 1111 1111 1111 max
			// if you want 16 buckets, all but last 4 bits are masked
			// for placing if on screen, this wil be more challanging	
			// 
			// This has to do with pan and zoom functionality

			// bucket indices is a parallel array to boids. It contains the bucket
			// that boid is going to and that boids index within bucket
			bucket_indices[i].bucket = bucket;
			// We set the index equal to the current count in that bucket
			bucket_indices[i].index_within_bucket = bucket_counters[bucket];
			bucket_counters[bucket]++;
		}


//////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\////////////////ALLOC////////////////////////////////////////
		
		// Prefix sum using the starting index
		uint32_t prefix_sum = 0;
		for (int i = 0; i < NUM_BUCKETS; i++) {
			buckets[i].boids = dest_boids + prefix_sum; //initiall we are 
			//setting the boid array pointer for the ith bucket equal to the 
			// current pointer to dest + a running tally. This mean setting the
			// start of boids to the start of dest to begin with
	
			// bucket_counters has the number of boids per bucket in each index
			// corresponding to a buckat
			buckets[i].count = bucket_counters[i];


			// Finally update the running total variable to place the start of boids
			//array for the next buckets[i] correctly
			prefix_sum += bucket_counters[i];
		}

///////////////////////////////////FILL///////////////////////////////////////
		
		for (int i = 0; i < MAXBOIDS; i++) {
			//bucket_indices[i].bucket // .bucket contains the first boids bucket number
			//buckets[/**current boid bucket**/] // = a bucket with a boids array 
			// ptr and a sizegt
			//buckets[bucket_indices[i].bucket].boids // references the start of the boid list
			// the boid to be grabbed is 
			//[bucket_indices[i].index_within_bucket]
			buckets[bucket_indices[i].bucket]
				.boids[bucket_indices[i].index_within_bucket]
				 = src_boids[i];
		}


		time = GetTime() - time;

		// Text Display, use {} to contain any local variables
		{
		char text[1000];  
		snprintf(text, 128, "Grid Build: %li(ms)", (long)(time * 1000000));
		DrawText(text, 10, 30, 20.0, GREEN);
		}


		time = GetTime();
		for (int i = 0; i < MAXBOIDS; i++) {
			//update_boid(&boids[0][i], &boids[0][0]);
			//For now, pass in the current boid and its bucket
			// When considering position, willl also be able to look at the 
			// bits to potentially select neighboring buckets
			// buckets(256)                  precisiton
			//                  [     right     ][      left     ]
			// xbucket: [1111]  0000 0000 0000 0000 0000 0000 0000 


			//buckets[bucket_indices[i].bucket].boids recovers a pointer to 
			// the first boid in a bucket
			update_boid(&dest_boids[i], buckets, bucket_indices[i].bucket);

			//update_boid(&dest_boids[i], dest_boids + start, );
		}

		time = GetTime() - time;

		// Text Display, use {} to contain any local variables
		{
		char text[1000];  
		snprintf(text, 128, "Update: %li(ms)", (long)(time * 1000000));
		DrawText(text, 10, 50, 20.0, GREEN);
		}
	


		//  Terminal Output
		if (BOID_INTERFACE)
			print_boid_interface(boids[0], &lines);

		time = GetTime();
		BeginDrawing();

		// Clearing background before drawing prevents buffer fragmentation
		ClearBackground(BLACK);
		// New Draw: 
		for(int i = 0; i < MAXBOIDS; i++) {
			draw(dest_boids + i);
		}

		DrawFPS(10, 10);
		EndDrawing();
		
		time = GetTime() - time;

		// Text Display, use {} to contain any local variables
		{
		char text[1000];  
		snprintf(text, 128, "Draw: %li(ms)", (long)(time * 1000000));
		DrawText(text, 10, 70, 20.0, GREEN);
		}

	}


	if (BOID_INTERFACE) {	
		fflush(stdout);
		for (int i = 0; i < lines + 6; i++) {
			fprintf(stdout, "\n");
		}
	}

	CloseWindow();
	free(boids[0]);
	free(boids[1]);
	free(bucket_indices);
	free(bucket_counters);
	free(buckets);



	return 0;
}

