#include <stdio.h>
#include <math.h>
#include "../ext/include/GL/glut.h"
#include "carsim.h" 
S_GeoWheel fAxe;
S_GeoWheel bAxe;

S_Car uCar;

void Init(void)
{
	GraphicsInit();
	InitCar();
}

int main(int argc, char *argv[])
{
	glutInitWindowSize(1200, 960);
	glutInitWindowPosition(140, 40);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInit(&argc, argv);
	Init();
	
	glutMainLoop();
	return 0;
}

void Keyboard(unsigned char Key, int MouseX, int MouseY)
{
	if (Key == 27)
		exit(0);

	ControlCar(Key);
	ControlCameraLight(Key);

	//printf("cur mouse: x=%3d     y=%3d\n", MouseX, MouseY);
}
