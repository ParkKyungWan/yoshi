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