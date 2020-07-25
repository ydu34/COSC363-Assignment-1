#include <iostream>
#include <cmath> 
#include <GL/glut.h>
#include "loadTGA.h"
#include <GL/freeglut.h>

#include <fstream>
#include <climits>
#include <math.h>

using namespace std;

GLuint txId[4];   //Texture ids
float angle=0, look_x, look_z=-1., eye_x, eye_z;  //Camera parameters
float look_y = 0, eye_y = 0;

//Colours 
float white_3f[3] = {1, 1, 1};
float black_3f[3] = {0, 0, 0};

//newton's graddle
bool left_ball_flag = false;
float left_ball_theta = 0;
bool right_ball_flag = false;
float right_ball_theta = 0;
bool right_ball_start = false;
float ball_2_theta = 0;
float ball_3_theta = 0;
float ball_4_theta = 0;
bool balls_swing_right = false;
bool balls_swing_left = false;

//cannon 
float *x, *y, *z;       //vertex coordinate arrays
int *t1, *t2, *t3;      //triangles
int nvrt, ntri;         //total number of vertices and triangles
float cannon_angle = 45;
float cannon_length = 70;
float cannon_y_offset = 40;
float cannon_x_offset = -25;
float ball_x0 = cannon_length * cos(cannon_angle*M_PI/180) + cannon_x_offset;
float ball_y0 = cannon_length * sin(cannon_angle*M_PI/180) + cannon_y_offset;
float ball_z0 = 0;
float ball_x = ball_x0;
float ball_y = ball_y0;
float ball_z = ball_z0;
float t = 0;
float gravity = 9.81;
float velocity = 67;
bool cannon_firing = false;

//exibit three 
//earth 
float earth_theta = 0;

//moon
float moon_theta = 0;
float moon_orbit_theta = 0;

enum {EARTH=0, MOON};
GLuint solarSystem[2];

//hourglass
const int N = 16;  // Total number of vertices on the base curve

float vy_init[N] = { 0, 6, 10, 14, 16, 18, 19, 19.5, 20.5, 21, 22, 24, 26, 30, 34, 40 };
float vx_init[N] = { 10, 12, 12, 10, 8, 4, 2, 1, 1, 2, 4, 8, 10, 12, 12, 10};
float vz_init[N] = { 0 };

//skybox
GLuint skybox_texture[1];

//skybox
enum {LEFT=0,BACK,RIGHT,FRONT,TOP,BOT};      //constants for the skybox faces, so we don't have to remember so much number
GLuint skyboxtxId[6]; //the ids for the textures

//-- Loads mesh data in OFF format    -------------------------------------
void loadMeshFile(const char* fname)
{
    ifstream fp_in;
    int num, ne;

    fp_in.open(fname, ios::in);
    if(!fp_in.is_open())
    {
        cout << "Error opening mesh file" << endl;
        exit(1);
    }

    fp_in.ignore(INT_MAX, '\n');                //ignore first line
    fp_in >> nvrt >> ntri >> ne;                // read number of vertices, polygons, edges

    x = new float[nvrt];                        //create arrays
    y = new float[nvrt];
    z = new float[nvrt];

    t1 = new int[ntri];
    t2 = new int[ntri];
    t3 = new int[ntri];

    for(int i=0; i < nvrt; i++)                         //read vertex list
        fp_in >> x[i] >> y[i] >> z[i];

    for(int i=0; i < ntri; i++)                         //read polygon list
    {
        fp_in >> num >> t1[i] >> t2[i] >> t3[i];
        if(num != 3)
        {
            cout << "ERROR: Polygon with index " << i  << " is not a triangle." << endl;  //not a triangle!!
            exit(1);
        }
    }

    fp_in.close();
    cout << " File successfully read." << endl;
}

//--------------------------------------------------------------------------------
void loadTexture()				
{
    // Floor and Walls
	glGenTextures(4, txId); 

	glBindTexture(GL_TEXTURE_2D, txId[0]);  
    loadTGA("textures/insideWall.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	

	glBindTexture(GL_TEXTURE_2D, txId[1]);  
    loadTGA("textures/snowFloor.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    
    glBindTexture(GL_TEXTURE_2D, txId[2]);
    loadTGA("textures/outsideWall.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	
    glBindTexture(GL_TEXTURE_2D, txId[3]);
    loadTGA("textures/marbleFloor.tga");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);   
    
    //Skybox
    glGenTextures(6, skyboxtxId); 

	glBindTexture(GL_TEXTURE_2D, skyboxtxId[BACK]);  //Use this texture
    loadTGA("textures/back.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);      //we repeat the pixels in the edge of the texture, it will hide that 1px wide line at the edge of the cube, which you have seen in the video
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);      //we do it for vertically and horizontally (previous line)
    
    glBindTexture(GL_TEXTURE_2D, skyboxtxId[FRONT]);  //Use this texture
    loadTGA("textures/front.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);      //we repeat the pixels in the edge of the texture, it will hide that 1px wide line at the edge of the cube, which you have seen in the video
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);      //we do it for vertically and horizontally (previous line)
    
    glBindTexture(GL_TEXTURE_2D, skyboxtxId[LEFT]);  //Use this texture
    loadTGA("textures/left.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);      //we repeat the pixels in the edge of the texture, it will hide that 1px wide line at the edge of the cube, which you have seen in the video
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);      //we do it for vertically and horizontally (previous line)
    
    glBindTexture(GL_TEXTURE_2D, skyboxtxId[RIGHT]);  //Use this texture
    loadTGA("textures/right.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);      //we repeat the pixels in the edge of the texture, it will hide that 1px wide line at the edge of the cube, which you have seen in the video
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);      //we do it for vertically and horizontally (previous line)
    
    glBindTexture(GL_TEXTURE_2D, skyboxtxId[TOP]);  //Use this texture
    loadTGA("textures/top.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);      //we repeat the pixels in the edge of the texture, it will hide that 1px wide line at the edge of the cube, which you have seen in the video
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);      //we do it for vertically and horizontally (previous line)
    
    glBindTexture(GL_TEXTURE_2D, skyboxtxId[BOT]);  //Use this texture
    loadTGA("textures/bot.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);      //we repeat the pixels in the edge of the texture, it will hide that 1px wide line at the edge of the cube, which you have seen in the video
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);      //we do it for vertically and horizontally (previous line)
    
    
    //Earth and Moon
    glGenTextures(2, solarSystem);
    
    glBindTexture(GL_TEXTURE_2D, solarSystem[EARTH]);
    loadTGA("textures/earthmap1k.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    
    glBindTexture(GL_TEXTURE_2D, solarSystem[MOON]);
    loadTGA("textures/moonmap1k.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
    
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	
}

//--------------------------------------------------------------------------------
void special(int key, int x, int y)
{
	if(key == GLUT_KEY_LEFT) angle -= 1;  //Change direction
	else if(key == GLUT_KEY_RIGHT) angle += 1;
	else if(key == GLUT_KEY_DOWN)
	{  //Move backward
		eye_x -= 0.1*sin(angle*M_PI/180);
		eye_z += 0.1*cos(angle*M_PI/180);
	}
	else if(key == GLUT_KEY_UP)
	{ //Move forward
		eye_x += 0.1*sin(angle*M_PI/180);
		eye_z -= 0.1*cos(angle*M_PI/180);
	}

	look_x = eye_x + sin(angle*M_PI/180);
	look_z = eye_z - cos(angle*M_PI/180);
    
    
    //~ glLightfv(GL_LIGHT2, GL_POSITION, lgt_pos);
	glutPostRedisplay();
}

//--------------------------------------------------------------------------------

void insideWalls() 
{
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,txId[0]);
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black_3f);
    
 	glBegin(GL_QUADS);
	////////////////////// BACK WALL ///////////////////////
    glNormal3f(0, 0, 1);
	glTexCoord2f(0.0, 6.0); glVertex3f(-10, 5, -15);
	glTexCoord2f(0.0, 0.0); glVertex3f(-10, -1.01, -15);
	glTexCoord2f(20.0, 0.0); glVertex3f(10, -1.01, -15);
	glTexCoord2f(20.0, 6.0); glVertex3f(10, 5, -15);
    
    ////////////////////// FRONT TOP WALL ///////////////////////
    glNormal3f(0, 0, -1);
	glTexCoord2f(0.0, 3.0); glVertex3f(-1, 5, 10);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1, 2, 10);
	glTexCoord2f(20.0, 0.0); glVertex3f(1, 2, 10);
	glTexCoord2f(20.0, 3.0); glVertex3f(1, 5, 10);


	////////////////////// FRONT RIGHT WALL ///////////////////////
    glNormal3f(0, 0, -1);
    glTexCoord2f(0.0, 6.0); glVertex3f(1, 5, 10);
    glTexCoord2f(0.0, 0.0); glVertex3f(1, -1.01, 10);
    glTexCoord2f(9.0, 0.0); glVertex3f(10, -1.01, 10);
    glTexCoord2f(9.0, 6.0); glVertex3f(10, 5, 10);
    
    ////////////////////// FRONT LEFT WALL ///////////////////////
    glNormal3f(0, 0, -1);
    glTexCoord2f(0.0, 6.0); glVertex3f(-10, 5, 10);
    glTexCoord2f(0.0, 0.0); glVertex3f(-10, -1.01, 10);
    glTexCoord2f(9.0, 0.0); glVertex3f(-1, -1.01, 10);
    glTexCoord2f(9.0, 6.0); glVertex3f(-1, 5, 10);

	////////////////////// LEFT SIDE WALL ///////////////////////
    glNormal3f(1, 0, 0);
	glTexCoord2f(0.0, 6.0); glVertex3f(-10, 5, -15);
  	glTexCoord2f(0.0, 0.0); glVertex3f(-10, -1.01, -15);
	glTexCoord2f(25.0, 0.0); glVertex3f(-10, -1.01, 10);
 	glTexCoord2f(25.0, 6.0); glVertex3f(-10, 5, 10);


	////////////////////// RIGHT SIDE WALL ///////////////////////
    glNormal3f(-1, 0, 0);
	glTexCoord2f(0.0, 6.0); glVertex3f(10, 5, -15);
	glTexCoord2f(0.0, 0.0); glVertex3f(10, -1.01, -15);
 	glTexCoord2f(25.0, 0.0); glVertex3f(10, -1.01, 10);
 	glTexCoord2f(25.0, 6.0); glVertex3f(10, 5, 10);
    
    ////////////////////// CEILING WALL ///////////////////////
    glNormal3f(0, -1, 0);
    glTexCoord2f(0.0, 25.0); glVertex3f(-10, 5, -15);
    glTexCoord2f(20.0, 25.0); glVertex3f(10, 5, -15);
 	glTexCoord2f(20.0, 0.0); glVertex3f(10, 5, 10);
 	glTexCoord2f(0.0, 0.0); glVertex3f(-10, 5, 10);

	glEnd();
    
    ////////////////////// FLOOR ///////////////////////
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,txId[3]);
    glPushMatrix();
    glScalef(0.5, 1, 0.5);
    glBegin(GL_QUADS);
    float s, t;
    float ratio = 0.1;
    for(int i = -20; i < 20; i+=1)
    {
        for(int j = -30;  j < 20; j+=1)
        {
            glNormal3f(0, 1, 0);
            s = i % 10;
            t = j % 10;
            glTexCoord2f(s*ratio, t*ratio); glVertex3f(i, -1, j);
            glTexCoord2f(s*ratio, t*ratio + ratio); glVertex3f(i, -1, j+1);
            glTexCoord2f(s*ratio + ratio, t*ratio + ratio); glVertex3f(i+1, -1, j+1);
            glTexCoord2f(s*ratio + ratio, t*ratio); glVertex3f(i+1, -1, j);
        }
    }
    glEnd();
    glPopMatrix();
    
}

void outsideWalls()
{
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,txId[2]);

 	glBegin(GL_QUADS);

	////////////////////// BACK WALL ///////////////////////
    glNormal3f(0, 0, -1);
	glTexCoord2f(0.0, 6.0); glVertex3f(-10.01, 5, -15.01);
	glTexCoord2f(0.0, 0.0); glVertex3f(-10.01, -1.01, -15.01);
	glTexCoord2f(20.0, 0.0); glVertex3f(10.01, -1.01, -15.01);
	glTexCoord2f(20.0, 6.0); glVertex3f(10.01, 5, -15.01);
    
    ////////////////////// FRONT TOP WALL ///////////////////////
    glNormal3f(0, 0, 1);
	glTexCoord2f(0.0, 3.0); glVertex3f(-10.01, 5, 10.01);
	glTexCoord2f(0.0, 0.0); glVertex3f(-10.01, 2, 10.01);
	glTexCoord2f(20.0, 0.0); glVertex3f(10.01, 2, 10.01);
	glTexCoord2f(20.0, 3.0); glVertex3f(10.01, 5, 10.01);


	////////////////////// FRONT RIGHT WALL ///////////////////////
    glNormal3f(0, 0, 1);
    glTexCoord2f(0.0, 3.0); glVertex3f(1.01, 2, 10.01);
    glTexCoord2f(0.0, 0.0); glVertex3f(1.01, -1.01, 10.01);
    glTexCoord2f(9.0, 0.0); glVertex3f(10.01, -1.01, 10.01);
    glTexCoord2f(9.0, 3.0); glVertex3f(10.01, 2, 10.01);
    
    ////////////////////// FRONT LEFT WALL ///////////////////////
    glNormal3f(0, 0, 1);
    glTexCoord2f(0.0, 3.0); glVertex3f(-10.01, 2, 10.01);
    glTexCoord2f(0.0, 0.0); glVertex3f(-10.01, -1.01, 10.01);
    glTexCoord2f(9.0, 0.0); glVertex3f(-1.01, -1.01, 10.01);
    glTexCoord2f(9.0, 3.0); glVertex3f(-1.01, 2, 10.01);

	////////////////////// LEFT SIDE WALL ///////////////////////
    glNormal3f(-1, 0, 0);
	glTexCoord2f(0.0, 6.0); glVertex3f(-10.01, 5, -15.01);
  	glTexCoord2f(0.0, 0.0); glVertex3f(-10.01, -1.01, -15.01);
	glTexCoord2f(25.0, 0.0); glVertex3f(-10.01, -1.01, 10.01);
 	glTexCoord2f(25.0, 6.0); glVertex3f(-10.01, 5, 10.01);


	////////////////////// RIGHT SIDE WALL ///////////////////////
    glNormal3f(1, 0, 0);
	glTexCoord2f(0.0, 6.0); glVertex3f(10.01, 5, -15.01);
	glTexCoord2f(0.0, 0.0); glVertex3f(10.01, -1.01, -15.01);
 	glTexCoord2f(25.0, 0.0); glVertex3f(10.01, -1.01, 10.01);
 	glTexCoord2f(25.0, 6.0); glVertex3f(10.01, 5, 10.01);
    
    ////////////////////// ROOF WALL ///////////////////////
    glDisable(GL_TEXTURE_2D);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0.0, 27.0); glVertex3f(-11, 5.01, -16);
    glTexCoord2f(22.0, 27.0); glVertex3f(11, 5.01, -16);
 	glTexCoord2f(22.0, 0.0); glVertex3f(11, 5.01, 11);
 	glTexCoord2f(0.0, 0.0); glVertex3f(-11, 5.01, 11);

	glEnd();
}


void walls()
{
    insideWalls();
    outsideWalls();
    glDisable(GL_TEXTURE_2D);
}
//--------------------------------------------------------------------------------

void floor()
{
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black_3f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,txId[1]);
    glEnable(GL_NORMALIZE);
    glBegin(GL_QUADS);
    for(int i = -100; i < 100; i++)
    {
        for(int j = -100;  j < 100; j++)
        {
            glNormal3f(0, 1, 0);
            glTexCoord2f(0.0, 1.0); glVertex3f(i, -1.001, j);
            glTexCoord2f(0.0, 0.0); glVertex3f(i, -1.001, j+1);
            glTexCoord2f(1.0, 0.0); glVertex3f(i+1, -1.001, j+1);
            glTexCoord2f(1.0, 1.0); glVertex3f(i+1, -1.001, j);
        }
    }
    glEnd();
}

//--------------------------------------------------------------------------------

void skybox(float size)
{
    glColor3f(1,1,1);
    glDisable(GL_LIGHTING); 
    glDisable(GL_DEPTH_TEST);      
    glEnable(GL_TEXTURE_2D);       

    ////////////////////// BACK  ///////////////////////
    glBindTexture(GL_TEXTURE_2D,skyboxtxId[BACK]);
    glBegin(GL_QUADS);
    
    glTexCoord2f(0, 0); glVertex3f(size/2, size/2, size/2);
	glTexCoord2f(1, 0); glVertex3f(-size/2, size/2, size/2);
	glTexCoord2f(1, 1); glVertex3f(-size/2, -size/2, size/2);
	glTexCoord2f(0, 1); glVertex3f(size/2, -size/2, size/2);

    glEnd();
	////////////////////// LEFT  ///////////////////////
    glBindTexture(GL_TEXTURE_2D,skyboxtxId[LEFT]);
    glBegin(GL_QUADS);
    
    glTexCoord2f(0, 0); glVertex3f(-size/2, size/2, size/2);
    glTexCoord2f(1, 0); glVertex3f(-size/2, size/2, -size/2);
    glTexCoord2f(1, 1); glVertex3f(-size/2, -size/2, -size/2);
    glTexCoord2f(0, 1); glVertex3f(-size/2, -size/2, size/2);
    
    glEnd();
	////////////////////// FRONT  ///////////////////////
    glBindTexture(GL_TEXTURE_2D,skyboxtxId[FRONT]);
    glBegin(GL_QUADS);
    
    glTexCoord2f(1, 0); glVertex3f(size/2, size/2, -size/2);
  	glTexCoord2f(0, 0); glVertex3f(-size/2, size/2, -size/2);
	glTexCoord2f(0, 1); glVertex3f(-size/2, -size/2, -size/2);
 	glTexCoord2f(1, 1); glVertex3f(size/2, -size/2, -size/2);

    glEnd();
	////////////////////// RIGHT  ///////////////////////
    glBindTexture(GL_TEXTURE_2D,skyboxtxId[RIGHT]);
    glBegin(GL_QUADS);
    
	glTexCoord2f(0, 0); glVertex3f(size/2, size/2, -size/2);
	glTexCoord2f(1, 0); glVertex3f(size/2, size/2, size/2);
 	glTexCoord2f(1, 1); glVertex3f(size/2, -size/2, size/2);
 	glTexCoord2f(0, 1); glVertex3f(size/2, -size/2, -size/2);

	glEnd();
    ////////////////////// TOP  ///////////////////////
    glBindTexture(GL_TEXTURE_2D,skyboxtxId[TOP]);
    glBegin(GL_QUADS);
    
	glTexCoord2f(1, 0); glVertex3f(size/2, size/2, size/2);
	glTexCoord2f(0, 0); glVertex3f(-size/2, size/2, size/2);
 	glTexCoord2f(0, 1); glVertex3f(-size/2, size/2, -size/2);
 	glTexCoord2f(1, 1); glVertex3f(size/2, size/2, -size/2);

	glEnd();
    ////////////////////// BOTTOM  ///////////////////////
    glBindTexture(GL_TEXTURE_2D,skyboxtxId[BOT]);
    glBegin(GL_QUADS);
    
	glTexCoord2f(1, 1); glVertex3f(size/2, -size/2, size/2); 
	glTexCoord2f(0, 1); glVertex3f(-size/2, -size/2, size/2); 
 	glTexCoord2f(0, 0); glVertex3f(-size/2, -size/2, -size/2); 
 	glTexCoord2f(1, 0); glVertex3f(size/2, -size/2, -size/2); 

	glEnd();
    
    glEnable(GL_LIGHTING);  
    glEnable(GL_DEPTH_TEST);
}

//--------------------------------------------------------------------------------

void cradleBall() 
{
    //Front
    glColor3f(1, 1, 1);
    glPushMatrix();
        glTranslatef(0, 2.3, 1);
        glRotatef(28, 1, 0, 0);
        glScalef(0.03, 4, 0.03);
        glutSolidCube(1);
    glPopMatrix();
    //Back
    glPushMatrix();
        glTranslatef(0, 2.3, -1);
        glRotatef(-28, 1, 0, 0);
        glScalef(0.03, 4, 0.03);
        glutSolidCube(1);
    glPopMatrix();
    //Ball
    glColor3f(0.84, 0.84, 0.84);
    GLfloat shine = 100;
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shine);
    glPushMatrix();
        glTranslatef(0, 0.3, 0);
        glScalef(0.4, 0.4, 0.4);
        glutSolidSphere(1, 36, 18);
    glPopMatrix();
}

void cradleBalls()
{
    //1
    glPushMatrix();
        glTranslatef(-1.6, 0.5, 0);
        glTranslatef(0, 4, 0);
        glRotatef(left_ball_theta, 0, 0, 1);
        glTranslatef(0, -4, 0);
        cradleBall();
    glPopMatrix();
    //2
    glPushMatrix();
        glTranslatef(-0.8, 0.5, 0);
        glTranslatef(0, 4, 0);
        glRotatef(ball_2_theta, 0, 0, 1);
        glTranslatef(0, -4, 0);
        cradleBall();
    glPopMatrix();
    //3
    glPushMatrix();
        glTranslatef(0, 0.5, 0);
        glTranslatef(0, 4, 0);
        glRotatef(ball_3_theta, 0, 0, 1);
        glTranslatef(0, -4, 0);
        cradleBall();
     
    glPopMatrix();
    //4
    glPushMatrix();
        glTranslatef(0.8, 0.5, 0);
        glTranslatef(0, 4, 0);
        glRotatef(ball_4_theta, 0, 0, 1);
        glTranslatef(0, -4, 0);
        cradleBall();
    glPopMatrix();
    //5
    glPushMatrix();
        glTranslatef(1.6, 0.5, 0);
        glTranslatef(0, 4, 0);
        glRotatef(right_ball_theta, 0, 0, 1);
        glTranslatef(0, -4, 0);
        cradleBall();
    glPopMatrix();
}

void cradleMove(int value) 
{
    float change_theta = 8;
    float change_theta_2 = 2;
    if (right_ball_start) {
        if (right_ball_flag) {
            if (right_ball_theta <= 0) {
                right_ball_theta -= change_theta_2;
            } else {
                right_ball_theta-=change_theta;
            }
        } else {
            right_ball_theta+=change_theta;
        }
        if (right_ball_theta >= 36) {
            right_ball_flag = true;
        } else if (right_ball_theta <= -4) {
            right_ball_flag = false;
            right_ball_start = false;
            ball_2_theta -=change_theta_2;
            ball_3_theta -=change_theta_2;
            ball_4_theta -=change_theta_2;
        } else if (right_ball_theta < 0) {
            ball_2_theta -=change_theta_2;
            ball_3_theta -=change_theta_2;
            ball_4_theta -=change_theta_2;
        }
        if (left_ball_theta > 0) {
            ball_2_theta -=change_theta_2;
            ball_3_theta -=change_theta_2;
            ball_4_theta -=change_theta_2;
            left_ball_theta -=change_theta_2;
        }
    } else {
        if (left_ball_flag) {
            if (left_ball_theta >= 0) {
                left_ball_theta += change_theta_2;
            } else {
                left_ball_theta+=change_theta;
            }
        } else {
            left_ball_theta-=change_theta;
        }
        if (left_ball_theta <= -36) {
            left_ball_flag = true;
        } else if (left_ball_theta >= 4) {
            left_ball_flag = false;
            right_ball_start = true;
            ball_2_theta +=change_theta_2;
            ball_3_theta +=change_theta_2;
            ball_4_theta +=change_theta_2;
        } else if (left_ball_theta > 0) {
            ball_2_theta +=change_theta_2;
            ball_3_theta +=change_theta_2;
            ball_4_theta +=change_theta_2;
        }
        if (right_ball_theta < 0) {
            ball_2_theta +=change_theta_2;
            ball_3_theta +=change_theta_2;
            ball_4_theta +=change_theta_2;
            right_ball_theta +=change_theta_2;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(1, cradleMove, 0); 
}

void exibitOne() 
{
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white_3f);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);   
    glTranslatef(0, 0.25, 0);
    glScalef(0.3, 0.3, 0.3);
    //Base
    glColor3f(0.1, 0.1, 0.1);
    glPushMatrix();
        glTranslatef(0, -0.5, 0);
        glScalef(7, 1, 5);
        glutSolidCube(1);
    glPopMatrix();
    glColor3f(0.84, 0.84, 0.84);
    //Front handle
    glPushMatrix();
        glTranslatef(-3, 2, 2);
        glScalef(0.2, 5, 0.2);
        glutSolidCube(1);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(3, 2, 2);
        glScalef(0.2, 5, 0.2);
        glutSolidCube(1);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(0, 4.5, 2);
        glScalef(6.25, 0.2, 0.2);
        glutSolidCube(1);
    glPopMatrix();
    //Back handle
    glPushMatrix();
        glTranslatef(-3, 2, -2);
        glScalef(0.2, 5, 0.2);
        glutSolidCube(1);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(3, 2, -2);
        glScalef(0.2, 5, 0.2);
        glutSolidCube(1);
    glPopMatrix();
        glPushMatrix();
        glTranslatef(0, 4.5, -2);
        glScalef(6.25, 0.2, 0.2);
        glutSolidCube(1);
    glPopMatrix();
    //The balls
    glPushMatrix();
        glTranslatef(0, -0.15, 0);
        cradleBalls();
    glPopMatrix();
    glFlush();
}

//--------------------------------------------------------------------------------
void firesTimer(int value)
{
    t= t + 0.1;
    ball_x = velocity * t * cos(cannon_angle*M_PI/180) + ball_x0; 
    ball_y = velocity * t * sin(cannon_angle*M_PI/180) - (0.5 * gravity * t * t) + ball_y0;
    if (ball_y >= 2) {
        glutPostRedisplay();
        glutTimerFunc(1, firesTimer, 0);
    } else {
        cannon_firing = false;
    }
}

void keyboard(unsigned char key, int x, int y) {
    if (cannon_firing == false) {
        switch (key) {
            case 32:
                cannon_firing = true;
                ball_x = ball_x0;
                ball_y = ball_y0; 
                t = 0;
                glutTimerFunc(1, firesTimer, 0);
            break;
            case 'z':
                if (cannon_angle < 60) {
                    cannon_angle++;
                    ball_x0 = cannon_length * cos(cannon_angle*M_PI/180) + cannon_x_offset;
                    ball_y0 = cannon_length * sin(cannon_angle*M_PI/180) + cannon_y_offset;
                    ball_x = ball_x0;
                    ball_y = ball_y0;
                }
            break;
            case 'x':
                if (cannon_angle > 5) {
                    cannon_angle--;
                    ball_x0 = cannon_length * cos(cannon_angle*M_PI/180) + cannon_x_offset;
                    ball_y0 = cannon_length * sin(cannon_angle*M_PI/180) + cannon_y_offset;
                    ball_x = ball_x0;
                    ball_y = ball_y0;
                }
            break;
        }
    }

}

//--Function to compute the normal vector of a triangle with index tindx ----------
void normal1(int tindx)
{
    float x1 = x[t1[tindx]], x2 = x[t2[tindx]], x3 = x[t3[tindx]];
    float y1 = y[t1[tindx]], y2 = y[t2[tindx]], y3 = y[t3[tindx]];
    float z1 = z[t1[tindx]], z2 = z[t2[tindx]], z3 = z[t3[tindx]];
    float nx, ny, nz;
    nx = y1*(z2-z3) + y2*(z3-z1) + y3*(z1-z2);
    ny = z1*(x2-x3) + z2*(x3-x1) + z3*(x1-x2);
    nz = x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2);
    glNormal3f(nx, ny, nz);
}

//--------draws the mesh model of the cannon----------------------------
void drawCannon()
{
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.2, 0.2, 0.2);

    //Construct the object model here using triangles read from OFF file
    glBegin(GL_TRIANGLES);
        for(int tindx = 0; tindx < ntri; tindx++)
        {
           normal1(tindx);
           glVertex3d(x[t1[tindx]], y[t1[tindx]], z[t1[tindx]]);
           glVertex3d(x[t2[tindx]], y[t2[tindx]], z[t2[tindx]]);
           glVertex3d(x[t3[tindx]], y[t3[tindx]], z[t3[tindx]]);
        }
    glEnd();
}

void normal2(float x1, float y1, float z1, 
            float x2, float y2, float z2,
		      float x3, float y3, float z3 )
{
	  float nx, ny, nz;
	  nx = y1*(z2-z3)+ y2*(z3-z1)+ y3*(z1-z2);
	  ny = z1*(x2-x3)+ z2*(x3-x1)+ z3*(x1-x2);
	  nz = x1*(y2-y3)+ x2*(y3-y1)+ x3*(y1-y2);
    
      glNormal3f(nx, ny, nz);
}

void drawSide() 
{
     const int n = 24;   //Size of the array

	float vx[] = { 0, 10, 10, 30, 30, 35, 35, 40, 40, 45, 45, 50, 
        50,
        45, 40, 35, 30, 25, 20, 15, 10, 5, 0,
        0};
	float vy[] = { 15, 15, 25, 25, 35, 35, 20, 20, 35, 35, 15, 15,
        0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        15};
	float vz[n] = { 0 };

	float wx[] = { 0, 10, 10, 30, 30, 35, 35, 40, 40, 45, 45, 50,
        50,
        45, 40, 35, 30, 25, 20, 15, 10, 5, 0,
        0};
	float wy[] = { 15, 15, 25, 25, 35, 35, 20, 20, 35, 35, 15, 15, 
        0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        15};
	float wz[] = { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8};

    glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i < n; i++) {
            if (i > 0) normal2(vx[i-1], vy[i-1], vz[i-1],
                                vx[i], vy[i], vz[i],
                                wx[i-1], wy[i-1], wz[i-1]);
                                
            glVertex3f(vx[i], vy[i], vz[i]);
            
            if (i > 0) normal2(wx[i-1], wy[i-1], wz[i-1],
                                vx[i], vy[i], vz[i],
                                wx[i], wy[i], wz[i]);
                                
            glVertex3f(wx[i], wy[i], wz[i]);
        }
    glEnd();
}

void drawFace() 
{
    const int n = 16;   //Size of the array

	float vx[] = { 0, 5, 10, 10, 15, 20, 25, 30, 30, 35, 35, 40, 40, 45, 45, 50};
	float vy[16] = { 0 };
	float vz[16] = { 0 };

	float wx[] = { 0, 5, 10, 10, 15, 20, 25, 30, 30, 35, 35, 40, 40, 45, 45, 50};
	float wy[] = { 15, 15, 15, 25, 25, 25, 25, 25, 35, 35, 20, 20, 35, 35, 15, 15};
	float wz[16] = { 0 };

	//Draw a quad strip using the above two polygonal lines
    glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i < n; i++) {
            glNormal3f(0, 0, 1);
                                
            glVertex3f(vx[i], vy[i], vz[i]);
            
    
            
            
            glVertex3f(wx[i], wy[i], wz[i]);
        }
    glEnd();
}

void closedCylinder(float radius, float height) 
{
   
    glPushMatrix();
    
        glScalef(1, 1, -1);
        GLUquadric *p = gluNewQuadric();
        gluQuadricDrawStyle (p, GLU_FILL);
        gluQuadricNormals (p, GLU_SMOOTH);
        gluDisk(p, 0, radius, 36, 36);
    glPopMatrix();

    glPushMatrix();
        
        glTranslatef(0, 0,  height);
   
        gluDisk(p, 0, radius, 36, 36);
    glPopMatrix();
    
    glPushMatrix();
    

        GLUquadric *q = gluNewQuadric();
        gluQuadricDrawStyle (q, GLU_FILL);
        gluQuadricNormals (q, GLU_SMOOTH);
        gluCylinder(q, radius, radius, height,  36, 36);
    glPopMatrix();
}

void cannonCartWheel()
{   
        float wheelLength = 5;
        float wheelRadius = 7;
        glColor3f(0.45, 0.305, 0.313);
        glPushMatrix();
        
            glScalef(1, 1, -1);
            GLUquadric *p = gluNewQuadric();
            gluQuadricDrawStyle (p, GLU_FILL);
            gluQuadricNormals (p, GLU_SMOOTH);
            gluDisk(p, 0, wheelRadius, 36, 36);
        glPopMatrix();
        
        glColor3f(0.45, 0.305, 0.313);
        glPushMatrix();
            glTranslatef(0, 0,  wheelLength);
            gluDisk(p, 0, wheelRadius, 36, 36);
        glPopMatrix();
        
        glColor3f(0.45, 0.305, 0.313);
        glPushMatrix();
            GLUquadric *q = gluNewQuadric();
            gluQuadricDrawStyle (q, GLU_FILL);
            gluQuadricNormals (q, GLU_SMOOTH);
            gluCylinder(q, wheelRadius, wheelRadius, wheelLength,  36, 36);
        glPopMatrix();
}

void cannonCart() 
{
    
    //Left side wheels
    glPushMatrix();
        glTranslatef(-47, 7, 23);
        cannonCartWheel();
    glPopMatrix();
    
    glPushMatrix();
        glTranslatef(-17, 7 , 23);
        cannonCartWheel();
    glPopMatrix();
    
    //Back Wheel pole
    glColor3f(0.2, 0.2, 0.2);
    glPushMatrix();
        glTranslatef(-47, 7, -31);
        closedCylinder(2, 62);
    glPopMatrix();
    
    
    //Right side wheels
    glPushMatrix();
        glTranslatef(-47, 7, -28);
        cannonCartWheel();
    glPopMatrix();
    
    glPushMatrix();
        glTranslatef(-17, 7 , -28);
        cannonCartWheel();
    glPopMatrix();
    
    //Front Wheel pole
    glColor3f(0.2, 0.2, 0.2);
    glPushMatrix();
        glTranslatef(-17, 7, -31);
        closedCylinder(2, 62);
    glPopMatrix();
    
    //Cart bot support
    glColor3f(0.298, 0.184, 0.121);
    glPushMatrix();
        glTranslatef(-32, 10, 0);
        glScalef(6, 1, 32);
        glutSolidCube(1);
    glPopMatrix();
    
    //Cart top front support
    glColor3f(0.298, 0.184, 0.121);
    glPushMatrix();
        glTranslatef(-14.5, 23, 0);
        glRotatef(45, 0, 0, 1);
        glScalef(4, 1, 32);
        glutSolidCube(1);
    glPopMatrix();
    
    //Cart top back support
    glColor3f(0.298, 0.184, 0.121);
    glPushMatrix();
        glTranslatef(-52, 18, 0);
        glScalef(4, 1, 32);
        glutSolidCube(1);
    glPopMatrix();
    
    //Right cannon holder
    glColor3f(0.2, 0.2, 0.2);
    glPushMatrix();
        glTranslatef(-24.5, 32, -19);
        glRotatef(90, 0, 1, 0);
        closedCylinder(0.5, 10);
    glPopMatrix();
    
    //Left cannon holder
    glColor3f(0.2, 0.2, 0.2);
    glPushMatrix();
        glTranslatef(-24.5, 32, 19);
        glRotatef(90, 0, 1, 0);
        closedCylinder(0.5, 10);
    glPopMatrix();
    
    //Cart left side
    glColor3f(0.298, 0.184, 0.121);
    glPushMatrix();
        glTranslatef(-57, 5, 15);
        glPushMatrix();
            drawSide();
            glPushMatrix();
            glScalef(1, 1, -1);
            drawFace(); // back
            glPopMatrix();
            glTranslatef(0, 0, 8);
            drawFace(); //front
        glPopMatrix();
    glPopMatrix();
    
    //Cart right side
    glColor3f(0.298, 0.184, 0.121);
    glPushMatrix();
        glTranslatef(-57, 5, -23);
        glPushMatrix();
            drawSide();
            glPushMatrix();
            glScalef(1, 1, -1);
            drawFace(); // back
            glPopMatrix();
            glTranslatef(0, 0, 8);
            drawFace(); //front
        glPopMatrix();
    glPopMatrix();
    
}

void cannon() 
{
    glPushMatrix();
        glScalef(1.15, 1.15, 1);
        cannonCart();
    glPopMatrix();
    
    glEnable(GL_NORMALIZE);
    glPushMatrix();
        glTranslatef(-5, 10, 0);
        glTranslatef(-20, 30, 0);
        glRotatef(cannon_angle, 0, 0, 1);
        glTranslatef(20, -30, 0);
        drawCannon();
    glPopMatrix();
       glPushMatrix();
        glTranslatef(-22.5, 40, -25);
        glRotatef(cannon_angle, 0, 0, 1);
        closedCylinder(2.8, 50);
    glPopMatrix();
    
    glColor3f(0.078, 0.090, 0.070);
    glPushMatrix();
        glTranslatef(ball_x, ball_y, ball_z);
        glutSolidSphere(5, 36, 18);
    glPopMatrix();
    
}

void exibitTwo() 
{
    glDisable(GL_TEXTURE_2D);
    //The platform for the cannon
    glColor3f(1, 1, 1);
    glPushMatrix();
        glTranslatef(2.5, 0, 0);
        glScalef(7, 1, 1);
        glutSolidCube(1);
    glPopMatrix();
        
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white_3f);
    //The cannon and the ball
    glPushMatrix();
        glTranslatef(0, 0.5, 0);
        glScalef(0.01, 0.01, 0.01);
        cannon();
    glPopMatrix();
    glFlush();
    
}
//--------------------------------------------------------------------------------

void earthMove(int value) 
{
    earth_theta+=12;
    glutPostRedisplay();
    glutTimerFunc(5, earthMove, 0); 
}

void moonMove(int value) 
{
    moon_theta+=0.444;
    glutPostRedisplay();
    glutTimerFunc(5, moonMove, 0);
}

void moonOrbit(int value)
{
    moon_orbit_theta+=0.444;
    glutPostRedisplay();
    glutTimerFunc(5, moonOrbit, 0);
    
}

void earthSpotlight()
{
    glPushMatrix();
        float spotdir[] = {0, -1, 0};
        float lgt_pos[] = {0, 3, 0, 1};
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 10);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 100);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotdir);
        glLightfv(GL_LIGHT1, GL_POSITION, lgt_pos);
    glPopMatrix();
    
 
}

void earthShadow() 
{
    float shadowMat[16] = 
    {3, 0, 0, 0,   
    0, 0, 0, -1,
    0, 0, 3, 0,
    0, 0, 0, 3};
    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
        glMultMatrixf(shadowMat);

        glColor4f(0.2, 0.2, 0.2, 1);
        glTranslatef(0, 1, 0);
        glScalef(-0.20, -0.20, -0.20);
        glRotatef(earth_theta, 0, 1, 0);
        glRotatef(90, -1, 0, 0);
        GLUquadric *q = gluNewQuadric();
        gluQuadricDrawStyle (q, GLU_FILL);
        gluQuadricNormals (q, GLU_SMOOTH);
        gluQuadricTexture(q, GL_TRUE);
        gluSphere(q, 1, 36, 18);
    glPopMatrix();
    
            
        
}

void moonSpotlight()
{
    glPushMatrix();
       
        float spotdir[] = {0, -1, 0};
        float lgt_pos[] = {1, 3, 0, 1};
        glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 4);
        glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 100);
        glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spotdir);
        glLightfv(GL_LIGHT2, GL_POSITION, lgt_pos);
    glPopMatrix();

 
}

void moonShadow() 
{
    float shadowMat[16] =
    {3, 0, 0, 0,
    -1, 0, 0, -1, 
    0, 0, 3, 0,
    0, 0, 0, 3};
    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
        glMultMatrixf(shadowMat);
        
        
        glTranslatef(1, 1, 0);
        glScalef(-0.054, -0.054, -0.054);
        glRotatef(moon_theta, 0, 1, 0);
        glRotatef(90, -1, 0, 0);
        glColor4f(0.2, 0.2, 0.2, 1);
        GLUquadric *p = gluNewQuadric();
        gluQuadricDrawStyle (p, GLU_FILL);
        gluQuadricNormals (p, GLU_SMOOTH);
        gluQuadricTexture(p, GL_TRUE);
        gluSphere(p, 1, 36, 18);
    glPopMatrix();
}

void box() 
{
    float y = 8;
    float length = 120;
    float width = 120;
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black_3f);
 
    glEnable(GL_NORMALIZE);
    glBegin(GL_TRIANGLES);
    for(int i = -length/2; i < length/2; i++)
    {
        for(int j = -width/2;  j < width/2; j++)
        {
            glNormal3f(0, 1, 0);
            glVertex3f(i, y, j);
            glVertex3f(i+1, y, j+1);
            glVertex3f(i+1, y, j);
            
            
            glVertex3f(i, y, j);
            glVertex3f(i, y, j+1);
            glVertex3f(i+1, y, j+1);
        }
    }
    glEnd();
    
    //Front
    glBegin(GL_QUADS);
        glNormal3f(0, 0, 1);
        glVertex3f(-length/2, 0, width/2);
        glVertex3f(-length/2, y, width/2);
        glVertex3f(length/2, y, width/2);
        glVertex3f(length/2, 0, width/2);
    glEnd();
    
    //Left
    glBegin(GL_QUADS);
        glNormal3f(-1, 0, 0);
        glVertex3f(-length/2, 0, -width/2);
        glVertex3f(-length/2, y, -width/2);
        glVertex3f(-length/2, y, width/2);
        glVertex3f(-length/2, 0, width/2);
    glEnd();
    
    //Right
    glBegin(GL_QUADS);
        glNormal3f(1, 0, 0);
        glVertex3f(length/2, 0, width/2);
        glVertex3f(length/2, y, width/2);
        glVertex3f(length/2, y, -width/2);
        glVertex3f(length/2, 0, -width/2);
    glEnd();
    
    //Back
    glBegin(GL_QUADS);
        glNormal3f(0, 0, -1);
        glVertex3f(-length/2, 0, -width/2);
        glVertex3f(-length/2, y, -width/2);
        glVertex3f(length/2, y, -width/2);
        glVertex3f(length/2, 0, -width/2);
    glEnd();

}

void exibitThree()
{   
    glPushMatrix();
        glColor3f(1, 1, 1);
        glScalef(0.025, 0.025, 0.025);
        box();
    glPopMatrix();
    
    
     
    glPushMatrix();
        glTranslatef(0, 0.201, 0);
        
        earthSpotlight();
        earthShadow();
    
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        
        glTranslatef(0, 1, 0);
        glScalef(-0.20, -0.20, -0.20);
        glRotatef(earth_theta, 0, 1, 0);
        glRotatef(90, -1, 0, 0);
        
        // Earth with texture
        
        glColor3f(1,1,1);
       
        glBindTexture(GL_TEXTURE_2D, solarSystem[EARTH]);
        GLUquadric *q = gluNewQuadric();
        gluQuadricDrawStyle (q, GLU_FILL);
        gluQuadricNormals (q, GLU_SMOOTH);
        gluQuadricTexture(q, GL_TRUE);
        gluSphere(q, 1, 36, 18);
    glPopMatrix();
    
    
    glPushMatrix();
        glTranslatef(0, 0.201, 0);
        glRotatef(moon_orbit_theta, 0, 1, 0);
        moonSpotlight();
        moonShadow();
        
       
        glTranslatef(1, 1, 0);
        glScalef(-0.054, -0.054, -0.054);
        glRotatef(moon_theta, 0, 1, 0);
        glRotatef(90, -1, 0, 0);
        
        // Moon with texture
        glEnable(GL_LIGHTING);
        glColor3f(1,1,1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, solarSystem[MOON]);
        GLUquadric *p = gluNewQuadric();
        gluQuadricDrawStyle (p, GLU_FILL);
        gluQuadricNormals (p, GLU_SMOOTH);
        gluQuadricTexture(p, GL_TRUE);
        gluSphere(p, 1, 36, 18);
    glPopMatrix();
}

//--------------------------------------------------------------------------------

void hourglassGlass() {
    float vx[N], vy[N], vz[N];
	float wx[N], wy[N], wz[N]; 

	for (int i = 0; i < N; i++)	
	{
		vx[i] = vx_init[i];
		vy[i] = vy_init[i];
		vz[i] = vz_init[i];
	}

    glEnable(GL_LIGHTING);

    float theta = 10 * M_PI/180;
    for (int j = 0; j < 36; j++) {
        for (int i = 0; i < N; i++) {
            wx[i] = vx[i]*cos(theta) + vz[i]*sin(theta);
            wy[i] = vy[i];
            wz[i] = -vx[i]*sin(theta) + vz[i]*cos(theta);
        }
        
        glBegin(GL_TRIANGLE_STRIP);
            for (int i = 0; i < N; i++) {
                if (i > 0) normal2(vx[i-1], vy[i-1], vz[i-1],
                                    wx[i-1], wy[i-1], wz[i-1],
                                    vx[i], vy[i], vz[i]);
                
                glTexCoord2f((float)j/36, (float)i/(N-1));
                glVertex3f(vx[i], vy[i], vz[i]);
                
                if (i > 0) normal2(wx[i-1], wy[i-1], wz[i-1],
                                    wx[i], wy[i], wz[i],
                                    vx[i], vy[i], vz[i]);                 
                
                glTexCoord2f((float)(j+1)/36, (float)i/(N-1));
                glVertex3f(wx[i], wy[i], wz[i]);
            }
        glEnd();
        
        for (int i = 0; i < N; i++) {
            vx[i] = wx[i];
            vy[i] = wy[i];
            vz[i] = wz[i];
        }
    }

	glFlush();
}
void hourglassPole() {
    glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        closedCylinder(1, 40);
    glPopMatrix();
}
void hourglassSupport() {
    glColor3f(0.3, 0.3 ,0.3);
    float radius = 14;
    //Left pole
    glPushMatrix();
        float x1 = radius * cos(120*M_PI/180);
        float z1 = radius * sin(120*M_PI/180);
        glTranslatef(x1, 3, z1);
        hourglassPole();
    glPopMatrix();
    
    //Right pole
    glPushMatrix();
        float x2 = radius * cos(240*M_PI/180);
        float z2 = radius * sin(240*M_PI/180);
        glTranslatef(x2, 3, z2);
        hourglassPole();
    glPopMatrix();
    
    //Back pole
    glPushMatrix();
        float x3 = radius * cos(360*M_PI/180);
        float z3 = radius * sin(360*M_PI/180);
        glTranslatef(x3, 3, z3);
        hourglassPole();
    glPopMatrix();
    
    
    //Bot support 
    glPushMatrix();
        glTranslatef(0, 0, 0);
        glRotatef(-90, 1, 0, 0);
        closedCylinder(18, 1.5);
    glPopMatrix();
    
    glPushMatrix();
        glTranslatef(0, 1.5, 0);
        glRotatef(-90, 1, 0, 0);
        closedCylinder(17, 1.5);
    glPopMatrix();
    
    //Top support
    glPushMatrix();
        glTranslatef(0, 44.5, 0);
        glRotatef(-90, 1, 0, 0);
        closedCylinder(18, 1.5);
    glPopMatrix();
    
    glPushMatrix();
        glTranslatef(0, 43, 0);
         glRotatef(-90, 1, 0, 0);
        closedCylinder(17, 1.5);
    glPopMatrix();
}

void hourglass() {
    glPushMatrix();
        
       
        hourglassSupport();
    glPopMatrix();
    glPushMatrix();
        glTranslatef(0, 3, 0);
         glColor4f(0.5, 0.5, 0.5, 0.2);
        hourglassGlass();
    glPopMatrix();
}

void exibitFour() {
    glDisable(GL_TEXTURE_2D);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white_3f);
    
    glScalef(0.05, 0.05, 0.05);
    glRotatef(180, 0, 1, 0);
    hourglass();
}
//--------------------------------------------------------------------------------

void display() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glMatrixMode(GL_PROJECTION);						
	glLoadIdentity();
    gluPerspective(60,1024.0/768.0,1,5000.0);

	glMatrixMode(GL_MODELVIEW);								
	glLoadIdentity();
	gluLookAt(eye_x, eye_y, eye_z, look_x, look_y, look_z, 0, 1, 0);	

    glPushMatrix();
        skybox(1000);
    glPopMatrix();
    
    glEnable(GL_LIGHTING);
    
    glPushMatrix();
        glTranslatef(0, 0, -30);
        walls();
        float lgt_pos_centre[] = {0.0f, 1.0f, 0.0f, 1.0f};  
        glLightfv(GL_LIGHT5, GL_POSITION, lgt_pos_centre);  
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-7, -1, -30);
        glRotatef(90, 0, 1, 0);
        exibitOne();
    glPopMatrix();

    glPushMatrix();
        floor();
    glPopMatrix();
    
    glPushMatrix();
        glTranslatef(7, -1, -32);
        glRotatef(-90, 0, 1, 0);
        exibitTwo();
    glPopMatrix();
    glPushMatrix();
        glTranslatef(0, -1, -40);
        exibitThree();
    glPopMatrix();
    
    glPushMatrix();
       glTranslatef(-7, -1, -25);
        exibitFour();
    glPopMatrix();
    
    glFlush();
	glutSwapBuffers();									
}

//--------------------------------------------------------------------------------

void initialise()
{ 
    loadMeshFile("Cannon.off");          
	loadTexture();	
	glEnable(GL_TEXTURE_2D);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glEnable(GL_BLEND); 
    //~ glClearColor(0.0,0.0,0.0,0.0);

    float grey[4] = {0.2, 0.2, 0.2, 0.1};
    float white[4]  = {1.0, 1.0, 1.0, 1.0};
    float black[4] = {0, 0, 0, 1};
    
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glMaterialfv(GL_FRONT, GL_SPECULAR, white);
    glMaterialf(GL_FRONT, GL_SHININESS, 100);
    
    //GL_LIGHT1
    glLightfv(GL_LIGHT1, GL_AMBIENT, grey);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT1, GL_SPECULAR, white);
    
    //GL_LIGHT2
    glLightfv(GL_LIGHT2, GL_AMBIENT, grey);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT2, GL_SPECULAR, white);
    
    //GL_LIGHT5
    glLightfv(GL_LIGHT5, GL_AMBIENT, black);
    glLightfv(GL_LIGHT5, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT5, GL_SPECULAR, white);
    
    glEnable(GL_LIGHTING);      //Enable OpenGL states
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT5);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    
    glClearColor(1., 1., 1., 1.);    //Background colour 
}


//--------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB| GLUT_DEPTH);
    glutInitWindowSize (1024, 768); 
    glutInitWindowPosition (10, 10);
    glutCreateWindow ("The Museum");
    initialise();

    glutDisplayFunc(display); 
    glutTimerFunc(1, cradleMove, 0);
    glutTimerFunc(5, earthMove, 0);
    glutTimerFunc(5, moonMove, 0);
    glutTimerFunc(5, moonOrbit, 0);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
