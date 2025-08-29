#include "ShapeLoader.h"

ShapeLoader::ShapeLoader(const char *filename)
{
	input=0;

	if (!loadImage.load(filename))
	{
		wdth=loadImage.getWdth();
		hght=loadImage.getHght();

		preprocess();

		findShapes();
	}
	else
		cout << ">> shape loader failed because '" << filename << "' failed to load <<" << endl;
}

ShapeLoader::~ShapeLoader()
{
	if (input)
		delete[] input;
}

void ShapeLoader::preprocess()
{
	int x, y, i;
	int yFactor, xFactor;
	int thresh;
	RGBpix clr0;
	RGBpix bknd, frgnd; //background and foreground colors
	bool *input1; //used in expanding iterations as 2nd input array
	int *value; //the RGB image is turned into this before being turned binary

	bknd=clr(0, 0, 20);
	frgnd=clr(0, 255, 0);

	if (input)
		delete[] input;

	input=new bool[wdth*hght];
	value=new int[wdth*hght];

	///set value int map from image RGB map

	thresh=BLACK_THRESH*765;

	for (y=0; y<hght; ++y)
	{
		yFactor=y*wdth;

		for (x=0; x<wdth; ++x)
		{
			clr0=loadImage.bits[x+yFactor];

			value[x+yFactor]=765-(clr0.r+clr0.g+clr0.b);
		}
	}


	///set input bool map from value map

	for (y=0; y<hght; ++y)
	{
		yFactor=y*wdth;

		for (x=0; x<wdth; ++x)
		{
			if (value[x+yFactor]>thresh)
				input[x+yFactor]=1;
			else
				input[x+yFactor]=0;
		}
	}


	///expand the bool map for a given number of iterations

	for (i=0; i<EXPAND_ITERS; ++i)
	{
		//note: input1 is never deleted but it is taken over by input and input is deleted
		input1=new bool[wdth*hght];

		//set edges to zero to stop them from being weird

		yFactor=(hght-1)*wdth;

		for (x=0; x<wdth; ++x)
		{
			input1[x]=0;
			input1[x+yFactor]=0;
		}
		for (y=0; y<hght; ++y)
		{
			input1[y*wdth]=0;
			input1[(y+1)*wdth-1]=0;
		}

		//run through and expand

		for (y=1; y<hght-1; ++y)
		{
			yFactor=y*wdth;

			for (x=1; x<wdth-1; ++x)
			{
				if (!input[x+yFactor])
				{
					if (input[x+yFactor+1] ||
						input[x+yFactor-1] ||
						input[x+yFactor+wdth] ||
						input[x+yFactor-wdth])
					{
						input1[x+yFactor]=1;
					}
					else
						input1[x+yFactor]=0;
				}
				else
					input1[x+yFactor]=1;
			}
		}

		//delete the only input array, point input to the new array and repeat if needed
		delete[] input;
		input=input1;
	}

	///set edges to zero to stop them from being weird

	yFactor=(hght-1)*wdth;

	for (x=0; x<wdth; ++x)
	{
		input[x]=0;
		input[x+yFactor]=0;
	}
	for (y=0; y<hght; ++y)
	{
		input[y*wdth]=0;
		input[(y+1)*wdth-1]=0;
	}

	///set image RGB map from input map so it can be saved

	for (y=0; y<IMG_HGHT; ++y)
	{
		yFactor=y*wdth*hght/IMG_HGHT;

		for (x=0; x<wdth; ++x)
		{
			xFactor=x*wdth/IMG_WDTH;
			inptPixMap[y][x]=input[xFactor+yFactor];
		}
	}

	delete[] value;
}

void ShapeLoader::findShapes()
{
	int i, x, y, x1, y1;
	bool *valid; //array of if that loc is valid to start a new character tracing from

	valid=new bool[hght*wdth];

	for (i=0; i<hght*wdth; ++i)
		valid[i]=1;

	for (x=0; x<wdth; ++x)
	{
		for (y=1; y<hght; ++y) //y must start at 1 so we can always look below current loc
		{
			if (checkPix(x, y) && checkPix(x, y-1) && valid[y*wdth+x])
			{
				if (findPerim(x, y))
				{
					findPointiness();
					findVerts();
					setLines();
				}

				for (x1=bbl; x1<=bbr; ++x1)
				{
					for (y1=bbb; y1<=bbt; ++y1)
					{
						valid[y1*wdth+x1]=0;
					}
				}
			}
		}
	}
}

bool ShapeLoader::findPerim(int x, int y)
{
	const int xDir[4]={-1, 0, 1,  0};
	const int yDir[4]={ 0, 1, 0, -1}; //used when tracing edge

	int dir;
	int xNew, yNew;

	int safty; //used to make sure loops terminate

	perimLng=0;
	dir=0;

	bbl=x; bbr=x; bbb=y; bbt=y;

	//go to a pixel on the edge
	//insure given pixel is on the line and there is room to move and stay on the line
	if (!checkPix(x, y) || !checkPix(x, y-1))
	{
		cout << ">> findPerim was not sent a location properly on the line <<" << endl;
		return 0;
	}

	while (x>0 && checkPix(x-1, y))
		x--;

	do
	{
		xPerim[perimLng]=x;
		yPerim[perimLng]=y;
		++perimLng;

		if (x<bbl)
			bbl=x;
		if (x>bbr)
			bbr=x;
		if (y<bbb)
			bbb=y;
		if (y>bbt)
			bbt=y; //it is necessary to calculate the perim bounding box to stop duplicate targets

		safty=0;

		do
		{
			if ((++dir)>=4)
				dir-=4;

			xNew=x+xDir[dir];
			yNew=y+yDir[dir];

			if (++safty>4)
			{
				cout << "loop in tracePerim took to long (1 pixel sized vision target could be to blame)" << endl;
				return 0;
			}

		} while (xNew<0 || xNew>=wdth || yNew<0 || yNew>=hght || !checkPix(xNew, yNew));

		x=xNew; y=yNew;

		if ((dir-=2)<0)
			dir+=4;

	} while ((x!=xPerim[0] || y!=yPerim[0]) && perimLng<MAX_PERIM_LNG);

	if (perimLng<MIN_PERIM_LNG)
	{
		return 0;
	}
	else if (perimLng>=MAX_PERIM_LNG)
	{
		return 0;
	}
	else
		return 1;
}

void ShapeLoader::findPointiness()
{
	int i;

	//int buffer=perimLng*POINTINESS_BUFFER; //this should be set to less then half the length of the shortest straight side
	int buffer=POINTINESS_BUFFER; //this should be set to less then half the length of the shortest straight side

	double x1, y1, x2, y2, x3, y3, x, y, m, dist;

	int loc1, loc2; //the two locations on the perimeter used to find the pointiness

	for (i=0; i<perimLng; ++i) {

		loc1=i-buffer;
		while (loc1<0)
			loc1+=perimLng;

		loc2=i+buffer;
		while (loc2>=perimLng)
			loc2-=perimLng;

		x1=xPerim[loc1];
		y1=yPerim[loc1];
		x2=xPerim[loc2];
		y2=yPerim[loc2];
		x3=xPerim[i];
		y3=yPerim[i];


		if (x2==x1) { //slope would be undefined
			x=x1;
			y=y3;
		}
		else {

			m=(y2-y1)/(x2-x1); //find the slope of the line connecting point 1 and 2

			x=(m*(y3-y1+m*x1)+x3)/(m*m+1);
			y=m*(x-x1)+y1; //find the point shared by the line connecting 1 and 2 and the line perpendicular to that which runs through 3
		}

		dist=dst(x-x3, y-y3); //find the distance between point 3 and the line between 1 and 2

		perimPointiness[i]=dist;
	}
}

void ShapeLoader::findVerts()
{
	//int		minSegmentSize=perimLng*MIN_SGMNT_LNG;	//the shortest a segment can be
	int		minSegmentSize=MIN_SGMNT_LNG;	//the shortest a segment can be
	//double	thresh=perimLng*VERT_THRESH;
	double	thresh=VERT_THRESH;

	bool	inc=1;			//if we are increasing
	int		perimPos;		//the current position on the perimeter
	int		vertListPos;	//where we are in the vert list

	//find all the verts

	vertNum=0;

	for (perimPos=0; perimPos<perimLng-1; ++perimPos) //perimLng-1 because we compare each perim position to the one after it
	{
		if (perimPointiness[perimPos]<perimPointiness[perimPos+1])
		{
			inc=1; //we are increasing
		}
		else
		{
			if (inc && perimPointiness[perimPos]>thresh) //if increasing is true then this is the first position with decreasing because inc would have been turned off otherwise
			{ //if it just changed from increasing to decreasing then we are at a relative max and should consider adding a vertex
				if (vertNum && perimPos-verts[vertNum-1]<minSegmentSize) //if we just had a vertex recently
				{
					if (perimPointiness[perimPos]>perimPointiness[verts[vertNum-1]]) //if this new max is higher then the last one
					{
						verts[vertNum-1]=perimPos; //replace the last vertex
					} //otherwise leave everything as is; no new vertex is added
				}
				else if (vertNum && verts[0]+perimLng-perimPos<=minSegmentSize) //if we have gone all the way around and are close to the start
				{
					if (perimPointiness[verts[0]]<perimPointiness[perimPos]) //if our new vertex is better then the worst old one
					{
						for (vertListPos=0; vertListPos<vertNum-1; ++vertListPos) //remove the lowest and shift all the rest backward to make room at the end
						{
							verts[vertListPos]=verts[vertListPos+1];
						}

						verts[vertNum-1]=perimPos;
					} //otherwise leave everything as is; no new vertex is added
				}
				else //add vertex
				{
					/*if (vertNum>=TOTAL_VERTS[clrRange]) //if the list is full then we must remove the least pointy vertex to make room
					{
						lowestVert=0;

						for (vertListPos=0; vertListPos<TOTAL_VERTS[clrRange]; ++vertListPos) //search for the vert with the lowest pointiness
						{
							if (perimPointiness[verts[vertListPos]]<perimPointiness[verts[lowestVert]])
								lowestVert=vertListPos;
						}

						if (perimPointiness[verts[lowestVert]]<perimPointiness[perimPos]) //if our new vertex is better then the worst old one
						{
							for (vertListPos=lowestVert; vertListPos<TOTAL_VERTS[clrRange]-1; ++vertListPos) //remove the lowest and shift all the rest backward to make room at the end
							{
								verts[vertListPos]=verts[vertListPos+1];
							}

							verts[TOTAL_VERTS[clrRange]-1]=perimPos;
						}
					}
					else*/
					{
						verts[vertNum]=perimPos;
						++vertNum;
					}
				}
			}
			inc=0;
		}
	}
}

void ShapeLoader::setLines()
{
	int i;

	PlygnBuilder builder;

	for (i=0; i<vertNum; ++i)
	{
		builder.addVert(grdnt(xPerim[verts[i]], 0, wdth, 0, WDTH), grdnt(yPerim[verts[i]], 0, hght, 0, HGHT));
	}

	builder.finish();
}

bool ShapeLoader::checkPix(unsigned int x, unsigned int y)
{ //returns 0 for background and 1 for character

	if (x>=(unsigned int)wdth || y>=(unsigned int)hght)
	{
		cout << ">> tried to look outside of array in checkPix <<" << endl;
		return 0;
	}
	else
		return input[x+y*wdth];
}