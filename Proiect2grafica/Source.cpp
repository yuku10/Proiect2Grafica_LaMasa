#include<gl/freeglut.h>
#include<math.h>
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include "SOIL.h"
#include <stdio.h> 

// angle of rotation for the camera direction
float angle = 0.0;
// actual vector representing the camera's direction
float lx = 0.0f, lz = -1.0f;
// XZ position of the camera
float x = 0.0f, z = 4.0f;


#ifndef CALLBACK
#define CALLBACK
#endif

using namespace std;
GLuint startList;

static GLint fogMode;

enum {
	X, Y, Z, W
};
enum {
	A, B, C, D
};

void CALLBACK errorCallback(GLenum errorCode)
{
	const GLubyte* estring;

	estring = gluErrorString(errorCode);
	fprintf(stderr, "Quadric Error: %s\n", estring);
	exit(0);
}


/* create a matrix that will project the desired shadow */
void shadowmatrix(GLfloat shadowMat[4][4],
	GLfloat groundplane[4],
	GLfloat lightpos[4])
{
	GLfloat dot;

	/* find dot product between light position vector and ground plane normal */
	dot = groundplane[X] * lightpos[X] +
		groundplane[Y] * lightpos[Y] +
		groundplane[Z] * lightpos[Z] +
		groundplane[W] * lightpos[W];

	shadowMat[0][0] = dot - lightpos[X] * groundplane[X];
	shadowMat[1][0] = 0.f - lightpos[X] * groundplane[Y];
	shadowMat[2][0] = 0.f - lightpos[X] * groundplane[Z];
	shadowMat[3][0] = 0.f - lightpos[X] * groundplane[W];

	shadowMat[X][1] = 0.f - lightpos[Y] * groundplane[X];
	shadowMat[1][1] = dot - lightpos[Y] * groundplane[Y];
	shadowMat[2][1] = 0.f - lightpos[Y] * groundplane[Z];
	shadowMat[3][1] = 0.f - lightpos[Y] * groundplane[W];

	shadowMat[X][2] = 0.f - lightpos[Z] * groundplane[X];
	shadowMat[1][2] = 0.f - lightpos[Z] * groundplane[Y];
	shadowMat[2][2] = dot - lightpos[Z] * groundplane[Z];
	shadowMat[3][2] = 0.f - lightpos[Z] * groundplane[W];

	shadowMat[X][3] = 0.f - lightpos[W] * groundplane[X];
	shadowMat[1][3] = 0.f - lightpos[W] * groundplane[Y];
	shadowMat[2][3] = 0.f - lightpos[W] * groundplane[Z];
	shadowMat[3][3] = dot - lightpos[W] * groundplane[W];

}

/* find the plane equation given 3 points */
void findplane(GLfloat plane[4],
	GLfloat v0[3], GLfloat v1[3], GLfloat v2[3])
{
	GLfloat vec0[3], vec1[3];

	/* need 2 vectors to find cross product */
	vec0[X] = v1[X] - v0[X];
	vec0[Y] = v1[Y] - v0[Y];
	vec0[Z] = v1[Z] - v0[Z];

	vec1[X] = v2[X] - v0[X];
	vec1[Y] = v2[Y] - v0[Y];
	vec1[Z] = v2[Z] - v0[Z];

	/* find cross product to get A, B, and C of plane equation */
	plane[A] = vec0[Y] * vec1[Z] - vec0[Z] * vec1[Y];
	plane[B] = -(vec0[X] * vec1[Z] - vec0[Z] * vec1[X]);
	plane[C] = vec0[X] * vec1[Y] - vec0[Y] * vec1[X];

	plane[D] = -(plane[A] * v0[X] + plane[B] * v0[Y] + plane[C] * v0[Z]);
}

void changeSize(int w, int h)
{

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;
	float ratio = w * 1.0 / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(90.0f, ratio, 0.1f, 100.0f);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

GLfloat position[] = { 0.0, 4.0, 0.0, 1.0 };
GLfloat floorshadow[4][4];

GLfloat ctrlpoints[4][4][3] = {
	{ { -1, -1, 2.0 },
	{ -0.5, -1.5, 2.0 },
	{ 0.5, -1.5, 2.0 },
	{ 1, -1, 2.0 } },

	{ { -1.5, -0.5, 2.0 },
	{ -0.5, -0.5, 0.0 },
	{ 0.5, -0.5, 0.0 },
	{ 1.5, -0.5, 2.0 } },

	{ { -1.5, 0.5, 2.0 },
	{ -0.5, 0.5, 0.0 },
	{ 0.5, 0.5, 0.0 },
	{ 1.5, 0.5, 2.0 } },

	{ { -1, 1, 2.0 },
	{ -0.5, 1.5, 2.0 },
	{ 0.5, 1.5, 2.0 },
	{ 1, 1, 2.0 } }
};

static void init(void)
{

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4,
		0, 1, 12, 4, &ctrlpoints[0][0][0]);
	glEnable(GL_MAP2_VERTEX_3);
	glMapGrid2f(20, 0.0, 1.0, 20, 0.0, 1.0);
	glShadeModel(GL_FLAT);
	glClearColor(0.0, 0.0, 0.0, 0.0);



	GLfloat mat_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 20.0 };
	GLfloat mat_diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat model_ambient[] = { 0.5, 0.5, 0.5, 1.0 };



	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	{

		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
		glLightfv(GL_LIGHT0, GL_POSITION, position);
	
	}

	glEnable(GL_FOG);
	{
		GLfloat fogColor[4] = { 0.7, 0.7, 0.7, 1.0 };

		fogMode = GL_EXP;
		glFogi(GL_FOG_MODE, fogMode);
		glFogfv(GL_FOG_COLOR, fogColor);
		glFogf(GL_FOG_DENSITY, 0.35);
		glHint(GL_FOG_HINT, GL_DONT_CARE);
		glFogf(GL_FOG_START, 1.0);
		glFogf(GL_FOG_END, 5.0);
	}
	glClearColor(0.7, 0.7, 0.7, 1.0);  /* fog color */


	GLUquadricObj* qobj;
	startList = glGenLists(1);
	qobj = gluNewQuadric();
	//gluQuadricCallback(qobj, GLU_ERROR, errorCallback);

	gluQuadricDrawStyle(qobj, GLU_FILL); /* flat shaded */

	gluQuadricNormals(qobj, GLU_FLAT);
	glNewList(startList, GL_COMPILE);
	gluCylinder(qobj, 0.1, 0.05, 0.2, 20, 5);
	glColor4f(0.9f, 0.0f, 0.05f, 1.0);

	glEndList();



}

void LoadTexture(void)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);	// Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	int width, height;
	unsigned char* image = SOIL_load_image("758919.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);


}
void lumina()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat pozitial0[] = { 10.0, 5.0, 10.0, 1.0 };
	GLfloat galben[] = { 0.909, 0.878, 0.490, 0.5 };
	glLightfv(GL_LIGHT0, GL_POSITION, pozitial0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, galben);
	glPushMatrix();
	glTranslated(10, 7, 10);
	glColor3f(1, 0, 0);
	glutSolidSphere(1, 20, 20);
	glPopMatrix();

}

void lumina2()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat pozitial0[] = { 10.0, 5.0, 10.0, 1.0 };
	GLfloat rosu[] = { 1.0,0.0,0.0,1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, pozitial0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, rosu);
	glPushMatrix();
	glTranslated(10, 7, 10);
	glColor3f(1, 0, 0);
	glutSolidSphere(1, 20, 20);
	glPopMatrix();

}
void draw_leg(float xt, float yt, float zt)
{

	glPushMatrix();
	glTranslatef(xt, yt, zt);
	glScalef(0.1, 1, 0.1);
	glutSolidCube(1.0);
	glPopMatrix();
}

void draw_table()
{
	
	glColor4f(0.184, 0.094, 0.015, 1.0);
	glPushMatrix();
	glScalef(1.5, 0.1, 1.5);
	glPushMatrix();
	glTranslatef(0, 10, 0);
	glutSolidCube(1.0);
	glPopMatrix();
	glPopMatrix();

	glColor4f(0.4f, 0.25f, 0.05f, 1.0);

	draw_leg(0.70, 0.5, -0.70);
	draw_leg(0.70, 0.5, 0.70);
	draw_leg(-0.70, 0.5, -0.70);
	draw_leg(-0.70, 0.5, 0.70);

}

void table_umbra()
{

	glPushMatrix();
	glScalef(1.5, 0.1, 1.5);
	glPushMatrix();
	glTranslatef(0, 10, 0);
	glutSolidCube(1.0);
	glPopMatrix();
	glPopMatrix();

	draw_leg(0.70, 0.5, -0.70);
	draw_leg(0.70, 0.5, 0.70);
	draw_leg(-0.70, 0.5, -0.70);
	draw_leg(-0.70, 0.5, 0.70);

}

void draw_chair()
{
	glColor4f(0.243, 0.109, 0.349, 1.0);

	glPushMatrix();
	glScalef(0.5, 0.05, 0.5);
	glPushMatrix();
	glTranslatef(0, 11, 0);
	glutSolidCube(1.0);
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glScalef(0.5, 0.7, 0.05);
	glPushMatrix();
	glTranslatef(0, 1.3, -4.5);
	glutSolidCube(1.0);
	glPopMatrix();
	glPopMatrix();

	glColor4f(0.4f, 0.25f, 0.05f, 1.0);

	draw_leg(0.20, 0.05, -0.20);
	draw_leg(0.20, 0.05, 0.20);
	draw_leg(-0.20, 0.05, -0.20);
	draw_leg(-0.20, 0.05, 0.20);
}

void chair_umbra()
{
	glPushMatrix();
	glScalef(0.5, 0.05, 0.5);
	glPushMatrix();
	glTranslatef(0, 11, 0);
	glutSolidCube(1.0);
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glScalef(0.5, 0.7, 0.05);
	glPushMatrix();
	glTranslatef(0, 1.3, -4.5);
	glutSolidCube(1.0);
	glPopMatrix();
	glPopMatrix();

	draw_leg(0.20, 0.05, -0.20);
	draw_leg(0.20, 0.05, 0.20);
	draw_leg(-0.20, 0.05, -0.20);
	draw_leg(-0.20, 0.05, 0.20);
}

void laMasa()
{
	draw_table();

	glPushMatrix();
	glTranslated(-1.3, 0, 0);
	glPushMatrix();
	glRotated(90, 0, 1, 0);
	draw_chair();
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0, 0, -1.3);
	glPushMatrix();
	glRotated(0, 0, 0, 0);
	draw_chair();
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glTranslated(1.3, 0, 0);
	glPushMatrix();
	glRotated(-90, 0, 1, 0);
	draw_chair();
	glPopMatrix();
	glPopMatrix();

}

void laMasa_umbra()
{
	table_umbra();

	glPushMatrix();
	glTranslated(-1.3, 0, 0);
	glPushMatrix();
	glRotated(90, 0, 1, 0);
	chair_umbra();
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0, 0, -1.3);
	glPushMatrix();
	glRotated(0, 0, 0, 0);
	chair_umbra();
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glTranslated(1.3, 0, 0);
	glPushMatrix();
	glRotated(-90, 0, 1, 0);
	chair_umbra();
	glPopMatrix();
	glPopMatrix();

}

void display(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// Set the camera
	gluLookAt(x, 1.6f, z,
		x + lx, 1.6f, z + lz,
		0.0f, 1.6f, 0.0f);

	//texture ground
	glColor4f(0.9f, 0.9f, 0.9f, 1.0);
	glEnable(GL_TEXTURE_2D);
	LoadTexture();

	for (int i = -100; i <= 100; i++)
		for (int j = -100; j <= 100; j++)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(1.0, 1.0); glVertex3f(1.0 * i, 0.0f, 1.0 * j);
			glTexCoord2f(1.0, 0.0); glVertex3f(1.0 * i, 0.0f, 1.0 * j + 1.0);
			glTexCoord2f(0.0, 0.0); glVertex3f(1.0 * i + 1.0, 0.0f, 1.0 * j + 1.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(1.0 * i + 1.0, 0.0f, 1.0 * j);
			glEnd();


		}
	glDisable(GL_TEXTURE_2D);

	//laMasa();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor4f(0.7, 0.7, 0.7, 0.3);  /* shadow color */

	glPushMatrix();
	glMultMatrixf((GLfloat*)floorshadow);
	laMasa_umbra();
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);

	laMasa();
	
	//lampa
	glColor4f(0.094, 0.137, 0.427, 1.0);

	glPushMatrix();

	glShadeModel(GL_SMOOTH);
	glTranslatef(0.0, 1.1, 0.0);
	glPushMatrix();
	glRotatef(275.0, 1.0, 0.0, 0.0);
	glCallList(startList);
	glPopMatrix();

	glColor4f(0.909, 0.878, 0.490, 0.5);
	lumina();
	glShadeModel(GL_FLAT);
	glTranslatef(0.0, 0.1, 0.0);
	glPushMatrix();
	glRotatef(275.0, 1.0, 0.0, 0.0);
	glCallList(startList);
	glPopMatrix();

	glPopMatrix();
	//farfurii
	lumina2();
	glColor4f(0, 0.5, 0, 0.7);
	glPushMatrix();
	glScalef(0.1, 0.1, 0.1);
	glPushMatrix();
	glTranslatef(5, 10, 0);
	glPushMatrix();
	glRotatef(240.0, 1.0, 1.0, 1.0);
	glEvalMesh2(GL_FILL, 0, 20, 0, 20);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	
	glColor4f(0, 0.5, 0, 0.7);
	glPushMatrix();
	glScalef(0.1, 0.1, 0.1);
	glPushMatrix();
	glTranslatef(-5, 10, 0);
	glPushMatrix();
	glRotatef(240.0, 1.0, 1.0, 1.0);
	glEvalMesh2(GL_FILL, 0, 20, 0, 20);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	glColor4f(0, 0.5, 0, 0.7);
	glPushMatrix();
	glScalef(0.1, 0.1, 0.1);
	glPushMatrix();
	glTranslatef(0, 10, -5);
	glPushMatrix();
	glRotatef(240.0, 1.0, 1.0, 1.0);
	glEvalMesh2(GL_FILL, 0, 20, 0, 20);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	glutSwapBuffers();
	glFlush();
}

void processSpecialKeys(int key, int xx, int yy) {

	float fraction = 0.1f;

	switch (key) {
	case GLUT_KEY_LEFT:
		angle -= 0.01f;
		lx = sin(angle);
		lz = -cos(angle);
		break;
	case GLUT_KEY_RIGHT:
		angle += 0.01f;
		lx = sin(angle);
		lz = -cos(angle);
		break;
	case GLUT_KEY_UP:
		x += lx * fraction;
		z += lz * fraction;
		break;
	case GLUT_KEY_DOWN:
		x -= lx * fraction;
		z -= lz * fraction;
		break;
	}
}


int main(int argc, char** argv) {

	// init GLUT and create window
	GLfloat plane[4];
	GLfloat v0[3], v1[3], v2[3];

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(320, 320);
	glutCreateWindow("Masa festiva in familie");
	init();
	v0[X] = 100.f;
	v0[Y] = 0.f;
	v0[Z] = 100.f;
	v1[X] = 100.f;
	v1[Y] = 0.f;
	v1[Z] = -100.f;
	v2[X] = -100.f;
	v2[Y] = 0.f;
	v2[Z] = 100.f;

	findplane(plane, v0, v1, v2);
	shadowmatrix(floorshadow, plane, position);
	// register callbacks


	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);
	glutIdleFunc(display);
	glutSpecialFunc(processSpecialKeys);

	// OpenGL init
	//glEnable(GL_DEPTH_TEST);

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}

