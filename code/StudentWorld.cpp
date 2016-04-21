#include "StudentWorld.h"
#include <algorithm>
#include <cstdlib>
#include <string>
#include "math.h"
#include <queue>
#include <random>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetDir) : GameWorld(assetDir) {
	s_frac = nullptr;
	s_tick = 0;
	numOil = 2;
	proAddTick = 0;
	numPro = 0;
	for (int i = 0; i < VIEW_WIDTH; i++) {
		for (int j = 0; j < VIEW_HEIGHT; j++) {
			s_dirt[i][j] = nullptr;
			distance[i][j] = -1;
		}
	}

}

StudentWorld::~StudentWorld() {
	//clear all sand objects, set the array to nullptr
	for (int i = 0; i < VIEW_WIDTH; i++) {
		for (int j = 0; j < VIEW_HEIGHT; j++) {
			if (s_dirt[i][j] != nullptr) {
				delete s_dirt[i][j];
			}
		}
	}
	//remove fracman object
	delete s_frac;
	//remove any alocated objects in otherObj
	for (unsigned int n = 0; n < otherObj.size(); n++) {
		delete otherObj[n];
	}
	//clean the vector to size of 0
	otherObj.clear();
	avaiSpot.clear();
	dirtFree.clear();
}

int StudentWorld::init() {
	//initialize built-in data member
	proAddTick = 0;
	numPro = 0;
	s_tick = 0;
	numOil = 2;
	//initialize available spots for random objects
	for (int n = 0; n < 60; n++) {
		for (int m = 20; m < 56; m++) {
			if (n < 27 || n>33) {
				Coord c(n, m);
				avaiSpot.push_back(c);
			}
		}
	}
	//initialize dirtFree
	for (int j = 4; j < 57; j++) {
		Coord c(30, j);
		dirtFree.push_back(c);
	}

	//initialize FrackMan
	s_frac = new FrackMan(this);
	//initialize all dirts
	for (int i = 0; i < VIEW_WIDTH; i++) {
		for (int j = 0; j < VIEW_HEIGHT; j++) {
			if(j < 60){
				if (i >= 30 && i <= 33 && j >= 4 && j <= 59) { //leave a column not filled
					s_dirt[i][j] = nullptr;
				}
				else {
					Dirt* d = new Dirt(i, j, this);	//dynamically allocate Dirt objects, initialize with position
					s_dirt[i][j] = d;
				}
			}
			else {
				s_dirt[i][j] = nullptr; //y>=60 not filled
			}
		}
	}

	//initialize distance array indicating shortest distance to (60, 60)
	breadthSearch(false);
	//initialize step array indicating shortest distance to fracman current position
	breadthSearch(true);

	//initialize other actors
	int s_level = getLevel();
	int B = std::min(s_level / 2 + 2, 6); //number of Boulders
	int G = std::max(5 - s_level / 2, 2);  //number of gold nuggets
	int L = std::min(2 + s_level, 20);  //number of oils
	numOil = L;  //save the number of oil as data member

	//Boulders initialization
	for (int i = 0; i < B; i++) {
		int m = randInt(0, avaiSpot.size()-1); //randomly pick a spot in available spots collection
		int X = avaiSpot[m].c_X;
		int Y = avaiSpot[m].c_Y;
		Boulder* temp = new Boulder(X, Y, this);  //create a new boulder
		pushDirtFree(X, Y);				//add the position to dirtFree so waterpool can be there
		otherObj.push_back(temp);		//push into otherObj
		removeDirt(X, Y);		//remove dirt around the boulder
		//remove the spots that are within radius 6 from the available spots collection
		unsigned int j = 0;
		while (j < avaiSpot.size()) {
			if (tooClose(X, Y, avaiSpot[j].c_X, avaiSpot[j].c_Y, 6)) { //if the position is within 6
				avaiSpot.erase(avaiSpot.begin() + j); //erase it from the collection
			}
			else {
				j++;
			}
		}
	}

	//Oil
	for (int i = 0; i < L; i++) {
		int m = randInt(0, avaiSpot.size() - 1);	//randomly pick a spot in available spots collection
		int X = avaiSpot[m].c_X;
		int Y = avaiSpot[m].c_Y;
		Oil* temp = new Oil(X, Y, this);		//create a new oil
		otherObj.push_back(temp);		//push into otherObj
		//remove the spots that are within radius 6 from the available spots collection
		unsigned int j = 0;
		while (j < avaiSpot.size()) {
			if (tooClose(X, Y, avaiSpot[j].c_X, avaiSpot[j].c_Y, 6)) {//if the position is within 6
				avaiSpot.erase(avaiSpot.begin() + j);//erase it from the collection
			}
			else {
				j++;
			}
		}
	}
	//Gold
	for (int i = 0; i < G; i++) {
		int m = randInt(0, avaiSpot.size() - 1);//randomly pick a spot in available spots collection
		int X = avaiSpot[m].c_X;
		int Y = avaiSpot[m].c_Y;
		Gold* temp = new Gold(X, Y, this, 0);//create a new gold
		otherObj.push_back(temp);//push into otherObj
		//remove the spots that are within radius 6 from the available spots collection
		unsigned int j = 0;
		while (j < avaiSpot.size()) { 
			if (tooClose(X, Y, avaiSpot[j].c_X, avaiSpot[j].c_Y, 6)) {//if the position is within 6
				avaiSpot.erase(avaiSpot.begin() + j);//erase it from the collection
			}
			else {
				j++;
			}
		}
	}

	return GWSTATUS_CONTINUE_GAME;

}

int StudentWorld::move() {
	s_tick++;  //increase the tick by one;
	//update display text
	setDisplayText();

	//actors do something
	//FrackMan do something
	if (s_frac->getAlive()) {  //if fracman is alive
		s_frac->doSomething();  //do something
		//if dead after do somthing, e.g., press escape key
		if (!s_frac->getAlive()) {  
			decLives();  //decrease lives
			return GWSTATUS_PLAYER_DIED;  //return
		}
		if (numOil == 0)  //if found all oil
			return GWSTATUS_FINISHED_LEVEL;  //proceed to next level
	}

	//other objects do something
	for (unsigned int i = 0; i < otherObj.size(); i++) {
		if(otherObj[i]->getAlive())  //if the object is alive
			otherObj[i]->doSomething();  //do something
		//if the fracman is dead after other object did something, decrease lives and return dead
		if (!s_frac->getAlive()) {
			decLives();
			return GWSTATUS_PLAYER_DIED;
		}
		//if found all oil, next level
		if (numOil == 0)
			return GWSTATUS_FINISHED_LEVEL;
	}

	//remove dead actors
	for (unsigned int n = 0; n < otherObj.size(); n++) {
		if (!otherObj[n]->getAlive()) {
			delete otherObj[n];
			otherObj.erase(otherObj.begin() + n);
		}
	}

	breadthSearch(false); // recalculate the distance matrix to (60, 60)
	breadthSearch(true);  //recaculate the distance matrix to Fracman

	//if fracman dead, return
	if (!s_frac->getAlive()) {
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}
	//if found all oil next level
	if (numOil == 0) {
		playSound(SOUND_FINISHED_LEVEL);
		return GWSTATUS_FINISHED_LEVEL;
	}
		

	//add new actors
	//Add Protester
	int s_level = getLevel();
	int T = std::max(200 - s_level, 25); //wait time to introduce another protester
	int P = std::min(int(2 + s_level * 1.5) , 15);  //max number of protester
	if ((s_tick ==1 ||(s_tick- proAddTick) > T) && numPro < P) { //if first tick, add; else if T has passed AND number less than P
		int Pr = std::min(s_level * 10 + 30, 90);  //probability of being hardcore
		int ran = randInt(1, 100);
		if (ran <= Pr) {
			HardcoreProtester* hp = new HardcoreProtester(this);  //create hardcore
			otherObj.push_back(hp);		//push into otherOhj
			numPro++;		//increase number
			proAddTick = s_tick;		//record the adding moment
		}
		else {
			RegularProtester* p = new RegularProtester(this);//create hardcore
			otherObj.push_back(p);//push into otherOhj
			numPro++;//increase number
			proAddTick = s_tick;//record the adding moment
		}
		
	}
	
	//Goodies
	int G = getLevel() * 25 + 300;
	int ran1 = randInt(1, G); //the probability of adding one goodie is 1/G
	if (ran1 == 1) {
		int ran2 = randInt(1, 5);
		if (ran2 == 1) {  //1/5 chance adding sonar
			Sonar* s = new Sonar(this);  //create sonar
			otherObj.push_back(s);		//push into otherObj
		}
		else {  //4/5 chance adding water pool
			int m = randInt(0, dirtFree.size()-1);   //randomly pick a position in dirt-free position collection
			int X = dirtFree[m].c_X;
			int Y = dirtFree[m].c_Y;
			WaterPool* wp = new WaterPool(X, Y, this);		//create a waterpool
			otherObj.push_back(wp);		//push into the otherObj
		}
	}

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp() {
	//clear all sand objects, set the array to nullptr
	for (int i = 0; i < VIEW_WIDTH; i++) {
		for (int j = 0; j < VIEW_HEIGHT; j++) {
			if (s_dirt[i][j] != nullptr) {
				delete s_dirt[i][j];
			}
		}
	}
	//remove fracman object
	delete s_frac;
	//remove any alocated objects in otherObj
	for (unsigned int n = 0; n < otherObj.size(); n++) {
			delete otherObj[n];
	}
	//clean the vector to size of 0
	otherObj.clear();
	avaiSpot.clear();
	dirtFree.clear();
}

//updating dispaly text
void StudentWorld::setDisplayText() {
	int score = getScore();
	string sc = to_string(score);
	sc.insert(sc.begin(), 6 - sc.length(), '0');

	int level = getLevel();
	string lv = to_string(level);
	lv.insert(lv.begin(), 2 - lv.length(), ' ');

	int lives = getLives();

	int health = s_frac->getHealth();
	string he = to_string(health);
	he.insert(he.begin(), 3 - he.length(), ' ');

	int squirts = s_frac->getSquirt();
	string w = to_string(squirts);
	w.insert(w.begin(), 2 - w.length(), ' ');

	int gold = s_frac->getGold();
	string g = to_string(gold);
	g.insert(g.begin(), 2 - g.length(), ' ');

	int sonar = s_frac->getSonar();
	string so = to_string(sonar);
	so.insert(so.begin(), 2 - so.length(), ' ');

	int barrelsLeft = numOil;
	string b = to_string(barrelsLeft);
	b.insert(b.begin(), 2 - b.length(), ' ');
	// Next, create a string from your statistics, of the form:
	// “Scr: 0321000 Lvl: 52 Lives: 3 Hlth: 80% Water: 20 Gld: 3 Sonar: 1 Oil Left: 2”
	string s = "Scr: " + sc + "  Lvl: " + lv + "  Lives: " + to_string(lives) + "  Hlth: " + he + "%" + "  Wtr: " + w + "  Gld: " + g + "  Sonar: " + so + "  Oil Left: " + b;
	// Finally, update the display text at the top of the screen with your
	// newly created stats
	setGameStatText(s); // calls our provided GameWorld::setGameStatText
}

int StudentWorld::getTick() const {
	return s_tick;
}

void StudentWorld::removeDirt(int d_X, int d_Y) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (s_dirt[d_X + i][d_Y + j] != nullptr) {
				delete s_dirt[d_X + i][d_Y + j];    //remove dirt object
				s_dirt[d_X + i][d_Y + j] = nullptr;  //set the pointer to be nullptr
			}
		}
	}	
}

//return the distance in radius from frackman
double StudentWorld::fracRadius(int d_X, int d_Y) const {
	return sqrt((s_frac->getX() - d_X)*(s_frac->getX() - d_X) + (s_frac->getY() - d_Y)*(s_frac->getY() - d_Y));
}

bool StudentWorld::protBribe(int d_X, int d_Y) const {
	for (unsigned int i = 0; i < otherObj.size(); i++) {
		//if ID is protester
		if (otherObj[i]->getID() == IID_PROTESTER || otherObj[i]->getID() == IID_HARD_CORE_PROTESTER) {
			//if radius distance is less than 3
			if (sqrt((otherObj[i]->getX() - d_X)*(otherObj[i]->getX() - d_X) + (otherObj[i]->getY() - d_Y)*(otherObj[i]->getY() - d_Y)) <= 3) {
				otherObj[i]->bribe();  //call the bribe function of that protester
				return true;
			}
		}
	}
	return false;
}

bool StudentWorld::protHit(int d_X, int d_Y, int annoy) const{
	bool hit = false;
	for (unsigned int i = 0; i < otherObj.size(); i++) {
		//if radius distance is less than 3 from the protester
		if (otherObj[i]->getID() == IID_PROTESTER || otherObj[i]->getID() == IID_HARD_CORE_PROTESTER) {
			if (sqrt((otherObj[i]->getX() - d_X)*(otherObj[i]->getX() - d_X) + (otherObj[i]->getY() - d_Y)*(otherObj[i]->getY() - d_Y)) <= 3) {
				otherObj[i]->annoyed(annoy);  //annoy the protestor
				hit = true;
			}
		}
	}
	return hit;
}

bool StudentWorld::checkDirt(int d_X, int d_Y) const {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (s_dirt[d_X + i][d_Y + j] != nullptr) { //check 4x4 squres at (d_X, d_Y)
				return true;
			}
		}
	}
	return false;
}

bool StudentWorld::checkBoulder(int d_X, int d_Y) const{  // if the 4x4 squares at (d_X, d_Y) less or equal than a radius of 3 away from the center of the boulder
	for (unsigned int n = 0; n < otherObj.size(); n++) { 
		if (otherObj[n]->getID() == IID_BOULDER) { //if object is boulder and not the same boulder try to fall
			//check distance from the center from fracman to the center of the boulder
			if (sqrt((d_X+2 - (otherObj[n]->getX() + 2))*(d_X+2 - (otherObj[n]->getX() + 2)) + (d_Y+2 - (otherObj[n]->getY() + 2))*(d_Y+2 - (otherObj[n]->getY() + 2))) <= 3)
				return true;
		}
	}
	return false;
}

bool StudentWorld::checkDirtBoulder(int d_X, int d_Y) const {
	//check dirt
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (s_dirt[d_X + i][d_Y + j] != nullptr) {
				return true;
			}
		}
	}
	//check boulder
	for (unsigned int n = 0; n < otherObj.size(); n++) {
		//check if the 4x4 squares has any overlap with the boulder
		if (otherObj[n]->getID() == IID_BOULDER && !(d_X == otherObj[n]->getX() && d_Y + 1 == otherObj[n]->getY())) { //if object is boulder and not the same boulder try to fall
			if (d_X > otherObj[n]->getX() - 4 && d_X < otherObj[n]->getX() + 4 && d_Y > otherObj[n]->getY() - 4 && d_Y < otherObj[n]->getY() + 4) {
				return true;
			}
		}
	}
	return false;
}

bool StudentWorld::faceFrac(int d_X, int d_Y, GraphObject::Direction dir) const {
	switch (dir) //based on the direction
	{
	case GraphObject::up: //if up
		if (d_Y < s_frac->getY() && d_X >= s_frac->getX() - 4 && d_X <= s_frac->getX() + 4)
			return true;
		break;
	case GraphObject::down: //if down
		if (d_Y > s_frac->getY() && d_X >= s_frac->getX() - 4 && d_X <= s_frac->getX() + 4)
			return true;
		break;
	case GraphObject::left:  //if left
		if (d_X > s_frac->getX() && d_Y >= s_frac->getY() - 4 && d_Y <= s_frac->getY() + 4)
			return true;
		break;
	case GraphObject::right://if right
		if (d_X < s_frac->getX() && d_Y >= s_frac->getY() - 4 && d_Y <= s_frac->getY() + 4)
			return true;
		break;
	}
	return false;
}

bool StudentWorld::alignFrac(int d_X, int d_Y, GraphObject::Direction &dir) const{
	if (d_X == s_frac->getX() && fracRadius(d_X, d_Y) > 4) { //if both are on same X
		//if protester is up
		if (d_Y > s_frac->getY()) {
			for (int i = s_frac->getY()+4; i < d_Y; i++) {
				if (checkDirtBoulder(d_X, i)){  //check is there is any boulder or dirt on the way
					return false;
				}
			}
			dir = GraphObject::Direction::down; //set direction
		}
		else {
			for (int i = d_Y +4; i < s_frac->getY(); i++) {
				if (checkDirtBoulder(d_X, i)) {//check is there is any boulder or dirt on the way
					return false;
				}
			}
			dir = GraphObject::Direction::up;
		}
		return true;
	}
	//if both have the same Y
	if (d_Y == s_frac->getY() && fracRadius(d_X, d_Y) >= 4) {
		if (d_X > s_frac->getX()) {
			for (int i = s_frac->getX()+4; i < d_X; i++) {
				if (checkDirtBoulder(i, d_Y))//check is there is any boulder or dirt on the way
					return false;
			}
			dir = GraphObject::Direction::left;
		}
		else {
			for (int i = d_X+4; i < s_frac->getX(); i++) {
				if (checkDirtBoulder(i, d_Y))//check is there is any boulder or dirt on the way
					return false;
			}
			dir = GraphObject::Direction::right;
		}
		return true;
	}
	return false;
}

bool StudentWorld::ableToMove(GraphObject::Direction dir, int X, int Y) const {
	switch (dir) //check direction
	{
	case GraphObject::up:
	{
		if (Y + 5 > VIEW_HEIGHT || checkDirtBoulder(X, Y + 1))  //if not out of boundary and not overlapping with any dirt or boulder
			return false;  
		else
			return true;
	}
	break;
	case GraphObject::down:
	{
		if (Y - 1 < 0 || checkDirtBoulder(X, Y - 1))//if not out of boundary and not overlapping with any dirt or boulder
			return false;
		else
			return true;
	}
	break;
	case GraphObject::left:
	{
		if (X - 1 < 0 || checkDirtBoulder(X - 1, Y))//if not out of boundary and not overlapping with any dirt or boulder
			return false;
		else
			return true;
	}
	break;
	case GraphObject::right:
	{
		if (X + 5 > VIEW_WIDTH || checkDirtBoulder(X + 1, Y))//if not out of boundary and not overlapping with any dirt or boulder
			return false;
		else
			return true;
	}
	break;
	default:
		break;
	}
	return false;
}

void StudentWorld::pushOther(Actor* a) {
	otherObj.push_back(a);
}

bool StudentWorld::tooClose(int x1, int y1, int x2, int y2, int distance) const {
	if (((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2)) <= distance * distance) {
		return true;
	}
	return false;
}

void StudentWorld::breadthSearch(bool frac) {
	if (!frac) {  //if calculate distance matrix for root at (60, 60)
		for (int i = 0; i < VIEW_WIDTH; i++) {  //initailize the array to be -1
			for (int j = 0; j < VIEW_HEIGHT; j++) {
				distance[i][j] = -1;
			}
		}
		queue<Coord> q;
		Coord c(60, 60, 0);  //right upper corner as destination
		distance[60][60] = 0;
		q.push(c);
		while (!q.empty()) {  //while the queue is not empty
			Coord current = q.front();
			q.pop();
			//for all the neighbors, if can move and has not been visited before, push into queue and distance array
			if (current.c_Y + 1 <= 60 && !checkDirtBoulder(current.c_X, current.c_Y + 1) && distance[current.c_X][current.c_Y + 1] == -1) {
				Coord up(current.c_X, current.c_Y + 1, current.distance + 1);
				distance[current.c_X][current.c_Y + 1] = current.distance + 1;
				q.push(up);
			}
			if (current.c_Y - 1 >= 0 && !checkDirtBoulder(current.c_X, current.c_Y - 1) && distance[current.c_X][current.c_Y - 1] == -1) {
				Coord down(current.c_X, current.c_Y - 1, current.distance + 1);
				distance[current.c_X][current.c_Y - 1] = current.distance + 1;
				q.push(down);
			}
			if (current.c_X + 1 <= 60 && !checkDirtBoulder(current.c_X + 1, current.c_Y) && distance[current.c_X + 1][current.c_Y] == -1) {
				Coord right(current.c_X + 1, current.c_Y, current.distance + 1);
				distance[current.c_X + 1][current.c_Y] = current.distance + 1;
				q.push(right);
			}
			if (current.c_X - 1 >= 0 && !checkDirtBoulder(current.c_X - 1, current.c_Y) && distance[current.c_X - 1][current.c_Y] == -1) {
				Coord left(current.c_X - 1, current.c_Y, current.distance + 1);
				distance[current.c_X - 1][current.c_Y] = current.distance + 1;
				q.push(left);
			}
		}
	}
	else {   //if calculate step matrix for root at Fracman, except set fracman to be destination, others are same
		for (int i = 0; i < VIEW_WIDTH; i++) {
			for (int j = 0; j < VIEW_HEIGHT; j++) {
				step[i][j] = -1;
			}
		}
		queue<Coord> q;
		Coord c(s_frac->getX(), s_frac->getY(), 0);
		step[s_frac->getX()][s_frac->getY()] = 0;
		q.push(c);
		while (!q.empty()) {
			Coord current = q.front();
			q.pop();
			if (current.c_Y + 1 <= 60 && !checkDirtBoulder(current.c_X, current.c_Y + 1) && step[current.c_X][current.c_Y + 1] == -1) {
				Coord up(current.c_X, current.c_Y + 1, current.distance + 1);
				step[current.c_X][current.c_Y + 1] = current.distance + 1;
				q.push(up);
			}
			if (current.c_Y - 1 >= 0 && !checkDirtBoulder(current.c_X, current.c_Y - 1) && step[current.c_X][current.c_Y - 1] == -1) {
				Coord down(current.c_X, current.c_Y - 1, current.distance + 1);
				step[current.c_X][current.c_Y - 1] = current.distance + 1;
				q.push(down);
			}
			if (current.c_X + 1 <= 60 && !checkDirtBoulder(current.c_X + 1, current.c_Y) && step[current.c_X + 1][current.c_Y] == -1) {
				Coord right(current.c_X + 1, current.c_Y, current.distance + 1);
				step[current.c_X + 1][current.c_Y] = current.distance + 1;
				q.push(right);
			}
			if (current.c_X - 1 >= 0 && !checkDirtBoulder(current.c_X - 1, current.c_Y) && step[current.c_X - 1][current.c_Y] == -1) {
				Coord left(current.c_X - 1, current.c_Y, current.distance + 1);
				step[current.c_X - 1][current.c_Y] = current.distance + 1;
				q.push(left);
			}
		}
	}

}
//based on the array map, find the optimal direction to move
GraphObject::Direction StudentWorld::optimalDirection(int d_X, int d_Y, bool frac) {
	if (!frac) {
		//based on the array map, find the optimal direction to move
		GraphObject::Direction d = GraphObject::Direction::none;
		//find the smaller distance value
		if (distance[d_X][d_Y + 1] != -1 && distance[d_X][d_Y + 1] < distance[d_X][d_Y]) {
			d = GraphObject::Direction::up;
		}
		else if (distance[d_X][d_Y - 1] != -1 && distance[d_X][d_Y - 1] < distance[d_X][d_Y]) {
			d = GraphObject::Direction::down;
		}
		else if (distance[d_X - 1][d_Y] != -1 && distance[d_X - 1][d_Y] < distance[d_X][d_Y]) {
			d = GraphObject::Direction::left;
		}
		else if (distance[d_X + 1][d_Y] != -1 && distance[d_X + 1][d_Y] < distance[d_X][d_Y]) {
			d = GraphObject::Direction::right;
		}
		return d;
	}
	else {
		GraphObject::Direction d = GraphObject::Direction::none;
		if (step[d_X][d_Y + 1] != -1 && step[d_X][d_Y + 1] < step[d_X][d_Y]) {
			d = GraphObject::Direction::up;
		}
		else if (step[d_X][d_Y - 1] != -1 && step[d_X][d_Y - 1] < step[d_X][d_Y]) {
			d = GraphObject::Direction::down;
		}
		else if (step[d_X - 1][d_Y] != -1 && step[d_X - 1][d_Y] < step[d_X][d_Y]) {
			d = GraphObject::Direction::left;
		}
		else if (step[d_X + 1][d_Y] != -1 && step[d_X + 1][d_Y] < step[d_X][d_Y]) {
			d = GraphObject::Direction::right;
		}
		return d;
	}

}

void StudentWorld::pushDirtFree(int x, int y) {
	Coord c(x, y);
	dirtFree.push_back(c);
}

void StudentWorld::makeVisible(int d_X, int d_Y, int radius) const {
	for (unsigned int n = 0; n < otherObj.size(); n++) {
		if (otherObj[n]->getVisible() == false){
			int X = otherObj[n]->getX();
			int Y = otherObj[n]->getY();
			if (((X - d_X)*(X - d_X) + (Y - d_Y)*(Y - d_Y)) < radius * radius)
				otherObj[n]->changeVisible(true);
		}
	}
}

void StudentWorld::changeOil(int o) {
	numOil = numOil + o;
}

void StudentWorld::changeFracGold(int g) const {
	s_frac->changeGold(g);
}

void StudentWorld::annoyFrac(int p) const {
	s_frac->annoyed(p);
}

void StudentWorld::changeSonar(int num) const {
	s_frac->changeSonar(num);
}

void StudentWorld::changeWater(int num) const {
	s_frac->changeWater(num);
}

bool StudentWorld::turn(int d_X, int d_Y, GraphObject::Direction current_dir, GraphObject::Direction &target_dir) {
	//if current direction is left or right
	if (current_dir == GraphObject::Direction::left || current_dir == GraphObject::Direction::right) {
		if (!checkDirtBoulder(d_X, d_Y + 1) && (d_Y+1)<=60) { //check if can move up
			target_dir = GraphObject::Direction::up;
		}
		
		if (!checkDirtBoulder(d_X, d_Y - 1) && (d_Y - 1) >= 0) { //check if can move down
			if (target_dir == GraphObject::Direction::none) {
				target_dir = GraphObject::Direction::down;
			}
			else {  //if can go both directions
				int random = randInt(1, 2); //randomly select one of the direction
				if(random == 1)
					target_dir = GraphObject::Direction::up;
				else
					target_dir = GraphObject::Direction::down;
			}
		}
	}
	//if current direction is up or down
	if (current_dir == GraphObject::Direction::up || current_dir == GraphObject::Direction::down) {
		if (!checkDirtBoulder(d_X-1, d_Y) && (d_X - 1) >= 0) { //chekc if can move left
			target_dir = GraphObject::Direction::left;
		}
		if (!checkDirtBoulder(d_X+1, d_Y) && (d_X + 1) <= 60) {
			if (target_dir == GraphObject::Direction::none)
				target_dir = GraphObject::Direction::right;  //check if can move right
			else {   //if both random pick
				int random = randInt(1, 2);
				if (random == 1)
					target_dir = GraphObject::Direction::left;
				else
					target_dir = GraphObject::Direction::right;
			}
		}
	}

	if (target_dir == GraphObject::Direction::none)
		return false;
	else
		return true;
}

int StudentWorld::moveToFrac(int d_X, int d_Y) const {
	return step[d_X][d_Y];
}

int StudentWorld::randInt(int min, int max) const
{
	if (max < min)
		swap(max, min);
	static random_device rd;
	static mt19937 generator(rd());
	uniform_int_distribution<> distro(min, max);
	return distro(generator);
}
