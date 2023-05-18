#pragma once
#include "main.h"

class Player
{
public:

	Player() {

	}
	Player(ID2D1Bitmap* img) {

		D2D1_SIZE_F size = img->GetSize();
		width = size.width / 4.5;
		height = size.height / 4.5;
		xloc = 540.0f; yloc = 393.0f;
		dir = ' ';
		nowImg = '1';


	}

	~Player() {

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
	void  setDir(char c) {
		dir = c;
	}

	char  getDir() {
		return dir;
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
	char dir;
	char nowImg;




};