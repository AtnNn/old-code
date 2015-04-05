// /entry:"mainCRTStartup" /subsystem:windows

// Written by Etienne Laurin
// My first OpenGL Application

#include "windows.h"
#include "gl/gl.h" 
#include "gl/glut.h"
#include <stdio.h>
#include <math.h>
#define PI 3.1415926
#define D(x) putchar(x);
int id_cube;
int angle=0;
int speed=5;
double dir=0;
int j=0;
int wh, ww, M;
double px=-1,py=-1;
char *data[]={
	"xxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
	"x         xs    x           x",
	"x         xxxxx x xxx xxxxx x",
	"x           x x     x x   x x",
	"x         x x x xxx x x x x x",
	"x         x x x x x x x x x x",
	"x         x x x   x x x x x x",
	"x v  t  v x x xxxxx x x xxx x",
	"x         x x       x   x x x",
	"x         x xxxxxxxxxxx x x x",
	"xeeeeeeeeex             x   x",
	"xxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
	NULL
};


void genmap(char *map[]){
	int t;
	char *c,**m;
	//int n=glGenLists(1);
	//glNewList(n, GL_COMPILE);
	glTranslatef(.5,0,.5);
	for(m=map;*m;m++){
		glPushMatrix();
		for(c=*m;*c;c++){
			glColor3f(!t,t,0);
			t=!t;
			switch(*c){
			case 'x':
				glutSolidCube(1);
				break;
			case 'v':
				glutSolidCone(.5,1,15,15);
				break;
			case 't':
				glutSolidTeapot(.3);
				break;
			case 'e':
				glColor3f(1.,1.,1.);
				glutSolidSphere(.05,10,10);
				glPushMatrix();
					glRotated(angle,0,1,0);
					glTranslatef(0,0,.2);
					glutSolidSphere(.01,10,10);
				glPopMatrix();
				glPushMatrix();
					glRotated(angle,0,0,1);
					glTranslatef(0,-.3,0);
					glutSolidSphere(.01,10,10);
				glPopMatrix();
				glPushMatrix();
					glRotated(angle,1,0,0);
					glTranslatef(0,.2,0);
					glutSolidSphere(.01,10,10);
				glPopMatrix();
				break;
			case 's':
				if(px<0){
				py=m-map+.5;
				px=c-*m+.5;
				}
			}
			glTranslatef(1,0,0);
		}
		glPopMatrix();
		glTranslatef(0,0,1);
	}
	//glEndList();
	//return n;
}

void display(void){
	double t;
	D('*');
	glClearColor(0,0,0,0); // selectionne la couleur noire (qui est celle par défaut)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	t=(double)abs(j)/10;
	gluLookAt(px,t,py,px+cos(dir),t,py+sin(dir),0,2,0);
	/*
	glColor3f(1.,0.,0.)	;
	glutWireSphere(30,30,30);
	glPushMatrix();
	glColor4f(0.,1.,0.,.3)	;
	glRotated(angle,0,1,0);
	glTranslatef(0,0,-5);
	glScaled(2,2,2);
	glutSolidSphere(2,10,10);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.,0.,1.)	;
	glRotated(angle,0,1,0);
	glTranslatef(0,0,5);
	glScaled(1,1,1);
	glutSolidSphere(2,10,10);
	glPopMatrix();
	*/
	//glCallList(M);
	genmap(data);
	glutSwapBuffers();
	glFlush();
}

void reshape(int w, int h) 
{
	wh=h; ww=w;
	D('[');
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0,(float) w/h,.02,100.);
	glMatrixMode(GL_MODELVIEW); // on revient sur la matrice MODELVIEW
	glLoadIdentity();	
}
void jump(int id){
	D('^');
	if(j>10)j=-11;
	glutPostRedisplay();
	if(j!=0){
		j++;
		glutTimerFunc(10,jump,1);
	}
}

void click(int b, int s, int x, int y){
	D('.');
	if(!j){j++;
	jump(1);}
}

void mouse(int x, int y){
	static s=0;
	D('-');
	s=!s;
	if(s)return;
	dir+=.002*(x-ww/2);
	while(dir>360)dir=dir-360;
	while(dir<-360)dir=dir+360;
	glutWarpPointer(ww/2,wh/2);
	glutPostRedisplay();
}

void special(int key, int x, int y){
	double ipx=px,ipy=py;
	D('>');
	switch(key){
	case GLUT_KEY_UP:
		px+=cos(dir)/3;
		py+=sin(dir)/3;
		break;
	case GLUT_KEY_DOWN:
		px-=cos(dir)/3;
		py-=sin(dir)/3;
		break;
	case GLUT_KEY_LEFT:
		px+=cos(dir-PI/2)/6;
		py+=sin(dir-PI/2)/6;
		break;
	case GLUT_KEY_RIGHT:
		px-=cos(dir-PI/2)/6;
		py-=sin(dir-PI/2)/6;
		break;
	}
	if(data[(int)py][(int)px]=='x'){
		if(data[(int)py][(int)ipx]=='x')
			py=ipy;
		if(data[(int)ipy][(int)px]=='x')
			px=ipx;

	}
	if(data[(int)py][(int)px]=='e'){
		printf("\n\nYou won!\n\n\n");
		exit(0);
	}
	glutPostRedisplay();
}

void clavier(unsigned char key, int x, int y){
	if(key==27)
		exit(0);
}

void rotate(int id){
	D('@');
	angle+=10;
	glutTimerFunc(10,rotate,2);
	glutPostRedisplay();
}

int main(int argc, char** argv){
	float pos[]={0.,0.,1.,1.};
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutCreateWindow("aMAZEing - AtnNn's first opengl game - with help from Tr_isK");
	glutFullScreen();
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glClearColor(0., 0., 0., 0.);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glLightfv(GL_LIGHT0,GL_POSITION,pos);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	//glCullFace(GL_BACK);
	//glEnable(GL_CULL_FACE);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutPassiveMotionFunc(mouse);
	glutSpecialFunc(special);
	glutKeyboardFunc(clavier);
	glutMouseFunc(click);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutTimerFunc(10,rotate,2);
	glutWarpPointer(ww/2,wh/2);
	//M=genmap(data);
	glutMainLoop();
	return 0;
	//glutTimerFunc(50,timer,1);
}








/*#include <stdio.h>
#include <gl/glut.h>
#include <math.h>
#define PI 3.1415926
double angle=0,px,py,pz,vx,vy,vz;

void display(void){
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(5*sin(angle),5,5*cos(angle),0,0,0,0,1,0);
	glPushMatrix();
	glColor3f(0.,0.,1.);
	glTranslatef(-1.,0.,0.);
	glutSolidSphere(.5,10,10);
	glTranslatef(2.,0.,0.);
	glColor3f(0.,0.9,0.);
	glutSolidCube(1);
	glPopMatrix();
	glutSwapBuffers();
}

void reshape(int w, int h){
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,(float)w/h,1,100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void timer(int id){
	angle+=0.1;
	while(angle>2*PI)angle-=2*PI;
	while(angle<2*PI)angle+=2*PI;
	glutTimerFunc(100,timer,id);
	glutPostRedisplay();
}

int main(int argc, char **argv){
	float pos[]={1.1,1.1,1.1,0.};
	glutInit(&argc, argv);
	glutInitDisplayMode ( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	//glutInitWindowPosition(100,100);
	glutInitWindowSize(640,480);
	glutCreateWindow ("opengl");

	//glutFullScreen();

	glShadeModel(GL_SMOOTH);
	glClearColor(0., 0., 0., 0.);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	//glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//glutSetCursor(GLUT_CURSOR_NONE);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	//glLightfv(2,GL_AMBIENT,amb);
	//glLightfv(2,GL_DIFFUSE,dif);

	glLightfv(GL_LIGHT0,GL_POSITION,pos);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	//glutSpecialFunc(special);
	//glutPassiveMotionFunc(passivemotion);
	glutTimerFunc(100,timer,1);

	//glEnable(GL_CULL_FACE);
	glutMainLoop();
	return 0;
}
*/
