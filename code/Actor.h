#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
class StudentWorld;
// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp
class Actor : public GraphObject {
public:
	Actor(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth, StudentWorld* world); // constructor 
	virtual ~Actor() ; //destructor

	//check data member
	int getState() const;  //get the state of the actor
	bool getAlive() const;  //return true if actor is alive
	StudentWorld* getWorld() const;  //return the world the actor is in
	bool getVisible() const;  //return true is the actor is visible
	int getID() const;   //return the ID of the actor

	//change data memeber
	void setState(int state);  //set the state of the actor
	void killActor();  //kill the actor so it can be removed at the end of the tick
	void changeVisible(bool vi);  //change the visibility of the actor to vi

	//perform
	virtual void doSomething() = 0; //pure virtual, every actor needs to do something every tick
	virtual void annoyed(int a);  //protester and frackman can be annoyed, reducing a hit points
	void move(Direction dir, int X, int Y); //move from (X, Y) toward dir one square
  	virtual void bribe();   //protester can be bribed

private:
	bool a_alive;  //true if actor is alive
	StudentWorld* a_world;  //the world actor is in
	bool a_visible;  //true if actor is visible
	int a_ID;   //actor`s ID
	int a_state;   //actor`s state
};

///////////////////////////////////////////////////////////////////////////
class Agent :public Actor {
public:
	Agent(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth, StudentWorld* world, int hit);
	virtual ~Agent();      //destructor
	virtual void annoyed(int hpoint) = 0;     //both frackman and protestor will have annoyed function to reduce hpoint
	virtual void bribe();   //protester will be bribed
	int getHitPoint() const;     //return the hit point of agent
	void setHitPoint(int change);  //set the hit point of the agent
private:
	virtual void doSomething() = 0;   //both frackman and protestor have do somthing function
	int p_hit;  //record the hit points of the agent
};

//===========================

class FrackMan : public Agent {
public:
	FrackMan(StudentWorld* world);		//constructor
	virtual ~FrackMan();	//destructor
	virtual void doSomething();   //do somthing every tick
	virtual void annoyed(int hpoint);  //reduce hpoints after annoyed
	
	int getHealth() const; //return the health of the frackman
	int getSquirt() const;      //return the water squirt the frackman has
	int getSonar() const;    //return the sonar the frackman has
	int getGold() const;       //return the gold the frackman has

	void changeGold(int g);       //change the number of gold to g
	void changeSonar(int num);       //change the number of sonar to num
	void changeWater(int num);      //change the number of water to num

private:
	void createSquirt();      //create a squirt object 
	int p_water;			//water frackman has
	int p_sonar;					//sonar frackman has
	int p_health;			//health of the frackman
	int p_gold;					//gold the frackman has
};

//===========================
class Protester :public Agent {
public:
	Protester(StudentWorld* world, int imageID, int hit);   //constructor
	virtual ~Protester();    //destructor
	virtual void doSomething();		//common things regular and hardcore protester do each tick
	virtual void annoyed(int hpoint);  //common things regular and hardcore protester do when annoyed
	virtual void bribe();  //common things regular and hardcore protester do when bribed
	void setRestTick(int n);	//set the resttick
	
private:
	virtual void specialSense() = 0;  //hardcore protestor has special sensing ability
	virtual void afterBribe() = 0;     //different things regular and hardcore protester do after being bribed
	virtual void afterAnnoy() = 0;  //different things regular and hardcore protester do after being annoyed
	int waitTick;   //the tick to wait before moving
	int stunTick;   //the tick to wait after stuned by water squirt
	int intersectNum;   //the tick to wait before turn at intersect
	int noShoutNum;   //thr tick to wait before next shout
	int unitMove;    //the number to move in current direction
	int restTick;   //rest tick between action
};

//===========================
class RegularProtester :public Protester {
public:
	RegularProtester(StudentWorld* world);  //constructor
	virtual ~RegularProtester();   //destructor
	
private:
	virtual void specialSense();  //nothing
	virtual void afterBribe();    //action after bribe
	virtual void afterAnnoy();   //action after annoyed
};

//===========================
class HardcoreProtester :public Protester {
public:
	HardcoreProtester(StudentWorld* world);  //constructor
	virtual ~HardcoreProtester();   //destrctor
	
private:
	virtual void specialSense();   //special sense that can detact frackman
	virtual void afterBribe();     //action after bribe
	virtual void afterAnnoy();     //action after annoyed
};

///////////////////////////////////////////////////////////////////////////
class Treasure:public Actor {
public:
	Treasure(int imageID, int X, int Y, StudentWorld* world, int state = 0);  //constructor
	virtual ~Treasure();    //destructor
	virtual void doSomething();   //common things all treasure object do
	void setLifetime(int l);   //set the lifetime
private:
	virtual void foundTreasure() = 0;   //differetn things objects do after finding treasure
	virtual void goldProtester();   //when protester meet gold
	int createTick;   //the creation tick of the object
	int lifetime;    //lifetime of the object, -1 is permanent
};

//===========================
class Oil :public Treasure {
public:
	Oil(int o_X, int o_Y, StudentWorld* world);    //constructor
	virtual ~Oil();    //destructor
private:
	virtual void foundTreasure();    //action after being found
};

//===========================
class Gold :public Treasure {
public:
	Gold(int g_X, int g_Y, StudentWorld* world, int state);   //constructor
	virtual ~Gold();   //destructor
	
private:
	virtual void foundTreasure();   //action after being found
	virtual void goldProtester();    //action when meet gold
	int g_state; //0 is permanent state; 1 is temp

};

//===========================
class Sonar :public Treasure {
public:
	Sonar(StudentWorld* world);  //constructor
	virtual ~Sonar();  //destructor
	
private:
	virtual void foundTreasure();   //action after being found

};

//===========================
class WaterPool : public Treasure {
public: 
	WaterPool(int w_x, int w_y, StudentWorld* world);   //constructor
	virtual ~WaterPool();   //destructor
private:
	virtual void foundTreasure();    //action after being found
};

///////////////////////////////////////////////////////////////////////////
class WaterSquirt :public Actor {
public:
	WaterSquirt(int w_X, int w_Y, Direction dir, StudentWorld* world);   //constructor
	virtual ~WaterSquirt();     //destructor
	virtual void doSomething();     //action during each tick
private:
	int step;    //step that has traveled
};

///////////////////////////////////////////////////////////////////////////
class Dirt : public Actor {
public:
	Dirt(int d_X, int d_Y, StudentWorld* world);	//constructor
	virtual ~Dirt();  //destructor
	virtual void doSomething();   //action each tick
};

///////////////////////////////////////////////////////////////////////////

class Boulder :public Actor {
public:
	Boulder(int b_X, int b_Y, StudentWorld* world);   //constructor
	virtual ~Boulder();   //destructor
	virtual void doSomething();   //action each tick
private:
	int waitTick;   //ticks being initialized
};


#endif // ACTOR_H_
