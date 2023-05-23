#pragma once
#include "main.h"
#include <cstdlib>
#include <ctime>

// 자원 안전 반환 매크로.
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

class HeyHo
{
public:

	HeyHo() {

	}
	HeyHo(ID2D1Bitmap* img, float px, float py) {

		D2D1_SIZE_F size = img->GetSize();
		width = size.width / 2;
		height = size.height / 2;

		srand((unsigned int)time(NULL));
		rand = std::rand() % 2;
		switch (rand) {

		case 0:
			xloc = -30.0f; yloc = std::rand() % 720;
			break;
		case 1:
			xloc = 1080.0f; yloc = std::rand() % 720;
			break;
		default:
			xloc = 80.0f; yloc = 180.0f; //활동범위 
			break;
		}


		direction[0] = (px - xloc);
		direction[1] = (py - yloc) ;
		
		nowImg = 0;
		speed = 200.0f;

	

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
		float pSize[2] = { width , height };
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

	int nextImg() {
		if (nowImg++ > 2) {
			nowImg = 0;
		}
		return nowImg;
	}
	float getSpd() {
		return speed;
	}
	float getDirectionX() {
		return direction[0];
	}
	float getDirectionY() {
		return direction[1];
	}
	int getRnd() {
		return rand;
	}

	bool isDamaged(float x, float y) {
		if (xloc < x && xloc+width > x && yloc < y && yloc+height > y ) {
			return true;

		}
		else {
			return false;
		}
	}
private:

	float width, height;
	float xloc, yloc;
	int nowImg;
	float speed;
	float direction[2];
	int rand;




};


struct dead_point {
	float dirx;
	float diry;
	float x;
	float y;
	int count;
};