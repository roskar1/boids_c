
#define MAXBOIDS 50000	// Max Amount of Boids
#define MAX_NUM_BOIDS 1000000 // Max possible amount
#define SIZE 100		// Size of Boid

#define BOID_INTERFACE 0 //1 for display output, 0 otherwise

#define NUM_BUCKETS 4096 //must be a power of 2



typedef struct {
	// float x;
	// float y;

	uint16_t x; 
	uint16_t y;
	
	float xv;
	float yv;
	
	uint16_t padding;
} Boid;


// Stores two integers which are used to sort boids properly from previous 
// frame to this frame
typedef struct {
	int n[4];
	int bucket;
	int index_within_bucket;
} BucketIndex;

typedef struct {
	Boid * boids;
	uint64_t count;
} Bucket;

typedef uint32_t BucketCounter;

