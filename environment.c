#include<math.h>
#include<GL/glut.h>
#include<GL/glu.h>
#include<GL/gl.h>

#include "cricket_header.h"

//#define ENV_FPS


void draw_sphere(double oX, double oY, double oZ){
  glPushMatrix();
  {
    if(globes->col)
      glColor3d(1.0, 0.0, 1.0);
    else if(globes->black){
      glColor3d(0.0, 0.0, 0.0);
    }else{
      glColor3d(1.0, 0.0, 0.0);
    }
    glTranslatef(oX, oY, oZ);
    glutSolidSphere(globes->obj_size, 18, 24);
  }
  glPopMatrix();
}
void start_mark(){
  static int t=0;
  if(globes->render!=OBJECT_ON){
    t=0;
    return;
  }
  if(t>30 ){
    return;
  }
  t++;
  
  double cX=globes->criX;
  double cY=globes->criY;
  double cA=globes->criA;
  glPushMatrix();
  {
    glColor3d(1.0, 0.0, 1.0);
    //    glTranslatef(cX-50*cos(cA), cY-50*sin(cA), 0.0);
    glTranslatef(470, 20, 0);
    glBegin(GL_QUADS);
    {
      glVertex3d(5, 5, 0.0);
      glVertex3d(-5, 5, 0.0);
      glVertex3d(-5, -5, 0.0);
      glVertex3d(5, -5, 0.0);
    }
    glEnd();
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
  case A_TRACK:
    if(globes->render==OBJECT_ON){
      draw_sphere(globes->objX, globes->objY, globes->objZ);
    }
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
  case A_TRACK:
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
  static double d=0.0, e=0.0;
  static double criX=1, criY=1, criA=1;
  double ang=0.0;
  /*
  if(globes->render==OBJECT_OFF){
    criA=globes->criA;
  }
  */
  if(globes->mode==A_TRACK){
    criX=globes->criX;
    criY=globes->criY;
    criA=globes->criA;
    ang=(e<180)?(e):(360-e);
    globes->objX=criX+150*cos(criA-M_PI/2+ang*M_PI/180);
    globes->objY=criY+150*sin(criA-M_PI/2+ang*M_PI/180);
    e=e+50.0/60;
    if(e>360)e-=360;
    globes->render=OBJECT_ON;
    return;
  }
  if(d==0 && globes->looming!=LOOMING_OFF){
    criX=globes->criX;
    criY=globes->criY;
    criA=globes->criA;
    d=globes->obj_home_distance;
  }
  if(d>0 && (globes->looming==LOOMING_SLOW || globes->looming==LOOMING_FAST)){
    if(globes->looming_angle==LOOMING_FRONT){
      globes->objX=criX+d*cos(criA);
      globes->objY=criY+d*sin(criA);
    }else if(globes->looming_angle==LOOMING_LEFT){
      globes->objX=criX+d*cos(criA+M_PI/2);
      globes->objY=criY+d*sin(criA+M_PI/2);
    }else if(globes->looming_angle==LOOMING_RIGHT){
      globes->objX=criX+d*cos(criA-M_PI/2);
      globes->objY=criY+d*sin(criA-M_PI/2);
    }
    globes->objZ=d*sin(globes->obj_angle*M_PI/180);
    globes->render=OBJECT_ON;
  }
  if(d>0 && globes->looming==LOOMING_SLOW){
    d-=globes->obj_velocity/60.0;
  }
  if(d>0 && globes->looming==LOOMING_FAST){
    d-=3*globes->obj_velocity/60.0;
  }
  if(d<=0){
    globes->render=OBJECT_OFF;
    d=0;
    globes->looming=LOOMING_OFF;
  }
  /*
  double centX=globes->criX, centY=globes->criY;
  
  static int t=0;
  if(++t>150) t=-150;
  int r=(t>0)?1:-1;
  globes->objX=centX+300*cos(-3.14*0.5-0.03*t*r)+300*sin(-3.14*0.5-0.03*t*r);
  globes->objY=centY-300*sin(-3.14*0.5-0.03*t*r)+300*cos(-3.14*0.5-0.03*t*r);
  */
  /*
  globes->objX=0;
  globes->objY=0;
  globes->objZ=10*(200-t%200);
  */
  /*
  globes->objX=centX+300*cos(globes->criA);
  globes->objY=centY+300*sin(globes->criA);
  globes->col=(globes->col+1)%2;
  */
  return;
}

static void *env_loop(){
#ifdef ENV_FPS
  struct timeval st_t, en_t, sub_t;
  int xxx=0;
  gettimeofday(&st_t, NULL);
  gettimeofday(&en_t, NULL);
#endif
  int i;
  while(globes->endstate==0){
    while(!globes->disp_update[0]
       || !globes->disp_update[1]
       || !globes->disp_update[2]
       || !globes->disp_update[3]
       || !globes->disp_update[4]){
      usleep(1000);
      }
    
    env_control();
    for(i=0;i<5;i++){
      globes->disp_update[i]=0;
    }


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
