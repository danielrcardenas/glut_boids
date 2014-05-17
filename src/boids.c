
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <GL/glut.h>
#include "boids.h"

GLint windW = 1000, windH = 800;
GLuint selectBuf[MAXSELECT];
GLfloat feedBuf[MAXFEED];
GLint vp[4];
float zRotation = 90.0;
float zoom = 1.0;
float refresh = 50;

float mouseX, mouseY;
GLint objectCount;
GLint numObjects;

GLenum linePoly = GL_FALSE;

// Bounds for position
int XMax = 300;
int XMin = -300;
int YMax = 300;
int YMin = -300;

// Adds two vectors
struct vector
AddVectors(struct vector v1, struct vector v2)
{
	struct vector ret;
	ret.x = v1.x + v2.x;
	ret.y = v1.x + v2.y;
	return ret;
}

float
square(float a)
{
	float ret;
	ret = a * a;
	return(ret);
}

int
check_within_radius(struct vector *centre, struct vector *v)
{
	// c^2 = a^2 + b^2
	if ( (square(v->x - centre->x) + square(v->y - centre->y)) < square(LOCAL_RADIUS) )
		return 1;
	else
		return 0;
}

float
GetMagnitude(struct vector *v)
{
	float t;
	t = v->x*v->x + v->y*v->y;
	return sqrt(t);	
}

static void
InitObjects(int num)
{
	GLint i;
	float x, y;

	if (num > MAXBOIDS) {
		num = MAXBOIDS;
	}
	if (num < 1) {
		num = 1;
	}
	objectCount = num;

	srand((unsigned int) time(NULL));
	for (i = 0; i < num; i++) {
		x = (rand() % 300) - 150;
		y = (rand() % 300) - 150;

		// Set the triangle variables for each boid

		boids[i].t.p1.x = x;
		printf("Setting boids[i].t.p1.x to: %f\n", boids[i].t.p1.x);
		boids[i].t.p2.x = x + 5;
		boids[i].t.p3.x = x + 2.5;
		boids[i].t.p1.y = y;
		boids[i].t.p2.y = y;
		boids[i].t.p3.y = y + 10;

		boids[i].color[0] = ((rand() % 100) + 50) / 150.0;
		boids[i].color[1] = ((rand() % 100) + 50) / 150.0;
		boids[i].color[2] = ((rand() % 100) + 50) / 150.0;

		boids[i].velocity.x = 0;
		boids[i].velocity.y = 0;

		// Initialise rotation angle to 0
		boids[i].rotate = 0.0f;
	}
	printf("memaddress: %d\n", &boids[0]);
}

static void
Init(void)
{
	numObjects = 100;
	InitObjects(numObjects);
}

static void
Timer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(refresh, Timer, 0);
}

static void
Reshape(int width, int height)
{
	windW = width;
	windH = height;
	glViewport(0, 0, windW, windH);
	glGetIntegerv(GL_VIEWPORT, vp);
}

static void
Render(GLenum mode)
{
	GLint i;

	for (i = 0; i < numObjects; i++) {
		if (mode == GL_SELECT) {
			glLoadName(i);
		}
		//glRotatef(boids[i].rotate, 0.0f, 0.0f, 0.5f);
		glColor3fv(boids[i].color);
		glBegin(GL_POLYGON);
		glVertex2f(boids[i].t.p1.x, boids[i].t.p1.y);
		glVertex2f(boids[i].t.p2.x, boids[i].t.p2.y);
		glVertex2f(boids[i].t.p3.x, boids[i].t.p3.y);
		glEnd();

		// Make the fish rotate
//		boids[i].rotate += 1.0f;
	}
}

static GLint
DoSelect(GLint x, GLint y)
{
	GLint hits;

	glSelectBuffer(MAXSELECT, selectBuf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(~0);

	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix(x, windH - y, 4, 4, vp);
	gluOrtho2D(-175, 175, -175, 175);
	glMatrixMode(GL_MODELVIEW);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glScalef(zoom, zoom, zoom);
	glRotatef(zRotation, 0, 0, 1);

	Render(GL_SELECT);

	glPopMatrix();

	hits = glRenderMode(GL_RENDER);
	if (hits <= 0) {
		return -1;
	}
	return selectBuf[(hits - 1) * 4 + 3];
}

static void
RecolorTri(GLint h)
{
	boids[h].color[0] = ((rand() % 100) + 50) / 150.0;
	boids[h].color[1] = ((rand() % 100) + 50) / 150.0;
	boids[h].color[2] = ((rand() % 100) + 50) / 150.0;
}

static void
DeleteTri(GLint h)
{
	boids[h] = boids[objectCount - 1];
	objectCount--;
}

static void
GrowTri(GLint h)
{
	float v[2];
	float *oldV;
	GLint i;

	v[0] = boids[h].v1[0] + boids[h].v2[0] + boids[h].v3[0];
	v[1] = boids[h].v1[1] + boids[h].v2[1] + boids[h].v3[1];
	v[0] /= 3;
	v[1] /= 3;

	for (i = 0; i < 3; i++) {
		switch (i) {
			case 0:
				oldV = boids[h].v1;
				break;
			case 1:
				oldV = boids[h].v2;
				break;
			case 2:
				oldV = boids[h].v3;
				break;
		}
		oldV[0] = 1.5 * (oldV[0] - v[0]) + v[0];
		oldV[1] = 1.5 * (oldV[1] - v[1]) + v[1];
	}
}

static void
MoveMouse(int x, int y)
{
	// Convert it to the window coordinates
	mouseX = (x / (float)windW) - 0.5f;
	mouseY = (x / (float)windH) - 0.5f; 
}

static void
AvoidPredator()
{
	int i;
	for (i = 0; i < MAXBOIDS; i++) {
		// Begin avoiding if the mouse only if the vectors are close
		boids[i].r4[0] = -0.01 * ((mouseX - boids[i].v1[0]) / 100);
		boids[i].r4[1] = -0.01 * ((mouseY - boids[i].v1[0]) / 100);
	}
}

static void
Mouse(int button, int state, int mouseX, int mouseY)
{
	GLint hit;
	if (state == GLUT_DOWN) {
		hit = DoSelect((GLint) mouseX, (GLint) mouseY);
		if (hit != -1) {
			if (button == GLUT_LEFT_BUTTON) {
				RecolorTri(hit);
			} else if (button == GLUT_MIDDLE_BUTTON) {
				GrowTri(hit);
			} else if (button == GLUT_RIGHT_BUTTON) {
				DeleteTri(hit);
			}
			glutPostRedisplay();
		}
	}
}

static void
DrawSlider(void)
{
	glClear(GL_COLOR_BUFFER_BIT);  
	/*glColor3f(0.0,0.4,0.2); 
	glPointSize(3.0);  

	glBegin(GL_LINES);
		glVertex2i(0.1f,0.1f);
		glVertex2i(0.75f,-0.75f);
	glEnd();*/


       glColor3f(0.0f, 0.0f, 0.0f);
       glRectf(-0.75f,0.75f, 0.75f, -0.75f);
       glutSwapBuffers();

}

static void
Draw(void)
{
	int i;

	struct vector *r0;
	struct vector *r1;
	struct vector *r2;
	struct vector *r3;

	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluOrtho2D(-175, 175, -175, 175);
	gluOrtho2D(-500, 500, -500, 500);
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glScalef(zoom, zoom, zoom);
	glRotatef(zRotation, 0, 0, 1);

	// R1	

	for (i = 0; i < MAXBOIDS; i++) {
		
		// Apply Rules
		r0 = StayInBounds(&boids[i]);
		r1 = MoveTowardsCentre(&boids[i]);
		r2 = MatchVelocity(&boids[i]);
		r3 = LimitSpeed(&boids[i]);

		// Add R0 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r0);
		// Add R1 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r1);
		// Add R2 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r2);
		
		if (boids[i].velocity.y > 1000) {
			exit(1);
		}
	
		// Apply velocity to position	
		VectorAdd(&boids[i].t.p1, &boids[i].t.p1, &boids[i].velocity);
		VectorAdd(&boids[i].t.p2, &boids[i].t.p2, &boids[i].velocity);
		VectorAdd(&boids[i].t.p3, &boids[i].t.p3, &boids[i].velocity);

		free(r0);
		free(r1);
	}

	DrawSlider();
	Render(GL_RENDER);
	glPopMatrix();
	glutSwapBuffers();
}

static void
DumpFeedbackVert(GLint * i, GLint n)
{
	GLint index;

	index = *i;
	if (index + 7 > n) {
		*i = n;
		printf("  ???\n");
		return;
	}
	printf("  (%g %g %g), color = (%4.2f %4.2f %4.2f)\n",
			feedBuf[index],
			feedBuf[index + 1],
			feedBuf[index + 2],
			feedBuf[index + 3],
			feedBuf[index + 4],
			feedBuf[index + 5]);
	index += 7;
	*i = index;
}

static void
DrawFeedback(GLint n)
{
	GLint i;
	GLint verts;

	printf("Feedback results (%d floats):\n", n);
	for (i = 0; i < n; i++) {
		switch ((GLint) feedBuf[i]) {
			case GL_POLYGON_TOKEN:
				printf("Polygon");
				i++;
				if (i < n) {
					verts = (GLint) feedBuf[i];
					i++;
					printf(": %d vertices", verts);
				} else {
					verts = 0;
				}
				printf("\n");
				while (verts) {
					DumpFeedbackVert(&i, n);
					verts--;
				}
				i--;
				break;
			case GL_LINE_TOKEN:
				printf("Line:\n");
				i++;
				DumpFeedbackVert(&i, n);
				DumpFeedbackVert(&i, n);
				i--;
				break;
			case GL_LINE_RESET_TOKEN:
				printf("Line Reset:\n");
				i++;
				DumpFeedbackVert(&i, n);
				DumpFeedbackVert(&i, n);
				i--;
				break;
			default:
				printf("%9.2f\n", feedBuf[i]);
				break;
		}
	}
	if (i == MAXFEED) {
		printf("...\n");
	}
	printf("\n");
}

static void
DoFeedback(void)
{
	GLint x;

	glFeedbackBuffer(MAXFEED, GL_3D_COLOR, feedBuf);
	(void) glRenderMode(GL_FEEDBACK);

	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-175, 175, -175, 175);
	glMatrixMode(GL_MODELVIEW);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glScalef(zoom, zoom, zoom);
	glRotatef(zRotation, 0, 0, 1);

	Render(GL_FEEDBACK);

	glPopMatrix();

	x = glRenderMode(GL_RENDER);

	if (x == -1) {
		x = MAXFEED;
	}

	DrawFeedback((GLint) x);
}

/* ARGSUSED1 */
static void
Key(unsigned char key, int x, int y)
{

	switch (key) {
		case 'z':
			zoom /= 0.75;
			glutPostRedisplay();
			break;
		case 'Z':
			zoom *= 0.75;
			glutPostRedisplay();
			break;
		case 'f':
			DoFeedback();
			glutPostRedisplay();
			break;
		case 'l':
			linePoly = !linePoly;
			if (linePoly) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			} else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			glutPostRedisplay();
			break;
		case 27:
			exit(0);
	}
}

/* ARGSUSED1 */
static void
SpecialKey(int key, int x, int y)
{

	switch (key) {
		case GLUT_KEY_LEFT:
			zRotation += 0.5;
			glutPostRedisplay();
			break;
		case GLUT_KEY_RIGHT:
			zRotation -= 0.5;
			glutPostRedisplay();
			break;
	}
}

void
VectorAdd(struct vector *ret, struct vector *a, struct vector *b)
{
	ret->x = a->x + b->x;
	ret->y = a->y + b->y;
}

void
VectorMinus(struct vector *ret, struct vector *a, struct vector *b)
{
	ret->x = a->x - b->x;
	ret->y = a->y - b->y;
}

void
VectorDivide(struct vector *ret, struct vector *a, float scalar)
{

	ret->x = a->x / scalar;
	ret->y = a->y / scalar;
}

struct vector *
StayInBounds(struct boid *b)
{
	struct vector *v = malloc(sizeof(struct vector));

	// Initialise values to 0
	v->x = 0;
	v->y = 0;

	//printf("t.p1.x: %f\n", b->t.p1.x);	
	
	if (b->t.p1.x < (float)XMin)
		v->x = 10;
	else if (b->t.p1.x > (float)XMax)
		v->x = -10;
	
	if (b->t.p1.y < (float)YMin)
		v->y = 10;
	else if (b->t.p1.y > (float)YMax) 
		v->y = -10;

	return v;
}

struct vector *
LimitSpeed(struct boid *b)
{
	struct vector *v = malloc(sizeof(struct vector));

	if (GetMagnitude(v) > MAX_SPEED) {
		VectorDivide(&b->velocity, &b->velocity, GetMagnitude(v));
		b->velocity.x = b->velocity.x * (float)MAX_SPEED;
		b->velocity.y = b->velocity.y * (float)MAX_SPEED;
	}
}

// Rule 1
struct vector *
MoveTowardsCentre(struct boid *b)
{
	struct vector *v = malloc(sizeof(struct vector));
	int i, j; 

	int boids_count = 0; // Count of boids in local radius

	v->x = 0;
	v->y = 0;

	for (i = 0; i < MAXBOIDS; i++) {
		// Check if its the same boid (memory addresses are the same)
		// If it is, skip it
		if (b == &boids[i])
			continue;
		
		if (check_within_radius(&b->t.p1, &boids[i].t.p1)) { 
			//printf("It is within the correct radius\n");
			VectorAdd(v, v, &boids[i].t.p1);
			boids_count++;
		}
		
	}
	
	// Nothing in the local radius
	if (v->x == 0 && v->y == 0)
		return v;

	// Calculate vector offset
	printf("Boids count: %d\n", boids_count);
	VectorDivide(v, v, (float)(boids_count));
	//VectorDivide(v, v, (float)(MAXBOIDS - 1));
	// 1% towards the centre = pcj = bj.position / 100
	VectorMinus(v, v, &boids[i].t.p1);	
	VectorDivide(v, v, 100);
	printf("The calculation yielded: x:%f, y:%f\n", v->x, v->y);
	
	return v; 
}

// Rule 2
static void
KeepDistance(void)
{
	int i, j;
	float distance, a, b, c_squared;
	// Initialise our new vector to 0
	for (i = 0; i < MAXBOIDS; i++) {
		boids[i].r2[0] = 0;
		boids[i].r2[1] = 0;
		for (j = 0; j < MAXBOIDS; j++) {
			if (i == j)
				continue;
			// Use pythagorus to determine distance between two boids
			a = (boids[i].v1[0] - boids[j].v1[0]);
			b = (boids[i].v1[1] - boids[j].v1[1]);
			c_squared = (a * a) + (b * b);
			distance = sqrt(c_squared);
			// If the distance is less than 10 away from each other, apply rule
			if (distance < 20) {
				boids[i].r2[0] = boids[i].r2[0] - (boids[j].v1[0] - boids[i].v1[0]);
				boids[i].r2[1] = boids[i].r2[1] - (boids[j].v1[1] - boids[i].v1[1]);
			}
 		} 
	}
}

// Rule 3
struct vector *
MatchVelocity(struct boid *b)
{
	int i, j;
	struct vector *v = malloc(sizeof(struct vector));

	int boids_count = 0;
	v->x = 0;
	v->y = 0;

	for (i = 0; i < MAXBOIDS; i++) {
		if (b == &boids[i])
			continue;

		if (check_within_radius(&b->t.p1, &boids[i].t.p1)) {
			VectorAdd(v, v, &boids[i].velocity);
			boids_count++;
		}
	}	

	if (!boids_count)
		return v;
		
	// Calcualte perceived velocity
	VectorDivide(v, v, boids_count);
	
	// Add a portion to the current velocity (lets say 1/8th)
	VectorMinus(v, v, &b->velocity);
	VectorDivide(v, v, 8.0);	

	return v;
}

static void
DetermineNewPositions(void)
{
	/* each boids new position can be determined through the following rules:
	   1. boids try to fly towards teh centre of mass of neighbouring boids
	   pcj = b1.position + b2.position + ... + bj-1.position + bj+1.position + ... + bn.position) / (N-1) */

	// Apply rule 1
}

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(windW, windH);
	glutCreateWindow("OpenGL Boids");
	Init();
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutSpecialFunc(SpecialKey);
	glutMouseFunc(Mouse);
	glutPassiveMotionFunc(MoveMouse);
	glutDisplayFunc(Draw);
	glutTimerFunc(0, Timer, 0);
	glutMainLoop();
	return 0;             /* ANSI C requires main to return int. */
}
