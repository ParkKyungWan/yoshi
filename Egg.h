#pragma once
#include "main.h"

class Egg
{
public:

	Egg() {

	}
	Egg(ID2D1Bitmap* img) {

		D2D1_SIZE_F size = img->GetSize();
		width = size.width / 9;
		height = size.height / 9;

		xloc = 540.0f; yloc = 393.0f;

	}

	~Egg() {

	}

	float getX() {
		return xloc;
	}

	float getY() {
		return yloc;
	}
	float* getSize() {
		float pSize[2] = { width, height };
		return pSize;
	}

	void setX(float f) {
		xloc = f;
	}

	void  setY(float f) {
		yloc = f;
	}

private:

	float width, height;
	float xloc, yloc;
	



};

////////////////////////////////////////////////////////

class Thrwd_Egg {

public:

	Thrwd_Egg() {

	}
	Thrwd_Egg(ID2D1Bitmap* img, float x, float y,float s, float d, float dir) {
		
		D2D1_SIZE_F size = img->GetSize();
		width = size.width / 9;
		height = size.height / 9; 
		xloc = x; yloc = y;
		pwS = s;  pwD = d;
		direction = dir;


	}

	~Thrwd_Egg() {

	}

	float getX() {
		return xloc;
	}

	float getY() {
		return yloc;
	}
	float* getSize() {
		float pSize[2] = { width, height };
		return pSize;
	}

	void setX(float f) {
		xloc = f;
	}

	void  setY(float f) {
		yloc = f;
	}

	void next() {
		pwD += (pwD >= 15.0f) ? 0.0f : 0.6f;
		xloc += pwS * direction;
		yloc += pwD;
	}

private:

	float width, height;
	float xloc, yloc;
	float pwS, pwD; //side, down
	float direction;
};