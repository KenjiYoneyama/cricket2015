#include<GL/glut.h>
#include<GL/glu.h>
#include<GL/gl.h>
#include<stdlib.h>
#include<sys/time.h>

#include "cricket_header.h"

struct grph_globes{
  GLubyte random_dots[64][64][3];
  int floor_width;
  int floor_height;
} *grph_globes;

void init_grph_globes(){
  int i,j;
  grph_globes=(struct grph_globes*)malloc(sizeof(struct grph_globes));
  for(i=0;i<64;i++){
    for(j=0;j<64;j++){
      if(rand()%10<7){
	grph_globes->random_dots[i][j][0]=0xff;
	grph_globes->random_dots[i][j][1]=0xff;
	grph_globes->random_dots[i][j][2]=0xff;
      }else{
	grph_globes->random_dots[i][j][0]=0xff;
	grph_globes->random_dots[i][j][1]=0x00;
	grph_globes->random_dots[i][j][2]=0x00;
      }
    }
  }
}

void idle_f(){
  if(globes->endstate==1) exit(0);
  glutPostRedisplay();
}
void idle_w(){
  if(globes->endstate==1) exit(0);
  glutPostRedisplay();
}

void disp_f_(){

  /* TODO */
  /* 2D->3D measure the cricket's eye height again first*/

  glClear(GL_COLOR_BUFFER_BIT);
  
  glEnable(GL_TEXTURE_2D);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, grph_globes->random_dots);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  
  glBegin(GL_QUADS);
  {
    glTexCoord2f(0.0, 0.0);
    glVertex2i(0, 0);
    glTexCoord2f(0.0, 1.0*2);
    glVertex2i(0, grph_globes->floor_height);
    glTexCoord2f(1.25*2, 1.0*2);
    glVertex2i(grph_globes->floor_width, grph_globes->floor_height);
    glTexCoord2f(1.25*2, 0.0);
    glVertex2i(grph_globes->floor_width, 0);
  }
  glEnd();

  glDisable(GL_TEXTURE_2D);
  
  glutSwapBuffers();
}


void draw_sphere(double oX, double oY){
  glPushMatrix();
  {
    glColor3d(1.0, 0.0, 0.0);
    glTranslatef(oX, oY, 90.0);
    glutSolidSphere(100, 18, 24);
  }
  glPopMatrix();
}

void obj_fnc(){
  double oX=300;
  double oY=0;
  static int ba=0;
  ba+=1;
  ba=ba%300;

  draw_sphere(oX-ba, oY+ba);
}

void obj_fnc2(){
  double oX=250;
  double oY=250;
  glPushMatrix();
  {
    glColor3d(1.0, 0.0, 0.0);
    glTranslatef(oX, oY, -96.0);
    glutSolidSphere(100, 18, 24);
  }
  glPopMatrix();

}

void dots_fnc(){
  glEnable(GL_TEXTURE_2D);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, grph_globes->random_dots);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  
  glBegin(GL_QUADS);
  {
    glTexCoord2f(0.0, 0.0);
    glVertex3d(-490, -490, 0);
    glTexCoord2f(0.0, 1.0*2*3);
    glVertex3d(-490, 490*2, 0);
    glTexCoord2f(1.0*2*3, 1.0*2*3);
    glVertex3d(490*2, 490*2, 0);
    glTexCoord2f(1.0*2*3, 0.0);
    glVertex3d(490*2, -490, 0);
  }
  glEnd();

  glDisable(GL_TEXTURE_2D);
}

void disp_f(){

  /* TODO */
  /* 2D->3D measure the cricket's eye height again first*/

  // get information of the cricket's position
  double cX=250;
  double cY=250;
  double eye=globes->eye_height;

  double cX_max=globes->floor_disp_width+globes->floor_bezel_left+globes->floor_bezel_right;
  double cY_max=globes->floor_disp_height+globes->floor_bezel_top+globes->floor_bezel_bottom;
  double DEP=1.0;

  double left=-(cX_max-cX-globes->wall_bezel_left)*DEP/(eye);
  double right=(cX-globes->wall_bezel_right)*DEP/(eye);
  double bottom=-(cY_max-cY-globes->wall_bezel_left)*DEP/(eye);
  double top=(cY-globes->wall_bezel_right)*DEP/(eye);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  glFrustum(left,
	    right,
	    bottom,
	    top,
	    DEP,
	    20000);
  glRotatef(180, 0.0, 0.0, 1.0);
  glTranslatef(-cX, -cY, -eye);
 
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  obj_fnc();
  dots_fnc();

  glutSwapBuffers();
}

void disp_wt(){
  // get information of the cricket's position
  double cX=250;
  double cY=250;

  double cX_max=globes->floor_disp_width+globes->floor_bezel_left+globes->floor_bezel_right;
  double cY_max=globes->floor_disp_height+globes->floor_bezel_top+globes->floor_bezel_bottom;
  double DEP=10.0;

  double left=(cX_max-cX-globes->wall_bezel_left)*DEP/(cY_max-cY);
  double right=-(cX-globes->wall_bezel_right)*DEP/(cY_max-cY);
  double bottom=(globes->wall_disp_height+globes->wall_bezel_top)*DEP/(cY_max-cY);
  double top=-(globes->wall_bezel_top)*DEP/(cY_max-cY);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  glFrustum(left,
	    right,
	    bottom,
	    top,
	    DEP,
	    20000);
  glRotatef(-90, 1.0, 0.0, 0.0);
  glTranslatef(-cX, -cY, -4.0);
 
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  obj_fnc();
  dots_fnc();

  glutSwapBuffers();
}

void disp_wb(){
  // get information of the cricket's position
  double cX=250;
  double cY=250;

  double cX_max=globes->floor_disp_width+globes->floor_bezel_left+globes->floor_bezel_right;
  double cY_max=globes->floor_disp_height+globes->floor_bezel_top+globes->floor_bezel_bottom;
  double DEP=10.0;

  double left=(cX-globes->wall_bezel_left)*DEP/(cY);
  double right=-(cX_max-cX-globes->wall_bezel_right)*DEP/(cY);
  double bottom=(globes->wall_disp_height+globes->wall_bezel_top)*DEP/(cY);
  double top=-(globes->wall_bezel_top)*DEP/(cY);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  glFrustum(left,
	    right,
	    bottom,
	    top,
	    DEP,
	    20000);
  glRotatef(-90, 1.0, 0.0, 0.0);
  glRotatef(180, 0.0, 0.0, 1.0);
  glTranslatef(-cX, -cY, 0.0);
 
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  obj_fnc();

  glutSwapBuffers();
}

void disp_wr(){
  // get information of the cricket's position
  double cX=250;
  double cY=250;

  double cX_max=globes->floor_disp_width+globes->floor_bezel_left+globes->floor_bezel_right;
  double cY_max=globes->floor_disp_height+globes->floor_bezel_top+globes->floor_bezel_bottom;
  double DEP=10.0;

  double left=(cY-globes->wall_bezel_left)*DEP/(cX_max-cX);
  double right=-(cY_max-cY-globes->wall_bezel_right)*DEP/(cX_max-cX);
  double bottom=(globes->wall_disp_height+globes->wall_bezel_top)*DEP/(cX_max-cX);
  double top=-(globes->wall_bezel_top)*DEP/(cX_max-cX);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  glFrustum(left,
	    right,
	    bottom,
	    top,
	    DEP,
	    20000);
  glRotatef(-90, 1.0, 0.0, 0.0);
  glRotatef(90, 0.0, 0.0, 1.0);
  glTranslatef(-cX, -cY, 0.0);
 
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  obj_fnc();

  glutSwapBuffers();
}

void disp_wl(){
  // get information of the cricket's position
  double cX=250;
  double cY=250;

  double cX_max=globes->floor_disp_width+globes->floor_bezel_left+globes->floor_bezel_right;
  double cY_max=globes->floor_disp_height+globes->floor_bezel_top+globes->floor_bezel_bottom;
  double DEP=10.0;

  double left=(cY_max-cY-globes->wall_bezel_left)*DEP/(cX_max-cX);
  double right=-(cY-globes->wall_bezel_right)*DEP/(cX_max-cX);
  double bottom=(globes->wall_disp_height+globes->wall_bezel_top)*DEP/(cX_max-cX);
  double top=-(globes->wall_bezel_top)*DEP/(cX_max-cX);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  glFrustum(left,
	    right,
	    bottom,
	    top,
	    DEP,
	    20000);
  glRotatef(-90, 1.0, 0.0, 0.0);
  glRotatef(-90, 0.0, 0.0, 1.0);
  glTranslatef(-cX, -cY, 0.0);
 
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  obj_fnc();

  glutSwapBuffers();
}

void floor_graphics(int width, int height, char *dsp_no){
  // display (floor)
  init_grph_globes();
  grph_globes->floor_width=width;
  grph_globes->floor_height=height;
  int fake_argc=3;
  char *fake_argv[]={"a", "-display", dsp_no};
  glutInit(&fake_argc, fake_argv);
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
  glutCreateWindow("floor");
  glutFullScreen();
  glutDisplayFunc(disp_f);
  glutIdleFunc(idle_f);
  glClearColor(1.0,1.0,1.0,1.0);
  //  glOrtho(0, width, height, 0, -1, 1);

  glutMainLoop();
}

void wall_graphics(unsigned char side, int width, int height, char *dsp_no){
  int fake_argc=3;
  char *fake_argv[]={"a", "-display", dsp_no};
  glutInit(&fake_argc, fake_argv);
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
  glutCreateWindow("wall");
  glutFullScreen();
  switch(side){
  case RIGHT:
    glutDisplayFunc(disp_wr);
    break;
  case TOP:
    glutDisplayFunc(disp_wt);
    break;
  case LEFT:
    glutDisplayFunc(disp_wl);
    break;
  case BOTTOM:
    glutDisplayFunc(disp_wb);
    break;
  default:
    break;
  }
  
  glutIdleFunc(idle_w);
  glClearColor(1.0,1.0,1.0,1.0);

  glutMainLoop();
}

