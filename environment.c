#include<math.h>
#include<GL/glut.h>
#include<GL/glu.h>
#include<GL/gl.h>

#include "cricket_header.h"

//#define ENV_FPS

void draw_sphere(double oX, double oY){
  glPushMatrix();
  {
    glColor3d(1.0, 0.0, 0.0);
    glTranslatef(oX, oY, 0.0);
    glutSolidSphere(100, 18, 24);
  }
  glPopMatrix();
}

void obj_fnc(){
  /*
  double oX=300;
  double oY=0;
  static int ba=0;
  ba+=1;
  ba=ba%300;
  draw_sphere(oX-ba, oY+ba);
  */
  switch(globes->mode){
  case 0:
    draw_sphere(globes->objX, globes->objY);
    break;
  case CALI_FBWW:
    glPushMatrix();
    {
      glColor3d(0.0, 0.0, 0.0);
      glBegin(GL_QUADS);
      glVertex3d(0,0,0);
      glVertex3d(0,490,0);
      glVertex3d(490,490,0);
      glVertex3d(490,0,0);
      glEnd();
    }
    glPopMatrix();
    break;
  case CALI_FWWB:
    glPushMatrix();
    {
      glColor3d(0.0, 0.0, 0.0);
      glBegin(GL_QUADS);
      glVertex3d(0,0,0);
      glVertex3d(0,490,0);
      glVertex3d(0,490,490);
      glVertex3d(0,0,490);

      glVertex3d(0,490,0);
      glVertex3d(490,490,0);
      glVertex3d(490,490,490);
      glVertex3d(0,490,490);

      glVertex3d(490,490,0);
      glVertex3d(490,0,0);
      glVertex3d(490,0,490);
      glVertex3d(490,490,490);

      glVertex3d(490,0,0);
      glVertex3d(0,0,0);
      glVertex3d(0,0,490);
      glVertex3d(490,0,490);
      glEnd();

      glColor3d(1.0, 1.0, 1.0);
      glBegin(GL_QUADS);
      glVertex3d(0,0,0);
      glVertex3d(0,490,0);
      glVertex3d(490,490,0);
      glVertex3d(490,0,0);
      glEnd();
    }
    glPopMatrix();
    break;
  case CALI_FBWB:
    glPushMatrix();
    {
      glColor3d(0.0, 0.0, 0.0);
      glBegin(GL_QUADS);
      glVertex3d(0,0,0);
      glVertex3d(0,490,0);
      glVertex3d(490,490,0);
      glVertex3d(490,0,0);

      glVertex3d(0,0,0);
      glVertex3d(0,490,0);
      glVertex3d(0,490,490);
      glVertex3d(0,0,490);

      glVertex3d(0,490,0);
      glVertex3d(490,490,0);
      glVertex3d(490,490,490);
      glVertex3d(0,490,490);

      glVertex3d(490,490,0);
      glVertex3d(490,0,0);
      glVertex3d(490,0,490);
      glVertex3d(490,490,490);

      glVertex3d(490,0,0);
      glVertex3d(0,0,0);
      glVertex3d(0,0,490);
      glVertex3d(490,0,490);
      glEnd();
    }
    glPopMatrix();
    break;
  default:
    break;
  }
}

void dots_fnc(GLubyte pattern[DOTS_SIZE][DOTS_SIZE][3]){
  switch(globes->mode){
  case 0:
    glEnable(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, pattern);
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
    break;
  default:
    break;
  }
}

static void env_control(){
  double centX=globes->criX, centY=globes->criY;
  double oX=globes->objX-centX;
  double oY=globes->objY-centY;

  globes->objX=centX+oX*cos(0.01)+oY*sin(0.01);
  globes->objY=centY-oX*sin(0.01)+oY*cos(0.01);

  return;
}

static void *env_loop(){
#ifdef ENV_FPS
  struct timeval st_t, en_t, sub_t;
  int xxx=0;
  gettimeofday(&st_t, NULL);
  gettimeofday(&en_t, NULL);
#endif

  while(globes->endstate==0){
    env_control();
    usleep(10000);
#ifdef ENV_FPS
    if(xxx++>200){
      xxx=0;
      gettimeofday(&en_t, NULL);
      timersub(&en_t, &st_t, &sub_t);
      printf("env_control_fps:%d\n",
	     (int)(200.0/((double)sub_t.tv_sec+0.000001*(double)sub_t.tv_usec)));
      gettimeofday(&st_t, NULL);
    }
#endif
  
  }
}

void environment_thread(pthread_t* env_thread){
  if(pthread_create(env_thread, NULL, env_loop, NULL) != 0)
    perror("pthread_create()");
}
