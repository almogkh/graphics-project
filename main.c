#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"GL/glut.h"

#define PI 3.14159265359f
#define ANIMATION_DELAY 40
#define CAMERA_ROTATION_SPEED 1.5f
#define CAMERA_SPEED 0.25f
#define CAMERA_MODEL 0
#define CAMERA_FREE 1
#define BALL_SPEED 0.07f
#define CELESTIAL_SELF_SPIN 1.8f
#define CELESTIAL_SPIN 1.2f

#define LIGHT_HEAD GL_LIGHT0
#define LIGHT_BALL GL_LIGHT1

typedef struct {
	GLfloat x;
	GLfloat y;
	GLfloat z;
} vec3;

//function declerations
void drawingCB(void);
void reshapeCB(int width, int height);
void keyboardCB(unsigned char key, int x, int y);
void keyboardSpecialCB(int key, int x, int y);
void animateBouncingBall(int value);
void animateSolarSystem(int value);
GLubyte *readBMP(char *imagepath, int *width, int *height);
void TerminationErrorFunc(char *ErrorString);
GLuint load_texture(char *name);
void VerticalCylinder(float radius, float height);
void DrawSign();
void DrawFence(float from_x, float from_z, float to_x, float to_z, float pole_r, float pole_n);

int FOVy = 60;
int camera_mode = CAMERA_MODEL;
int lighting = 1, head_light = 1;
float angleX = 0, angleY = 0, radius = 5;
vec3 cameraPos = { 0, 3, 5 };
vec3 cameraForward = { 0, 0, -1 };
vec3 cameraForwardXZ = { 0, 0, -1 };
vec3 cameraRight = { 1, 0, 0 };

float ballPosition = 0;
float dxBall = BALL_SPEED;

float earthAngle = 0, earthSelf = 0, sunSelf = 0, moonSelf = 0, moonAngle = 0;

GLuint ground, ball, wall, sun, moon, earth;
GLuint wood;
GLuint wood2;
GLuint wood2front;
GLuint lamp;

int main(int argc, char **argv)
{
	//initizlizing GLUT
	glutInit(&argc, argv);

	//initializing window
	glutInitWindowSize(700, 700);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("Graphics project - Children's park");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	glEnable(LIGHT_BALL);

	//registering callbacks
	glutDisplayFunc(drawingCB);
	glutReshapeFunc(reshapeCB);
	glutKeyboardFunc(keyboardCB);
	glutSpecialFunc(keyboardSpecialCB);
	glutTimerFunc(ANIMATION_DELAY, animateBouncingBall, 0);
	glutTimerFunc(ANIMATION_DELAY, animateSolarSystem, 0);

	ground = load_texture("ground.bmp");
	ball = load_texture("tex3.bmp");
	wall = load_texture("wall.bmp");
	sun = load_texture("sun.bmp");
	moon = load_texture("moon.bmp");
	earth = load_texture("earth.bmp");
	wood = load_texture("wood.bmp");
	wood2 = load_texture("wood2.bmp");
	wood2front = load_texture("wood2front.bmp");
	lamp = load_texture("lamp.bmp");
	glBindTexture(GL_TEXTURE_2D, 0);


	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//starting main loop
	glutMainLoop();
}

GLuint load_texture(char *name)
{
	int width, height;
	GLubyte *data = readBMP(name, &width, &height);
	GLuint id;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	free(data);
	return id;
}

vec3 vec3_add(vec3 a, vec3 b)
{
	vec3 res;
	res.x = a.x + b.x;
	res.y = a.y + b.y;
	res.z = a.z + b.z;
	return res;
}

vec3 vec3_scale(vec3 a, float s)
{
	vec3 res;
	res.x = s * a.x;
	res.y = s * a.y;
	res.z = s * a.z;
	return res;
}

vec3 vec3_neg(vec3 a)
{
	return vec3_scale(a, -1);
}

vec3 vec3_sub(vec3 a, vec3 b)
{
	return vec3_add(a, vec3_neg(b));
}

void drawGround(void)
{
	int i, j;
	glBindTexture(GL_TEXTURE_2D, ground);
	glColor3f(0, 0.3, 0);
	glNormal3f(0, 1, 0);
	glBegin(GL_QUADS);
	for (i = -200; i < 200; i += 2) {
		for (j = -200; j < 200; j += 2) {
			glTexCoord2f(0, 0);
			glVertex3f(i, 0, j);
			glTexCoord2f(1, 0);
			glVertex3f(i + 2, 0, j);
			glTexCoord2f(1, 1);
			glVertex3f(i + 2, 0, j + 2);
			glTexCoord2f(0, 1);
			glVertex3f(i, 0, j + 2);
		}
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawBouncingBall(void)
{
	GLfloat ballLight[] = { ballPosition, 0, 0, 1 };
	GLfloat ballColor[] = { 2, 0.7, 0.7, 1 };
	GLfloat ballAmbient[] = { 0.4, 0.4, 0.4, 1 };
	float scale = 1;
	float translatePos = ballPosition > 0 ? -0.4 : 0.4;
	GLUquadric *sphere = gluNewQuadric();
	gluQuadricTexture(sphere, GL_TRUE);

	glBindTexture(GL_TEXTURE_2D, wall);
	glBegin(GL_QUADS);
	//glNormal3f(ballPosition + 1, -1, -1);
	glNormal3f(1, 0, 0);
	glTexCoord2f(0, 1);
	glVertex3f(-1, 1, 1);
	//glNormal3f(ballPosition + 1, 1, -1);
	glTexCoord2f(0, 0);
	glVertex3f(-1, -1, 1);
	//glNormal3f(ballPosition + 1, 1, 1);
	glTexCoord2f(1, 0);
	glVertex3f(-1, -1, -1);
	//glNormal3f(ballPosition + 1, -1, 1);
	glTexCoord2f(1, 1);
	glVertex3f(-1, 1, -1);
	//glNormal3f(ballPosition - 1, -1, -1);
	glNormal3f(-1, 0, 0);
	glTexCoord2f(1, 1);
	glVertex3f(1, 1, 1);
	//glNormal3f(ballPosition - 1, 1, -1);
	glTexCoord2f(1, 0);
	glVertex3f(1, -1, 1);
	//glNormal3f(ballPosition - 1, 1, 1);
	glTexCoord2f(0, 0);
	glVertex3f(1, -1, -1);
	//glNormal3f(ballPosition - 1, -1, 1);
	glTexCoord2f(0, 1);
	glVertex3f(1, 1, -1);
	glEnd();

	if (ballPosition < -0.6)
		scale = 1 - ((ballPosition + 0.6) / (-1 + 0.6));
	else if (ballPosition > 0.6)
		scale = 1 - ((ballPosition - 0.6) / (1 - 0.6));

	glBindTexture(GL_TEXTURE_2D, ball);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glPushMatrix();
	glTranslatef(ballPosition, 0, 0);
	glTranslatef(-translatePos * scale, 0, 0);
	glScalef(scale, 2 - scale, 2 - scale);
	glTranslatef(translatePos, 0, 0);

	glLightfv(LIGHT_BALL, GL_POSITION, ballLight);
	glLightfv(LIGHT_BALL, GL_DIFFUSE, ballColor);
	glLightfv(LIGHT_BALL, GL_AMBIENT, ballAmbient);
	glLightf(LIGHT_BALL, GL_QUADRATIC_ATTENUATION, 0.2);

	gluSphere(sphere, 0.4, 30, 30);
	gluDeleteQuadric(sphere);
	glPopMatrix();

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawSolarSystem(void)
{
	GLUquadric *sphere = gluNewQuadric();
	GLUquadric *disk = gluNewQuadric();
	gluQuadricTexture(sphere, GL_TRUE);
	glColor3f(0.4, 0.4, 0.4);

	glBindTexture(GL_TEXTURE_2D, sun);

	glPushMatrix();
	glRotatef(sunSelf, 0, 1, 0);
	gluSphere(sphere, 1, 30, 30);
	glPopMatrix();

	glPushMatrix();
	glRotatef(earthAngle, 0, 1, 0);
	glTranslatef(3, 0, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glDisable(GL_LIGHTING);
	gluDisk(disk, 0.95, 1, 30, 30);
	if (lighting)
		glEnable(GL_LIGHTING);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, earth);
	glPushMatrix();
	glRotatef(earthSelf, -1, 1, 0);
	gluSphere(sphere, 0.4, 100, 100);

	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, moon);

	glRotatef(moonAngle, 0, 1, 0);
	glTranslatef(1, 0, 0);
	glRotatef(moonSelf, 0, 1, 0);
	gluSphere(sphere, 0.25, 30, 30);

	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glDisable(GL_LIGHTING);
	gluDisk(disk, 2.95, 3, 100, 100);
	if (lighting)
		glEnable(GL_LIGHTING);
	glPopMatrix();

	gluDeleteQuadric(sphere);
	gluDeleteQuadric(disk);
}

void drawingCB(void)
{
	GLenum er;
	GLfloat light0Pos[] = { 0, 0, 0, 1 };
	GLfloat light0Color[] = { 1, 1, 1, 1 };
	GLfloat light0Ambient[] = { 0.2, 0.2, 0.2, 1 };
	vec3 center = vec3_add(cameraPos, cameraForward);
	vec3 up = { 0, 1, 0 };

	//clearing the background
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//initializing modelview transformation matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLightfv(LIGHT_HEAD, GL_POSITION, light0Pos);
	glLightfv(LIGHT_HEAD, GL_DIFFUSE, light0Color);
	glLightfv(LIGHT_HEAD, GL_AMBIENT, light0Ambient);

	if (camera_mode == CAMERA_MODEL) {
		glEnable(LIGHT_HEAD);
		glLightf(LIGHT_HEAD, GL_QUADRATIC_ATTENUATION, 0);

		glTranslatef(0, 0, -radius);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);
	}

	if (camera_mode == CAMERA_FREE) {
		if (head_light) {
			glEnable(LIGHT_HEAD);
			glLightf(LIGHT_HEAD, GL_QUADRATIC_ATTENUATION, 0.007);
		}
		else {
			glDisable(LIGHT_HEAD);
		}
		if (angleX < -45)
			up = cameraForwardXZ;
		gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, center.x, center.y, center.z, up.x, up.y, up.z);
		drawGround();
	}
	//glTranslatef(0, 2, 0);
	//drawSolarSystem();
	drawBouncingBall();

	//swapping buffers and displaying
	glutSwapBuffers();

	//check for errors
	er = glGetError();  //get errors. 0 for no error, find the error codes in: https://www.opengl.org/wiki/OpenGL_Error
	if (er) printf("error: %d\n", er);
}

void DrawFence(float from_x, float from_z, float to_x, float to_z, float pole_r, float pole_n) {
	glPushMatrix();
	float direction_x = to_x - from_x;
	float direction_z = to_z - from_z;
	float direction_size = sqrt((double)direction_x * direction_x + (double)direction_z * direction_z);
	float direction_normal_x = direction_x / direction_size;
	float direction_normal_z = direction_z / direction_size;
	float pole_dist = direction_size / pole_n;
	glTranslated(from_x, 0, from_z);
	for (int i = 0; i < pole_n; i++) {
		VerticalCylinder(pole_r, 1);
		glTranslatef(direction_normal_x * pole_dist, 0, direction_normal_z * pole_dist);
	}
	glPopMatrix();
}


void DrawSign() {
	float sign_width = 2;
	float sign_height = 2;
	float cylinder_height = 1;
	float cylinder_radius = 0.15;
	
	glBindTexture(GL_TEXTURE_2D, wood2front);
	glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	// Front TBD MAY BE OTHER TEXTURE
	glBegin(GL_POLYGON);
	
	glTexCoord2f(0, 0); glVertex3f(-sign_width / 2,	cylinder_height,				0.05f);
	glTexCoord2f(1, 0); glVertex3f(	sign_width / 2,	cylinder_height,				0.05f);
	glTexCoord2f(1, 1); glVertex3f(	sign_width / 2,	cylinder_height + sign_height,	0.05f);
	glTexCoord2f(0, 1); glVertex3f(-sign_width / 2,	cylinder_height + sign_height,	0.05f);
	glEnd();
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glBindTexture(GL_TEXTURE_2D, wood2);
	glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	// Back
	glBegin(GL_POLYGON);
	glVertex3f(-sign_width / 2, cylinder_height,				-0.05f);
	glVertex3f(+sign_width / 2, cylinder_height,				-0.05f);
	glVertex3f(+sign_width / 2, cylinder_height + sign_height,	-0.05f);
	glVertex3f(-sign_width / 2, cylinder_height + sign_height,	-0.05f);
	glEnd();

	// Right
	glBegin(GL_POLYGON);
	glVertex3f(sign_width / 2 + 0.001f, cylinder_height,				-0.05f);
	glVertex3f(sign_width / 2 + 0.001f, cylinder_height,				+0.05f);
	glVertex3f(sign_width / 2 + 0.001f, cylinder_height + sign_height,	+0.05f);
	glVertex3f(sign_width / 2 + 0.001f, cylinder_height + sign_height,	-0.05f);
	glEnd();

	// Left
	glBegin(GL_POLYGON);
	glVertex3f(-sign_width / 2 + 0.001f, cylinder_height,				-0.05f);
	glVertex3f(-sign_width / 2 + 0.001f, cylinder_height,				+0.05f);
	glVertex3f(-sign_width / 2 + 0.001f, cylinder_height + sign_height,	+0.05f);
	glVertex3f(-sign_width / 2 + 0.001f, cylinder_height + sign_height,	-0.05f);
	glEnd();

	// Top
	glBegin(GL_POLYGON);
	glVertex3f(+sign_width / 2, cylinder_height + sign_height,	+0.05f);
	glVertex3f(-sign_width / 2, cylinder_height + sign_height,	+0.05f);
	glVertex3f(-sign_width / 2, cylinder_height + sign_height,	-0.05f);
	glVertex3f(+sign_width / 2, cylinder_height + sign_height,	-0.05f);
	glEnd();

	//disabling automatic texture coordinates generation
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);


	glBindTexture(GL_TEXTURE_2D, wood);

	glPushMatrix();
	glTranslatef(- (sign_width/2) - cylinder_radius, 0, 0);
	VerticalCylinder(cylinder_radius, cylinder_height + sign_height);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(sign_width/2 + cylinder_radius, 0, 0);
	VerticalCylinder(cylinder_radius, cylinder_height + sign_height);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, -0.05f - cylinder_radius*2);
	VerticalCylinder(cylinder_radius, cylinder_height + sign_height + 0.7);
	glTranslatef(0, cylinder_height + sign_height + 0.5, -cylinder_radius);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	VerticalCylinder(cylinder_radius*0.6, 1.5);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

	glTranslated(0, 0.05, 1.54);
	glRotatef(210.0f, 1.0f, 0.0f, 0.0f);
	VerticalCylinder(cylinder_radius * 0.6, 0.3);
	glTranslated(0, 0.3, 0);

	glBindTexture(GL_TEXTURE_2D, lamp);
	glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glutSolidSphere(0.085,20,20);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glPopMatrix();

}

void VerticalCylinder(float radius, float height) {
	glPushMatrix();
	GLUquadricObj* quadratic;
	quadratic = gluNewQuadric();
	gluQuadricTexture(quadratic, GL_TRUE);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	gluCylinder(quadratic, radius, radius, height, 32, 32);
	glPopMatrix();
}


void reshapeCB(int width, int height)
{
	float zNear, zFar;

	zNear = 1; zFar = 1000;

	//update viewport
	glViewport(0, 0, width, height);

	//clear the transformation matrices (load identity)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//projection
	gluPerspective(FOVy, (GLdouble)width / height, zNear, zFar);
}

float wrapAngle(float angle, float maxAngle)
{
	if (angle < 0)
		return angle + maxAngle;
	if (angle >= maxAngle)
		return angle - maxAngle;
	return angle;
}

void resetCamera(void)
{
	angleX = angleY = 0;
	FOVy = 60;
	if (camera_mode == CAMERA_MODEL) {
		radius = 5;
	}
	else if (camera_mode == CAMERA_FREE) {
		cameraPos.x = 0;
		cameraPos.y = 2;
		cameraPos.z = 5;
		cameraForward.x = 0;
		cameraForward.y = 0;
		cameraForward.z = -1;
		cameraForwardXZ = cameraForward;
		cameraRight.x = 1;
		cameraRight.y = 0;
		cameraRight.z = 0;
	}
}

void computeCameraVectors(void)
{
	float cosa, sina;
	vec3 camFacing = { 0, 0, -1 };
	vec3 camFacingXZ = { 0, 0, -1 };
	vec3 temp;

	if (angleX > -2)
		angleX = -2;
	else if (angleX < -90)
		angleX = -90;

	cosa = cos(angleX * PI / 180);
	sina = sin(angleX * PI / 180);
	temp.x = camFacing.x;
	temp.y = cosa * camFacing.y - sina * camFacing.z;
	temp.z = sina * camFacing.y + cosa * camFacing.z;

	cosa = cos(angleY * PI / 180);
	sina = sin(angleY * PI / 180);
	cameraForward.x = cosa * temp.x + sina * temp.z;
	cameraForward.y = temp.y;
	cameraForward.z = -sina * temp.x + cosa * temp.z;

	cameraForwardXZ.x = cosa * camFacingXZ.x + sina * camFacingXZ.z;
	cameraForwardXZ.y = camFacingXZ.y;
	cameraForwardXZ.z = -sina * camFacingXZ.x + cosa * camFacingXZ.z;

	cosa = cos(-PI / 2);
	sina = sin(-PI / 2);
	cameraRight.x = cosa * cameraForwardXZ.x + sina * cameraForwardXZ.z;
	cameraRight.y = cameraForwardXZ.y;
	cameraRight.z = -sina * cameraForwardXZ.x + cosa * cameraForwardXZ.z;
}

void rotateAroundPoint(void)
{
	float cosa = cos((90 - fabs(angleX)) * PI / 180);
	float len = cameraPos.y / cosa;

	cameraPos = vec3_add(cameraPos, vec3_scale(cameraForward, len));
	computeCameraVectors();
	cameraPos = vec3_sub(cameraPos, vec3_scale(cameraForward, len));
}

void keyboardCB(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
		exit(0);
		break;
	case '+':
		if (camera_mode == CAMERA_FREE) {
			FOVy = FOVy - 1;
			reshapeCB(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		}
		else {
			radius -= 0.05;
		}
		glutPostRedisplay();
		break;
	case '-':
		if (camera_mode == CAMERA_FREE) {
			FOVy = FOVy + 1;
			reshapeCB(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		}
		else {
			radius += 0.05;
		}
		glutPostRedisplay();
		break;
	case '1':
		camera_mode = CAMERA_MODEL;
		resetCamera();
		glutPostRedisplay();
		break;
	case '2':
		camera_mode = CAMERA_FREE;
		resetCamera();
		glutPostRedisplay();
		break;
	case 'w':
		cameraPos = vec3_add(cameraPos, vec3_scale(cameraForwardXZ, CAMERA_SPEED));
		glutPostRedisplay();
		break;
	case 's':
		cameraPos = vec3_sub(cameraPos, vec3_scale(cameraForwardXZ, CAMERA_SPEED));
		glutPostRedisplay();
		break;
	case 'd':
		cameraPos = vec3_add(cameraPos, vec3_scale(cameraRight, CAMERA_SPEED));
		glutPostRedisplay();
		break;
	case 'a':
		cameraPos = vec3_sub(cameraPos, vec3_scale(cameraRight, CAMERA_SPEED));
		glutPostRedisplay();
		break;
	case 'r':
		cameraPos.y += CAMERA_SPEED;
		glutPostRedisplay();
		break;
	case 'f':
		cameraPos.y -= CAMERA_SPEED;
		if (cameraPos.y < 2)
			cameraPos.y = 2;
		glutPostRedisplay();
		break;
	case 'q':
		angleY = wrapAngle(angleY - CAMERA_ROTATION_SPEED, 360);
		rotateAroundPoint();
		glutPostRedisplay();
		break;
	case 'e':
		angleY = wrapAngle(angleY + CAMERA_ROTATION_SPEED, 360);
		rotateAroundPoint();
		glutPostRedisplay();
		break;
	case 'l':
		lighting = !lighting;
		lighting ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
		glutPostRedisplay();
		break;
	case 'h':
		head_light = !head_light;
		glutPostRedisplay();
		break;
	}
}

void keyboardSpecialCB(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_LEFT:
		angleY += CAMERA_ROTATION_SPEED;
		break;
	case GLUT_KEY_RIGHT:
		angleY -= CAMERA_ROTATION_SPEED;
		break;
	case GLUT_KEY_UP:
		angleX += CAMERA_ROTATION_SPEED;
		break;
	case GLUT_KEY_DOWN:
		angleX -= CAMERA_ROTATION_SPEED;
		break;
	}

	angleY = wrapAngle(angleY, 360);
	if (camera_mode == CAMERA_MODEL)
		angleX = wrapAngle(angleX, 360);

	if (camera_mode == CAMERA_FREE) {
		computeCameraVectors();
	}
	glutPostRedisplay();
}

void animateBouncingBall(int value)
{
	ballPosition += dxBall;

	if (ballPosition >= 0.9) {
		ballPosition = 0.9;
		dxBall = -dxBall;
	}
	else if (ballPosition <= -0.9) {
		ballPosition = -0.9;
		dxBall = -dxBall;
	}

	glutPostRedisplay();
	glutTimerFunc(ANIMATION_DELAY, animateBouncingBall, 0);
}

void animateSolarSystem(int value)
{
	sunSelf = wrapAngle(sunSelf + CELESTIAL_SELF_SPIN, 360);
	earthSelf = wrapAngle(earthSelf + CELESTIAL_SELF_SPIN, 360);
	moonSelf = wrapAngle(moonSelf + CELESTIAL_SELF_SPIN, 360);
	earthAngle = wrapAngle(earthAngle + CELESTIAL_SPIN, 360);
	moonAngle = wrapAngle(moonAngle + CELESTIAL_SPIN, 360);
	glutPostRedisplay();
	glutTimerFunc(ANIMATION_DELAY, animateSolarSystem, 0);
}

// Function to load bmp file
// buffer for the image is allocated in this function, you should free this buffer
GLubyte *readBMP(char *imagepath, int *width, int *height)
{
	unsigned char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins
	unsigned int imageSize;   // = width*height*3
	unsigned char *data;
	unsigned char tmp;
	int i;

	// Open the file
	FILE *file = fopen(imagepath, "rb");
	if (!file) {
		TerminationErrorFunc("Image could not be opened\n");
	}

	if (fread(header, 1, 54, file) != 54) { // If not 54 bytes read : problem
		TerminationErrorFunc("Not a correct BMP file\n");
	}

	if (header[0] != 'B' || header[1] != 'M') {
		TerminationErrorFunc("Not a correct BMP file\n");
	}

	// Read ints from the byte array
	dataPos = *(int *)&(header[0x0A]);
	imageSize = *(int *)&(header[0x22]);
	*width = *(int *)&(header[0x12]);
	*height = *(int *)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)
		imageSize = *width * *height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)
		dataPos = 54; // The BMP header is done that way

	// Create a buffer
	data = malloc(imageSize * sizeof(GLubyte));
	if (data == NULL) {
		TerminationErrorFunc("Failed to allocate memory\n");
	}

	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);

	//swap the r and b values to get RGB (bitmap is BGR)
	for (i = 0; i < *width * *height; i++)
	{
		tmp = data[i * 3];
		data[i * 3] = data[i * 3 + 2];
		data[i * 3 + 2] = tmp;
	}

	//Everything is in memory now, the file can be closed
	fclose(file);

	return data;
}

void TerminationErrorFunc(char *ErrorString)
{
	char string[256];
	printf(ErrorString);
	fgets(string, 256, stdin);

	exit(0);
}
