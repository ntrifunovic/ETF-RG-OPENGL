#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <math.h>

#include <algorithm>
#include <list>

#include "neheTGATexture.h"

#define PI 3.14159265

#define TEZE 0
#define LAKSE 1
#define UGASI 2
#define TEKSTURA 3
#define START 4
#define RESI 5

#define DISK_HEIGHT 15
#define BASE_SIZE 25

#define NO_SOURCE -1
#define NOT_STARTED -1

int numDisks = 5;

float anglex = 30.0f;
float angley = 0.0f;

float distance = 500.0f;

int lookAt = 0;

GLfloat AmbijentalnoSvetlo[]=	{ 1.0f, 1.0f, 1.0f, 0.3f };
GLfloat DifuznoSvetlo[]=	{ 1.0f, 1.0f, 1.0f, 0.1f };
GLfloat PozicijaSvetla[][4]=	{ { 0.0f, 300.0f, 100.0f, 0.1f }, { 0.0f, 10.0f, 500.0f, 0.7f }};

#define NUMBER_OF_TEXTURES 4

imageTGA img[NUMBER_OF_TEXTURES], dno;

int current_texture = 0;

std::list<int> discs[3];

std::list<std::pair<int, int>> solution;

int startTime = NOT_STARTED;
int errorTime = NOT_STARTED;
int nextMoveTime = NOT_STARTED;
int moveWait = 500;

bool simulacija = false;

int source = NO_SOURCE;
int moves = 0;

void generateSolution(int n, int source, int destination) {
  if(n==0) return;
  
  int other = 3 - source - destination;
  generateSolution(n-1, source, other);
  solution.push_back(std::make_pair(source, destination));
  generateSolution(n-1, other, destination);
}

void move(int source, int destination) {
  discs[destination].push_back(discs[source].back());
  discs[source].pop_back();
  
  moves++;
}

void changeSize(int w, int h) {
  
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;
  
	float ratio =  w * 1.0 / h;
  
	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);
  
	// Reset Matrix
	glLoadIdentity();
  
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);
  
	// Set the correct perspective.
	gluPerspective(45.0f, ratio, 0.1f, 1000.0f);
  
	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

void DrawDisc(int size, int position, int xcenter) {
  glPushMatrix();
  {
    glTranslatef(xcenter, position*DISK_HEIGHT + DISK_HEIGHT/2, 0);
    glRotatef(90, -1, 0, 0);
    glutSolidTorus(DISK_HEIGHT/2, BASE_SIZE*(1+0.5/(numDisks+1.0f)*size), 20, 20);
  }
  glPopMatrix();

}

void DrawPost(int xcenter)
{
  glPushMatrix();
  {
    glTranslatef(xcenter, 0, 0);
    glRotatef(90, -1, 0, 0);
    
    glBegin(GL_TRIANGLE_FAN);
    {
      glTexCoord2f(0.5, 0.5);
      glVertex3f(0, 0, (numDisks + 1)*DISK_HEIGHT);
      
      const int iterations = 30;
      
      for (int i = 0; i <= iterations; i++) {
        glTexCoord2f(0.5+sin(2*PI/iterations*i)/2, 0.5+cos(2*PI/iterations*i)/1);
        glVertex3f(sin(2*PI/iterations*i)*BASE_SIZE, cos(2*PI/iterations*i)*BASE_SIZE, 0);
      }
    }
    glEnd();
  }
  glPopMatrix();
}

void renderString(GLint x, GLint y, std::string s) {
  glDisable(GL_COLOR_MATERIAL);
  void * font = GLUT_BITMAP_9_BY_15;

  glRasterPos2i(x, y);
  
  for (std::string::iterator i = s.begin(); i != s.end(); ++i)
  {
    char c = *i;
    //this does nothing, color is fixed for Bitmaps when calling glRasterPos
    //glColor3f(1.0, 0.0, 1.0);
    glutBitmapCharacter(font, c);
  }
  glEnable(GL_COLOR_MATERIAL);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();
  
	// Podesi kameru
	gluLookAt(BASE_SIZE*lookAt, 0.0f, distance,
            0.0f, 0.0f,  0.0f,
            0.0f, 1.0f,  0.0f);
  
  glRotatef(anglex, 1.0f, 0.0f, 0.0f);
	glRotatef(angley, 0.0f, 1.0f, 0.0f);

 
  if (startTime != NOT_STARTED) {
    int eplasedTime = glutGet(GLUT_ELAPSED_TIME);

    if(simulacija) {
      if(!solution.empty() && eplasedTime > nextMoveTime) {
        move(solution.front().first, solution.front().second);
        solution.pop_front();
        nextMoveTime = eplasedTime + moveWait;
      }
    } else {
      glColor3f(1.0f, 1.0f, 10.0f);  // needs to be called before RasterPos
      renderString(50, 120, "Moves:" + std::to_string(moves));
      int timeSinceStart =  eplasedTime - startTime;
      renderString(-50, 120, "Time:" + std::to_string(timeSinceStart/1000) + "s");
    
      if (eplasedTime < errorTime) {
        glColor4d(1, 0, 0, 0.3);
        glPushMatrix();
        {
          glTranslatef(0, 50, 100);
          glutSolidSphere(10, 10, 10);
        }
        glPopMatrix();
      }
    }
  }

  
  glLightfv(GL_LIGHT0, GL_AMBIENT, AmbijentalnoSvetlo);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, DifuznoSvetlo);
  glLightfv(GL_LIGHT0, GL_POSITION, PozicijaSvetla[0]);
  glLightfv(GL_LIGHT1, GL_POSITION, PozicijaSvetla[1]);
  
  glColor3d(1, 1, 1);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, dno.getId());
  
  glBegin(GL_QUADS);
  {
    glTexCoord2f(0, 0);
    glVertex3f(-5*BASE_SIZE, 0, -5*BASE_SIZE);
    glTexCoord2f(0, 1);
    glVertex3f(-5*BASE_SIZE, 0, 5*BASE_SIZE);
    glTexCoord2f(1, 1);
    glVertex3f(5*BASE_SIZE, 0, 5*BASE_SIZE);
    glTexCoord2f(1, 0);
    glVertex3f(5*BASE_SIZE, 0, -5*BASE_SIZE);
               
  }
  glEnd();

  // glColor3d(1, 0, 0);
  
  glBindTexture(GL_TEXTURE_2D, img[current_texture].getId());
  
  DrawPost(-3*BASE_SIZE);
  DrawPost(0);
  DrawPost(+3*BASE_SIZE);
  
  glDisable(GL_TEXTURE_2D);
  
  //glDisable(GL_LIGHTING);
  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc (GL_ONE, GL_ONE);
  
  glColor4d(0.8, 0.2, 1, 0.75);
  
  for(int i = 0; i < 3; i++) {
    int pos = 0;
    for (std::list<int>::iterator it=discs[i].begin(); it != discs[i].end(); it++) {
      DrawDisc(*it, pos++, (i-1)*3*BASE_SIZE);
    }
  }
  
  //glDisable(GL_BLEND);
  //glEnable(GL_LIGHTING);
  
	glutSwapBuffers();
}

// Callback funkcija za tastaturu
void kbd(unsigned char key, int x, int y) {
  switch (key) {
    case 27:
      exit(0);
      break;
    case '[':
      distance -= 10;
      break;
    case ']':
      distance += 10;
      break;
    case '.':
      moveWait = std::max(0, moveWait - 10);
      break;
    case ',':
      moveWait += 10;
      break;
    case '1':
    case '2':
    case '3':
      lookAt = key - '2';
      break;
  }
}

void skbd(int key, int x, int y) {
  switch (key) {
    case GLUT_KEY_UP:
      anglex = std::min(anglex + 1, 180.0f);
      break;
    case GLUT_KEY_DOWN:
      anglex = std::max(anglex - 1, 0.0f);
      break;
    case GLUT_KEY_LEFT:
      angley += 1;
      break;
    case GLUT_KEY_RIGHT:
      angley -= 1;
      break;
  }
}

void menu(int value) {
  switch (value) {
    case LAKSE:
      if(numDisks > 1) {
        discs[0].pop_front();
        numDisks--;
      }
      break;
    case TEZE:
      discs[0].push_front(numDisks);
      numDisks++;
      break;
    case TEKSTURA:
      current_texture = (current_texture+1)%NUMBER_OF_TEXTURES;
      break;
    case START:
      startTime = glutGet(GLUT_ELAPSED_TIME);
      break;
    case RESI:
      startTime = glutGet(GLUT_ELAPSED_TIME);
      simulacija = true;
      generateSolution(numDisks, 0, 2);
      break;
    case UGASI:
      exit(0);
      break;
  }
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
  if(startTime == NOT_STARTED)
    return;
  
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_UP) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      glDisable(GL_LIGHTING);
      
      glColor3d(1, 0, 0);
      DrawPost(-3*BASE_SIZE);
      glColor3d(0, 1, 0);
      DrawPost(0);
      glColor3d(0, 0, 1);
      DrawPost(+3*BASE_SIZE);
      
      glEnable(GL_LIGHTING);
      
      // get color information from frame buffer
      unsigned char pixel[3];
      
      GLint viewport[4];
      glGetIntegerv(GL_VIEWPORT, viewport);
      
      glReadPixels(x, viewport[3] - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
      int selected = pixel[0] == 255 ? 0 : pixel[1] == 255 ? 1 : pixel[2] == 255 ? 2 : NO_SOURCE;
      
      if (selected == NO_SOURCE)
        return;
      
      if (source == NO_SOURCE) {
        if (discs[selected].size()>0) {
          source = selected;
        }
      } else {
        
        if (discs[selected].size() == 0 || discs[selected].back() > discs[source].back()) {
          move(source, selected);
        } else {
          errorTime = glutGet(GLUT_ELAPSED_TIME) + 700;
        }
        
        source = NO_SOURCE;
      }
    }
  }
}

int main(int argc, char **argv) {
  
	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Hanoi");
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  
  glShadeModel (GL_SMOOTH);
  
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, AmbijentalnoSvetlo);
  
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  
  
  dno.loadTGA("dno.tga");
  
  for (int i = 0; i<NUMBER_OF_TEXTURES; i++) {
    img[i].loadTGA(std::to_string(i) + ".tga");
  }
  
  for (int i = 0; i < numDisks; i++) {
    discs[0].push_back(numDisks-i);
  }
  
	// register callbacks
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);
  glutKeyboardFunc(kbd);
  glutSpecialFunc(skbd);
  glutMouseFunc(mouse);
  
  glutCreateMenu(menu);
  glutAddMenuEntry("Teze", TEZE);
  glutAddMenuEntry("Lakse", LAKSE);
  glutAddMenuEntry("Ugasi", UGASI);
  glutAddMenuEntry("Promeni Teksturu", TEKSTURA);
  glutAddMenuEntry("START !!!", START);
  glutAddMenuEntry("Resi :)", RESI);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  
	// enter GLUT event processing cycle
	glutMainLoop();
  
	return 1;
}
