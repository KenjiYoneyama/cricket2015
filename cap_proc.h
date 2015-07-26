#include "v4l2_mmap.h"

int dead_line=10;

static void *cap_loop(){

#ifdef CAP_FPS
  struct timeval st_t, en_t, sub_t;
  int xxx=0;
  gettimeofday(&st_t, NULL);
  gettimeofday(&en_t, NULL);
#endif

  while(cam_globes.endstate==0){
    cap_one_frame();

#ifdef CAP_FPS
    if(xxx++>100){
      xxx=0;
      gettimeofday(&en_t, NULL);
      timersub(&en_t, &st_t, &sub_t);
      printf("cap_fps:%d\n",
	     (int)(100.0/((double)sub_t.tv_sec+0.000001*(double)sub_t.tv_usec)));
      gettimeofday(&st_t, NULL);
    }
#endif

  }
}

static void capturing_thread(pthread_t* cap_thread){
  if(pthread_create(cap_thread, NULL, cap_loop, NULL) != 0)
    perror("pthread_create()");
}

static void image_processing(){
  int i, j, k, t, x0, x1;
  int y, u, v, r=0, g=0, b=0;
  unsigned char* yuyv=(unsigned char*)buffers->start;
  for(j=0;j<CAM_HEIGHT;j++){
    for(i=0;i<CAM_WIDTH;i++){
      y=yuyv[(j*CAM_WIDTH+i)*2];
      u=yuyv[((j*CAM_WIDTH+i)/2)*4+1]-128;  // even:Yuyv odd:yuYv
      v=yuyv[((j*CAM_WIDTH+i)/2)*4+3]-128;

      r=(1.0*(y-16)/219
	 +1.602*v/224)*255;       // red, the color of the floor and dots
      g=(1.0*(y-16)/219
      	 -0.344*u/224
      	 -0.714*v/224)*255;       // green, no need
      
      /*
      b=50-(( (y-29>0) ? y-29 : -y+29 )*0.6
       	    +( (u-14>0) ? u-14 : -u+14 )*1.5
	    + ( (v+6>0) ? v+6 : -v-6 )*1.2);  // blue, the color of the marker
      */
      b=(1.0*(y-16)/219
	 +1.77*u/224)*255;

      r=(r>255)?255:(r<0)?0:r; 
      g=(g>255)?255:(g<0)?0:g; 
      b=(b>255)?255:(b<0)?0:b; 

      cam_globes.y_bits[j][i][0]=r;
      cam_globes.y_bits[j][i][1]=g;
      cam_globes.y_bits[j][i][2]=b;

      cam_globes.y_bits[j+400][i][0]=r;
      cam_globes.y_bits[j+400][i][1]=r;
      cam_globes.y_bits[j+400][i][2]=r;
      cam_globes.y_bits[j+400][i+350][0]=g;
      cam_globes.y_bits[j+400][i+350][1]=g;
      cam_globes.y_bits[j+400][i+350][2]=g;
      cam_globes.y_bits[j+400][i+700][0]=b;
      cam_globes.y_bits[j+400][i+700][1]=b;
      cam_globes.y_bits[j+400][i+700][2]=b;

    }
  }
}

static void *proc_loop(){
  while(cam_globes.endstate==0){
    image_processing();
  }
}

static void processing_thread(pthread_t* proc_thread){
  if(pthread_create(proc_thread, NULL, proc_loop, NULL) != 0)
    perror("pthread_create()");
}
      

void disp0(){
  glClear(GL_COLOR_BUFFER_BIT);
  
  glEnable(GL_TEXTURE_2D);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, cam_globes.y_bits);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  
  glBegin(GL_QUADS);
  {
    glTexCoord2f(0.0, 0.0);
    glVertex2f(1, 1);
    glTexCoord2f(0.0, 1.0);
    glVertex2f(1, -1);
    glTexCoord2f(1.0, 1.0);
    glVertex2f(-1, -1);
    glTexCoord2f(1.0, 0.0);
    glVertex2f(-1, 1);
  }
  glEnd();
  
  glDisable(GL_TEXTURE_2D);

  glutSwapBuffers();
}

void idle0(){
  if(cam_globes.endstate==1) exit(0);
  glutPostRedisplay();
}

void key0(unsigned char key, int x, int y){
  double angle, dist;
  switch(key){
  case 'q':
  case 'Q':
  case '\033':
    cam_globes.endstate=1;
    break;
  default:
    break;
  }
}

void monitoring_window(){
  int fake_argc=3;
  char *fake_argv[]={"a", "-display", ":0.0"};
  glutInit(&fake_argc, fake_argv);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(1024, 1024);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutCreateWindow("aaaa");
  glutKeyboardFunc(key0);
  glutDisplayFunc(disp0);
  glutIdleFunc(idle0);
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glutMainLoop();
}
