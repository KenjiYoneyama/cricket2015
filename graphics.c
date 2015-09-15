#include<GL/glut.h>
#include<GL/glu.h>
#include<GL/gl.h>
#include<stdlib.h>
#include<sys/time.h>

#include "cricket_header.h"

struct grph_globes{
  GLubyte random_dots[DOTS_SIZE][DOTS_SIZE][3];
} *grph_globes;

void init_grph_globes(){
  srand(28);
  int i,j;
  grph_globes=(struct grph_globes*)malloc(sizeof(struct grph_globes));
  for(i=0;i<DOTS_SIZE;i++){
    for(j=0;j<DOTS_SIZE;j++){
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

void disp_f(){
  // get information of the cricket's position
  double cX=globes->criX;
  double cY=globes->criY;
  double eye=globes->eye_height;

  double cX_max=globes->floor_disp_width+globes->floor_bezel_left+globes->floor_bezel_right;
  double cY_max=globes->floor_disp_height+globes->floor_bezel_top+globes->floor_bezel_bottom;
  double DEP=1.0;

  double left=-(cX_max-cX-globes->floor_bezel_left)*DEP/(eye);
  double right=(cX-globes->floor_bezel_right)*DEP/(eye);
  double bottom=-(cY_max-cY-globes->floor_bezel_bottom)*DEP/(eye);
  double top=(cY-globes->floor_bezel_top)*DEP/(eye);

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

  dots_fnc(grph_globes->random_dots);
  obj_fnc();

  glutSwapBuffers();
}

void disp_wt(){
  // get information of the cricket's position
  double cX=globes->criX;
  double cY=globes->criY;
  double eye=globes->eye_height;

  double cX_max=globes->floor_disp_width+globes->floor_bezel_left+globes->floor_bezel_right;
  double cY_max=globes->floor_disp_height+globes->floor_bezel_top+globes->floor_bezel_bottom;
  double DEP=10.0;

  double left=(cX_max-cX-globes->wall_bezel_left)*DEP/(cY_max-cY);
  double right=-(cX-globes->wall_bezel_right)*DEP/(cY_max-cY);
  double bottom=(globes->wall_disp_height+globes->wall_bezel_top-eye)*DEP/(cY_max-cY);
  double top=-(eye-globes->wall_bezel_top)*DEP/(cY_max-cY);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  glFrustum(left,
	    right,
	    bottom,
	    top,
	    DEP,
	    20000);
  glRotatef(-90, 1.0, 0.0, 0.0);
  glTranslatef(-cX, -cY, -eye);
 
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  obj_fnc();

  glutSwapBuffers();
}

void disp_wb(){
  // get information of the cricket's position
  double cX=globes->criX;
  double cY=globes->criY;
  double eye=globes->eye_height;

  double cX_max=globes->floor_disp_width+globes->floor_bezel_left+globes->floor_bezel_right;
  double cY_max=globes->floor_disp_height+globes->floor_bezel_top+globes->floor_bezel_bottom;
  double DEP=10.0;

  double left=(cX-globes->wall_bezel_left)*DEP/(cY);
  double right=-(cX_max-cX-globes->wall_bezel_right)*DEP/(cY);
  double bottom=(globes->wall_disp_height+globes->wall_bezel_top-eye)*DEP/(cY);
  double top=-(eye-globes->wall_bezel_top)*DEP/(cY);

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
  glTranslatef(-cX, -cY, -eye);
 
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  obj_fnc();

  glutSwapBuffers();
}

void disp_wr(){
  // get information of the cricket's position
  double cX=globes->criX;
  double cY=globes->criY;
  double eye=globes->eye_height;

  double cX_max=globes->floor_disp_width+globes->floor_bezel_left+globes->floor_bezel_right;
  double cY_max=globes->floor_disp_height+globes->floor_bezel_top+globes->floor_bezel_bottom;
  double DEP=10.0;

  double left=(cY-globes->wall_bezel_left)*DEP/(cX_max-cX);
  double right=-(cY_max-cY-globes->wall_bezel_right)*DEP/(cX_max-cX);
  double bottom=(globes->wall_disp_height+globes->wall_bezel_top-eye)*DEP/(cX_max-cX);
  double top=-(eye-globes->wall_bezel_top)*DEP/(cX_max-cX);

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
  glTranslatef(-cX, -cY, -eye);
 
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  obj_fnc();

  glutSwapBuffers();
}

void disp_wl(){
  // get information of the cricket's position
  double cX=globes->criX;
  double cY=globes->criY;
  double eye=globes->eye_height;

  double cX_max=globes->floor_disp_width+globes->floor_bezel_left+globes->floor_bezel_right;
  double cY_max=globes->floor_disp_height+globes->floor_bezel_top+globes->floor_bezel_bottom;
  double DEP=10.0;

  double left=(cY_max-cY-globes->wall_bezel_left)*DEP/(cX_max-cX);
  double right=-(cY-globes->wall_bezel_right)*DEP/(cX_max-cX);
  double bottom=(globes->wall_disp_height+globes->wall_bezel_top-eye)*DEP/(cX_max-cX);
  double top=-(eye-globes->wall_bezel_top)*DEP/(cX_max-cX);

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
  glTranslatef(-cX, -cY, -eye);
 
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  obj_fnc();

  glutSwapBuffers();
}

void floor_graphics(int width, int height, char *dsp_no){
  // display (floor)
  init_grph_globes();
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

