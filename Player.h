#pragma once
#include "main.h"

class Player
{
public:

	Player() {

	}
	Player(ID2D1Bitmap* img) {

		D2D1_SIZE_F size = img->GetSize();
		width = size.width / 8;
		height = size.height / 8;
		xloc = 540.0f; yloc = 393.0f;
		dir = ' ';
		nowImg = '1';
		direction = 'D';
		throwing == false;


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
		float pSize[2] = { width, height};
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
		if ( c != ' ')direction = c;
		
	}

	char  getDir() {
		return dir;
	}

	char  getDirection() {
		return direction;
	}
	char getNImg() {
		return nowImg;
	}

	void setNImg(char c) {
		nowImg = c;
	}

	void setHP(int type, float hp) {
		if (type == 0) {
			//damage
			HP -= hp;
		}
		else if (type == 1) {
			//heal
			HP += hp;
		}
		else {
			//default
			HP = hp;
		}
	}

	float getHP() {
		return HP;
	}
	void ready() {
		throwing = true;
	}
	void throw_egg() {
		throwing = false;
	}
	bool isThrowing() {
		return throwing;
	}
private:

	float width, height;
	float xloc, yloc;
	char dir;
	char direction;
	char nowImg;
	float HP;
	bool throwing;



};