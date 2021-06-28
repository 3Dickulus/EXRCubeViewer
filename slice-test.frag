// Output generated from file: Examples/Historical 3D Fractals/Mandelbulb.frag
// Created: Mon Jun 20 21:29:15 2016
#info Mandelbulb Distance Estimator
#define providesInit

#include "MathUtils.frag"
#include "DE-Slicer.frag"
#group Mandelbulb

// Number of fractal iterations.
uniform int Iterations;  slider[0,9,100]

// Number of color iterations.
uniform int ColorIterations;  slider[0,9,100]

// Mandelbulb exponent (8 is standard)
uniform float Power; slider[0,8,16]

// Bailout radius
uniform float Bailout; slider[0,5,30]

// Alternate is slightly different, but looks more like a Mandelbrot for Power=2
uniform bool AlternateVersion; checkbox[false]

uniform vec3 RotVector; slider[(0,0,0),(1,1,1),(1,1,1)]

uniform float RotAngle; slider[0.00,0,180]

uniform bool Julia; checkbox[false]
uniform vec3 JuliaC; slider[(-2,-2,-2),(0,0,0),(2,2,2)]

uniform float time;
mat3 rot;
void init() {
	 rot = rotationMatrix3(normalize(RotVector), RotAngle);
}

// This is my power function, based on the standard spherical coordinates as defined here:
// http://en.wikipedia.org/wiki/Spherical_coordinate_system
//
// It seems to be similar to the one Quilez uses:
// http://www.iquilezles.org/www/articles/mandelbulb/mandelbulb.htm
//
// Notice the north and south poles are different here.
void powN1(inout vec3 z, float r, inout float dr) {
	// extract polar coordinates
	float theta = acos(z.z/r);
	float phi = atan(z.y,z.x);
	dr =  pow( r, Power-1.0)*Power*dr + 1.0;

	// scale and rotate the point
	float zr = pow( r,Power);
	theta = theta*Power;
	phi = phi*Power;

	// convert back to cartesian coordinates
	z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
}

// This is a power function taken from the implementation by Enforcer:
// http://www.fractalforums.com/mandelbulb-implementation/realtime-renderingoptimisations/
//
// I cannot follow its derivation from spherical coordinates,
// but it does give a nice mandelbrot like object for Power=2
void powN2(inout vec3 z, float zr0, inout float dr) {
	float zo0 = asin( z.z/zr0 );
	float zi0 = atan( z.y,z.x );
	float zr = pow( zr0, Power-1.0 );
	float zo = zo0 * Power;
	float zi = zi0 * Power;
	dr = zr*dr*Power + 1.0;
	zr *= zr0;
	z  = zr*vec3( cos(zo)*cos(zi), cos(zo)*sin(zi), sin(zo) );
}



// Compute the distance from `pos` to the Mandelbox.
float DE(vec3 pos, int ii) {
	vec3 z=pos;
	float r;
	float dr=1.0;
	int i=0;
	r=length(z);
	while(r<Bailout && (i<Iterations)) {
		if (AlternateVersion) {
			powN2(z,r,dr);
		} else {
			powN1(z,r,dr);
		}
		z+=(Julia ? JuliaC : pos);
		r=length(z);
		z*=rot;
		if (i<ColorIterations) orbitTrap = min(orbitTrap, abs(vec4(z.x,z.y,z.z,r*r)));
		i++;
	}
//	if ((type==1) && r<Bailout) return 0.0;
	return 0.5*log(r)*r/dr;
	/*
	Use this code for some nice intersections (Power=2)
	float a =  max(0.5*log(r)*r/dr, abs(pos.y));
	float b = 1000;
	if (pos.y>0)  b = 0.5*log(r)*r/dr;
	return min(min(a, b),
		max(0.5*log(r)*r/dr, abs(pos.z)));
	*/
}





#preset Default
FOV = 0.62536
Eye = 2.482349,-1.229274,0.2133503
Target = -5.365284,2.820102,-0.5254719
Up = 0.3095371,0.4471408,-0.8371328
DepthToAlpha = false
AntiAlias = 1
Detail = -3
DetailNormal = -2.8
DetailAO = -0.5
FudgeFactor = 1
MaxRaySteps = 164
BoundingSphere = 3
Dither = 0.51754
AO = 0,0,0,0.85185
Specular = 4
SpecularExp = 16.364
SpotLight = 1,1,1,1
SpotLightDir = 0.63626,0.5
CamLight = 1,1,1,1.53846
CamLightMin = 0.12121
Glow = 1,1,1,0.43836
Fog = 0
HardShadow = 0.35385
Reflection = 0
BaseColor = 1,1,1
OrbitStrength = 1
X = 1,1,1,0.2478632
Y = 0.345098,0.666667,0,1
Z = 1,0.666667,0,1
R = 0.0784314,1,0.941176,0.0940171
BackgroundColor = 0.607843,0.866667,0.560784
GradientBackground = 0.3261
CycleColors = false
Cycles = 4.04901
FloorNormal = 0,0,0
FloorHeight = 0
FloorColor = 1,1,1
XLevel = 1.00650383
PlaneZoom = 1.25
GraphZoom = 1
ZLevel = 1.743119
Delta = -1
RAD = 0
Iterations = 10
ColorIterations = 10
Power = 7
Bailout = 3
AlternateVersion = true
RotVector = 1,1,1
RotAngle = 0
Julia = true
JuliaC = 0.8135,0,0
XLevel1:Linear:0:-1.0962:1.00671:1:256:0.3:1:1.7:1:0
#endpreset

