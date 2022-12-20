/*******************************************************************
		   Hierarchical Multi-Part Model Example
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <GL/glew.h>
#include <gl/glut.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "QuadMesh.h"
#include "stb_image.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


#define toRad 0.01745329251

using namespace std;

const int vWidth = 650;    // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

// Note how everything depends on robot body dimensions so that can scale entire robot proportionately
// just by changing robot body scale
float robotBodyWidth = 4.0;
float robotBodyLength = 4.5;
float robotBodyDepth = 5.0;
float headWidth = 0.45*robotBodyWidth;
float headLength = 0.55*robotBodyLength;
float headDepth = 0.3*robotBodyDepth;
float hipWidth = 0.55* robotBodyWidth;
float hipLength = 0.25*robotBodyLength;
float hipDepth = 0.55 * robotBodyDepth;
float gunLength = 0.5*headLength;
float gunWidth = 0.5*headWidth;
float gunDepth = 0.5 * headLength;
float legLength = robotBodyLength;
float legWidth = 0.2 * robotBodyWidth;
float legDepth = 0.2 * robotBodyDepth;
float stanchionLength = robotBodyLength;
float stanchionRadius = 0.1*robotBodyDepth;
float baseWidth = 2 * robotBodyWidth;
float baseLength = 0.25*stanchionLength;

GLuint  program = 0;
GLint	texMapLocation;
GLint	normalLocation;
GLint   ligntPosLocation;



// Control Robot body rotation on base
float robotAngle = 0.0;

// Control arm rotation
float gunAngle = -40.0;

// Control hip rotation
float hipAngle = 0.0;

// Control Knee rotation
float kneeAngle = 0.0;

// Lighting/shading and material properties for robot - upcoming lecture - just copy for now
// Robot RGBA material properties (NOTE: we will learn about this later in the semester)
GLfloat robotBody_mat_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat robotBody_mat_specular[] = { 0.45f,0.55f,0.45f,1.0f };
GLfloat robotBody_mat_diffuse[] = { 0.1f,0.35f,0.1f,1.0f };
GLfloat robotBody_mat_shininess[] = { 32.0F };


GLfloat robotArm_mat_ambient[] = { 0.0215f, 0.1745f, 0.0215f, 0.55f };
GLfloat robotArm_mat_diffuse[] = { 0.5f,0.0f,0.0f,1.0f };
GLfloat robotArm_mat_specular[] = { 0.7f, 0.6f, 0.6f, 1.0f };
GLfloat robotArm_mat_shininess[] = { 32.0F };

GLfloat gun_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gun_mat_diffuse[] = { 0.01f,0.0f,0.01f,0.01f };
GLfloat gun_mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat gun_mat_shininess[] = { 100.0F };

GLfloat robotLowerBody_mat_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat robotLowerBody_mat_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat robotLowerBody_mat_specular[] = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
GLfloat robotLowerBody_mat_shininess[] = { 76.8F };

GLfloat robotEye_mat_ambient[] = { 1.0f,0.0f,0.0f,1.0f };


// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

int startingZ = -90;
int startingZ2 = -95;

// Mouse button
int currentButton;

// A flat open mesh
QuadMesh *groundMesh = NULL;

// Structure defining a bounding box, currently unused
typedef struct BoundingBox {
	VECTOR3D min;
	VECTOR3D max;
} BBox;

typedef struct {
	int active;
	double x, y, z, dx, dy, dz, angleX, angleY;
} Bullet;

static Bullet bullets[50];

double canonX = 0.0;
double canonY = 0.0;

// Bullets for each robot
static Bullet bulletsR1[20];
static Bullet bulletsR2[20];

// Bullet Functionality for Player
void createBullet() {
	for (int c = 0; c < 50; c++) {
		if (bullets[c].active == 0) {
			bullets[c].active = 1;
			bullets[c].x = 0;
			bullets[c].y = 0;
			bullets[c].z = 20;
			bullets[c].angleX = canonX;
			bullets[c].angleY = canonY;
			break;
		}
	}
}

int hit1 = 0;
int hit2 = 0;
int hitCannon = 0;

// Robot Hit Functionality

void hitDetection(Bullet b) 
{
	if (hit1 == 0) {
		if (b.active == 1) {
			if (b.z <= (startingZ + 3) && b.z >= (startingZ - 3)) {
				if (b.x >= 8 && b.x <= 12) {
					if (b.y >= -1 && b.y <= 2) hit1 = 1;
				}
			}
		}
	}
	if (hit2 == 0) {
		if (b.active == 1) {
			if (b.z <= (startingZ2 + 3) && b.z >= (startingZ2 - 3)) {
				if (b.x <= -8 && b.x >= -12) {
					if (b.y >= -1 && b.y <= 2) hit2 = 1;
				}
			}
		}
	}
}

// Functionality when cannon is hit
void cannonHit(Bullet b){
	if (hitCannon == 0) {
		if (b.z <= 22 && b.z >= 16 && b.x >= -2.5 && b.x <= 2.5 && b.y >= -1 && b.y <= 3) {
			hitCannon = 1;
		}
	}
}

void moveBullet() {
	for (int c = 0; c < 50; c++) {

		// For Player

		if (bullets[c].active == 1) {
			bullets[c].z -= 2.5;
			if (bullets[c].angleY != 0) {
				if (bullets[c].angleY < 0) {
					bullets[c].x -= 2 * tan((180 + bullets[c].angleY) * toRad);
				}
				else {
					bullets[c].x += 2 * tan((180 - bullets[c].angleY) * toRad);
				}
			}
			if (bullets[c].angleX > 0) {
				bullets[c].y += 2 * tan((180 + bullets[c].angleX) * toRad);
			}
			hitDetection(bullets[c]);
			if (bullets[c].z <= -90) bullets[c].active = 0;
		}
	}

	// For Robots
	for (int c = 0; c < 20; c++) {
		if (bulletsR1[c].active == 1) {
			bulletsR1[c].z += 2.5;
			if (bulletsR1[c].angleY != 0) {
				if (bulletsR1[c].angleY < 0) {
					bulletsR1[c].x -= 2 * tan((180 + bulletsR1[c].angleY) * toRad);
				}
				else {
					bulletsR1[c].x += 2 * tan((180 - bulletsR1[c].angleY) * toRad);
				}
			}
			if (bulletsR1[c].angleX > 0) {
				bulletsR1[c].y += 2 * tan((180 + bulletsR1[c].angleX) * toRad);
			}
			else {
				bulletsR1[c].y -= 2 * tan((180 - bulletsR1[c].angleX) * toRad);
			}
			cannonHit(bulletsR1[c]);
			if (bulletsR1[c].z >= 20) {
				bulletsR1[c].active = 0;
			}
		}
	}

	for (int c = 0; c < 20; c++) {
		if (bulletsR2[c].active == 1) {
			bulletsR2[c].z += 2.5;
			if (bulletsR2[c].angleY != 0) {
				if (bulletsR2[c].angleY < 0) {
					bulletsR2[c].x -= 2 * tan((180 + bulletsR2[c].angleY) * toRad);
				}
				else {
					bulletsR2[c].x += 2 * tan((180 - bulletsR2[c].angleY) * toRad);
				}
			}
			if (bulletsR2[c].angleX > 0) {
				bulletsR2[c].y += 2 * tan((180 + bulletsR2[c].angleX) * toRad);
			}
			else {
				bulletsR2[c].y -= 2 * tan((180 - bulletsR2[c].angleX) * toRad);
			}
			cannonHit(bulletsR2[c]);
			if (bulletsR2[c].z >= 20) {
				bulletsR2[c].active = 0;
			}
		}
	}
}


// Bullet functionality for robot

void createBulletRobot() {

	srand(time(NULL));

	if (hit2 == 0) {
		for (int c = 0; c < 20; c++) {
			if (bulletsR1[c].active == 0) {
				bulletsR1[c].active = 1;
				bulletsR1[c].z = startingZ2 + 2;
				bulletsR1[c].y = 1;
				bulletsR1[c].x = -10;
				bulletsR1[c].angleX = rand() % 100 - 50;
				bulletsR1[c].angleY = rand() % 100 - 50;
				break;
			}
		}
	}
	if (hit1 == 0) {
		for (int c = 0; c < 20; c++) {
			if (bulletsR2[c].active == 0) {
				bulletsR2[c].active = 1;
				bulletsR2[c].z = startingZ + 2;
				bulletsR2[c].y = 1;
				bulletsR2[c].x = 10;
				bulletsR2[c].angleX = rand() % 100 - 50;
				bulletsR2[c].angleY = rand() % 100 - 50;
				break;
			}
		}
	}
}




// Draws bullet
void drawBullet(Bullet b) {
	glPushMatrix();
	glTranslatef(b.x, b.y, b.z);
	glRotatef(b.angleX, 1.0, 0.0, 0);
	glRotatef(b.angleY, 0.0, 1.0, 0);
	glScalef(0.5, 0.5, 1);
	glutSolidCube(1);
	glPopMatrix();
}

// Default Mesh Size
int meshSize = 100;

// Prototypes for functions in this module
void initOpenGL(int w, int h);

void display(void);
void reshape(int w, int h);
void mouseMotionHandler(int xMouse, int yMouse);
void mouseButtonHandler(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void robotShoot(int param);
void stepAnimation();
void walkAnimation(int param);
void hitDetection(Bullet b);
void drawBox(GLfloat size, GLenum type);
void meshExport();
void createBullet();
void drawRobot();
void drawBody();
void drawHead();
void drawHip();
void drawRightLeg();
void drawLeftLeg();


int lastX;
int lastY;


static char* readShaderSource(const char* shaderFile)
{
	errno_t err;

	struct stat statBuf;
	FILE* fp;
	if ((err = fopen_s(&fp, shaderFile, "r")) != 0)
	{
		exit(0);
	}

	char* buf;

	stat(shaderFile, &statBuf);
	buf = (char*)malloc((statBuf.st_size + 1) * sizeof(char));
	fread(buf, 1, statBuf.st_size, fp);
	buf[statBuf.st_size] = '\0';
	fclose(fp);
	return buf;
}

/* error printing function */

static void checkError(GLint status, const char *msg)
{
	if (!status)
	{
		printf("%s\n", msg);
		exit(EXIT_FAILURE);
	}
}

static void initShader(const GLchar* vShaderFile, const GLchar* fShaderFile)
{
	GLint status = glGetError() == GL_NO_ERROR;
	GLchar *vSource, *fSource;
	GLuint vShader, fShader;
	GLuint texMapLocation;

	/* read shader files */

	vSource = readShaderSource(vShaderFile);
	checkError(status, "Failed to read vertex shader");

	fSource = readShaderSource(fShaderFile);
	checkError(status, "Failed to read fragment shader");

	/* create program and shader objects */

	vShader = glCreateShader(GL_VERTEX_SHADER);
	fShader = glCreateShader(GL_FRAGMENT_SHADER);
	program = glCreateProgram();

	/* attach shaders to the program object */

	glAttachShader(program, vShader);
	glAttachShader(program, fShader);

	/* read shaders */

	glShaderSource(vShader, 1, (const GLchar**)&vSource, NULL);
	glShaderSource(fShader, 1, (const GLchar**)&fSource, NULL);

	/* compile shaders */

	glCompileShader(vShader);
	glCompileShader(fShader);

	/* error check */

	glGetShaderiv(vShader, GL_COMPILE_STATUS, &status);
	//checkError(status, "Failed to compile the vertex shader.");

	glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);
	//checkError(status, "Failed to compile the fragment shader.");

	/* link */

	glLinkProgram(program);
	glGetShaderiv(program, GL_LINK_STATUS, &status);
	//checkError(status, "Failed to link the shader program object.");

	/* use program object */

	glUseProgram(program);

	/* set up uniform parameter */

	texMapLocation = glGetUniformLocation(program, "texMap");
}



int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("3D Hierarchical Example");

	if (glewInit() != GLEW_OK)
		exit(EXIT_FAILURE);

	// Initialize GL
	initOpenGL(vWidth, vHeight);
	initShader("vertex.glsl", "fragment.glsl"); 


	// Register callback functions
	
	
	walkAnimation(0);
	robotShoot(0);
	glutDisplayFunc(display);
	
	glutReshapeFunc(reshape);
	glutMouseFunc(mouseButtonHandler);
	glutMotionFunc(mouseMotionHandler);
	glutIdleFunc(stepAnimation);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	


	
	// Start event loop, never returns
	glutMainLoop();

	return 0;
}

unsigned int texture;
GLuint texHandle;
GLuint head;
GLuint Rleg;


// Set up OpenGL. For viewport and projection setup see reshape(). 
void initOpenGL(int w, int h)
{

	const GLfloat  lightPos[4] = { 5.0f, 15.0f, 15.0f, 1.0f };
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	

	

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective


	glGenTextures(1, &texHandle);
	glBindTexture(GL_TEXTURE_2D, texHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	int width, height, nrChannels;
	unsigned char *data = stbi_load("steel2.tmp", &width, &height, &nrChannels, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	stbi_image_free(data);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// Other initializatuion
	// Set up ground quad mesh
	VECTOR3D origin = VECTOR3D(-300.0f, 0.0f, 300.0f);
	VECTOR3D dir1v = VECTOR3D(10.0f, 0.0f, 0.0f);
	VECTOR3D dir2v = VECTOR3D(0.0f, 0.0f, -10.0f);
	groundMesh = new QuadMesh(meshSize, 200.0);
	groundMesh->InitMesh(meshSize, origin, 200.0, 200, dir1v, dir2v);

	VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
	VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
	VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
	float shininess = 0.2;
	groundMesh->SetMaterial(ambient, diffuse, specular, shininess);
	

}

double lookX = 0;
double lookY = 0;


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	glLoadIdentity();
	// Create Viewing Matrix V
	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, lookX, lookY, 0.0, 0.0, 1.0, 0.0);

	// Draw Robot

	// Apply modelling transformations M to move robot
	// Current transformation matrix is set to IV, where I is identity matrix
	// CTM = IV
	if (hit1 == 0) {
		glPushMatrix();
		glTranslatef(10, 0, startingZ);
		drawRobot();
		glPopMatrix();
	}
	
	if (hit2 == 0) {
		glPushMatrix();
		glTranslatef(-10, 0, startingZ2);
		drawRobot();
		glPopMatrix();
	}


	// Draw ground
	glPushMatrix();
	glTranslatef(0, -10, 0);
	groundMesh->DrawMesh(meshSize);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 18);
	if (hitCannon == 0) {
		glRotatef(canonY, 0, 1.0, 0);
		glRotatef(canonX, 1.0, 0, 0);
	}
	else {
		glRotatef(0, 0, 1.0, 0);
		glRotatef(-25, 1.0, 0, 0);
	}
	
	glRotatef(90, 1.0, 0, 0);
	
	meshExport();
	glPopMatrix();

	if (hitCannon == 0) {
		for (int c = 0; c < 50; c++) {
			if (bullets[c].active == 1) {
				drawBullet(bullets[c]);
			}
		}
	}

	for (int c = 0; c < 20; c++) {
		if (bulletsR1[c].active == 1) drawBullet(bulletsR1[c]);
		if (bulletsR2[c].active == 1) drawBullet(bulletsR2[c]);
	}

	


	glutSwapBuffers();   // Double buffering, swap buffers

}

void drawRobot()
{

	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	// spin robot on base. 
	glRotatef(robotAngle, 0.0, 1.0, 0.0);

	drawBody();
	drawHead();
	
	//drawRightArm();
	glPopMatrix();

	glPushMatrix();

	glRotatef(hipAngle, 0.0, 1.0, 0.0);

	drawHip();

	glPushMatrix();
	glRotatef(kneeAngle, 1.0, 0.0, 0.0);
	drawRightLeg();
	glPopMatrix();

	glPushMatrix();
	glRotatef(-kneeAngle, 1.0, 0.0, 0.0);
	drawLeftLeg();
	glPopMatrix();

	glPopMatrix();

	glPopMatrix();

	glDisable(GL_TEXTURE_2D);

}
void drawBox(GLfloat size, GLenum type)
{
	

	static GLfloat n[6][3] =
	{
	  {-1.0, 0.0, 0.0},
	  {0.0, 1.0, 0.0},
	  {1.0, 0.0, 0.0},
	  {0.0, -1.0, 0.0},
	  {0.0, 0.0, 1.0},
	  {0.0, 0.0, -1.0}
	};
	static GLint faces[6][4] =
	{
	  {0, 1, 2, 3},
	  {3, 2, 6, 7},
	  {7, 6, 5, 4},
	  {4, 5, 1, 0},
	  {5, 6, 2, 1},
	  {7, 4, 0, 3}
	};
	GLfloat v[8][3];
	GLint i;

	v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

	for (i = 5; i >= 0; i--) {
		glBegin(type);
		glNormal3fv(&n[i][0]);
		glTexCoord2f(0, 0);
		glVertex3fv(&v[faces[i][0]][0]);
		glTexCoord2f(0, 1);
		glVertex3fv(&v[faces[i][1]][0]);
		glTexCoord2f(1, 1);
		glVertex3fv(&v[faces[i][2]][0]);
		glTexCoord2f(1, 0);
		glVertex3fv(&v[faces[i][3]][0]);
		glEnd();
	}
}

void drawBody()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLowerBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLowerBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLowerBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLowerBody_mat_shininess);

	glPushMatrix();
	glScalef(robotBodyWidth, robotBodyLength, robotBodyDepth);
	drawBox(1, GL_QUADS);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, robotBodyLength * 0.5, robotBodyDepth*0.3);
	glScalef(robotBodyWidth * 0.2, 1.2, 1.5);
	drawBox(1, GL_QUADS);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, robotBodyLength * 0.5, robotBodyDepth*0.35);
	glScalef(robotBodyWidth * 0.2, 1.2, robotBodyDepth*0.2);
	drawBox(1, GL_QUADS);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(0.0, 2.5, 2.25);
	glScalef(robotBodyWidth * 0.05, 0.25, robotBodyDepth*0.05);
	drawBox(1, GL_QUADS);
	glPopMatrix();
	



}
void drawHead()
{
	// Set robot material properties per body part. Can have seperate material properties for each part

	glPushMatrix();
	// Position head with respect to parent (body)
	glTranslatef(0, 0, 0.5*robotBodyWidth + 0.5*headWidth); // this will be done last

	// Build Head
	glPushMatrix();
	glScalef(headWidth, headLength, headDepth);
	glRotatef(45.0, 0, 0, 1);
	drawBox(1, GL_QUADS);
	glPopMatrix();

	//  Gun
	glMaterialfv(GL_FRONT, GL_AMBIENT, gun_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, gun_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, gun_mat_shininess);

	glPushMatrix();
	glTranslatef(0,0,headWidth*0.5);
	glScalef(0.5*headWidth, 0.5*headLength, 0.5*headDepth);
	glRotatef(gunAngle, 0.0, 0.0, 1.0);
	glRotatef(45.0, 0, 0, 1);
	drawBox(1, GL_QUADS);
	glPopMatrix();


	GLUquadricObj *gun;
	gun = gluNewQuadric();
	gluQuadricDrawStyle(gun, GLU_FILL);

	glPushMatrix();
	glTranslatef(0,0,0.5*headWidth);
	glRotatef(gunAngle, 0.0, 0.0, 1.0);
	gluCylinder(gun, 0.2, 0.2, 1, 32, 32);
	glPopMatrix();

	glMaterialfv(GL_FRONT, GL_AMBIENT, robotEye_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);


	glPopMatrix();
}

void drawHip()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLowerBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLowerBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLowerBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLowerBody_mat_shininess);

	GLUquadricObj *myCy;
	myCy = gluNewQuadric();
	gluQuadricDrawStyle(myCy, GLU_FILL);

	glPushMatrix();
	glTranslatef(0,- robotBodyLength * 0.5 - 0.5* hipLength, 0);
	glScalef(hipWidth, hipLength, hipDepth);
	drawBox(1, GL_QUADS);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(hipWidth * 0.5, -robotBodyLength * 0.5 - 0.4* hipLength, 0);
	glRotatef(90.0, 0, 1, 0);
	gluCylinder(myCy, 0.35, 0.35, 1.5, 32, 32);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(-hipWidth / 0.83, -robotBodyLength * 0.5 - 0.4* hipLength, 0);
	glRotatef(90.0, 0, 1, 0);
	gluCylinder(myCy, 0.35, 0.35, 1.5, 32, 32);
	glPopMatrix();
}

void drawRightLeg()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLowerBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLowerBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLowerBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLowerBody_mat_shininess);

	glPushMatrix();

	
	glPushMatrix();
	glTranslatef(hipWidth * 0.5 + robotBodyWidth*0.5, -robotBodyLength * 0.5 - 0.5* legLength, 0);
	glScalef(legWidth, legLength, legDepth);
	drawBox(1, GL_QUADS);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(hipWidth * 0.5 + robotBodyWidth * 0.5,( -robotBodyLength * 0.5 - 0.5* legLength) * 1.5, 0);
	drawBox(1.2, GL_QUADS);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(hipWidth * 0.5 + robotBodyWidth * 0.5, (-robotBodyLength * 0.5 - 0.5* legLength) * 2, 0);
	glScalef(legWidth, legLength, legDepth);
	drawBox(1, GL_QUADS);
	glPopMatrix();

	glPopMatrix();


}

void drawLeftLeg()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLowerBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLowerBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLowerBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLowerBody_mat_shininess);

	glPushMatrix();
	glTranslatef(hipWidth * -0.5 - robotBodyWidth * 0.5, -robotBodyLength * 0.5 - 0.5* legLength, 0);
	glScalef(legWidth, legLength, legDepth);
	drawBox(1, GL_QUADS);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(hipWidth * -0.5 - robotBodyWidth * 0.5, (-robotBodyLength * 0.5 - 0.5* legLength) * 1.5, 0);
	drawBox(1.2, GL_QUADS);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(hipWidth * -0.5 - robotBodyWidth * 0.5, (-robotBodyLength * 0.5 - 0.5* legLength) * 2, 0);
	glScalef(legWidth, legLength, legDepth);
	drawBox(1, GL_QUADS);
	glPopMatrix();
}


// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

bool stop = false;
bool stepStop = false;
unsigned char indicate = 0;

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	indicate = key;
	switch (key)
	{
	case 32:
		createBullet();
		break;
	}


	glutPostRedisplay();   // Trigger a window redisplay
}




void walkAnimation(int param)
{
	moveBullet();
	gunAngle += 5; 
	startingZ += 1;
	startingZ2 += 1;
	glutPostRedisplay();
	glutTimerFunc(100, walkAnimation, 0);
}

void robotShoot(int param) {
	createBulletRobot();
	glutPostRedisplay();
	glutTimerFunc(1000, robotShoot, 0);
}

void meshExport() {


	GLuint vertexVBO;
	GLuint normalsVBO;
	GLuint quadsVBO;

	GLdouble *vertices;
	GLuint *quads;

	int v = 0;
	int q = 0;

	ifstream in("Object.obj", ios::in);
	string line;

	vertices = (double *)malloc(6 * (16 * 33) * sizeof(double));
	quads = (GLuint *)malloc(12 * (33 * 16 * sizeof(GLuint)));

	while (getline(in, line))
	{
		if (line.substr(0, 2) == "v ") {
			istringstream s(line.substr(2));
			s >> vertices[v];
			s >> vertices[v + 1];
			s >> vertices[v + 2];
			v += 3;
		}
		if (line.substr(0, 3) == "vn ") {
			istringstream s(line.substr(3));
			s >> vertices[v];
			s >> vertices[v + 1];
			s >> vertices[v + 2];
			v += 3;
		}
		if (line.substr(0, 2) == "f ") {
			istringstream s(line.substr(2));
			s >> quads[q];
			s >> quads[q + 1];
			s >> quads[q + 2];
			s >> quads[q + 3];
			q += 4;
		}
	}

	

	glGenBuffers(1, &vertexVBO);
	glGenBuffers(1, &quadsVBO);


	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadsVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * v, vertices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * q, quads, GL_STATIC_DRAW);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_DOUBLE, 0, 0);
	glNormalPointer(GL_DOUBLE, 0, (void*)((v / 2) * sizeof(GLdouble)));

	glDrawElements(GL_QUADS, q, GL_UNSIGNED_INT, (void*)0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	free(vertices);
	free(quads);
}
int a = 0;

void stepAnimation() {
	a++;
	if (a <= 1000) {
		if (hipAngle >= -15 && kneeAngle >= -5) {
			kneeAngle -= 0.1;
			hipAngle -= 0.05;
		}
	}
	else if (a < 2000 && a > 1000) {
		if (hipAngle <= 15 && kneeAngle <= 5) {
			kneeAngle += 0.1;
			hipAngle += 0.05;
		}
		
	}
	if (a == 2000) {
		a = 0;
	}
	
	
		
}




// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{

	// Help key
	if (key == GLUT_KEY_F1)
	{

	}
	else if (indicate == 'b')
	{
		if (key == GLUT_KEY_RIGHT) robotAngle += 2.0;
		if (key == GLUT_KEY_LEFT) robotAngle -= 2.0;
	}
	else if (indicate == 'h')
	{
		if (key == GLUT_KEY_RIGHT) hipAngle += 2.0;
		if (key == GLUT_KEY_LEFT) hipAngle -= 2.0;
	}
	else if (indicate == 'k')
	{
		if( kneeAngle <= 10) if (key == GLUT_KEY_RIGHT) kneeAngle += 2.0;
		if (kneeAngle >= -10) if (key == GLUT_KEY_LEFT) kneeAngle -= 2.0;
	} 
	else if (indicate == 'R')
	{
		hit1 = 0;
		hit2 = 0;
		hitCannon = 0;
		canonX = 0;
		canonY = 0;
		for (int c = 0; c < 50; c++) {
			if (bullets[c].active == 1) bullets[c].active = 0;
			if (c < 10) {
				if (bulletsR1[c].active == 1) bulletsR1[c].active = 0;
				if (bulletsR2[c].active == 1) bulletsR2[c].active = 0;
			}
		}
		startingZ = -90;
		startingZ2 = -95;
		lookX = 0;
		lookY = 0;
	}

	// Do transformations with arrow keys
	//else if (...)   // GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
	//{
	//}

	glutPostRedisplay();   // Trigger a window redisplay
}



void mouseButtonHandler(int button, int state, int x, int y) {
	lastX = x;
	lastY = y;
	glutPostRedisplay();
}

// Mouse motion callback - use only if you want to 
bool first = true;

int lastX2;
int lastY2;

void mouseMotionHandler(int x, int y)
{

	lastX2 -= x;
	lastY2 -= y;
	if (hitCannon == 0) {
		if (lastX2 > 0 && lookX >= -6) { lookX -= 0.15; }
		if (lastX2 < 0 && lookX <= 6) { lookX += 0.15; }
		if (lastY2 > 0 && lookY < 3) lookY += 0.05;
		if (lastY2 < 0 && lookY > 0) lookY -= 0.15;
	}
	lastX2 = x;
	lastY2 = y;

	if (first) {
		lastX = 0;
		lastY = 0;
		first = false;
	}
	else {
		lastX = lastX + x;
		lastY = lastY + y;
	}

	
	if (hitCannon == 0) {
		canonY = 360 + (0.5 * -lastX);
		canonX = 360 + (0.5* -lastY);
		if (canonY < -50) canonY = -50;
		if (canonY > 50) canonY = 50;
		if (canonX < 0) canonX = 0;
		if (canonX > 50) canonX = 50;
		lastX = x;
		lastY = y;
	}

	
}