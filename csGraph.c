#include <stdio.h>
#include <math.h>
#include "../ext/include/GL/glut.h"
#include "carsim.h"

S_Spatial sLight;
double CameraPos = 0;
double CameraDist = 1;

double t1 = 1., t2 = 0.9;
bool cameraModeFollow = false;

void ControlCameraLight(unsigned char Key)
{
	// CAMERA
	// op -- rotate
	if (Key == 'o')
		CameraPos -= 0.3;
	if (Key == 'p')
		CameraPos += 0.3;
	// kl -- change orbit radius
	if (Key == 'k')
		CameraDist -= 0.1;
	if (Key == 'l')
		CameraDist += 0.1;
	if (Key == 'i')
		cameraModeFollow = !cameraModeFollow;

	// LIGHT
	// if (Key == 'z') {
	// 	sLight.angleX += 0.3;
	// }
	// else if (Key == 'x') {
	// 	sLight.angleX -= 0.3;
	// }

	// tune
	if (Key == 'f') {
		t1 -= 0.01;
	} else if (Key == 'g') {
		t1 += 0.01;
	}
	if (Key == 'v') {
		t2 += 0.01;
	}
	else if (Key == 'b') {
		t2 -= 0.01;
	}
	printf(" t1=%f     t2=%f\n", (float)t1, (float)t2);

}

void ApplySpacialPosRot(S_Spatial *sp)
{
	glTranslated(sp->transX, sp->transY, sp->transZ);
	//glRotated(sp->angleX, 1, 0, 0);
	glRotated(sp->angleY, 0, 1, 0);
	glRotated(sp->angSpin, 0, 0, 1);
}

void DrawWheel(S_GeoWheel *whl)
{
	glPushMatrix();

	ApplySpacialPosRot(&whl->sp);

	GLfloat material_diffuse[] = { 1.0, 1.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);

	glutSolidTorus(whl->R / 2, whl->R, whl->slices, whl->rings);

	//glutWireSphere(whl->R + whl->R / 2, 20, 10);

	glPopMatrix();
}

void SetupCamera()
{
	// a shift along the car with uCar.sp.angleY direction
	double steerRadians = uCar.sp.angleY * M_PI / 180;
	double shiftLen = 0.4;
	double lookX =  cos(steerRadians) * shiftLen  + uCar.sp.transX;
	double lookZ = -sin(steerRadians) * shiftLen  + uCar.sp.transZ;

	// where to position camera
	double dx = CameraDist * sin(CameraPos);
	double dz = CameraDist * cos(CameraPos);
	if (cameraModeFollow) {
		dx = CameraDist * sin(CameraPos + steerRadians);
		dz = CameraDist * cos(CameraPos + steerRadians);
		dx += uCar.sp.transX;
		dz += uCar.sp.transZ;
	}

	// relay
	gluLookAt(
		dx,    0.4,            dz,
		lookX, uCar.sp.transY, lookZ,
		0.0f, 1.0f, 0.0f
	);
}

void CalcLightTranslation()
{
	sLight.transX = sin(sLight.angleX);
	sLight.transY = 1.f;
	sLight.transZ = cos(sLight.angleX);
}

void SetupLight()
{
	CalcLightTranslation();

	glPushMatrix();

		GLfloat light0_pos[]       = {   (float)sLight.transX,   (float)sLight.transY,   (float)sLight.transZ, 1.0};
		GLfloat light0_direction[] = { - (float)sLight.transX, - (float)sLight.transY, - (float)sLight.transZ };
		GLfloat light0_diffuse[] = { 1., 1., 1. };

		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
		glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light0_direction);

	glPopMatrix();
}

void PushTankProceduralBody() 
{
	#define slices  16
	double arrX[slices+1], arrY[slices+1];
	double R = 0.09, H = 0.28;

	// prepare vertices
	int i;
	for (i = 0; i <= slices; i++) {
		double a = i * 2 * M_PI / slices;
		arrX[i] = R * cos(a);
		arrY[i] = R * sin(a);
	}

	// side surface
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= slices; i++) {
		glNormal3d(arrX[i], arrY[i], 0);
		glVertex3d(arrX[i], arrY[i], H);
		glVertex3d(arrX[i], arrY[i], -H);
	}
	glEnd();

	// back surface
	glBegin(GL_TRIANGLE_FAN);
	glNormal3d(0, 0, H);
	glVertex3d(arrX[0], arrY[0], -H);
	for (i = 1; i <= slices; i++) {
		glNormal3d(0, 0, H);
		glVertex3d(arrX[i], arrY[i], -H);
	}
	glEnd();
}

void DrawTank(S_Car* car) {
	GLfloat material_diffuse[] = { 1.0f, 0.1f, 0.1f, 1.f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);

	glPushMatrix();
	{
		glTranslated(-0.21 + 0.335, 0.15, 0);
		glRotated(90, 0, 1, 0);
		PushTankProceduralBody();
	}
	glPopMatrix();
}

void DrawSquares(GLfloat * mat1, GLfloat * mat2, double shift)
{
	glPushMatrix();
	{
		glScaled(0.2, 0.0001, 0.2);
		glTranslated(-20, 0, shift);

		GLfloat *mat = mat1;
		for (int i = 0; i < 40; i++)
		{
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mat);
			glutSolidCube(1);
			glTranslated(1.0, 0, 0);
			mat = (mat == mat1) ? mat2 : mat1;
		}
	}
	glPopMatrix();
}

void DrawRoad()
{
	GLfloat material_diff1[] = { 0.1f, 0.3f, .5f, 1.0f };
	GLfloat material_diff2[] = { 0.1f, 0.5f, .3f, 1.0f };

	for (int j = -10; j < 10; j++) {
		glPushMatrix();
			glTranslated(0, 0, j * 0.4);
			
			DrawSquares(material_diff1, material_diff2, -0.5);
			DrawSquares(material_diff2, material_diff1, 0.5);

		glPopMatrix();
	}
}

void DrawCabin(S_Car * car)
{
	double R = 0.19, H = 0.18;
	GLfloat material_diffuse[] = { 0.1f, 0.1f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);

	// cabin top
	glPushMatrix();
		glTranslated(0.16 + 0.335, 0.17, 0);
		glScaled(0.76, 0.9, 0.8);

		glutSolidCube(R);
	glPopMatrix();
	glPushMatrix();
		glTranslated(0.25 + 0.335, 0.13, 0);
		glScaled    (0.5,  0.45, 0.8);

		glutSolidCube(R);
	glPopMatrix();

	// bumper
	glPushMatrix();
		glTranslated(0.29 + 0.335, 0.1, 0);
		glScaled(0.2, 0.2, 1);

		glutSolidCube(H);
	glPopMatrix();
}

void DrawMainBody(S_Car * car)
{
	DrawTank(car);

	DrawCabin(car);
}

void DrawCar(S_Car *car)
{
	glPushMatrix();

	ApplySpacialPosRot(&car->sp);

	// main body
	DrawMainBody(car);

	// wheels
	for (int i = 0; i < WHEELS_NUM ; i++) {
		DrawWheel(&car->wheels[i].geo);
	}

	glPopMatrix();
}

void Display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	{
		// static part
		SetupLight();
		SetupCamera();
		DrawRoad();

		// dynamic part
		AnimateSystem(&uCar);
		DrawCar(&uCar);
	}
	glPopMatrix();
	
	glutSwapBuffers();
}

void GraphicsInit()
{
	// setup openGL params
	{
		//glMatrixMode(GL_MODELVIEW);

		//glEnable(GL_COLOR_MATERIAL);
		// window with a title
		char window_title[] = "Car Simulator";
		glutCreateWindow(window_title);
		gluPerspective(90.f, 5. / 3., 0.1, 1000.0);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_NORMALIZE);
		glEnable(GL_LIGHTING);
		//glEnable(GL_CULL_FACE);
		//glEnable(GL_COLOR_MATERIAL);
		//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

		// dark gray background
		glClearColor(0.f, 0.f, 0.f, 1.f);

		// register callbacks
		glutDisplayFunc(Display);
		glutIdleFunc(Display);
		glutKeyboardFunc(Keyboard);

		// camera params
		//cameraX = 0;
		//cameraZ = -2 * cameraStep;
	}

	// the lab data
	CameraPos = 2 * 0.3;
	CameraDist -= 2 * 0.1;

	// - cone
	// light
	sLight.angleX = 1.8;
	CalcLightTranslation();
}
