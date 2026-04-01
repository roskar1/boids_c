
#define MAXBOIDS 50000	// Max Amount of Boids
#define MAX_NUM_BOIDS 1000000 // Max possible amount
#define SIZE 0.5			// Size of Boid

#define SEPARATION 0.000075
#define COHESION 0.5
#define ALIGNMENT 0.0001
#define RESOLVE 0.101 //0.101
#define TARGETSPEED 4
#define RANGE 200

#define BOID_INTERFACE 0 //1 for display output, 0 otherwise

#define NUM_BUCKETS 4096 //must be a power of 2



typedef struct {
	float x; 
	float y;

	float xv;
	float yv;
} Boid;


// Stores two integers which are used to sort boids properly from previous 
// frame to this frame
typedef struct {
	int bucket;
	int index_within_bucket;
} BucketIndex;

typedef struct {
	Boid * boids;
	uint64_t count;
} Bucket;

typedef uint32_t BucketCounter;

