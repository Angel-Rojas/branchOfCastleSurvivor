//
// Modified by: Angel Rojas
//		Christy G
//		Nygel Aton
//		Abdullah Aljahdali
//
//
// filename: main.cpp
// date: Spring 2018
// purpose: 'Castle Survivor' Group Project
//
// X11 OpenGL initial
//
//
//program: asteroids.cpp
//author:  Gordon Griesel
//date:    2014 - 2018
//mod spring 2015: added constructors
//mod spring 2018: X11 wrapper class
//This program is a game starting point for a 3350 project.
//
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
using namespace std;
#include <unistd.h>
#include <X11/Xlib.h>
//#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <X11/keysym.h>
#include <GL/glx.h>
// #include "log.h"
#include "fonts.h"

// These lines were 'merged' from Abdullah's original main.cpp file
#include "Common.h"
#include "angelR.h"
#include "nygelA.h"
//#include "abdullahA.h"
#include "christy.h"


//defined types
typedef float Flt;
typedef float Vec[3];
typedef Flt	Matrix[4][4];

//macros
#define rnd() (((Flt)rand())/(Flt)RAND_MAX)
#define random(a) (rand()%(a))
#define VecZero(v) (v)[0]=0.0,(v)[1]=0.0,(v)[2]=0.0
#define MakeVector(x, y, z, v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecCopy(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2]
#define VecDot(a,b)	((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecSub(a,b,c) (c)[0]=(a)[0]-(b)[0]; \
						(c)[1]=(a)[1]-(b)[1]; \
						(c)[2]=(a)[2]-(b)[2]


//constants
//const float TIMESLICE = 1.0f;
//const float GRAVITY = -0.2f;
#define PI 3.141592653589793
#define ALPHA 1
#define red 0x00ff0000
#define PROFILING_ON

//commonly used 'magic numbers'
const float RIGHT_ANGLE = 90.0;
const float WHOLE_ANGLE = 360.0f;
const float MINIMUM_TIME = 0.1;
const int ZERO = 0;
const int MAX_BULLETS = 11;
const Flt MINIMUM_ASTEROID_SIZE = 60.0;

//-------------------------------------------------------------------------
// EXTERNAL variables, and Setup timers
//
//const double OOBILLION = 1.0 / 1e9;
extern struct timespec timeStart, timeCurrent;
extern double timeDiff(struct timespec *start, struct timespec *end);
extern void timeCopy(struct timespec *dest, struct timespec *source);
extern int zombie_pos;
extern int zombie_kills;
extern int next_level;
extern int wave_count;
extern string POW;
extern int YOFFSET; 
extern int MOREYOFFSET; 
extern int XOFFSET;
extern string FULLH;
extern string THREE4sH;
extern string HALFH;
extern string QUARTERH;
extern string EMPTYH;
extern int HEALTHOFFSET;
extern int HEALTHPOS;
extern int HALVED;
extern bool Next;
extern int Game_mode;
extern int State;

//-------------------------------------------------------------------------

class Global {
public:
	int counter;
	int xres, yres;
	long double playTime;
	char keys[65536];
	Global() {
		xres = 1250;
		yres = 900;
		playTime = 0.0;
		memset(keys, ZERO, 65536);
	}
} gl;

class Ship {
public:
	Vec dir;
	Vec pos;
	Vec vel;
	float angle;
	float color[3];
public:
	Ship() {
		VecZero(dir);
		pos[ZERO] = (Flt)(0);
		pos[1] = (Flt)(gl.yres/2);
		pos[2] = 0.0f;
		VecZero(vel);
		angle = 0.0;
		color[ZERO] = color[1] = color[2] = 1.0;
	}
};

class Bullet {
public:
	Vec pos;
	Vec vel;
	float color[3];
	struct timespec time;
public:
	Bullet() { }
};

class Asteroid {
public:
	Vec pos;
	Vec vel;
	int nverts;
	Flt radius;
	Vec vert[8];
	float angle;
	float rotate;
	float color[3];
	// Fixed Warning: Asteroid was already declared
	// struct Asteroid *prev;
	Asteroid *prev;
	Asteroid *next;
public:
	Asteroid() {
		prev = NULL;
		next = NULL;
	}
};

class Game {
public:
	Ship ship;
	Asteroid *ahead;
	Bullet *barr;
	int nasteroids;
	int nastdestroyed;
	int nbullets;
	struct timespec bulletTimer;
	struct timespec mouseThrustTimer;
	bool mouseThrustOn;
public:
	Game() {
		ahead = NULL;
		barr = new Bullet[MAX_BULLETS];
		nasteroids = ZERO;
		nbullets = ZERO;
		nastdestroyed = ZERO;
		mouseThrustOn = false;
		//build 15 asteroids...
		for (int j=0; j<15; j++) {
			Asteroid *a = new Asteroid;
			a->nverts = 8;
			a->radius = rnd()*80.0 + 40.0;
			Flt r2 = a->radius / 2.0;
			Flt angle = 0.0f;
			Flt inc = (PI * 2.0) / (Flt)a->nverts;
			for (int i=0; i<a->nverts; i++) {
				a->vert[i][0] = sin(angle) * (r2 + rnd() * a->radius);
				a->vert[i][1] = cos(angle) * (r2 + rnd() * a->radius);
				angle += inc;
			}
			initZombiePosition(gl.xres,zombie_pos);
			a->pos[ZERO] = zombie_pos;
			a->pos[1] = (Flt)(rand() % gl.yres);
			a->pos[2] = 0.0f;
			a->angle = 0.0;
			a->rotate = rnd() * 4.0 - 2.0;
			a->color[ZERO] = rnd() + 0.4;
			a->color[1] = rnd() + 0.3;
			a->color[2] = rnd() + 0.2;
			a->vel[0] = -1;//(Flt)(rnd()*2.0-1.0);
			a->vel[1] = (Flt)(rnd()*2.0-1.0);
			//std::cout << "asteroid" << std::endl;
			//add to front of linked list
			a->next = ahead;
			if (ahead != NULL)
				ahead->prev = a;
			ahead = a;
			++nasteroids;
		}
		clock_gettime(CLOCK_REALTIME, &bulletTimer);
	}
	~Game() {
		delete [] barr;
	}
} g;

//X Windows variables
class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	X11_wrapper() {
		GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
		//GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
		XSetWindowAttributes swa;
		setup_screen_res(gl.xres, gl.yres);
		dpy = XOpenDisplay(NULL);
		if (dpy == NULL) {
			std::cout << "\n\tcannot connect to X server" << std::endl;
			exit(EXIT_FAILURE);
		}
		Window root = DefaultRootWindow(dpy);
		XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
		if (vi == NULL) {
			std::cout << "\n\tno appropriate visual found\n" << std::endl;
			exit(EXIT_FAILURE);
		}
		Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		swa.colormap = cmap;
		swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
			PointerMotionMask | MotionNotify | ButtonPress | ButtonRelease |
			StructureNotifyMask | SubstructureNotifyMask;
		win = XCreateWindow(dpy, root, 0, 0, gl.xres, gl.yres, 0,
				vi->depth, InputOutput, vi->visual,
				CWColormap | CWEventMask, &swa);
		set_title();
		glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
		glXMakeCurrent(dpy, win, glc);
		// show_mouse_cursor(0);
	}
	~X11_wrapper() {
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
	}
	void set_title() {
		//Set the window title bar.
		XMapWindow(dpy, win);
		XStoreName(dpy, win, "Castle Survivor");
	}
	void check_resize(XEvent *e) {
		//The ConfigureNotify is sent by the
		//server if the window is resized.
		if (e->type != ConfigureNotify)
			return;
		XConfigureEvent xce = e->xconfigure;
		if (xce.width != gl.xres || xce.height != gl.yres) {
			//Window size did change.
			reshape_window(xce.width, xce.height);
		}
	}
	void reshape_window(int width, int height) {
		//window has been resized.
		setup_screen_res(width, height);
		glViewport(0, 0, (GLint)width, (GLint)height);
		glMatrixMode(GL_PROJECTION); glLoadIdentity();
		glMatrixMode(GL_MODELVIEW); glLoadIdentity();
		glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
		set_title();
	}
	void setup_screen_res(const int w, const int h) {
		gl.xres = w;
		gl.yres = h;
	}
	void swapBuffers() {
		glXSwapBuffers(dpy, win);
	}
	bool getXPending() {
		return XPending(dpy);
	}
	XEvent getXNextEvent() {
		XEvent e;
		XNextEvent(dpy, &e);
		return e;
	}
	void set_mouse_position(int x, int y) {
		XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
	}
	void show_mouse_cursor(const int onoff) {
		if (onoff) {
			//this removes our own blank cursor.
			XUndefineCursor(dpy, win);
			return;
		}
		//vars to make blank cursor
		Pixmap blank;
		XColor dummy;
		char data[1] = {0};
		Cursor cursor;
		//make a blank cursor
		blank = XCreateBitmapFromData (dpy, win, data, 1, 1);
		if (blank == None)
			std::cout << "error: out of memory." << std::endl;
		cursor = XCreatePixmapCursor(dpy, blank, blank, &dummy, &dummy, 0, 0);
		XFreePixmap(dpy, blank);
		//this makes you the cursor. then set it using this function
		XDefineCursor(dpy, win, cursor);
		//after you do not need the cursor anymore use this function.
		//it will undo the last change done by XDefineCursor
		//(thus do only use ONCE XDefineCursor and then XUndefineCursor):
	}
} x11;

//function prototypes
void init_opengl();
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();
void resetKillCount(int&);
void incrementWave();
void resetWave();
void displayWave(int,int);
bool changeBoolean(bool&);
void displayHealth(int,int,int);
void playerState(int,int,int);
extern void timerN(double);
extern bool waveCountDown(int,int);
double timer();
void powText();
void printWelcome();
float initZombiePosition(int,int&);
double angelsTimer(int,int);
void displayMenu(int,int);
void pauseGame(int,int);
void endGameScreen();
void gameOver(int,int);

//=========================================================================
// M A I N
//=========================================================================
int main()
{
	// logOpen();
	init_opengl();
	srand(time(NULL));
	x11.set_mouse_position(100, 100);
	int done=0;
	while (!done) {
	    	while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			x11.check_resize(&e);
			check_mouse(&e);
			done = check_keys(&e);
		}
		physics();
		// Check to see if end of game (max zombies killed)
		if (gl.counter == 5) {
			changeBoolean(Next);
			//cout << "BEFORE RESET next:" << Next << endl;
			gl.counter = 0;
		} if (zombie_kills == 5 && Next == 1) {
			endGameScreen();
			//cout << " AFTER RESET next:" << Next << endl;
			//cout << " AFTER RESET zombies:" << zombie_kills << endl;
		} else {
			render();
		}
		x11.swapBuffers();
	}
	// cleanup_fonts();
	// logClose();
	return 0;
}

void init_opengl()
{
	//OpenGL initialization
	glViewport(0, 0, gl.xres, gl.yres);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//This sets 2D mode (no perspective)
	glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
	//
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_CULL_FACE);
	//
	//Clear the screen to black
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void normalize2d(Vec v)
{
	Flt len = v[0]*v[0] + v[1]*v[1];
	if (len == 0.0f) {
		v[0] = 1.0;
		v[1] = 0.0;
		return;
	}
	len = 1.0f / sqrt(len);
	v[0] *= len;
	v[1] *= len;
}

void check_mouse(XEvent *e)
{
	//Was a mouse button clicked?
	static int savex = 0;
	static int savey = 0;
	//static int ct=0;
	if (e->type != ButtonPress &&
			e->type != ButtonRelease &&
			e->type != MotionNotify)
		return;
	if (e->type == ButtonRelease)
		return;
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button is down
			//a little time between each bullet
			powText();
			struct timespec bt;
			clock_gettime(CLOCK_REALTIME, &bt);
			double ts = timeDiff(&g.bulletTimer, &bt);
			if (ts > MINIMUM_TIME) {
				timeCopy(&g.bulletTimer, &bt);
				//shoot a bullet...
				if (g.nbullets < MAX_BULLETS) {
					Bullet *b = &g.barr[g.nbullets];
					timeCopy(&b->time, &bt);
					b->pos[0] = g.ship.pos[0];
					b->pos[1] = g.ship.pos[1];
					b->vel[0] = g.ship.vel[0];
					b->vel[1] = g.ship.vel[1];
					//convert ship angle to radians
					Flt rad = ((g.ship.angle+RIGHT_ANGLE) / WHOLE_ANGLE) * PI * 2.0;
					//convert angle to a vector
					Flt xdir = cos(rad);
					Flt ydir = sin(rad);
					b->pos[0] += xdir*20.0f;
					b->pos[1] += ydir*20.0f;
					b->vel[0] += xdir*6.0f + rnd()*MINIMUM_TIME;
					b->vel[1] += ydir*6.0f + rnd()*MINIMUM_TIME;
					b->color[0] = 1.0f;
					b->color[1] = 1.0f;
					b->color[2] = 1.0f;
					++g.nbullets;
				}
			}
		}
		struct timespec bt;
		clock_gettime(CLOCK_REALTIME, &bt);
		double ts = timeDiff(&g.bulletTimer, &bt);
		if (e->xbutton.button==3) {
			//Right button is down
			if (ts > MINIMUM_TIME) {
				timeCopy(&g.bulletTimer, &bt);
				//shoot a bullet...
				if (g.nbullets < MAX_BULLETS) {
					Bullet *b = &g.barr[g.nbullets];
					timeCopy(&b->time, &bt);
					b->pos[0] = g.ship.pos[0];
					b->pos[1] = g.ship.pos[1];
					b->vel[0] = g.ship.vel[0];
					b->vel[1] = g.ship.vel[1];
					//convert ship angle to radians
					Flt rad = ((g.ship.angle+RIGHT_ANGLE) / WHOLE_ANGLE) * PI * 2.0;
					//convert angle to a vector
					Flt xdir = cos(rad);
					Flt ydir = sin(rad);
					b->pos[0] += xdir*20.0f;
					b->pos[1] += ydir*20.0f;
					b->vel[0] += xdir*6.0f + rnd()*MINIMUM_TIME;
					b->vel[1] += ydir*6.0f + rnd()*MINIMUM_TIME;
					b->color[0] = 1.0f;
					b->color[1] = 1.0f;
					b->color[2] = 1.0f;
					++g.nbullets;
				}
			}
		}
	} // end of ButtonPress
	// if (e->type == MotionNotify) {
	//	if (savex != e->xbutton.x || savey != e->xbutton.y) {
	//		//Mouse moved
	//		int xdiff = savex - e->xbutton.x;
	//		int ydiff = savey - e->xbutton.y;
	//		if (++ct < 10)
	//			return;
	//		if (xdiff > 0) {
	//			//mouse moved along the x-axis.
	//			g.ship.angle += 0.05f * (float)xdiff;
	//			if (g.ship.angle >= 360.0f)
	//				g.ship.angle -= 360.0f;
	//		}
	//		else if (xdiff < 0) {
	//			g.ship.angle += 0.05f * (float)xdiff;
	//			if (g.ship.angle < 0.0f)
	//				g.ship.angle += 360.0f;
	//		}
	//		if (ydiff > 0) {
	//			//mouse moved along the y-axis.
	//			//apply thrust
	//			//convert ship angle to radians
	//			Flt rad = ((g.ship.angle+RIGHT_ANGLE) / WHOLE_ANGLE) * PI * 2.0;
	//			//convert angle to a vector
	//			Flt xdir = cos(rad);
	//			Flt ydir = sin(rad);
	//			g.ship.vel[0] += xdir * (float)ydiff * 0.001f;
	//			g.ship.vel[1] += ydir * (float)ydiff * 0.001f;
	//			Flt speed = sqrt(g.ship.vel[0]*g.ship.vel[0]+
	//				g.ship.vel[1]*g.ship.vel[1])/100;
	//			if (speed > 15.0f) {
	//				speed = 15.0f;
	//				normalize2d(g.ship.vel);
	//				g.ship.vel[0] *= speed;
	//				g.ship.vel[1] *= speed;
	//			}
	//			g.mouseThrustOn = true;
	//			clock_gettime(CLOCK_REALTIME, &g.mouseThrustTimer);
	//		}
	//		// x11.set_mouse_position(100, 100);
	//		// savex = savey = 100;
	//	}
	// }

	if (e->type == MotionNotify) {
		if (savex != e->xbutton.x || savey != e->xbutton.y) {
			// //Mouse moved
			g.ship.angle = (e->xbutton.y%(gl.yres))*-1;
		}
	}
}

int check_keys(XEvent *e)
{
	//keyboard input?
	static int shift=0;
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);
	//Log("key: %i\n", key);
	if (e->type == KeyRelease) {
		gl.keys[key]=0;
		if (key == XK_Shift_L || key == XK_Shift_R)
			shift=0;
		return 0;
	}
	gl.keys[key]=1;
	if (key == XK_Shift_L || key == XK_Shift_R) {
		shift=1;
		return 0;
	}
	(void)shift;
	switch (key) {
		case XK_Escape:
			return 1;
		case XK_space:
			Game_mode = 2;
			break;
		case XK_p:
			Game_mode = 1;
			break;
		case XK_o:
			//cout << "BEFORE zombiekills(o)reset:" <<zombie_kills <<endl;
			endTheGame(zombie_kills,Next,Game_mode);
			/*cout <<"Game mode: " << Game_mode << " "
			<< "Zombie Kills: " << zombie_kills << " "
			<< "Next bool: " << Next << endl;*/
			break;
		case XK_i:
			// Angel testing something
			//statePlayerDead(gl.State);
			//gl.State = 1;
			//cout << "State(health) changed to 3/4s" << gl.State << endl;
			State = 4;
			Game_mode = 4;
			break;
		case XK_t:
			startOver(zombie_kills,State,Game_mode);
			break;
		case XK_u:
			break;
		case XK_m:
			// Angel testing something
			//Game_mode = MENU;
			break;
		case XK_Down:
			break;
		case XK_equal:
			break;
		case XK_minus:
			break;
	}
	return 0;
}

void deleteAsteroid(Game *g, Asteroid *node)
{
	//Remove a node from doubly-linked list.
	//Must look at 4 special cases below.
	if (node->prev == NULL) {
		if (node->next == NULL) {
			//only 1 item in list.
			g->ahead = NULL;
		} else {
			//at beginning of list.
			node->next->prev = NULL;
			g->ahead = node->next;
		}
	} else {
		if (node->next == NULL) {
			//at end of list.
			node->prev->next = NULL;
		} else {
			//in middle of list.
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}
	}
	delete node;
	node = NULL;
    hello();//nygel's test function
}

void buildAsteroidFragment(Asteroid *ta, Asteroid *a)
{/*
	//build ta from a
	ta->nverts = 8;
	ta->radius = a->radius / 2.0;
	Flt r2 = ta->radius / 2.0;
	Flt angle = 0.0f;
	Flt inc = (PI * 2.0) / (Flt)ta->nverts;
	for (int i=0; i<ta->nverts; i++) {
		ta->vert[i][0] = sin(angle) * (r2 + rnd() * ta->radius);
		ta->vert[i][1] = cos(angle) * (r2 + rnd() * ta->radius);
		angle += inc;
	}
	ta->pos[0] = a->pos[0] + rnd()*10.0-5.0;
	ta->pos[1] = a->pos[1] + rnd()*10.0-5.0;
	ta->pos[2] = 0.0f;
	ta->angle = 0.0;
	ta->rotate = a->rotate + (rnd() * 4.0 - 2.0);
	ta->color[0] = rnd();
	ta->color[1] = rnd();
	ta->color[2] = rnd();
	ta->vel[0] = a->vel[0] + (rnd()*2.0-1.0);
	ta->vel[1] = a->vel[1] + (rnd()*2.0-1.0);
	*/
}
void physics()
{
	Flt d0,d1,dist;
	//Update ship position
	//g.ship.pos[0] += g.ship.vel[0];
	//g.ship.pos[1] += g.ship.vel[1];
	//Check for collision with window edges
	// if (g.ship.pos[0] < 0.0) {
	//	g.ship.pos[0] += (float)gl.xres;
	// }
	// else if (g.ship.pos[0] > (float)gl.xres) {
	//	g.ship.pos[0] -= (float)gl.xres;
	// }
	// else if (g.ship.pos[1] < 0.0) {
	//	g.ship.pos[1] += (float)gl.yres;
	// }
	// else if (g.ship.pos[1] > (float)gl.yres) {
	//	g.ship.pos[1] -= (float)gl.yres;
	// }
	//
	//Update bullet positions
	struct timespec bt;
	clock_gettime(CLOCK_REALTIME, &bt);
	int i=0;
	while (i < g.nbullets) {
		Bullet *b = &g.barr[i];
		//How long has bullet been alive?
	        double ts = timeDiff(&b->time, &bt);
		if (ts > 2.5) {
			//time to delete the bullet.
			memcpy(&g.barr[i], &g.barr[g.nbullets-1],
				sizeof(Bullet));
			g.nbullets--;
			//do not increment i.
			continue;
		}
		//move the bullet
		b->pos[0] += b->vel[0];
		b->pos[1] += b->vel[1];
		//Check for collision with window edges
		/*if (b->pos[0] < 0.0) {
			b->pos[0] += (float)gl.xres;
		}
		else if (b->pos[0] > (float)gl.xres) {
			b->pos[0] -= (float)gl.xres;
		}
		else if (b->pos[1] < 0.0) {
			b->pos[1] += (float)gl.yres;
		}
		else if (b->pos[1] > (float)gl.yres) {
			b->pos[1] -= (float)gl.yres;
		} */
		i++;
	}
	//
	//Update asteroid positions meaning Asteroid movement
	Asteroid *a = g.ahead;
	switch (Game_mode) {
		case 0:
			break;
		case 1:
			while (a) {
				/* THE FOLLOWING 2 LINES WILL MOVE OUR OBJECT */
				a->pos[0] += a->vel[0];
				//a->pos[1] += a->vel[1];
				//Check for collision with window edges
				if (a->pos[0] < -100.0) {
					a->pos[0] += (float)gl.xres+200;
				}
				else if (a->pos[0] > (float)gl.xres+100) {
					a->pos[0] -= (float)gl.xres+200;
				}
				else if (a->pos[1] < -100.0) {
					a->pos[1] += (float)gl.yres+200;
				}
				else if (a->pos[1] > (float)gl.yres+100) {
					a->pos[1] -= (float)gl.yres+200;
				}
				// The following line causes the object to rotate
				//a->angle += a->rotate;
				a = a->next;
			}//end of while
			break;
		case 2:
			//Game_mode set to PAUSED
			break;
		case 3:
			//Game_mode set to WIN
			break;
		case 4:
			//Game_mode set to GAMEOVER
			break;
	// end of Switch
	}
	//Asteroid collision with bullets?
	//If collision detected:
	//   1. delete the bullet
	//   2. break the asteroid into pieces
	//      if asteroid small, delete it
	a = g.ahead;
	while (a) {
		//is there a bullet within its radius?
		int i=0;
		while (i < g.nbullets) {
			Bullet *b = &g.barr[i];
			d0 = b->pos[0] - a->pos[0];
			d1 = b->pos[1] - a->pos[1];
			dist = (d0*d0 + d1*d1);
			if (dist < (a->radius*a->radius)) {
				//cout << "asteroid hit." << endl;
				//this asteroid is hit.
				// Increment asteroids destroyed
				g.nastdestroyed++;
				if (a->radius > MINIMUM_ASTEROID_SIZE) {
					/* THE FOLLOWING COMMENTED CODE BREAKS UP AN OBJECT INTO
					 * LITTLER OBJECTS
						//break it into pieces.
					Asteroid *ta = a;
					//buildAsteroidFragment(ta, a);
					int r = rand()%10+5;
					for (int k=0; k<r; k++) {
						//get the next asteroid position in the array
						Asteroid *ta = new Asteroid;
						//buildAsteroidFragment(ta, a);
						//add to front of asteroid linked list
						ta->next = g.ahead;
						if (g.ahead != NULL)
							g.ahead->prev = ta;
						g.ahead = ta;
						g.nasteroids++;
					}
				} else {*/
					a->color[0] = 1.0;
					a->color[1] = MINIMUM_TIME;
					a->color[2] = MINIMUM_TIME;
					//asteroid is too small to break up
					//delete the asteroid and bullet
					Asteroid *savea = a->next;
					deleteAsteroid(&g, a);
					a = savea;
					g.nasteroids--;
					// increment a destroyed asteroid
					g.nastdestroyed++;
					// Follow 2 lines are used as tracking numbers
					incrementZombiesKilled(zombie_kills);
					gl.counter++;
				}
				//delete the bullet...
				memcpy(&g.barr[i], &g.barr[g.nbullets-1], sizeof(Bullet));
				g.nbullets--;
				if (a == NULL)
					break;
			}
			i++;
		}
		if (a == NULL)
			break;
		a = a->next;
	}
	//---------------------------------------------------
	//check keys pressed now
	if (gl.keys[XK_Left]) {
		g.ship.angle += 4.0;
		if (g.ship.angle >= 360.0f)
			g.ship.angle -= 360.0f;
	}
	if (gl.keys[XK_Right]) {
		g.ship.angle -= 4.0;
		if (g.ship.angle < 0.0f)
			g.ship.angle += 360.0f;
	}
	if (gl.keys[XK_Up]) {
		//apply thrust
		//convert ship angle to radians
		Flt rad = ((g.ship.angle+RIGHT_ANGLE) / WHOLE_ANGLE) * PI * 2.0;
		//convert angle to a vector
		Flt xdir = cos(rad);
		Flt ydir = sin(rad);
		g.ship.vel[0] += xdir*0.02f;
		g.ship.vel[1] += ydir*0.02f;
		Flt speed = sqrt(g.ship.vel[0]*g.ship.vel[0]+
				g.ship.vel[1]*g.ship.vel[1]);
		if (speed > 10.0f) {
			speed = 10.0f;
			normalize2d(g.ship.vel);
			g.ship.vel[0] *= speed;
			g.ship.vel[1] *= speed;
		}
	}
	if (gl.keys[XK_space]) {
		//a little time between each bullet
		struct timespec bt;
		clock_gettime(CLOCK_REALTIME, &bt);
		double ts = timeDiff(&g.bulletTimer, &bt);
		if (ts > MINIMUM_TIME) {
			timeCopy(&g.bulletTimer, &bt);
			if (g.nbullets < MAX_BULLETS) {
				//shoot a bullet...
				//Bullet *b = new Bullet;
				Bullet *b = &g.barr[g.nbullets];
				timeCopy(&b->time, &bt);
				b->pos[0] = g.ship.pos[0];
				b->pos[1] = g.ship.pos[1];
				b->vel[0] = g.ship.vel[0];
				b->vel[1] = g.ship.vel[1];
				//convert ship angle to radians
				Flt rad = ((g.ship.angle+RIGHT_ANGLE) / WHOLE_ANGLE) * PI * 2.0;
				//convert angle to a vector
				Flt xdir = cos(rad);
				Flt ydir = sin(rad);
				b->pos[0] += xdir*20.0f;
				b->pos[1] += ydir*20.0f;
				b->vel[0] += xdir*6.0f + rnd()*0.1;
				b->vel[1] += ydir*6.0f + rnd()*0.1;
				b->color[0] = 1.0f;
				b->color[1] = 1.0f;
				b->color[2] = 1.0f;
				g.nbullets++;
			}
		}
	}
	if (g.mouseThrustOn) {
		//should thrust be turned off
		struct timespec mtt;
		clock_gettime(CLOCK_REALTIME, &mtt);
		double tdif = timeDiff(&mtt, &g.mouseThrustTimer);
		if (tdif < -0.3)
			g.mouseThrustOn = false;
	}
}
void render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	//-- DO NOT TRY TO PRINT TEXT ABOVE THIS LINE ---
	Bullet *b = &g.barr[0];
	for (int i=0; i<g.nbullets; i++) {
		//Log("draw bullet...\n");
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_POINTS);
			glVertex2f(b->pos[0],      b->pos[1]);
			glVertex2f(b->pos[0]-1.0f, b->pos[1]);
			glVertex2f(b->pos[0]+1.0f, b->pos[1]);
			glVertex2f(b->pos[0],      b->pos[1]-1.0f);
			glVertex2f(b->pos[0],      b->pos[1]+1.0f);
			glColor3f(0.8, 0.8, 0.8);
			glVertex2f(b->pos[0]-1.0f, b->pos[1]-1.0f);
			glVertex2f(b->pos[0]-1.0f, b->pos[1]+1.0f);
			glVertex2f(b->pos[0]+1.0f, b->pos[1]-1.0f);
			glVertex2f(b->pos[0]+1.0f, b->pos[1]+1.0f);
		glEnd();
		++b;
		}
	switch (Game_mode) {
		// The below case is the MAIN RENDER function
		case 1:
			// nygel timer
			//timerN(0);
			//------------christy header------
			void header(int , int, int, int);
			//
			header(gl.xres, gl.yres, gl.xres, gl.yres);
			//wave timer -- nygel?
			if (waveCountDown(gl.xres,gl.yres) == false)
			    Game_mode = 2;
			//-------------christy timer-----
			//
			#ifdef PROFILING_OFF //----turns of the timer are the print name
			timer();
			
			//----------- christy printname---
			void printName();
			//
			printName();
			//-------------------------------
			#endif

			playerState(State,gl.yres,gl.xres);
			Rect r;
			r.bot = gl.yres - 20;
			r.left = 10;
			r.center = 0;
			// Lets display our kills and Wave number.
			zombieKillCount(gl.yres,zombie_kills);
			displayWave(gl.yres,10);
			//
			ggprint8b(&r, 16, red, "Castle Survivor!");
			// //ggprint8b(&r, 16, 0x00ffff00, "n asteroids: %i", g.nasteroids);
			// //ggprint8b(&r, 16, 0x00ffff00, "n asteroids destroyed: %i", g.nastdestroyed);
			// //
			//-------------
			//Draw the ship
			glColor3fv(g.ship.color);
			glPushMatrix();
			glTranslatef(g.ship.pos[0], g.ship.pos[1], g.ship.pos[2]);
			glRotatef(g.ship.angle, 0.0f, 0.0f, 1.0f);
			glBegin(GL_TRIANGLES);
				glVertex2f(-12.0f, -10.0f);
				glVertex2f(  0.0f, 20.0f);
				glVertex2f(  0.0f, -6.0f);
				glVertex2f(  0.0f, -6.0f);
				glVertex2f(  0.0f, 20.0f);
				glVertex2f( 12.0f, -10.0f);
			glEnd();
			glColor3f(1.0f, 0.0f, 0.0f);
			glBegin(GL_POINTS);
				glVertex2f(0.0f, 0.0f);
			glEnd();
			glPopMatrix();
			if (gl.keys[XK_Up] || g.mouseThrustOn) {
				int i;
				//draw thrust
				Flt rad = ((g.ship.angle+RIGHT_ANGLE) / WHOLE_ANGLE) * PI * 2.0;
				//convert angle to a vector
				Flt xdir = cos(rad);
				Flt ydir = sin(rad);
				Flt xs,ys,xe,ye,r;
				glBegin(GL_LINES);
					for (i=0; i<16; i++) {
						xs = -xdir * 11.0f + rnd() * 4.0 - 2.0;
						ys = -ydir * 11.0f + rnd() * 4.0 - 2.0;
						r = rnd()*40.0+40.0;
						xe = -xdir * r + rnd() * 18.0 - 9.0;
						ye = -ydir * r + rnd() * 18.0 - 9.0;
						glColor3f(rnd()*.3+.7, rnd()*.3+.7, 0);
						glVertex2f(g.ship.pos[0]+xs,g.ship.pos[1]+ys);
						glVertex2f(g.ship.pos[0]+xe,g.ship.pos[1]+ye);
					}
				glEnd();
			}
			//------------------
			//Draw the asteroids
			{
				Asteroid *a = g.ahead;
				while (a) {
					//Log("draw asteroid...\n");
					glColor3fv(a->color);
					glPushMatrix();
					glTranslatef(a->pos[0], a->pos[1], a->pos[2]);
					glRotatef(a->angle, 0.0f, 0.0f, 1.0f);
					// change from LINE_LOOP to POLYGON to fill color
					glBegin(GL_POLYGON);
						//Log("%i verts\n",a->nverts);
						for (int j=0; j<a->nverts; j++) {
							glVertex2f(a->vert[j][0], a->vert[j][1]);
						}
					glEnd();
					glPopMatrix();
					glColor3f(1.0f, 0.0f, 0.0f);
					// below is where the Red Center Dot is made
					//glBegin(GL_POLYGON);
					//	glVertex2f(a->pos[0], a->pos[1]);
					glEnd();
					a = a->next;
				}
			}
			//----------------
			//Draw the bullets
			/*Bullet *b = &g.barr[0];
			for (int i=0; i<g.nbullets; i++) {
				//Log("draw bullet...\n");
				glColor3f(1.0, 1.0, 1.0);
				glBegin(GL_POINTS);
					glVertex2f(b->pos[0],      b->pos[1]);
					glVertex2f(b->pos[0]-1.0f, b->pos[1]);
					glVertex2f(b->pos[0]+1.0f, b->pos[1]);
					glVertex2f(b->pos[0],      b->pos[1]-1.0f);
					glVertex2f(b->pos[0],      b->pos[1]+1.0f);
					glColor3f(0.8, 0.8, 0.8);
					glVertex2f(b->pos[0]-1.0f, b->pos[1]-1.0f);
					glVertex2f(b->pos[0]-1.0f, b->pos[1]+1.0f);
					glVertex2f(b->pos[0]+1.0f, b->pos[1]-1.0f);
					glVertex2f(b->pos[0]+1.0f, b->pos[1]+1.0f);
				glEnd();
				++b;
				}*/
			angelsTimer(gl.xres,gl.yres);
			break;
		case 0:
			/*Bullet *b = &g.barr[0];
			for (int i=0; i<g.nbullets; i++) {
				//Log("draw bullet...\n");
				glColor3f(1.0, 1.0, 1.0);
				glBegin(GL_POINTS);
					glVertex2f(b->pos[0],      b->pos[1]);
					glVertex2f(b->pos[0]-1.0f, b->pos[1]);
					glVertex2f(b->pos[0]+1.0f, b->pos[1]);
					glVertex2f(b->pos[0],      b->pos[1]-1.0f);
					glVertex2f(b->pos[0],      b->pos[1]+1.0f);
					glColor3f(0.8, 0.8, 0.8);
					glVertex2f(b->pos[0]-1.0f, b->pos[1]-1.0f);
					glVertex2f(b->pos[0]-1.0f, b->pos[1]+1.0f);
					glVertex2f(b->pos[0]+1.0f, b->pos[1]-1.0f);
					glVertex2f(b->pos[0]+1.0f, b->pos[1]+1.0f);
				glEnd();
				++b;
				} */
			displayMenu(gl.yres, gl.xres);
			break;
		case 2:
			pauseGame(gl.xres, gl.yres);
			break;
		case 3:
			endGameScreen();	
			break;
		case 4:
			gameOver(gl.xres, gl.yres);	
			break;
	}
	
}
