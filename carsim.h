//
// CAR SIMULATION project
//

#define M_PI   3.1415926
#define M_2PI  (2. * M_PI)

typedef int bool;
enum {
	false = 0, true
};

typedef struct {
	double x, y, z;
} S_Dot;

typedef struct {
	double transX, transY, transZ;
	double angleX, angleY, angSpin;
} S_Spatial;

typedef struct {
	S_Spatial sp;
	double R;
	int slices;
	int rings;
} S_GeoWheel;

typedef struct {
	S_Spatial sp;
	S_GeoWheel geo;
	double angV;
	double linV;
	double physR;
	bool   isSteering;
} S_Wheel;

enum { // wheels indices
	LFRONT = 0,
	RFRONT,
	LMID,
	RMID,
	LBACK,
	RBACK,

	WHEELS_NUM
};

typedef struct {
	// positions to draw
	S_Spatial sp;
	S_Wheel wheels[WHEELS_NUM];

	// controls
	double linV;
	float steerAngle;

	// calc
	S_Spatial deltaSp;
	double longerF;
	double halfBase;
} S_Car;

// all interfaces
// -- data
extern S_Car uCar;
// -- func
void InitCar();
void GraphicsInit();
void AnimateSystem(S_Car *car);
void Keyboard(unsigned char Key, int MouseX, int MouseY);
void ControlCar(unsigned char Key);
void ControlCameraLight(unsigned char Key);
