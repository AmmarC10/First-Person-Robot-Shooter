#include <stdio.h>
#include <windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <string.h>
#include "surfaceModeller.h"
#include "subdivcurve.h"
#include <iostream>
#include <fstream>

using namespace std;

GLdouble worldLeft = -12;
GLdouble worldRight = 12;
GLdouble worldBottom = -9;
GLdouble worldTop = 9;
GLdouble worldCenterX = 0.0;
GLdouble worldCenterY = 0.0;
GLdouble wvLeft = -12;
GLdouble wvRight = 12;
GLdouble wvBottom = -9;
GLdouble wvTop = 9;

GLint glutWindowWidth = 800;
GLint glutWindowHeight = 600;
GLint viewportWidth = glutWindowWidth;
GLint viewportHeight = glutWindowHeight;

// screen window identifiers
int window2D, window3D;

int window3DSizeX = 800, window3DSizeY = 600;
GLdouble aspect = (GLdouble)window3DSizeX / window3DSizeY;



int main(int argc, char* argv[])
{
	glutInit(&argc, (char **)argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(glutWindowWidth,glutWindowHeight);
	glutInitWindowPosition(50,100);  
	
	// The 2D Window
	window2D = glutCreateWindow("Profile Curve"); 
	glutDisplayFunc(display2D);
	glutReshapeFunc(reshape2D);
	// Initialize the 2D profile curve system
	init2DCurveWindow(); 
	// A few input handlers
	glutMouseFunc(mouseButtonHandler2D);
	glutMotionFunc(mouseMotionHandler2D);
	glutPassiveMotionFunc(mouseHoverHandler2D);
	glutMouseWheelFunc(mouseScrollWheelHandler2D);
	glutSpecialFunc(specialKeyHandler2D);
	glutKeyboardFunc(keyboardHandler2D);
	
	// The 3D Window
	window3D = glutCreateWindow("Surface of Revolution"); 
	glutPositionWindow(900,100);  
	glutDisplayFunc(display3D);
	glutReshapeFunc(reshape3D);
	glutMouseFunc(mouseButtonHandler3D);
	glutMouseWheelFunc(mouseScrollWheelHandler3D);
	glutMotionFunc(mouseMotionHandler3D);
	glutKeyboardFunc(keyboardHandler3D);

	if (glewInit() != GLEW_OK)
		exit(EXIT_FAILURE);

	// Initialize the 3D system
	init3DSurfaceWindow();

	// Annnd... ACTION!!
	glutMainLoop(); 

	return 0;
}

/************************************************************************************
 *
 *
 * 2D Window and Profile Curve Code
 *
 * Fill in the code in the empty functions
 ************************************************************************************/

// The profile curve is a subdivision curve
SubdivisionCurve subcurve;

int hoveredCtlPt = -1;
int currentCurvePoint = 0;

// Use little circles to draw subdivision curve control points
Circle circles[MAXCONTROLPOINTS];
int numCirclePoints = 30;
double circleRadius = 0.2;


void init2DCurveWindow() 
{ 
	glLineWidth(3.0);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);
	initSubdivisionCurve();
	initControlPointCircles();
} 

void display2D()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();
	gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	draw2DScene();	
	glutSwapBuffers();
}


void draw2DScene() 
{
	drawAxes();
	drawSubdivisionCurve();
	drawControlPoints();
}

void drawAxes()
{
	glPushMatrix();
	glColor3f(1.0, 0.0, 0);
	glBegin(GL_LINE_STRIP);
	glVertex3f(0, 8.0, 0);
	glVertex3f(0, -8.0, 0);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(-8, 0.0, 0);
	glVertex3f(8, 0.0, 0);
	glEnd();
	glPopMatrix();
}

void drawSubdivisionCurve() 
{
	// Subdivide the given curve
	computeSubdivisionCurve(&subcurve);
	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	glBegin(GL_LINE_STRIP);
	for (int i=0; i<subcurve.numCurvePoints; i++)
	{
		glVertex3f(subcurve.curvePoints[i].x, subcurve.curvePoints[i].y, 0.0);
	}
	glEnd();
	glPopMatrix();
}

void drawControlPoints()
{
	int i, j;
	for (i=0; i<subcurve.numControlPoints; i++){
		glPushMatrix();
		glColor3f(1.0f,0.0f,0.0f); 
		glTranslatef(circles[i].circleCenter.x, circles[i].circleCenter.y, 0);
		// for the hoveredCtlPt, draw an outline and change its color
		if (i == hoveredCtlPt)
		{ 
			// outline
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP); 
			for(j=0; j < numCirclePoints; j++) {
				glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0); 
			}
			glEnd();
			// color change
			glColor3f(0.5,0.0,1.0);
		}
		glBegin(GL_LINE_LOOP); 
		for(j=0; j < numCirclePoints; j++) {
			glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0); 
		}
		glEnd();
		glPopMatrix();
	}
}

void initSubdivisionCurve() {
	// Initialize 5 control points of the subdivision curve

	GLdouble x, y;

	x = 2 * cos(M_PI*0.5);
	y = 2 * sin(M_PI*0.5);
	subcurve.controlPoints[0].x = x;
	subcurve.controlPoints[0].y = y;

	x = 2 * cos(M_PI*0.25);
	y = 2 * sin(M_PI*0.25);
	subcurve.controlPoints[1].x = x;
	subcurve.controlPoints[1].y = y;

	x = 2 * cos(M_PI*0.0);
	y = 2 * sin(M_PI*0.0);
	subcurve.controlPoints[2].x = x;
	subcurve.controlPoints[2].y = y;

	x = 2 * cos(-M_PI*0.25);
	y = 2 * sin(-M_PI*0.25);
	subcurve.controlPoints[3].x = x;
	subcurve.controlPoints[3].y = y;

	x = 2 * cos(-M_PI * 0.5);
	y = 2 * sin(-M_PI * 0.5);
	subcurve.controlPoints[4].x = x;
	subcurve.controlPoints[4].y = y;

	subcurve.numControlPoints = 5;
	subcurve.subdivisionSteps = 3;
}

void initControlPointCircles()
{
	int num = subcurve.numControlPoints;
	for (int i=0; i < num; i++){
		constructCircle(circleRadius, numCirclePoints, circles[i].circlePoints);
		circles[i].circleCenter = subcurve.controlPoints[i];
	}
}

void screenToWorldCoordinates(int xScreen, int yScreen, GLdouble *xw, GLdouble *yw)
{
	GLdouble xView, yView;
	screenToCameraCoordinates(xScreen, yScreen, &xView, &yView);
	cameraToWorldCoordinates(xView, yView, xw, yw);
}

void screenToCameraCoordinates(int xScreen, int yScreen, GLdouble *xCamera, GLdouble *yCamera)
{
	*xCamera = ((wvRight-wvLeft)/glutWindowWidth)  * xScreen; 
	*yCamera = ((wvTop-wvBottom)/glutWindowHeight) * (glutWindowHeight-yScreen); 
}

void cameraToWorldCoordinates(GLdouble xcam, GLdouble ycam, GLdouble *xw, GLdouble *yw)
{
	*xw = xcam + wvLeft;
	*yw = ycam + wvBottom;
}

void worldToCameraCoordiantes(GLdouble xWorld, GLdouble yWorld, GLdouble *xcam, GLdouble *ycam)
{
	double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
	double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
	*xcam = worldCenterX - wvCenterX + xWorld;
	*ycam = worldCenterY - wvCenterY + yWorld;
}

int currentButton;

void mouseButtonHandler2D(int button, int state, int xMouse, int yMouse)
{
	int i;
	
	currentButton = button;
	if (button == GLUT_LEFT_BUTTON)
	{  
		switch (state) {      
		case GLUT_DOWN:
			if (hoveredCtlPt > -1)
			{
			  screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCtlPt].circleCenter.x, &circles[hoveredCtlPt].circleCenter.y);
			  screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCtlPt].x, &subcurve.controlPoints[hoveredCtlPt].y);
			}
			break;
		case GLUT_UP:
			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}
	}    
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		switch (state) {      
		case GLUT_DOWN:
			break;
		case GLUT_UP:
			if (hoveredCtlPt == -1 && subcurve.numControlPoints < MAXCONTROLPOINTS)
			{ 
				GLdouble newPointX;
				GLdouble newPointY;
				screenToWorldCoordinates(xMouse, yMouse, &newPointX, &newPointY);
				subcurve.controlPoints[subcurve.numControlPoints].x = newPointX;
				subcurve.controlPoints[subcurve.numControlPoints].y = newPointY;			
				constructCircle(circleRadius, numCirclePoints, circles[subcurve.numControlPoints].circlePoints);
				circles[subcurve.numControlPoints].circleCenter = subcurve.controlPoints[subcurve.numControlPoints];
				subcurve.numControlPoints++;
			} 
			else if (hoveredCtlPt > -1 && subcurve.numControlPoints > MINCONTROLPOINTS) 
			{
				subcurve.numControlPoints--;
				for (i=hoveredCtlPt; i<subcurve.numControlPoints; i++)
				{
					subcurve.controlPoints[i].x = subcurve.controlPoints[i+1].x;
					subcurve.controlPoints[i].y = subcurve.controlPoints[i+1].y;
					circles[i].circleCenter = circles[i+1].circleCenter;
				}
			}
			
			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}
	}

	glutSetWindow(window2D);
	glutPostRedisplay();
}

void mouseMotionHandler2D(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON) {  
		if (hoveredCtlPt > -1) 
		{
			screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCtlPt].circleCenter.x, &circles[hoveredCtlPt].circleCenter.y);
			screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCtlPt].x, &subcurve.controlPoints[hoveredCtlPt].y);
			
		}
	}    
	glutPostRedisplay();
	glutSetWindow(window3D);
	glutPostRedisplay();
	glutSetWindow(window2D);
}

void mouseHoverHandler2D(int xMouse, int yMouse)
{
	hoveredCtlPt = -1;
	GLdouble worldMouseX, worldMouseY;
	screenToWorldCoordinates(xMouse, yMouse, &worldMouseX, &worldMouseY);
	// See if we're hovering over a circle
	for (int i=0; i<subcurve.numControlPoints; i++){
		GLdouble distToX = worldMouseX - circles[i].circleCenter.x;
		GLdouble distToY = worldMouseY - circles[i].circleCenter.y;
		GLdouble euclideanDist = sqrt(distToX*distToX + distToY*distToY);
		if (euclideanDist < 0.5)
		  hoveredCtlPt = i;
	}
	glutPostRedisplay();
}

void mouseScrollWheelHandler2D(int button, int dir, int xMouse, int yMouse)
{
	GLdouble worldViewableWidth;
	GLdouble worldViewableHeight;
	GLdouble cameraOnCenterX;
	GLdouble cameraOnCenterY;
	GLdouble anchorPointX, anchorPointY;
	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
	double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
	double wvWidth   = wvRight - wvLeft;
	double wvHeight  = wvTop   - wvBottom;
	
	worldToCameraCoordiantes(worldCenterX, worldCenterY, &cameraOnCenterX, &cameraOnCenterY);
	if (wvWidth >= (worldRight-worldLeft)*1.2)
	{
		anchorPointX = cameraOnCenterX;
		anchorPointY = cameraOnCenterY;
	} else 
	{
		// else, anchor the zoom to the mouse
		screenToWorldCoordinates(xMouse, yMouse, &anchorPointX, &anchorPointY);
	}
	GLdouble anchorToCenterX = anchorPointX - wvCenterX;
	GLdouble anchorToCenterY = anchorPointY - wvCenterY;

	// Set up maximum shift
	GLdouble maxPosShift = 50;
	GLdouble maxNegShift = -50;	
	anchorToCenterX = (anchorToCenterX > maxPosShift)? maxPosShift : anchorToCenterX;
	anchorToCenterX = (anchorToCenterX < maxNegShift)? maxNegShift : anchorToCenterX;
	anchorToCenterY = (anchorToCenterY > maxPosShift)? maxPosShift : anchorToCenterY;
	anchorToCenterY = (anchorToCenterY < maxNegShift)? maxNegShift : anchorToCenterY;

	// Move the world centre closer to this point.
	wvCenterX += anchorToCenterX/4;
	wvCenterY += anchorToCenterY/4;
	
	if (dir > 0) 
	{
		// Zoom in to mouse point
		clipWindowWidth = wvWidth*0.8;
		clipWindowHeight= wvHeight*0.8;
		wvRight =  wvCenterX + clipWindowWidth/2.0;
		wvTop   =  wvCenterY + clipWindowHeight/2.0;
		wvLeft  =  wvCenterX - clipWindowWidth/2.0;
		wvBottom=  wvCenterY - clipWindowHeight/2.0;
	}
	else 
	{
		// Zoom out
		clipWindowWidth = wvWidth*1.25;
		clipWindowHeight= wvHeight*1.25;
		wvRight =  wvCenterX + clipWindowWidth/2.0;
		wvTop   =  wvCenterY + clipWindowHeight/2.0;
		wvLeft  =  wvCenterX - clipWindowWidth/2.0;
		wvBottom=  wvCenterY - clipWindowHeight/2.0;
	}
	glutPostRedisplay();
}

void keyboardHandler2D(unsigned char key, int x, int y)
{
	int i;
	
	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
	double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
	double wvWidth   = wvRight - wvLeft;
	double wvHeight  = wvTop   - wvBottom;

	switch(key)
	{
	case 'q':
	case 'Q':
	case 27:
		// Esc, q, or Q key = Quit 
		exit(0);
		break;
	case 107:
	case '+':
		clipWindowWidth = wvWidth*0.8;
		clipWindowHeight= wvHeight*0.8;
		wvRight =  wvCenterX + clipWindowWidth/2.0;
		wvTop   =  wvCenterY + clipWindowHeight/2.0;
		wvLeft  =  wvCenterX - clipWindowWidth/2.0;
		wvBottom=  wvCenterY - clipWindowHeight/2.0;
		break;
	case 109:
	case '-':
		clipWindowWidth = wvWidth*1.25;
		clipWindowHeight= wvHeight*1.25;
		wvRight =  wvCenterX + clipWindowWidth/2.0;
		wvTop   =  wvCenterY + clipWindowHeight/2.0;
		wvLeft  =  wvCenterX - clipWindowWidth/2.0;
		wvBottom=  wvCenterY - clipWindowHeight/2.0;
		break;
	
	default:
		break;
	}
	glutPostRedisplay();
}

void specialKeyHandler2D(int key, int x, int y)
{
	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
	double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
	double wvWidth   = wvRight - wvLeft;
	double wvHeight  = wvTop   - wvBottom;

	switch(key)	
	{
	case GLUT_KEY_LEFT:
		wvLeft -= 5.0;
		wvRight-= 5.0;
		break;
	case GLUT_KEY_RIGHT:
		wvLeft += 5.0;
		wvRight+= 5.0;
		break;
	case GLUT_KEY_UP:
		wvTop   += 5.0;
		wvBottom+= 5.0;
		break;
	case GLUT_KEY_DOWN:
		wvTop   -= 5.0;
		wvBottom-= 5.0;
		break;
		// Want to zoom in/out and keep  aspect ratio = 2.0
	case GLUT_KEY_F1:
		clipWindowWidth = wvWidth*0.8;
		clipWindowHeight= wvHeight*0.8;
		wvRight =  wvCenterX + clipWindowWidth/2.0;
		wvTop   =  wvCenterY + clipWindowHeight/2.0;
		wvLeft  =  wvCenterX - clipWindowWidth/2.0;
		wvBottom=  wvCenterY - clipWindowHeight/2.0;
		break;
	case GLUT_KEY_F2:
		clipWindowWidth = wvWidth*1.25;
		clipWindowHeight= wvHeight*1.25;
		wvRight =  wvCenterX + clipWindowWidth/2.0;
		wvTop   =  wvCenterY + clipWindowHeight/2.0;
		wvLeft  =  wvCenterX - clipWindowWidth/2.0;
		wvBottom=  wvCenterY - clipWindowHeight/2.0;
		break;
	}
	glutPostRedisplay();
}


void reshape2D(int w, int h)
{
	glutWindowWidth = (GLsizei) w;
	glutWindowHeight = (GLsizei) h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}



/************************************************************************************
 *
 *
 * 3D Window and Surface of Revolution Code 
 *
 * Fill in the code in the empty functions
 ************************************************************************************/
 // Ground Mesh material
GLfloat groundMat_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
GLfloat groundMat_specular[] = { 0.01, 0.01, 0.01, 1.0 };
GLfloat groundMat_diffuse[] = { 0.4, 0.4, 0.7, 1.0 };
GLfloat groundMat_shininess[] = { 1.0 };

GLfloat light_position0[] = { 4.0, 8.0, 8.0, 1.0 };
GLfloat light_diffuse0[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_position1[] = { -4.0, 8.0, 8.0, 1.0 };
GLfloat light_diffuse1[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat model_ambient[] = { 0.5, 0.5, 0.5, 1.0 };

//
// Surface of Revolution consists of vertices and quads
//
// Set up lighting/shading and material properties for surface of revolution
GLfloat quadMat_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat quadMat_specular[] = { 0.45, 0.55, 0.45, 1.0 };
GLfloat quadMat_diffuse[] = { 0.1, 0.35, 0.1, 1.0 };
GLfloat quadMat_shininess[] = { 10.0 };


// Quads and Vertices of the surface of revolution
typedef struct Vertex
{
	GLdouble x, y, z;
	Vector3D normal;
	int numQuads;
	int quadIndex[4];
} Vertex;

typedef struct Quad
{
	int vertexIndex[4]; // 4 vertex indices in clockwise order
	Vector3D normal;
} Quad;

// Quads
Quad *qarray;
boolean quadArrayAllocated = false;

// Vertices 
#define NUMBEROFSIDES 16 // You may want to lower this to 4 or 5 when debugging

Vertex *varray; 
boolean varrayAllocated = false;

GLuint vertexVBO;
GLuint normalsVBO;
GLuint quadsVBO;

GLdouble *vertices = new GLdouble;
GLdouble *normals = new GLdouble;
GLuint *quads = new GLuint;

GLdouble fov = 60;
int lastMouseX;
int lastMouseY;

boolean drawAsLines = false;
boolean drawAsPoints = false;
boolean drawNormals = false;

boolean eyeXCh, eyeYCh, eyeZCh = false;

GLdouble eyeX = 0.0, eyeY = 3.0, eyeZ = 10.0;
GLdouble radius = eyeZ;
GLdouble zNear = 0.1, zFar = 40.0;

int vLength;
int qLength;


void init3DSurfaceWindow()
{
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
	
	glViewport(0, 0, (GLsizei)window3DSizeX, (GLsizei)window3DSizeY);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}


void reshape3D(int w, int h)
{
	glutWindowWidth = (GLsizei) w;
	glutWindowHeight = (GLsizei) h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	gluPerspective(fov,aspect,zNear,zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void display3D()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Set up the Viewing Transformation (V matrix)	
	gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	drawGround();

    // Build and Draw Surface of Revolution (Quad Mesh)
	buildVertexArray();
	buildQuadArray();
	computeQuadNormals();
	computeVertexNormals();

	// Draw quad mesh
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);

	if (drawAsLines)
	  drawQuadsAsLines();
	else if (drawAsPoints)
		drawQuadsAsPoints();
	else
	  drawQuads();

	glPopMatrix();

	meshExport();
	glutSwapBuffers();
}

void drawGround() 
{
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, groundMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, groundMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, groundMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, groundMat_shininess);
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glVertex3f(-22.0f, -4.0f, -22.0f);
	glVertex3f(-22.0f, -4.0f, 22.0f);
	glVertex3f(22.0f, -4.0f, 22.0f);
	glVertex3f(22.0f, -4.0f, -22.0f);
	glEnd();
	glPopMatrix();
}

void buildVertexArray()
{
	int row, col;
	double newX, newZ;
	GLdouble cumulativeTheta = 0;
	GLdouble theta = 360/(GLdouble)NUMBEROFSIDES; // = 18

	// Allocate memeory for the vertex array - needs to be done only once
	if (!varrayAllocated)
	{
		varray = (Vertex *)malloc(subcurve.numCurvePoints* NUMBEROFSIDES * sizeof(Vertex));
		varrayAllocated = true;
	}
	/*  -------------
		|  |  |  |  |
		-------------
		|  |  |  |  |
		-------------
		|  |  |  |  |
		-------------
		|  |  |  |  |
		-------------
	*/		
	
	
	for (row = 0; row < subcurve.numCurvePoints; row++) 
	{
		for (col = 0; col < NUMBEROFSIDES; col++) 
		{
			Vector3D newVector = rotateAroundY(subcurve.curvePoints[row].x, 0, cumulativeTheta);
			varray[row*NUMBEROFSIDES + col].numQuads = 0;
			varray[row*NUMBEROFSIDES + col].y = subcurve.curvePoints[row].y;
			varray[row*NUMBEROFSIDES + col].x = newVector.x;
			varray[row*NUMBEROFSIDES + col].z = newVector.z;
			cumulativeTheta += theta;
			}
		}
	}
	
	



Vector3D rotateAroundY(double x, double z, double theta)
{	
	Vector3D newVector;
	
	newVector.x =  cos(theta/180*M_PI) * x + sin(theta/180.0*M_PI) * z;
	newVector.z = -sin(theta/180*M_PI) * x + cos(theta/180.0*M_PI) * z;
	return newVector;
}

void buildQuadArray()
{
	int col, row, numQuads;
	
	if (!quadArrayAllocated)
	{
		qarray = (Quad *)malloc(sizeof(Quad)*(subcurve.numCurvePoints-1)*NUMBEROFSIDES);
	}
	/*  -------------
		|  |  |  |  |
		-------------
		|  |  |  |  |
		-------------
		|  |  |  |  |
		-------------
		|  |  |  |  |
		-------------
	*/
	for (row = 0; row < subcurve.numCurvePoints-1; row++)
	{
		for (col = 0; col < NUMBEROFSIDES; col++)
		{
			int nextCol;

			// if the we are at last column, then the next column must wrap around back to 0th column
			if (col == NUMBEROFSIDES - 1)
				nextCol = 0;
			else
				nextCol = col + 1;

			qarray[row*NUMBEROFSIDES + col].vertexIndex[0] = row * NUMBEROFSIDES + col;
			numQuads = varray[row*NUMBEROFSIDES + col].numQuads;
			varray[row*NUMBEROFSIDES + col].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
			varray[row*NUMBEROFSIDES + col].numQuads++;


			qarray[row*NUMBEROFSIDES + col].vertexIndex[1] = (row+1) * NUMBEROFSIDES + col;
			numQuads = varray[(row+1)*NUMBEROFSIDES + col].numQuads;
			varray[(row+1)*NUMBEROFSIDES + col].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
			varray[(row+1)*NUMBEROFSIDES + col].numQuads++;

			qarray[row*NUMBEROFSIDES + col].vertexIndex[2] = (row + 1) * NUMBEROFSIDES + nextCol;
			numQuads = varray[(row + 1)*NUMBEROFSIDES + nextCol].numQuads;
			varray[(row + 1)*NUMBEROFSIDES + nextCol].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
			varray[(row + 1)*NUMBEROFSIDES + nextCol].numQuads++;

			qarray[row*NUMBEROFSIDES + col].vertexIndex[3] = row * NUMBEROFSIDES + nextCol;
			numQuads = varray[row*NUMBEROFSIDES + nextCol].numQuads;
			varray[row*NUMBEROFSIDES + nextCol].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
			varray[row*NUMBEROFSIDES + nextCol].numQuads++;
		}
	}

}

void computeQuadNormals() 
{
	for (int row = 0; row < subcurve.numCurvePoints - 1; row++)
	{
		for (int col = 0; col < NUMBEROFSIDES; col++)
		{
			double normal_x = 0.0;
			double normal_y = 0.0;
			double normal_z = 0.0;

			int i, j;
			Vertex *vi;
			Vertex *vj;
			Vector3D normal;
			normal.x = normal.y = normal.z = 0;

			for (i = 0, j = 1; i < 4; i++, j++) 
			{
				if (j == 4) j = 0;
				vi = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[i]];
				vj = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[j]];
				
				normal.x += (((vi->z) + (vj->z)) * ((vj->y) - (vi->y)));
				normal.y += (((vi->x) + (vj->x)) * ((vj->z) - (vi->z)));
				normal.z += (((vi->y) + (vj->y)) * ((vj->x) - (vi->x)));
			}
			// Flip them - must be a problem in code above
			normal.x *= -1.0;
			normal.y *= -1.0;
			normal.z *= -1.0;
			Vector3D normalized = normalize(normal);
			qarray[row*NUMBEROFSIDES + col].normal = normalized;
		}
	}
}

void computeVertexNormals() 
{
	int col, row, i;
	Vector3D vn;


	for (row = 0; row < subcurve.numCurvePoints; row++)
	{
		for (col = 0; col < NUMBEROFSIDES; col++)
		{
			int numQuads = varray[row*NUMBEROFSIDES + col].numQuads;
			vn.x = vn.y = vn.z = 0;

			for (int i = 0; i < numQuads; i++)
			{
				int quadIndex = varray[row*NUMBEROFSIDES + col].quadIndex[i];
				vn.x += qarray[quadIndex].normal.x;
				vn.y += qarray[quadIndex].normal.y;
				vn.z += qarray[quadIndex].normal.z;
			}
			varray[row*NUMBEROFSIDES + col].normal = normalize(vn);

		}
	}
}

void meshExport()
{
	ofstream MyFile("Object.obj");
	MyFile << "object Mesh\n";
	int counter = 0;
	int v = 0;
	for (int c = 0; c < vLength/6; c++) {
		counter++;
		if (counter >= 1 && counter <= 3) {
			MyFile << "v " << vertices[v] << " " << vertices[v + 1] << " " << vertices[v + 2] << "\n";
		}
		v += 6;
		if (counter == 6) counter = 0;
	}
	counter = 0;
	v = 0;
	for (int c = 0; c < vLength / 6; c++) {
		counter++;
		if (counter > 3 && counter <= 6) {
			MyFile << "vn " << vertices[v + 3] << " " << vertices[v + 4] << " " << vertices[v + 5] << "\n";
		}
		v += 6;
		if (counter == 6) counter = 0;
	}
	for (int i = 0; i < qLength; i+=4) {
		MyFile << "f " << quads[i] << " " << quads[i + 1] << " " << quads[i + 2] << " " << quads[i + 3] << "\n";
	}

}


void drawQuads() 
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);
	vertices = (double *)malloc(6 * (NUMBEROFSIDES * subcurve.numCurvePoints) * sizeof(double));
	quads = (GLuint *)malloc(12 * (subcurve.numCurvePoints * NUMBEROFSIDES * sizeof(GLuint)));
	int v = 0;
	int q = 0;
	int index = 0;
	int temp = 0;
	
	for (int row = 0; row < subcurve.numCurvePoints - 1; row++)
	{
		for (int col = 0; col < NUMBEROFSIDES; col++)
		{
			Vertex *vertex = &varray[temp];
			glNormal3f(vertex->normal.x, vertex->normal.y, vertex->normal.z);
			glVertex3f(vertex->x, vertex->y, vertex->z);
			vertices[v] = vertex->x;
			vertices[v + 1] = vertex->y;
			vertices[v + 2] = vertex->z;
			vertices[v + 3] = vertex->normal.x;
			vertices[v + 4] = vertex->normal.y;
			vertices[v + 5] = vertex->normal.z;
			v += 6;
			quads[q] = qarray[temp].vertexIndex[index];
			quads[q + 1] = qarray[temp].vertexIndex[index + 1];
			quads[q + 2] = qarray[temp].vertexIndex[index + 2];
			quads[q + 3] = qarray[temp].vertexIndex[index + 3];
			q += 4;
			temp++;
		}
	}

	vLength = v;
	qLength = q;
	glGenBuffers(1, &vertexVBO);
	glGenBuffers(1, &quadsVBO);
	

	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadsVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * v, vertices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * q, quads, GL_STATIC_DRAW);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	unsigned int stride = sizeof(GLdouble) * (3 + 3);
	
	glVertexPointer(3, GL_DOUBLE, stride, 0);
	glNormalPointer(GL_DOUBLE, stride, (void*) (3 * sizeof(GLdouble)));

	glDrawElements(GL_QUADS, q, GL_UNSIGNED_INT, (void*)0); 
}






void drawQuadsAsPoints()
{
	// Fill in this code
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);
	glPushMatrix();
	for (int row = 0; row < subcurve.numCurvePoints - 1; row++)
	{
		for (int col = 0; col < NUMBEROFSIDES; col++)
		{
			glBegin(GL_POINTS);
			for (int i = 0; i < 4; i++)
			{
				Vertex *vertex = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[i]];
				glNormal3f(vertex->normal.x, vertex->normal.y, vertex->normal.z);
				glVertex3f(vertex->x, vertex->y, vertex->z);
			}
			glEnd();
		}
	}
	glPopMatrix();
}
void drawQuadsAsLines() 
{
 	// Fill in this code and draw mesh using lines
	
	glPushMatrix();
	for (int row = 0; row < subcurve.numCurvePoints - 1; row++)
	{
		for (int col = 0; col < NUMBEROFSIDES; col++)
		{
			glBegin(GL_LINE_LOOP);
			for (int i = 0; i < 4; i++)
			{
				Vertex *vertex = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[i]];
				glNormal3f(vertex->normal.x, vertex->normal.y, vertex->normal.z);
				glVertex3f(vertex->x, vertex->y, vertex->z);
			}
			glEnd();
		}
	}
	glPopMatrix();
	// Fill in this code and draw normal vectors as well
	if (drawNormals) {
		glPushMatrix();
		for (int row = 0; row < subcurve.numCurvePoints - 1; row++)
		{
			for (int col = 0; col < NUMBEROFSIDES; col++)
			{
				glBegin(GL_LINES);
				for (int i = 0; i < 4; i++)
				{
					Vertex *vertex = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[i]];
					glNormal3f(vertex->normal.x, vertex->normal.y, vertex->normal.z);
					glVertex3f(vertex->x, vertex->y, vertex->z);
					glVertex3f(vertex->x + vertex->normal.x, vertex->y + vertex->normal.y, vertex->z + vertex->normal.z);
				}
				glEnd();
			}
		}
		glPopMatrix();
	}

}

// A few utility functions - use VECTOR3D.h or glm if you prefer
Vector3D crossProduct(Vector3D a, Vector3D b)
{
	Vector3D cross;
	
	cross.x = a.y * b.z - b.y * a.z;
	cross.y = a.x * b.z - b.x * a.z;
	cross.z = a.x * b.y - b.x * a.y;
	return cross;
}

float DotProduct(Vector3D lhs, Vector3D rhs)
{
	return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

Vector3D fourVectorAverage(Vector3D a, Vector3D b, Vector3D c, Vector3D d)
{
	Vector3D average;
	average.x = (a.x + b.x + c.x + d.x)/4.0;
	average.y = (a.y + b.y + c.y + d.y)/4.0;
	average.z = (a.z + b.z + c.z + d.z)/4.0;
	return average;
}

Vector3D normalize(Vector3D a)
{
	GLdouble norm = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	Vector3D normalized;
	normalized.x = a.x/norm;
	normalized.y = a.y/norm;
	normalized.z = a.z/norm;
	return normalized;
}

// Mouse and Key Handling
//
//
void mouseButtonHandler3D(int button, int state, int x, int y)
{
	currentButton = button;
	lastMouseX = x;
	lastMouseY = y;
	}


void mouseScrollWheelHandler3D(int button, int dir, int xMouse, int yMouse) 
{
	// Fill in this code for zooming in and out
	if (dir > 0) fov -= 0.5;
	if (dir < 0) fov += 0.5;
	
	
}

// gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

void mouseMotionHandler3D(int x, int y)
{
	int dx = x - lastMouseX;
	int dy = y - lastMouseY;
	float angle = 0.0;
	if (currentButton == GLUT_LEFT_BUTTON)
	{
      // Fill in this code to control camera "orbiting" around surface
		eyeX = radius * sin(dx*0.02);
		eyeZ = cos(dx*0.02) * radius;
	
	}
	if (currentButton == GLUT_RIGHT_BUTTON) 
	{
      // Fill in this code to control camera elevation. Limit the elevation angle
		if (dy <= 60 && dy >= 0) {
			eyeY = sin(dy*0.02) * radius;
		}

	}
	else if (currentButton == GLUT_MIDDLE_BUTTON) 
	{
		// Fill in this code for zooming or ignore and use the scroll wheel
	}
	
	lastMouseX = x;
	lastMouseX = y;
	glutPostRedisplay();
}

void keyboardHandler3D(unsigned char key, int x, int y)
{
	
	switch (key)
	{
	case 'q':
	case 'Q':
	case 27:
		// Esc, q, or Q key = Quit 
		exit(0);
		break;
	case 'l':
		if (drawAsLines)
			drawAsLines = false;
		else
			drawAsLines = true;
		break;
	case 'p':
		if (drawAsPoints)
			drawAsPoints = false;
		else
			drawAsPoints = true;
		break;
	case 'n':
		if (drawNormals)
			drawNormals = false;
		else
			drawNormals = true;
	default:
		break;
	}
	glutPostRedisplay();
}