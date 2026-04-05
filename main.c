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

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

///////////////////////////////////////////////////////////////////////////////
static int SCREENW; //These contain absolue maxes for W and H
static int SCREENH;

static float edge_x = 100000.0f; //Because of zoom this number cannot by UINT32_MAX
static float edge_y = 100000.0f;

//static int screen_radius_x;
//static int screen_radius_y;

static RenderTexture boidTexture;


static float TARGETSPEED = 20.0f;
static float SEPARATION = 0.1f;
static float COHESION = 5.0f;
static float ALIGNMENT = 10.0f;
static float RESOLVE = 1.0;
static float RANGE = 200.0f;

///////////////////////////////////////////////////////////////////////////////
void init_boid(Boid * ptr) {
	// Max Precision random generation
	ptr -> x = (uint16_t)(((float)rand() / (float)RAND_MAX) * UINT16_MAX);
	ptr -> y = (uint16_t)(((float)rand() / (float)RAND_MAX) * UINT16_MAX);
	
	return;
}

///////////////////////////////////////////////////////////////////////////////
void update_boid(Boid * ptr, Bucket * buckets, int b, int n[4]) {
	
	// Capture start of bucket this boid exists in:
	Boid * arr;
	int limit;
	
	/**
	float x = ptr -> x;
	float y = ptr -> y;
	float otherx;
	float othery;
	**/

	float x = (float)ptr -> x;
	float y = (float)ptr -> y; 
	float otherx;
	float othery;

	float xv = ptr -> xv; 
	float yv = ptr -> yv;
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


	for (int j = 0; j < 4; j++) {

	arr = buckets[*(n + j)].boids;
	limit = buckets[*(n + j)].count;
	for (int i = 0; i < limit; i++) {
		
		if (&arr[i] != ptr) { //Ensure a Boid does not check itself

			/**
			otherx = arr[i].x;
			othery = arr[i].y;
			**/

			otherx = (float)arr[i].x;
			othery = (float)arr[i].y;

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
				
				//printf("MagDist: %f\n", mag_dist); Usually between 90 and 200
				if (mag_dist < 90.0f && mag_dist > 0.0f) {
					sep_force = (10.0f / (0.01f * (mag_dist + 10))) - 10;
					vel = Vector2Add(vel, Vector2Scale(distance, (float)(sep_force * -SEPARATION)));
				}
			}
		}
	}

	} //second for loop
	
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

	//Infinite borders (Updated to where UINT16_MAX is the border)
	if (pos.x >= (float)UINT16_MAX) pos.x = 1.0f;
	if (pos.x <= 0.0f) pos.x = ((float)UINT16_MAX - 1.0f);
	if (pos.y >= (float)UINT16_MAX) pos.y = 1.0f;
	if (pos.y <= 0.0f) pos.y = ((float)UINT16_MAX - 1.0f);

	/**
	ptr -> x = pos.x;
	ptr -> y = pos.y;
	**/

	ptr -> x = (uint16_t)pos.x;
	ptr -> y = (uint16_t)pos.y;
	ptr -> xv = vel.x;
	ptr -> yv = vel.y;
	
	return;	
}

///////////////////////////////////////////////////////////////////////////////
// Pass in the boid array
void draw(Boid * ptr) {

	float x, y, xv, yv, theta;
	Rectangle source, dest;
	Vector2 origin;

	for (int i = 0; i < MAXBOIDS; i++) {
		// Scale the UINT16 max value by the edge dimensions
		x = (((float)((ptr + i) -> x)) / UINT16_MAX) * (float)edge_x;
		y = (((float)((ptr + i) -> y)) / UINT16_MAX) * (float)edge_y;
		xv = (float)((ptr + i) -> xv);
		yv = (float)((ptr + i) -> yv);
	
		source = (Rectangle){0, 0, (float)boidTexture.texture.width, (float)boidTexture.texture.height };
		origin = (Vector2){ (float)boidTexture.texture.width / 2, (float)boidTexture.texture.height / 2 };
		dest = (Rectangle){ x, y, (float)boidTexture.texture.width, (float)boidTexture.texture.height };

		theta = (atan2f(yv, xv) * RAD2DEG) + 180.0f;
		DrawTexturePro(
			boidTexture.texture,
			source,
			dest,
			origin,
			theta,
			WHITE
		);
	}







	/**
	// Scale the UINT16 max value by the edge dimensions
	x = (((float)(ptr -> x)) / UINT16_MAX) * (float)edge_x;
	y = (((float)(ptr -> y)) / UINT16_MAX) * (float)edge_y;

	xv = (float)(ptr -> xv);
	yv = (float)(ptr -> yv);

	pos = (Vector2) { x, y };
	vel = (Vector2) { xv, yv };
	
	Rectangle source = {0, 0, (float)boidTexture.texture.width, (float)boidTexture.texture.height };
	Vector2 origin = { (float)boidTexture.texture.width / 2, (float)boidTexture.texture.height / 2 };
	Rectangle dest = { x, y, (float)boidTexture.texture.width, (float)boidTexture.texture.height };
	//Rectangle dest = { x, y, SIZE, SIZE };
	//float theta = RAD2DEG * (acos(-vel.x / (Vector2Length(vel) + 0.0001f)));
	float theta = (atan2f(vel.y, vel.x) * RAD2DEG) + 180.0f;
	DrawTexturePro(
		boidTexture.texture,
		source,
		dest,
		origin,
		theta,
		WHITE
		);
	**/

	// Draw Poly Method
	/**
	float x;
	float y;
	float xv;
	float yv;
	float theta;
	Vector2 vel;
	Vector2 pos;

	// Scale the UINT16 max value by the edge dimensions
	x = (((float)(ptr -> x)) / UINT16_MAX) * (float)edge_x;
	y = (((float)(ptr -> y)) / UINT16_MAX) * (float)edge_y;

	xv = (float)(ptr -> xv);
	yv = (float)(ptr -> yv);

	pos = (Vector2) { x, y };
	vel = (Vector2) { xv, yv };

	theta = RAD2DEG * (acos(vel.x / (Vector2Length(vel) + 0.1f))); //float div
	DrawPoly(pos, 3, SIZE, theta, WHITE); 		
	**/

	return;
}

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

	Camera2D camera = { 0 };
	camera.zoom = 2.0f;



	// Initialize boids
	for (int i = 0; i < MAXBOIDS; i++) {
		// Pass in the ADDRESS of the Boid at a[i] so that it can be modified
		// Always initialize for row 0
		init_boid(&boids[0][i]);
	}

	// More Configuration setup
	SetTargetFPS(90); //Caps fps at 60
	

	int accum = 0;
	Boid *src_boids; //ptr to the start of the previous frame's boids
	Boid *dest_boids; //ptr to the start of the current frame's boids

	//texture
	int renderSize = SIZE;
	boidTexture = LoadRenderTexture(renderSize * 2, renderSize * 2);

	BeginTextureMode(boidTexture);
		Vector2 center = {SIZE, SIZE};

		//Vector2 center = { renderSize, renderSize };
		DrawPoly(center, 3, (float)SIZE, 180.0f, WHITE);

	EndTextureMode();

	while(!WindowShouldClose())
	{
					

		// Pan Functionality
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			Vector2 delta = GetMouseDelta();
			delta = Vector2Scale(delta, -1.0f/camera.zoom);
			camera.target = Vector2Add(camera.target, delta);
		}

		// Zoom Functionality
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            // Get the world point that is under the mouse
             Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

            // Set the offset to where the mouse is
            camera.offset = GetMousePosition();

            // Set the target to match, so that the camera maps the world space point
            // under the cursor to the screen space point under the cursor at any zoom
            camera.target = mouseWorldPos;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            // Zoom increment
            // Uses log scaling to provide consistent zoom speed
            float deltaX = GetMouseDelta().x;
			float scale = 0.005f*deltaX;
            camera.zoom = Clamp(expf(logf(camera.zoom)+scale), 0.00125f, 64.0f);
        }



		time = GetTime();

		// Accumulator. flips the start index for the source and the every update
		src_boids = boids[accum & 1]; //if accum = 0, src = 0, if accum = 1, src = 1
		dest_boids = boids[((accum + 1) & 1)];
		accum++; 

		// Reset bucket count and bucket frame every frame
		memset(bucket_indices, 0, sizeof(BucketIndex) * MAXBOIDS);
		memset(bucket_counters, 0, sizeof(BucketCounter) * NUM_BUCKETS);
		

		int scalefactor = sqrt(NUM_BUCKETS); //for 4096 buckets this is 64
		int bucket; //stores current bucket
		int nb[4]; //always 4 neighbor buckets
		char down;
		char right;
		uint16_t nMask;
		uint16_t xbucket;
		uint16_t ybucket;
/////////////////////////////////Count/////////////////////////////////////////
		for (int i = 0; i < MAXBOIDS; i++) {
							
			//bucket bitmasking method - only need the first 6 bits for 4096 buckets
			// This puts a cap on buckets
			xbucket = src_boids[i].x >> (16 - (int)floor(log2(scalefactor))); // == 10
			ybucket = src_boids[i].y >> (16 - (int)floor(log2(scalefactor)));

			bucket = xbucket + (ybucket * scalefactor);

			// bit 7 is 2^9
			nMask = 65023; //(2^16 - 1) - 2^9 = 65023 = 1111 1101 1111 1111
			down = 0;
			right = 0;
			if (src_boids[i].x & ~ nMask) right = 1; 
			if (src_boids[i].y & ~ nMask) down = 1; //if the y has a 1, DOWN

			

		// Correct
		if (down & right) { //Case 1 down & right

			// bot left; nb[0]
			if (bucket >= scalefactor * (scalefactor - 1)) { 
				// if the bucket is bottom row
				nb[0] = bucket % scalefactor;
			} else { nb[0] = bucket + scalefactor; }

			// bot right; nb[1]
			if (((nb[0] + 1) % scalefactor) == 0) {
			//if nb[0] is in the rightmost column
				nb[1] = nb[0] + 1 - scalefactor;
			} else { nb[1] = nb[0] + 1; }

			// top left; nb[2]
			nb[2] = bucket;

			// top right; nb[3]
			if (((nb[2] + 1) % scalefactor) == 0) {
			//if nb[2] is in the rightmost column
				nb[3] = nb[2] + 1 - scalefactor;
			} else { nb[3] = nb[2] + 1; }

		// Correct
		} else if (down) { //Case 2 down & left				

			// bot right; nb[1]
			if (bucket >= scalefactor * (scalefactor - 1)) { 
				// if the bucket is bottom row
				nb[1] = bucket % scalefactor;
			} else { nb[1] = bucket + scalefactor; }
				
			// bot left; nb[0]
			if ((nb[1] % scalefactor) == 0) { 
				// nb[1] in leftmost column
				nb[0] = (nb[1] - 1) + scalefactor;
			} else { nb[0] = nb[1] - 1; }

			// top left; nb[2]
			if ((bucket % scalefactor) == 0) { 
				// bucket in leftmost column
				nb[2] = (bucket - 1) + scalefactor;
			} else { nb[2] = bucket - 1; }

			// top right; nb[3]
			nb[3] = bucket;

		// Correct
		} else if (right) { //Case 3 up & right
				
			// bot left; nb[0]
			nb[0] = bucket;

			// bot right; nb[1]
			if (((nb[0] + 1) % scalefactor) == 0) {
				//if bucket is in the rightmost columun
				nb[1] = (nb[0] + 1) - scalefactor;
			} else { nb[1] = nb[0] + 1; }
					
			// top left; nb[2]
			if (bucket < scalefactor) { 
				// if the bucket is on the top row
				nb[2] = bucket + (scalefactor * (scalefactor - 1));
			} else { nb[2] = bucket - scalefactor; }
			
			// top right; nb[3]
			if (((nb[2] + 1) % scalefactor) == 0) {
				// if nb[2] is in the rightmost column
				nb[3] = (nb[2] + 1) - scalefactor;
			} else { nb[3] = nb[2] + 1; }

		// Correct
		} else { //Case down left
			
			// bot left; nb[0]
			if ((bucket % scalefactor) == 0) { 
				// bucket in leftmost column
				nb[0] = (bucket - 1) + scalefactor;
			} else { nb[0] = bucket - 1; }
			
			// bot right; nb[1]
			nb[1] = bucket;

			// top left; nb[2]
			if (nb[0] < scalefactor) { 
				// if the nb[0] is on the top row
				nb[2] = nb[0] + (scalefactor * (scalefactor - 1));
			} else { nb[2] = nb[0] - scalefactor; }

			if (bucket < scalefactor) { 
				// if the bucket is on the top row
				nb[3] = bucket + (scalefactor * (scalefactor - 1));
			} else { nb[3] = bucket - scalefactor; }
		}


			/**	
			// Bucket float method
			float x = (float)src_boids[i].x;
			float y = (float)src_boids[i].y;
			int bucketx = (int)floorf((x / UINT16_MAX) * scalefactor);
			int buckety = (int)floorf((y / UINT16_MAX) * scalefactor);
			bucket = bucketx + (buckety * scalefactor);
			**/

			// int bucket[4];

			bucket_indices[i].bucket = bucket;
			memcpy(bucket_indices[i].n, nb, sizeof(nb));
			// We set the index equal to the current count in that bucket
			bucket_indices[i].index_within_bucket = bucket_counters[bucket];
			bucket_counters[bucket]++;
		}

		///////////////////////////ALLOC///////////////////////////////////////	
		// Prefix sum using the starting index
		uint32_t prefix_sum = 0;
		for (int i = 0; i < NUM_BUCKETS; i++) {
			buckets[i].boids = dest_boids + prefix_sum; //initially we are 
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
		snprintf(text, 128, "Grid Build: %li(ms)", (long)(time * 1000));
		DrawText(text, 10, 30, 20.0, GREEN);
		}


		time = GetTime();
		for (int i = 0; i < MAXBOIDS; i++) {
			//update_boid(&boids[0][i], &boids[0][0]);

			//Consider only first bit in x and y pos for neighboring 3 buckets

			//buckets[bucket_indices[i].bucket].boids recovers a pointer to 
			// the first boid in a bucket
			update_boid(&dest_boids[i], buckets, bucket_indices[i].bucket, bucket_indices[i].n);

			//update_boid(&dest_boids[i], dest_boids + start, );
		}

		time = GetTime() - time;

		// Text Display, use {} to contain any local variables
		{
		char text[1000];  
		snprintf(text, 128, "Update: %li(ms)", (long)(time * 1000));
		DrawText(text, 10, 50, 20.0, GREEN);
		}

		time = GetTime();
		BeginDrawing();

		// Clearing background before drawing prevents buffer fragmentation
		ClearBackground(BLACK);

		BeginMode2D(camera);


		// Putting in func elimintates stack frame overhead(assuming compiler does not already do that
		draw(dest_boids);

		// Draw loop for individual draw function
		/**
			for(int i = 0; i < MAXBOIDS; i++) {
				draw(dest_boids + i);
			}
		**/

		EndMode2D();

		DrawFPS(10, 10);
		EndDrawing();
		
		time = GetTime() - time;

		// Text Display, use {} to contain any local variables
		{
		char text[1000];  
		snprintf(text, 128, "Draw: %li(ms)", (long)(time * 1000));
		DrawText(text, 10, 70, 20.0, GREEN);
		}


		{ // GUI Pass
		//GuiSlider((Rectangle){ 550, 10, 200, 10 }, "Speed", TextFormat("%.2f", var_name, &var_nam, min, max


		GuiSlider((Rectangle){ 1500, 10, 200, 10 }, "Speed",
			TextFormat("%.2f", TARGETSPEED), &TARGETSPEED, 0, 100);
		
		GuiSlider((Rectangle){ 1500, 30, 200, 10 }, "Seperation", 
			TextFormat("%.4f", SEPARATION), &SEPARATION, 0, 0.5);
		GuiSlider((Rectangle){ 1500, 45, 200, 10 }, "Cohesion",
			TextFormat("%.2f", COHESION), &COHESION, 0, 15);
		GuiSlider((Rectangle){ 1500, 60, 200, 10 }, "Alignment",
			TextFormat("%.4f", ALIGNMENT), &ALIGNMENT, 0, 15);
		GuiSlider((Rectangle){ 1500, 80, 200, 10 }, "Resolve",
			TextFormat("%.4f", RESOLVE), &RESOLVE, 0, 1);

		GuiSlider((Rectangle){ 1500, 100, 200, 10 }, "Range",
			TextFormat("%.2f", RANGE), &RANGE, 0, 300);


			
		}














	}

	CloseWindow();
	UnloadRenderTexture(boidTexture);
	free(boids[0]);
	free(boids[1]);
	free(bucket_indices);
	free(bucket_counters);
	free(buckets);



	return 0;
}

