#include <raylib.h>
#include <unistd.h>
#include <math.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <rlgl.h>


#define MAXBOIDS 8192	// Max Amount of Boids
#define SCREENW 1792	// Screen Width
#define SCREENH 1120	// Screen Height
#define SIZE 2			// Size of Boid

#define SEPARATION 0.00005
#define COHESION 0.2
#define ALIGNMENT 0.1
#define RESOLVE 0.101
#define TARGETSPEED 6
#define RANGE 200

#define BOID_INTERFACE 1 //1 for display output, 0 otherwise

#define NUMBUCKETS 256 //must be a power of 4

//int SCREENW;
//int SCREENH;

///////////////////////////////////////////////////////////////////////////////
// This is the BOID type object; one contiguous word in memory for ease of 
// access by the compiler. 
// Contents:
//	x: The x position of the boid. Allocated 16 bits by the bit field, this
//		field contains 11 bits of float above the decimal and 5 bits of float
//		after the decimal. Value is unsigned since the window's range is 
//		strictly positive.
//	y: The y position of the boid. Allocated 16 bits by the bit field, this     
//      field contains 11 bits of float above the decimal and 5 bits of float   
//      after the decimal. Value is unsigned since the window's range is 
//		strictly positive.
//	xv: The x velocity of the boid. Allocated 16 bits by the bit field, this 
//		field contains 8 bits of float above th decimal and 8 bits of float 
//		after the decimal. Value is signed.
//  yv: The y velocity of the boid. Allocated 16 bits by the bit field, this    
//      field contains 8 bits of float above th decimal and 8 bits of float     
//      after the decimal. Value is signed. 
//
// Total space occupied: 8 bytes(1 word)             
///////////////////////////////////////////////////////////////////////////////
typedef struct {
	// a uint64_t is an unsigned 64 bit integer(no dexplicit decimals)
	uint64_t x : 16; //bit-field initialization container_type : width
	uint64_t y : 16;

	int64_t xv : 16;
	int64_t yv : 16;
} Boid;


///////////////////////////////////////////////////////////////////////////////
// Node object for Linked List data structure
// Contents:
//	data: a pointer to the boid that this node contains
//	nect: a pointer to the next node in the list
///////////////////////////////////////////////////////////////////////////////
typedef struct Node{
	Boid * data; // A pointer to a boid in the huge array
	struct Node * next; // A pointer to the next object in linked list
} Node;


///////////////////////////////////////////////////////////////////////////////
// An initializer method for the boids
// INPUT:
//	a pointer to the boid to be initialized
// OUTPUT
//	None
///////////////////////////////////////////////////////////////////////////////
void init_boid(Boid * ptr) {
	// Set a random initial x and y coordinate position
	ptr -> x = (rand() % SCREENW) << 5; //bit shift left for precision
	ptr -> y = (rand() % SCREENH) << 5;
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
//void update_boid(Boid * ptr, Boid * arr) {
// Head is the first node in the bucket ptr is located in
void update_boid(Boid * ptr, Node * head) {
	/*
	 - Extract float values. Dividing by 2^(precision amount) is the same
	as shifting the decimal point
	 - Multiplying by a small decimal in lines 37-40: Compiler is not allowed
	to optimize float division(iterative guess and check algorithm ~O(logn)),
	thus we must do it ourselves
	*/

	float x = (float)(ptr -> x) * 0.03125f; // val * 1/2^5
	float y = (float)(ptr -> y) * 0.03125f;
	float xv = (float)(ptr -> xv) * 0.00390625f; // val * 1/2^8
	float yv = (float)(ptr -> yv) * 0.00390625f;
	
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
	/**	
	for (int i = 0; i < MAXBOIDS; i++) {
		if (&arr[i] != ptr) { //Ensure a Boid does not check itself
			otherx = (float)(arr[i].x) * 0.03125f;
			othery = (float)(arr[i].y) * 0.03125f;
			otherxv = (float)(arr[i].xv) * 0.00390625;
			otheryv = (float)(arr[i].yv) * 0.00390625;
			other_pos = (Vector2){ otherx, othery };
			other_vel = (Vector2){ otherxv, otheryv };
	**/
	
	while (head != NULL) {
		if (head -> data != ptr) {
			otherx = (float)(head -> data -> x) * 0.03125f;
			othery = (float)(head -> data -> y) * 0.03125f;
			otherxv = (float)(head -> data -> xv) * 0.00390625;
			otheryv = (float)(head -> data -> yv) * 0.00390625;
			other_pos = (Vector2){ otherx, othery };
			other_vel = (Vector2){ otherxv, otheryv };
	
			distance = Vector2Subtract(other_pos, pos);
			mag_dist = Vector2Length(distance);

			if (mag_dist < RANGE) {
				count += 1;
				sum_pos = Vector2Add(sum_pos, other_pos);
				sum_vel = Vector2Add(sum_vel, other_vel);

				// Separation Inverse
			
				// If the distance is not acceptable, do not calculate for sep
				if (mag_dist < 90.0f && mag_dist > 0.0f) {
					sep_force = (10.0f / (0.01f * (mag_dist + 10))) - 10;
					vel = Vector2Add(vel, Vector2Scale(distance, (float)(sep_force * -SEPARATION * SIZE)));
				}
			
				// Separation Linear
				/**
				if (mag_dist < 0.0f) {
					mag_dist = 1.0f;
				}
				if (mag_dist < 100.0f && mag_dist > 0.0f) {
					sep_force = (-mag_dist + 100);
					vel = Vector2Add(vel, Vector2Scale(distance, (float)(sep_force * -SEPARATION)));
				}
				**/
			}
		}
		head = head -> next;
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

	/**
	float theta = RAD2DEG * (acos(vel.x / (Vector2Length(vel) + 0.1f))); //float div
	if (vel.y < 0) 
		theta = 360 - theta;
	**/

	//Infinite borders
	if (pos.x > (float)SCREENW) {
		pos.x = 1.0f;
	}
	if (pos.x <= 0.0f) {
		pos.x = (float)(SCREENW - 1);
	}
	if (pos.y > (float)SCREENH) {
		pos.y = 1.0f;
	}
	if (pos.y <= 0.0f) {
		pos.y = (float)(SCREENH - 1);
	}
		
	//DrawPoly(pos, 3, SIZE, theta, WHITE);
	//printf("theta: %f\n", theta);
	
	// Repack updated data
	ptr -> x = (uint64_t)(pos.x * 32.0f);
	ptr -> y = (uint64_t)(pos.y * 32.0f);
	ptr -> xv = (int64_t)(vel.x * 256.0f);
	ptr -> yv = (int64_t)(vel.y * 256.0f);

	
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
			
		x = (float)(a[i].x) * 0.03125f; // val * 1/2^5
		y = (float)(a[i].y) * 0.03125f;
		xv = (float)(a[i].xv) * 0.00390625f; // val * 1/2^8
		yv = (float)(a[i].yv) * 0.00390625f;
		
			
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
	
	x = (float)(ptr -> x) * 0.03125f; // val * 1/2^5
	y = (float)(ptr -> y) * 0.03125f;
	xv = (float)(ptr -> xv) * 0.00390625f; // val * 1/2^8
	yv = (float)(ptr -> yv) * 0.00390625f;

	pos = (Vector2) { x, y };
	vel = (Vector2) { xv, yv };
		
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

	//SCREENW = GetScreenWidth();
	//`SCREENH	= GetScreenHeight();
	// Allocate Memory
	// a is an array of boids, not pointers to boids
	Boid * a = calloc(MAXBOIDS, sizeof(Boid));
	if (a == NULL) {
		fprintf(stderr, "calloc failed\n");
		exit(1);
	}
	
	// Initialize boids
	for (int i = 0; i < MAXBOIDS; i++) {
		// Pass in the ADDRESS of the Boid at a[i]
		init_boid(&a[i]);
	}

//=============================================================================
	// Node POOL
	// Goal is to minimize allocations during the frames, instead perform them 
	// beforehand
	// Instantiate a large malloc containing all the nodes that will be used
	// in a frame
	Node *NODEPOOL = malloc(MAXBOIDS * sizeof(Node));
	if (NODEPOOL == NULL) {
		fprintf(stderr, "NodePool malloc failed\n");
		exit(1);
	}
	// Keep track of a pointer which stores the next available node to be used
	int pool_index = 0;

	// Instead of mallocing later, just grab a node from this pool

//=============================================================================
	// Linked List	
	// Initialize array(a pointer) to pointers to nodes
	Node **buckets = malloc(NUMBUCKETS * sizeof(Node *));
	if (buckets == NULL) {
		fprintf(stderr, "Bucket malloc failed\n");
		exit(1);
	}
	for (int i = 0; i < NUMBUCKETS; i++) {
		buckets[i] = NULL; //go through and initialize all the buckets to NULL
	}
//=============================================================================
	

	InitWindow(SCREENW, SCREENH, "Boids");
	MaximizeWindow();
	SetTargetFPS(60); //Caps fps at 60
	int lines = fmin((double)25, (double)MAXBOIDS);
	
	//Game Loop
	while(!WindowShouldClose())
	{
		// Place Boids in Buckets
		// "node" is a pointer to a node
		Node * node = NULL;
		
		int scalefactor = sqrt(NUMBUCKETS); //for 16 buckets this is 4
		// Place the boids in their buckets
		for (int i = 0; i < MAXBOIDS; i++) {

			// Get position data
			float x = (float)(a[i].x) * 0.03125f; // val * 1/2^5
			float y = (float)(a[i].y) * 0.03125f;

			if (x > 1791.0f) x = 1791.0f;
				
			if (y > 1119) y = 1119.0f;



			// Floorf is used for float flooring for 0-based buckets
			int xbucket = (int)floorf((x / SCREENW) * scalefactor); 
			int ybucket = (int)floorf((y / SCREENH) * scalefactor);



				
			// Compute bucket index labeled "bucket"
			int bucket = xbucket + (ybucket * scalefactor);
			
			if (bucket < 0 || bucket >= NUMBUCKETS) {
				fprintf(stderr, "Invalid Bucket\n");
				exit(1);
			}
			//Node Pool Implementation
			node = &(NODEPOOL[pool_index]); //use address operator to derive a ptr
			pool_index += 1; //Increment Pool index

			//node's data field asks for a pointer to a boid
			node -> data = &a[i];

			// buckets is an array of Node pointers. Set this node's next
			// pointer to the current head of one of the buckets, a node ptr
			//printf("Bucket#: %d\n", bucket);
			node -> next = buckets[bucket];

			// Replace that bucket's head with the latest boid added to the
			// bucket. if this is the first Node placed in the bucket the 
			// current head will be	NULL, which is ideal. The last Node in the 
			// the bucket will have a NULL next pointer
			buckets[bucket] = node;
		}

		//  Terminal Output
		if (BOID_INTERFACE)
			print_boid_interface(a, &lines);

		for (int i = 0; i < NUMBUCKETS; i++) {
			Node * head = buckets[i];
			Node * current = head;
			while (current != NULL) {
				update_boid(current -> data, head);
				current = current -> next;
			}
		}

		BeginDrawing();

		// New Draw: 
		for(int i = 0; i < MAXBOIDS; i++) {
			draw(&a[i]);

			if (i > 0 && i % 2000 == 0) {
				rlDrawRenderBatchActive();
			}
		}


		/**							
		for (int i = 0; i < MAXBOIDS; i++) {
			update_boid(&a[i], a);
		}
		**/
		ClearBackground(BLACK);
		EndDrawing();
		


		/* Frame Clean Up Steps */
		for (int i = 0; i < NUMBUCKETS; i++) {
			buckets[i] = NULL;
		}
		pool_index = 0;
	}


	if (BOID_INTERFACE) {	
		fflush(stdout);
		for (int i = 0; i < lines + 6; i++) {
			fprintf(stdout, "\n");
		}
	}

	CloseWindow();
	free(NODEPOOL);
	free(buckets);
	free(a);
	return 0;
}

