// 
// PHYSICS module -- all about motion / recalc etc
//

#include <stdio.h>
#include <math.h>
#include "../ext/include/GL/glut.h"
#include "carsim.h"

void ControlCar(unsigned char Key)
{
	// CAR
	// ws -- speed
	int maxAngle = 30;
	
	if (Key == 's')
		uCar.linV /= 1.33;
	if (Key == 'w')
		uCar.linV *= 1.33;

	// ad -- steer
	if (Key == 'a') {
		if (uCar.steerAngle < maxAngle)
			uCar.steerAngle += 5;
	}

	if (Key == 'd') {
		if (uCar.steerAngle > -maxAngle)
			uCar.steerAngle -= 5;
	}
}

void PositionWheels()
{
	S_Wheel right;
	S_Wheel left;

	// longerons
	double front2back = 0.6;
	uCar.longerF = 0.2 + 0.335;
	uCar.halfBase = 0.1;

	// front
	left.geo.sp.transX  = uCar.longerF;
	right.geo.sp.transX = uCar.longerF;
	left.geo.sp.transZ = -uCar.halfBase;
	right.geo.sp.transZ = uCar.halfBase;
	uCar.wheels[LFRONT].geo = left.geo;
	uCar.wheels[RFRONT].geo = right.geo;

	// back
	left.geo.sp.transX  -= front2back;
	right.geo.sp.transX -= front2back;
	uCar.wheels[LBACK].geo = left.geo;
	uCar.wheels[RBACK].geo = right.geo;

	// middle
	left.geo.sp.transX  += 0.13;
	right.geo.sp.transX += 0.13;
	uCar.wheels[LMID].geo = left.geo;
	uCar.wheels[RMID].geo = right.geo;
}

void InitCarWheels()
{
	int slices = 10;
	int rings = 10;
	double thorRad = 0.04;

	PositionWheels();

	for (int i = 0; i < WHEELS_NUM; i++) {
		uCar.wheels[i].geo.slices = slices;
		uCar.wheels[i].geo.rings = rings;
		uCar.wheels[i].geo.sp.angleX = 0; // lays them down 
		uCar.wheels[i].geo.sp.angleY = 0; // flips across the car
		uCar.wheels[i].geo.sp.angSpin = 0;// actual spin
		uCar.wheels[i].geo.R = thorRad;
		uCar.wheels[i].physR = uCar.wheels[i].geo.R * 1.5;
		uCar.wheels[i].geo.sp.transY = uCar.wheels[i].physR;
		uCar.wheels[i].isSteering = false;
	}

 	uCar.wheels[LFRONT].isSteering = true;
 	uCar.wheels[RFRONT].isSteering = true;
}

void InitCar()
{
	InitCarWheels();

	// speed
	uCar.linV = 0.003;
	//uCar.linV = 0.0;
}

void DebugSpinWheels(S_Car * car)
{
	int spinInc = -2;
	for (int i = 0; i < WHEELS_NUM; i++) {
		car->wheels[i].geo.sp.angSpin += spinInc;
	}
}

void DebugSteer(S_Car * car)
{
	for (int i = 0; i < WHEELS_NUM; i++) {
		if (car->wheels[i].isSteering) {
			car->wheels[i].geo.sp.angleY += 0.05;
		}
	}
}

void RestrictFallingOut(S_Car * car)
{
	static double limJump = 3.5;

	while (car->sp.transX > limJump) {
		car->sp.transX -= 2 * limJump;
	}

	while (car->sp.transX < -limJump) {
		car->sp.transX += 2 * limJump;
	}

	while (car->sp.transZ > limJump) {
		car->sp.transZ -= 2 * limJump;
	}

	while (car->sp.transZ < -limJump) {
		car->sp.transZ += 2 * limJump;
	}

	//printf("car X = %3.1f\n", (float)(car->sp.transX));
}

// turn  (debug)
//car->deltaSp.angleY = uCar.steerAngle - uCar.sp.angleY;
extern double t1;

void CalcDeltaSpatial(S_Car * car)
{
	// IN parameter uCar.steerAngle
	double radStWheel = car->steerAngle * M_PI / 180;

	// new delta for front wheels
	double fwDeltaBase = sin(radStWheel) * car->linV;
	double moveAlong   = cos(radStWheel) * car->linV;

	// delta angle covers that shift
	double sinDelta = fwDeltaBase / (car->longerF);

	// turn
	double radDelta = asin(sinDelta);
	car->deltaSp.angleY = radDelta / M_PI * 180;

	// move to uCar.sp.angleY direction
	double radSteer = uCar.sp.angleY * M_PI / 180;
	car->deltaSp.transX = cos(radSteer) * moveAlong;
	car->deltaSp.transY = 0.0;
	car->deltaSp.transZ = -sin(radSteer) * moveAlong;
}

void SteerWheels(S_Car * car)
{
	for (int i = 0; i < WHEELS_NUM; i++) {
		if (car->wheels[i].isSteering) {
			car->wheels[i].geo.sp.angleY = car->steerAngle;
		}
	}
}

void ApplySpatial(S_Car * car)
{
	// control steering wheels
	SteerWheels(car);

	// calc all spacial
	CalcDeltaSpatial(car);

	// apply it
	car->sp.transX += car->deltaSp.transX;
	car->sp.transY += car->deltaSp.transY;
	car->sp.transZ += car->deltaSp.transZ;
	car->sp.angleY += car->deltaSp.angleY;

	// moved too far => jump back
	RestrictFallingOut(car);
}

double LenXZ(S_Spatial* deltaHub)
{
	double sq = deltaHub->transX * deltaHub->transX +
		        deltaHub->transZ * deltaHub->transZ;
	return sqrt(sq);
}

void CalcSpin(S_Wheel *whl, S_Spatial* deltaHub)
{
	double lenDeltaHub = LenXZ(deltaHub);

	// arc len is R * angleRad 
	double angleRad = lenDeltaHub  / whl->physR;

	// convert to degrees
	double degrees = angleRad * 180 / M_PI;

	// apply
	whl->geo.sp.angSpin -= degrees;

	// cut
	while (whl->geo.sp.angSpin < 0) {
		whl->geo.sp.angSpin += 360;
	}
}

void AnimateSystem(S_Car *car)
{
	// update steer in steering wheels
	// calc hubs movement
	// balance positions of steering axis
	
	// calc all spacial
	ApplySpatial(car);

	// for all wheels 
	for (int i = 0; i < WHEELS_NUM; i++) {
		// calc real hub movement
		S_Spatial deltaHub = car->deltaSp;

		// calc spin
		CalcSpin(&car->wheels[i], &deltaHub);
	}

	//DebugSteer(car);
	//DebugSpinWheels(car);
}
