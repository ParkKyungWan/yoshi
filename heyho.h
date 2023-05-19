#pragma once
#include "main.h"

class HeyHo
{
public:

	HeyHo() {

	}
	HeyHo(ID2D1Bitmap* img) {

		D2D1_SIZE_F size = img->GetSize();
		width = size.width / 2;
		height = size.height / 2;
		xloc = 50.0f; yloc = 200.0f;
		nowImg = 0;
	

	}

	~HeyHo() {

	}

	float getX() {
		return xloc;
	}

	float getY() {
		return yloc;
	}
	float* getSize() {
		float pSize[2] = { width + xloc, height + yloc };
		return pSize;
	}

	void setX(float f) {
		xloc = f;
	}

	void  setY(float f) {
		yloc = f;
	}
	char getNImg() {
		return nowImg;
	}

	void setNImg(char c) {
		nowImg = c;
	}
private:

	float width, height;
	float xloc, yloc;
	int nowImg;




};