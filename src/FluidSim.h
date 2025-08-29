#ifndef FLUID_SIM_H
#define FLUID_SIM_H

///config constants

//simple test settings
/*const int WDTH=640, HGHT=480; //res of the voxel map
const double STRT_DNSTY=1; //start density
const double INIT_DX=24, INIT_DY=0; //the starting velocity of particles
const double RNDM_MV=0; //amount of randomness to apply to movement
const double DNSTY_FORCE=0;//6/STRT_DNSTY; //amount of force each particle exerts on each other particle
const double BOX_BOUNCE=1; //how much velocity to retain after hitting a wall
const double OBJ_BOUNCE=1; //how much a particle bounces off an object
const bool BOX_EDGES[]={1, 0, 1, 0}; //if 0 particles are deleted when they leave the box, order is left, right, bottom, top
const double PRTCL_FRCTN=0; //how much each particle tends toward the average movement of their location
*/

//low res settings
const int WDTH=320, HGHT=240; //res of the voxel map
const double STRT_DNSTY=24; //start density
const double INIT_DX=6, INIT_DY=0; //the starting velocity of particles
const double RNDM_MV=0.2; //amount of randomness to apply to movement
const double DNSTY_FORCE=0/STRT_DNSTY; //amount of force each particle exerts on each other particle
const double OBJ_BOUNCE=1; //how much a particle bounces off an object
const bool BOX_EDGES[]={1, 0, 1, 1}; //if 0 particles are deleted when they leave the box, order is left, right, bottom, top
const double PRTCL_FRCTN=0.3; //how much each particle tends toward the average movement of their location
const bool USE_AVG_DRAG=1; //if to use average drag instead of instantaneous drag on collision edges


//high res settings
/*const int WDTH=640, HGHT=480; //res of the voxel map
const double STRT_DNSTY=12; //start density
const double INIT_DX=16, INIT_DY=0; //the starting velocity of particles
//const double MAX_STABLE_PTRCL_SPD=100; //if a particle is going faster it may go through edges
const double RNDM_MV=1; //amount of randomness to apply to movement
const double DNSTY_FORCE=6/STRT_DNSTY; //amount of force each particle exerts on each other particle
const double OBJ_BOUNCE=1; //how much a particle bounces off an object
const bool BOX_EDGES[]={1, 0, 1, 1}; //if 0, particles are deleted when they leave the box, order is left, right, bottom, top
const double PRTCL_FRCTN=1; //how much each particle tends toward the average movement of their location
const bool USE_AVG_DRAG=1; //if to use average drag instead of instantaneous drag
*/


const int FRAME_NUM=240; //number of frames to render

struct Voxel //what the raster map of the area is made of; technically not a voxel since its 2D but voxel sounds cooler
{
	int d; //density (number of particles inside)

	double dx, dy; //the sum of the vectors of the particles inside

	double dAvg, dxAvg, dyAvg; //sum of the various values throughout all time
		//(calling it average is not entirely accurate but less confusing then sum)
};

struct ClsnEdge //the edge of a polygon or of the arena that particles collide with
{
	double x1, y1;
	double x2, y2; //the start and end points

	double bbl, bbr, bbb, bbt; //the bounding box

	double xf, yf; //the net force exerted on the edge

	ClsnEdge *nxt; //used to make the linked list
};

const int MAX_OBJ=64; //max number of objects

struct Object //objects for calculating resistance and stuff; made up of a section of the collision edge list
{
	ClsnEdge *frstEdge; //where in the list this object starts
	int edges; //how many edges are in the object

	double bbl, bbr, bbb, bbt; //bounding box

	double xf, yf; //the sum of all the forces on the edges

	double drag; //the drag coefficient
};

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "../Widap_includes/WidapStd.h"
#include "../Widap_includes/WidapColors.h"
#include "../Widap_includes/WidapImage.h"

extern Voxel area[HGHT][WDTH];
extern ClsnEdge *frstClsnEdge;
extern int clsnEdgeCycles; //the number of cycles since collision edge force has been reset
extern Object obj[MAX_OBJ];
extern int objNum; //the number of objects
extern int cycle;
extern int prtclsNum;

using namespace std;

void base();
void init();

void addClsnEdge(double x1, double y1, double x2, double y2);
void resetClsnEdgeFrc();
void addObj(ClsnEdge *frst, int edgeNum);

class PlygnBuilder
{
public:

	PlygnBuilder()
	{
		vertNum=0;
		xStrt=0;
		yStrt=0;
		xLst=0;
		yLst=0;
	}

	void strtNew()
	{
		vertNum=0;
	}

	void addVert(double x, double y)
	{
		if (!vertNum)
		{
			xStrt=x;
			yStrt=y;
			xLst=x;
			yLst=y;
			vertNum++;
		}
		else
		{
			addClsnEdge(xLst, yLst, x, y);
			xLst=x;
			yLst=y;
			vertNum++;
		}
	}

	void finish()
	{
		addClsnEdge(xLst, yLst, xStrt, yStrt);

		addObj(frstClsnEdge, vertNum);

		vertNum=0;
	}

private:

	double xLst, yLst;
	double xStrt, yStrt;
	int vertNum;
};


#endif // FLUID_SIM_H