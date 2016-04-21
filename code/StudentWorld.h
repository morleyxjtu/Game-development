#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"

#include <vector>
#include <list>
#include <string>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir);
	virtual ~StudentWorld();
	//called by system files
	virtual int init();
	virtual int move();
	virtual void cleanUp();

	//setting display text
	void setDisplayText();

	//checking objects
	int getTick() const;  //get current tick
	double fracRadius(int d_X, int d_Y) const;  //return the radius distance to fracman
	bool protBribe(int d_X, int d_Y) const;  //bribe any protester within radius of 3; if can bribe, return true, otherwise false
	bool protHit(int d_X, int d_Y, int annoy) const; //hit any protesters within radius of 3, with "annoy" hitpoints
	bool checkDirt(int d_X, int d_Y) const;		//check if there is dirt at 4x4 square at (d_X, d_Y)
	bool checkBoulder(int d_X, int d_Y)const;	//check if there is boulder at 4x4 square at (d_X, d_Y)
	bool checkDirtBoulder(int d_X, int d_Y) const;	//check if there is dirt or Boulder at 4x4 square at (d_X, d_Y)
	bool faceFrac(int d_X, int d_Y, GraphObject::Direction dir) const; //check if protester at (d_X, d_Y) with dir is facing fracman
	bool ableToMove(GraphObject::Direction dir, int d_X, int d_Y) const;// check if able to move to dir one square from (d_X, d_Y)
	int moveToFrac(int d_X, int d_Y) const;  //return the moves from protester to frac man in shortest path
	
	//set objects
	void removeDirt(int d_X, int d_Y); //clear a 4x4 region of dirt at (d_X, d_Y)
	void pushOther(Actor* a);  //push actor a into the otherObj vector
	void pushDirtFree(int d_X, int d_Y);	//push (d_X, d_Y) into dirt free vector
	void makeVisible(int d_X, int d_Y, int radius) const; //set objects within a circle at (d_X, d_Y) with "radius" to be visible
	void changeOil(int o);		//change the number of oil by o
	void changeFracGold(int g) const;  //change the gold held by fracman by g
	void annoyFrac(int p) const; //frac man reduce p hit points
	bool alignFrac(int d_X, int d_Y, GraphObject::Direction &dir) const; //check if (d_X, d_Y) is aligned with fracman, change dir to direction to move to fracman
	void changeSonar(int num) const; //change the number of sonar fracman has
	void changeWater(int num) const;  //change the number of water fracman has
	void breadthSearch(bool frac);  //if true, search the distance from fracman; if false, search the distance from (60, 60)
	GraphObject::Direction optimalDirection(int d_X, int d_Y, bool frac); //if true, return the optimal direction to move to fracman; false, return the optimal direction to (60, 60)
	bool turn(int d_X, int d_Y, GraphObject::Direction current_dir, GraphObject::Direction &target_dir); //if (d_X, d_Y) is at interset, based on current direction, calculate target direction, then return true;
	int randInt(int min, int max) const;  //create random number between min and max inclusive



private:
	//data member saving actors
	FrackMan* s_frac;  //pointer to Fracman
	Dirt* s_dirt[VIEW_WIDTH][VIEW_HEIGHT];  //array saving dirt
	std::vector<Actor*> otherObj;		//vector to save all actors except Fracman and dirt

	//helper struct saving coordinate and distance info
	struct Coord {
		int c_X;
		int c_Y;
		int distance;  //distance label
		Coord(int x, int y) {  //c`tor that not needs distance label
			c_X = x;
			c_Y = y;
			distance = 0;
		}
		Coord(int x, int y, int d) {  //c`tor that needs distance label
			c_X = x;
			c_Y = y;
			distance = d;
		}
	};
	bool tooClose(int x1, int y1, int x2, int y2, int distance) const; //if the distance between (x1, y1) and (x2, y2) smaller or equal than distance, return true
	int distance[VIEW_WIDTH][VIEW_HEIGHT]; //distance matrix to (60, 60)
	int step[VIEW_WIDTH][VIEW_HEIGHT]; //distance matrix to fracman
	std::vector<Coord> avaiSpot;  //available spot to put boulder, gold and oil
	std::vector<Coord> dirtFree;  //dirt free place to put waterpool
	
	//int data members
	int s_tick;  //record current stick in this level
	int proAddTick;  //record the tick when last protester is added
	int numPro;		//record the number of protester in field
	int numOil;		//record the number of oil left in field
};

#endif // STUDENTWORLD_H_
