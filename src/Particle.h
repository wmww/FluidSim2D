#include "FluidSim.h"

#ifndef PRTCL_H
#define PRTCL_H

//extern bool closeToEdge[HGHT][WDTH];

class Prtcl
{
public:
	Prtcl *nxt, *prv; //next and previous in the list

	//RGBpix color;

	Prtcl(double xPos, double yPos, double xDlta=0, double yDlta=0, bool addToArea=1);
	~Prtcl();

	static const int MAX_ITRTN=24; //maximum number of iterations to try before giving up

	//bool toDlt; //set to one if particle needs to be deleted at the next opportunity
	float x, y;
	float dx, dy;
	float dxOld, dyOld; //the last values, used to keep area velocity map from drifting
	//double dxAvg, dyAvg;
	//double xStrt, yStrt;

	void rndmEffct(); //random particle motion
	void dnstyEffct(); //particles are repelled by high density regions
	void prtclFrctnEffct(); //particles tend toward the velocity of surrounding area
	void clsnEdgeEffct(double prvClsnTme=0, ClsnEdge *prvClsnEdge=0, int rcrsn=0); //receives the time (0-1) the previous collision occurred and what edge that was with. also, what level of recursion we are on
	bool isInObj(); //returns one if the particle is inside an object
	//void setClr();
	bool isInArea() {return (x>=0 && x<WDTH && y>=0 && y<HGHT);}

private:
};

//void setCloseToEdge();

extern Prtcl *frstPrtcl;

#endif // PRTCL_H