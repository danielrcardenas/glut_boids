#include<sys/queue.h>

#define MAXBOIDS 200
#define MAXSELECT 100
#define MAXFEED 300
#define SOLID 1
#define LINE 2
#define POINT 3
#define LOCAL_RADIUS 500
#define MAX_RADIUS 100
#define MAX_SPEED 25
#define MAX_FISH_TYPES 3

struct vector {
	float x;
	float y;
} vector;

struct prey {
	struct vector centre;
	float radius;
	float color[3];
	LIST_ENTRY(prey) pointers;
} prey;

struct slider {
	// Lines for the slider
	struct vector line_p1;
	struct vector line_p2;

	// Rectangle to slide
	struct vector rect_p1;
	struct vector rect_p2;
} slider[1];

struct triangle {
	struct vector p1;
	struct vector p2;
	struct vector p3;

	struct vector centre;
} triangle;

struct boid {
	struct triangle t;
	struct vector velocity;
	float color[3];
	float rotate;

	float v1[2];
	float v2[2];
	float v3[2];

} boids[MAXBOIDS];

static void Timer(int value);
struct vector * MoveTowardsCentre(struct boid *b);
void LimitSpeed(struct boid *b);
struct vector * KeepDistance(struct boid *b);
struct vector * MatchVelocity(struct boid *b);
void VectorAdd(struct vector *ret, struct vector *a, struct vector *b);
void VectorMinus(struct vector *ret, struct vector *a, struct vector *b);
void VectorMultiply(struct vector *ret, struct vector *a, float scalar);
void VectorDivide(struct vector *ret, struct vector *a, float scalar);
struct vector * StayInBounds(struct boid *b);
int check_within_radius(struct vector *centre, struct vector *v, float radius);
