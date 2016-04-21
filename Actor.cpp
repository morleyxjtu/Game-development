#include "Actor.h"
#include <algorithm>
#include <cstdlib>
#include "StudentWorld.h"

using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

Actor::Actor(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth, StudentWorld* world): GraphObject (imageID, startX, startY, dir, size, depth){
	a_alive = true;
	a_world = world;
	changeVisible(true);
	a_ID = imageID;
	a_state = 0;
}

Actor::~Actor(){
}

//check data member
int Actor::getState() const {
	return a_state;   //return state of actor
}

bool Actor::getAlive() const {
	return a_alive;   //return true if alive
}

StudentWorld* Actor::getWorld() const {
	return a_world;
}

bool Actor::getVisible() const {
	return a_visible;
}

int Actor::getID() const {
	return a_ID;
}

//change data member
void Actor::killActor() {
	a_alive = false;    
}

void Actor::changeVisible(bool vi) {
	setVisible(vi);
	a_visible = vi;
}

void Actor::setState(int s) {
	a_state = s;
}

//perform

void Actor::annoyed(int a){}

void Actor::move(Direction dir, int X, int Y) {
	//based on the direction input, move to next square
	//should have already checked if able to move before move
	switch (dir)  
	{
	case GraphObject::up:  
	{
		moveTo(X, Y + 1);
	}
	break;
	case GraphObject::down:
	{
		moveTo(X, Y - 1);
	}
	break;
	case GraphObject::left:
	{
		moveTo(X-1, Y);
	}
	break;
	case GraphObject::right:
	{
		moveTo(X+1, Y);
	}
	break;
	default:
		break;
	}
}

void Actor::bribe(){}

///////////////////////////////////////////////////////////////////////////
Agent::Agent(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth, StudentWorld* world, int hit):Actor(imageID, startX, startY, dir, size, depth, world) {
	p_hit = hit;   //initialize the hit points
}

Agent::~Agent(){}

void Agent::bribe(){}

int Agent::getHitPoint() const {
	return p_hit;
}

void Agent::setHitPoint(int change) {
	p_hit = p_hit - change;
}

//===========================
FrackMan::FrackMan(StudentWorld* world) : Agent(IID_PLAYER, 30, 60, right, 1.0, 0, world, 10) {
	//initialize all the info to display
	p_water = 5;
	p_sonar = 1;
	p_health = 100;
	p_gold = 0;
}

FrackMan::~FrackMan(){}

void FrackMan::doSomething(){
	if (!getAlive())    //if not alive return
		return;

	StudentWorld* f_world = getWorld();
	int f_X, f_Y;
	f_X = getX();
	f_Y = getY();

	//if the image overlaps with any Dirt
	if (f_world->checkDirt(f_X, f_Y)){
		f_world->removeDirt(f_X, f_Y);     //remove overlapping dirt
		f_world->pushDirtFree(f_X, f_Y);    //push that area into dirt free array
		f_world->playSound(SOUND_DIG);     //play the sound
	}
	else {  //check if human press a key
		int keyValue;   //store the key value
		if (f_world->getKey(keyValue)) {
			switch (keyValue)
			{
			//if press the arrow key
			case KEY_PRESS_LEFT:
				if (getDirection() != left) //if current direction different from press key, change direction
					setDirection(left);
				else if (f_X>0 && !f_world->checkBoulder(f_X - 1, f_Y)){ //in oil field and no boulder in that direction
					moveTo(f_X - 1, f_Y);  //move to target location
				}
				else {
					moveTo(f_X, f_Y);  //if cannot move, do the moving motion at same location
				}
				break;
			case KEY_PRESS_RIGHT:
				if (getDirection() != right)
					setDirection(right);
				else if (f_X<VIEW_WIDTH-4 && !f_world->checkBoulder(f_X + 1, f_Y)) { //in oil field and no boulder in that direction
					moveTo(f_X + 1, f_Y);
				}
				else {
					moveTo(f_X, f_Y);
				}
				break;
			case KEY_PRESS_UP:
				if (getDirection() != up)
					setDirection(up);
				else if (f_Y<VIEW_HEIGHT - 4 && !f_world->checkBoulder(f_X, f_Y+1)) { //in oil field and no boulder in that direction
					moveTo(f_X, f_Y + 1);
				}
				else {
					moveTo(f_X, f_Y);
				}
				break;
			case KEY_PRESS_DOWN:
				if (getDirection() != down)
					setDirection(down);
				else if (f_Y>0 && !f_world->checkBoulder(f_X, f_Y - 1)) { //in oil field and no boulder in that direction
					moveTo(f_X, f_Y - 1);
				}
				else {
					moveTo(f_X, f_Y);
				}
				break;
			//if current water is >0, create a squirt object, reduce count by 1
			case KEY_PRESS_SPACE:
				if (p_water > 0) {
					createSquirt();  //will create squirt object based on current direction of fracman; 
					f_world->playSound(SOUND_PLAYER_SQUIRT);  //play sound anyway
					p_water--;  //waste water anyway
				}
				break;
			//press ESCAPE, set itself to dead, move() will detect this
			case KEY_PRESS_ESCAPE:
				killActor();
				break;
			//
			case 'Z':case 'z':
				if (p_sonar > 0) {
					p_sonar--;
					f_world->makeVisible(f_X, f_Y, 12);
				}
				break;
			case KEY_PRESS_TAB:
				if (p_gold > 0) {
					p_gold--;
					Gold* g = new Gold(f_X, f_Y, f_world, 1);
					f_world->pushOther(g);
				}
				break;
			}
		}
	}
}

void FrackMan::annoyed(int hpoint) {
	setHitPoint(hpoint);
	if (getHitPoint() <= 0) {
		killActor();
		getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
		return;
	}
	p_health -= 20;
}

//check if can create squirt based current direction, if yes, create, if no, do nothing
void FrackMan::createSquirt() {
	StudentWorld* world = getWorld();
	int X, Y;
	X = getX();
	Y = getY();

	switch (getDirection()) //based on current direction
	{
	case up: //if facing up
		//check if destination for squirt is out of boundary or has dirt or boulder there
		if (Y + 4 <= VIEW_HEIGHT && !world->checkDirtBoulder(X, Y + 4)){
			WaterSquirt* ws = new WaterSquirt(X, Y + 4, up, world);  //if OK, create squirt there
			world->pushOther(ws);  //push the object into vector in the world
		}
		break;
	case down: //if facing down, do the same thing as up
		if (Y - 4 >= 0 && !world->checkDirtBoulder(X, Y - 4)) {
			WaterSquirt* ws = new WaterSquirt(X, Y - 4, down, world);
			world->pushOther(ws);
		}
		break;
	case right: //if facing right, do the same thing
		if (X + 4 <= VIEW_WIDTH && !world->checkDirtBoulder(X + 4, Y)) {
			WaterSquirt* ws = new WaterSquirt(X + 4, Y, right, world);
			world->pushOther(ws);
		}
		break;
	case left://if facing left, do the same thing
		if (X - 4 >= 0 && !world->checkDirtBoulder(X - 4, Y)) {
			WaterSquirt* ws = new WaterSquirt(X - 4, Y, left, world);
			world->pushOther(ws);
		}
		break;
	}
}

int FrackMan::getHealth() const{
	return p_health;
}

int FrackMan::getSquirt() const {
	return p_water;
}

int FrackMan::getSonar() const {
	return p_sonar;
}

int FrackMan::getGold() const {
	return p_gold;
}

void FrackMan::changeGold(int g) {
	p_gold = p_gold + g;
}

void FrackMan::changeSonar(int num) {
	p_sonar += num;
}

void FrackMan::changeWater(int num) {
	p_water += num;
}

//===========================
Protester::Protester(StudentWorld* world, int imageID, int hit):Agent(imageID, 60, 60, left, 1.0, 0, world, hit) {
	waitTick = std::max(int(3 - world->getLevel() / 4), 0);
	stunTick = std::min(int(100 - world->getLevel() * 10), 50);
	restTick = waitTick;
	intersectNum = 200;
	noShoutNum = 0;
	unitMove = 34;
	
}
Protester::~Protester(){}

void Protester::doSomething(){

	//1. check if the Protester is alive
	if (!getAlive())
		return;

	StudentWorld* world = getWorld();
	int X = getX();
	int Y = getY();
	//2. check if Protester is in rest state
	if (restTick != 0) {
		restTick--;
		return;
	}
	else { //if not in rest state
		intersectNum--;  //intersect turn counter reduced by one
		if (getState() == 2) {  //if in post shouting state
			noShoutNum--;  //reduce counter by one
			if (noShoutNum == 0)  //if shouting ends
				setState(0);  //set to nornmal state
			else
				return;
		}
		restTick = waitTick;  //in case changed rest tick, now change back
	}

	//3. if the protester is in leave-the-oil-field state
	if (getHitPoint() <= 0 || getState() == 1) {	//"killed" by Fracman or Boulder
		setState(1); //set the state to be 1: leaving state
		if (X == 60 && Y == 60) {  //3.a if at (60, 60), remove protester
			killActor();
			return;
		}
		else {		//3.b move one square closer to the exit
			Direction d = world->optimalDirection(X, Y, false);		//optimalDirection search the best route
			setDirection(d);
			move(d, X, Y);
			return;
		}
	}

	//4. if within 4 units of the Frackman AND facing Fracman
	if (world->fracRadius(X, Y) <= 4 && world->faceFrac(X, Y, getDirection())) {
		if (getState() == 0) { //if not shouted within last non-resting 15 ticks, always attack in first round by not in leaving state
			world->playSound(SOUND_PROTESTER_YELL);  //play sound
			world->annoyFrac(2);		//annoy Fracman by 2 points
			setState(2);  //rest for 5 non-resting tick
			noShoutNum = 20;  //at least 15
			return;		//return
		}
	}

	specialSense(); //only make sense for Hard Protester
	Direction d_align = none;
	// 5. if can see Fracman(align, more than 4 units away, no boulder or sand blocking)
	if (world->alignFrac(X, Y, d_align)) {
		setDirection(d_align); //a.Turn to Fracman
		move(d_align, X, Y);		//move one step toward him
		unitMove = 0;		//b.set unitMove to zero
		return;
	}

	//6. cannot see the fracman and too far away from it
	else { 
		unitMove--; //decrease unitMove by one
		//if finished walking
		if (unitMove <= 0) { 
			Direction d[] = { left, right, up, down };
			while (true) {
				int random = world->randInt(0, 3); //pick a random direction out of four
				Direction dir = d[random];
				if (world->ableToMove(dir, X, Y)) { //if can move in that direction, move; if not, re-choose
					setDirection(dir);
					break;  //break out the loop
				}
			}
			unitMove = 34; //pick a new value for unitMove
		}
		else { //if not finished walking
			if (intersectNum <= 0) {
				Direction d = none;
				if (world->turn(X, Y, getDirection(), d)) { //true:sits at intersection and pass the feasible direction to d
					setDirection(d);	//set direction
					unitMove = 34;		//set unitMove
					intersectNum = 200; //record the moment
				}
			}
		}
		//8. if can move in current direction, move
		if (world->ableToMove(getDirection(), X, Y)) { //if can move in current direction
			move(getDirection(), X, Y);
		}
		else {//if not, set unitMove = 0, rechoose a direction in next tick
			unitMove = 0;
		}
	}
}

void Protester::annoyed(int hpoint){
	if (getState() != 1) {  //only can be annoyed when not in leaving state
		setHitPoint(hpoint);
		if (getHitPoint() > 0) {
			getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
			restTick = stunTick;
		}
		else {
			setState(1);
			getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
			restTick = 0;
			afterAnnoy();
		}
	}
}

void Protester::bribe(){
	getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
	afterBribe();
}

void Protester::setRestTick(int n) {
	restTick = n;
}

//===========================
RegularProtester::RegularProtester(StudentWorld* world) :Protester(world, IID_PROTESTER, 5) {
}

RegularProtester::~RegularProtester() {}

void RegularProtester::specialSense(){
}

void RegularProtester::afterBribe() {
	getWorld()->increaseScore(25);
	setState(1);
	setRestTick(0);
}

void RegularProtester::afterAnnoy() {
	getWorld()->increaseScore(100);
}

//===========================
HardcoreProtester::HardcoreProtester(StudentWorld* world) :Protester(world, IID_HARD_CORE_PROTESTER, 20) {
}

HardcoreProtester::~HardcoreProtester() {}

void HardcoreProtester::specialSense() {
	StudentWorld* world = getWorld();
	int X = getX();
	int Y = getY();
	int M = 16 + world->getLevel() * 2;
	if (world->moveToFrac(X, Y) <= M) {  //if the step to the fracman is less than M
		Direction d = world->optimalDirection(X, Y, true);  //find the optimal direction to move
		setDirection(d);
		move(d, X, Y);
		return;
	}
}

void HardcoreProtester::afterBribe() {
	getWorld()->increaseScore(50);
	int stare = std::min(int(100 - getWorld()->getLevel() * 10), 50);
	setRestTick(stare);  //set the rest tick to be stare
}

void HardcoreProtester::afterAnnoy() {
	getWorld()->increaseScore(500);
}

///////////////////////////////////////////////////////////////////////////

Treasure::Treasure(int imageID, int X, int Y, StudentWorld* world, int state):Actor(imageID, X, Y, right, 1.0, 2, world){
	if (state == 1) { //if start in temparory state
		setState(state);
		createTick = world->getTick();
		lifetime = std::min(int(300 - 10 * world->getLevel()), 100);  
	}
	
}

Treasure::~Treasure() {}

void Treasure::doSomething(){
	if (!getAlive())
		return;

	StudentWorld* world = getWorld();
	int X, Y;
	X = getX();
	Y = getY();

	//if currently not visible AND fracman is within raidus of 4, show the object
	if (!getVisible() && world->fracRadius(X, Y) <= 4) {
		changeVisible(true);
		return;
	}
	//if within radius of 3 of fracman 
	if (world->fracRadius(X, Y) <= 3) {
		foundTreasure();
	}
	//if gold within radius of 3 of protester
	goldProtester();
	//object in temporary state only, disappear
	if (getState() == 1 && (world->getTick() - createTick) >= lifetime)
		killActor();
}

void Treasure::goldProtester() {} //except for gold, other class return nothing

void Treasure::setLifetime(int l) {
	lifetime = l;
}

//==========================================

Oil::Oil(int o_X, int o_Y, StudentWorld* world) : Treasure(IID_BARREL, o_X, o_Y, world, 0) {
	changeVisible(false);
};

Oil::~Oil(){}

void Oil::foundTreasure() {
	killActor();
	StudentWorld* world = getWorld();
	world->playSound(SOUND_FOUND_OIL);
	world->increaseScore(1000);
	world->changeOil(-1);
}

//===========================
Gold::Gold(int g_X, int g_Y, StudentWorld* world, int state) :Treasure(IID_GOLD, g_X, g_Y, world, state) {
	if (state == 0) {
		changeVisible(false);
	}
	else {
		setLifetime(100);
	}
}

Gold::~Gold(){}

void Gold::foundTreasure() {
	if (getState() == 0) {  //only can be picked up by fracman in permanent state
		killActor();
		StudentWorld* world = getWorld();
		world->playSound(SOUND_GOT_GOODIE);
		world->increaseScore(10);
		world->changeFracGold(1);
	}
}

void Gold::goldProtester() {
	if (getState() == 1 && getWorld()->protBribe(getX(), getY())) {
		killActor();
		getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
	}
}

//===========================
Sonar::Sonar(StudentWorld* world):Treasure(IID_SONAR, 0, 60, world, 1){
}

Sonar::~Sonar() {}

void Sonar::foundTreasure(){
	killActor();
	getWorld()->playSound(SOUND_GOT_GOODIE);
	getWorld()->changeSonar(1);
	getWorld()->increaseScore(75);
	return;
}

//===========================
WaterPool::WaterPool(int w_x, int w_y, StudentWorld* world) :Treasure(IID_WATER_POOL, w_x, w_y, world, 1) {
}

WaterPool::~WaterPool(){}

void WaterPool::foundTreasure(){
	killActor();
	getWorld()->playSound(SOUND_GOT_GOODIE);
	getWorld()->changeWater(5);
	getWorld()->increaseScore(100);
	return;
}

///////////////////////////////////////////////////////////////////////////
WaterSquirt::WaterSquirt(int b_X, int b_Y, Direction dir, StudentWorld* world) :Actor(IID_WATER_SPURT, b_X, b_Y, dir, 1, 1, world) //boulder initialization
{
	step = 0;
}

WaterSquirt::~WaterSquirt() {  };

void WaterSquirt::doSomething() {
	StudentWorld* world = getWorld();
	int X = getX();
	int Y = getY();


	if (step > 4) {  //if travel out of 4 steps , kill
		killActor();
		return;
	}

	if (step == 0) {
		step++;
	}
	else {
		if (world->protHit(X, Y, 2)) {   //if hit protester
			killActor();
			return;
		}
		if (world->ableToMove(getDirection(), X, Y)) {
			move(getDirection(), X, Y);
			step++;
		}
		else
			killActor();
	}
}

///////////////////////////////////////////////////////////////////////////
Dirt::Dirt(int d_X, int d_Y, StudentWorld* world) :Actor(IID_DIRT, d_X, d_Y, right, 0.25, 3, world) {//how to take advantage of default values?

}

Dirt::~Dirt() {}

void Dirt::doSomething(){}

///////////////////////////////////////////////////////////////////////////
Boulder::Boulder(int b_X, int b_Y, StudentWorld* world) :Actor(IID_BOULDER, b_X, b_Y, down, 1, 1, world) //boulder initialization
{
}

Boulder::~Boulder() {}

void Boulder::doSomething() {
	//if not alive, return
	if (!getAlive())
		return;

	StudentWorld* b_world = getWorld();
	int X, Y;
	X = getX();
	Y = getY();
	int ini_X, ini_Y;
	ini_X = X;
	ini_Y = Y;
	if (getState() == 0) {  //if in stable state
		if (Y == 0 || b_world->checkDirtBoulder(X, Y - 1))
			return;
		setState(1); //waiting state
		waitTick = b_world->getTick(); //start counting ticks
		return;
	}
	//if in waiting state and 30 ticks has passed
	if (getState() == 1 && (b_world->getTick() - waitTick) == 30) {
		setState(2);  //set into falling state
		b_world->playSound(SOUND_FALLING_ROCK);
	}
	//if in falling state
	if (getState() == 2) {
		if (Y == 0 || b_world->checkDirtBoulder(X, Y - 1)) {//if falls onto oil field bottom or falls onto dirt or Boulder
			killActor(); //kill the boulder
			return;
		}
		//if boulder comes with
		if (b_world->fracRadius(X, Y) <= 3) {
			b_world->annoyFrac(100);
		}
		b_world->protHit(X, Y, 100);
		moveTo(X, Y - 1);
	}
}








