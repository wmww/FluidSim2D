#include "Graphics.h"

#ifndef SHAPE_LOADER_H
#define SHAPE_LOADER_H

const double BLACK_THRESH=0.5; //the threshold (only applies if not using delta color
const int EXPAND_ITERS=0; //number of iterations to expand during preprocessing
const int MAX_PERIM_LNG=8000; //the longest a perimeter can be
const int MIN_PERIM_LNG=20; //the smallest a perimeter can be
const int POINTINESS_BUFFER=6; //in pixels
const double MIN_SGMNT_LNG=POINTINESS_BUFFER;
const double VERT_THRESH=0.01;
const int MAX_VERTS=1000;

class ShapeLoader
{
public:
	ShapeLoader(const char *filename);
	~ShapeLoader();

private:
	WidapImage loadImage;
	bool* input;
	int xPerim[MAX_PERIM_LNG], yPerim[MAX_PERIM_LNG]; //the cords of the current perimeter
	int perimLng; //the length of the current perimeter
	int bbl, bbr, bbb, bbt; //the perimeter bounding box
	double perimPointiness[MAX_PERIM_LNG];
	int verts[MAX_VERTS];
	int vertNum;

	int wdth, hght;

	void preprocess();
	void findShapes();
	bool findPerim(int x, int y);
	void findPointiness();
	void findVerts();
	void setLines();

	bool checkPix(unsigned int x, unsigned int y);
};

#endif // SHAPE_LOADER_H