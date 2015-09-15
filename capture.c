#include "cricket_header.h"
#include "v4l2_mmap.h"
#include<math.h>
#include<string.h>

//#define CAP_FPS
//#define PRC_FPS

int dead_line=10;
unsigned char ss_button=0;

struct cap_globes{
  int left_blank;
  int left_bezel;
  int right_blank;
  int right_bezel;
  int top_blank;
  int top_bezel;
  int bottom_blank;
  int bottom_bezel;
  unsigned char red_thresh;
  unsigned char blue_thresh;
  unsigned char red_thresh_dark;
  unsigned char blue_thresh_dark;
  
} *cap_globes;

void init_cap_globes(){
  FILE *fp;
  char line[255];
  double params;
  cap_globes=(struct cap_globes*)malloc(sizeof(struct cap_globes));
  if((fp=fopen("cap_settings.txt", "r"))==NULL){
    printf("FOE:cap_settings.txt\n");
    exit(EXIT_FAILURE);
  }

  while(fscanf(fp, "%s%lf", line, &params) != EOF){
    if(strcmp("LEFT_BLANK", line)==0){
      cap_globes->left_blank=params;
    }else if(strcmp("LEFT_BEZEL", line)==0){
      cap_globes->left_bezel=params;
    }else if(strcmp("RIGHT_BLANK", line)==0){
      cap_globes->right_blank=params;
    }else if(strcmp("RIGHT_BEZEL", line)==0){
      cap_globes->right_bezel=params;
    }else if(strcmp("TOP_BLANK", line)==0){
      cap_globes->top_blank=params;
    }else if(strcmp("TOP_BEZEL", line)==0){
      cap_globes->top_bezel=params;
    }else if(strcmp("BOTTOM_BLANK", line)==0){
      cap_globes->bottom_blank=params;
    }else if(strcmp("BOTTOM_BEZEL", line)==0){
      cap_globes->bottom_bezel=params;
    }else if(strcmp("RED_THRESH", line)==0){
      cap_globes->red_thresh=params;
    }else if(strcmp("BLUE_THRESH", line)==0){
      cap_globes->blue_thresh=params;
    }else if(strcmp("RED_THRESH_DARK", line)==0){
      cap_globes->red_thresh_dark=params;
    }else if(strcmp("BLUE_THRESH_DARK", line)==0){
      cap_globes->blue_thresh_dark=params;
    }
  }
  fclose(fp);
}

static void *cap_loop(){

#ifdef CAP_FPS
  struct timeval st_t, en_t, sub_t;
  int xxx=0;
  gettimeofday(&st_t, NULL);
  gettimeofday(&en_t, NULL);
#endif

  while(globes->endstate==0){
    cap_one_frame();

#ifdef CAP_FPS
    if(xxx++>200){
      xxx=0;
      gettimeofday(&en_t, NULL);
      timersub(&en_t, &st_t, &sub_t);
      printf("cap_fps:%d\n",
	     (int)(200.0/((double)sub_t.tv_sec+0.000001*(double)sub_t.tv_usec)));
      gettimeofday(&st_t, NULL);
    }
#endif

  }
}

void capturing_thread(pthread_t* cap_thread){
  if(pthread_create(cap_thread, NULL, cap_loop, NULL) != 0)
    perror("pthread_create()");
}

static void image_processing(){
  int i, j, k, t, x0, x1;
  int y, u, v, r=0, g=0, b=0;
  unsigned char* yuyv=(unsigned char*)buffers->start;
  unsigned char red_map[CAM_HEIGHT][CAM_WIDTH];
  unsigned char red_map_sm[CAM_HEIGHT][CAM_WIDTH];  // smoothed
  unsigned char blue_map[CAM_HEIGHT][CAM_WIDTH];
  unsigned char blue_map_sm[CAM_HEIGHT][CAM_WIDTH]; // smoothed
  FILE *fp, *fp_cal;
  static int count_ss=0;
  char line[255];
  int red_flag, blue_flag;
  unsigned char loc_ss_button=0;

  static int wb_list[7][CAM_HEIGHT][CAM_WIDTH]={0};
  static int wb_count=1500;

  if(ss_button) loc_ss_button=1;
  if(loc_ss_button==1){
    sprintf(line, "ScreenShot%d.txt", count_ss++);
    fp=fopen(line, "w");
  }
  for(j=0;j<CAM_HEIGHT;j++){
    for(i=0;i<CAM_WIDTH;i++){
      y=yuyv[(j*CAM_WIDTH+i)*2];
      u=yuyv[((j*CAM_WIDTH+i)/2)*4+1]-128;  // even:Yuyv odd:yuYv
      v=yuyv[((j*CAM_WIDTH+i)/2)*4+3]-128;

      r=(1.0*(y-16)/219
	 +1.602*v/224)*255;       // red, the color of the floor and dots
      g=(1.0*(y-16)/219
      	 -0.344*u/224
      	 -0.714*v/224)*255;       // green, marker
      b=(1.0*(y-16)/219
	 +1.77*u/224)*255;        // blue, marker
     
      r=(r>255)?255:(r<0)?0:r; 
      g=(g>255)?255:(g<0)?0:g; 
      b=(b>255)?255:(b<0)?0:b; 

      switch(globes->mode){
      case CALIBRATION:
	if(i==0 && j==0 && !(--wb_count)){
	  wb_count=1500;
	  globes->mode=CALI_FWWW;
	}
	break;
      case CALI_FWWW:
	if(i==0 && j==0 && !(--wb_count)){
	  wb_count=1500;
	  globes->mode=CALI_FBWW;
	}
	if(wb_count==10) wb_list[0][j][i]=y;
	break;
      case CALI_FBWW:
	if(i==0 && j==0 && !(--wb_count)){
	  wb_count=1500;
	  globes->mode=CALI_FWWB;
	}
	if(wb_count==10) wb_list[1][j][i]=y;
	break;
      case CALI_FWWB:
	if(i==0 && j==0 && !(--wb_count)){
	  wb_count=1500;
	  globes->mode=CALI_ACT;
	}
	if(wb_count==10) wb_list[2][j][i]=y;
	break;
      case CALI_ACT:
	wb_list[4][j][i]=(wb_list[0][j][i]-wb_list[1][j][i]>20)?0xff:0;
	wb_list[5][j][i]=(wb_list[2][j][i]>70)?0xff:0;
	cam_globes.y_bits[j][i+350][0]=wb_list[4][j][i];
	cam_globes.y_bits[j][i+350][1]=wb_list[5][j][i];
	cam_globes.y_bits[j][i+700][0]=(wb_list[4][j][i] && wb_list[5][j][i])?0xff:0;
	if(i==0 && j==0 && !(--wb_count)){
	  wb_count=1500;
	  globes->mode=0;
	}
	if(wb_count==1400){
	  if(i==0 && j==0){
	    if((fp_cal=fopen("filter.txt", "w"))==NULL){
	      printf("FOE:filter.txt\n");
	      exit(EXIT_FAILURE);
	    }
	  }
	  if(wb_list[4][j][i] && wb_list[5][j][i]){
	    fprintf(fp_cal, "%d %d\n", j, i);
	  }
	}
	if(wb_count==1399 && i==0 && j==0){
	  fclose(fp_cal);
	}
	
	break;
      default:
	break;
      }

      red_flag=0;
      blue_flag=0;

      if(j<cap_globes->top_blank || j>CAM_HEIGHT-cap_globes->bottom_blank 
	 || i<cap_globes->left_blank || i>CAM_WIDTH-cap_globes->right_blank){
	red_map[j][i]=1;
	blue_map[j][i]=0;
      }else if(j<cap_globes->top_bezel || j>CAM_HEIGHT-cap_globes->bottom_bezel 
	       || i<cap_globes->left_bezel || i>CAM_WIDTH-cap_globes->right_bezel){
	if(r>=cap_globes->red_thresh_dark){
	  /* TODO */
	  red_map[j][i]=1;
	  if(b>cap_globes->blue_thresh_dark){
	    /* TODO */ 
	    blue_map[j][i]=1;
	  }else{
	    blue_map[j][i]=0;
	  }

	}else{
	  red_map[j][i]=0;
	  blue_map[j][i]=0;
	}

      }else{
	if(r>cap_globes->red_thresh){
	  /* TODO */ 
	  red_map[j][i]=1;
	  if(b>cap_globes->blue_thresh){
	    /* TODO */
	    blue_map[j][i]=1;
	  }else{
	    blue_map[j][i]=0;
	  }	

	}else{
	  red_map[j][i]=0;
	  blue_map[j][i]=0;
	}
      }

      cam_globes.y_bits[j][i][0]=r;
      cam_globes.y_bits[j][i][1]=g;
      cam_globes.y_bits[j][i][2]=b;

      if(red_map[j][i]){
	cam_globes.y_bits[j+400][i][0]=r;
	cam_globes.y_bits[j+400][i][1]=r;
	cam_globes.y_bits[j+400][i][2]=r;
      }else{
	cam_globes.y_bits[j+400][i][0]=0xff;
	cam_globes.y_bits[j+400][i][1]=0;
	cam_globes.y_bits[j+400][i][2]=0;
      }
      cam_globes.y_bits[j+400][i+350][0]=g;
      cam_globes.y_bits[j+400][i+350][1]=g;
      cam_globes.y_bits[j+400][i+350][2]=g;
      if(red_map[j][i] && blue_map[j][i]){
	cam_globes.y_bits[j+400][i+700][0]=0;
	cam_globes.y_bits[j+400][i+700][1]=0;
	cam_globes.y_bits[j+400][i+700][2]=0xff;
      }else{
	cam_globes.y_bits[j+400][i+700][0]=b;
	cam_globes.y_bits[j+400][i+700][1]=b;
	cam_globes.y_bits[j+400][i+700][2]=b;
      }
      if(j==cap_globes->top_blank || j==CAM_HEIGHT-cap_globes->bottom_blank
	 || i==cap_globes->left_blank || i==CAM_WIDTH-cap_globes->right_blank){
	cam_globes.y_bits[j][i][1]=0xff;
      }

      if(loc_ss_button==1){
	fprintf(fp, "%d %d %d\n", y, u, v);
      }

    }
  }
  if(loc_ss_button==1)
    fclose(fp);
  if(loc_ss_button==1){
    ss_button=0;
    loc_ss_button=0;
  }
}

static void *proc_loop(){
#ifdef PRC_FPS
  struct timeval st_t, en_t, sub_t;
  int xxx=0;
  gettimeofday(&st_t, NULL);
  gettimeofday(&en_t, NULL);
#endif

  while(globes->endstate==0){
    image_processing();
    usleep(10);
#ifdef PRC_FPS
    if(xxx++>200){
      xxx=0;
      gettimeofday(&en_t, NULL);
      timersub(&en_t, &st_t, &sub_t);
      printf("process_fps:%d\n",
	     (int)(200.0/((double)sub_t.tv_sec+0.000001*(double)sub_t.tv_usec)));
      gettimeofday(&st_t, NULL);
    }
#endif


  }
}

void processing_thread(pthread_t* proc_thread){
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
  if(globes->endstate==1) exit(0);
  glutPostRedisplay();
}

void key0(unsigned char key, int x, int y){
  double angle, dist;
  switch(key){
  case 'q':
  case 'Q':
  case '\033':
    globes->endstate=1;
    cam_globes.endstate=1;
    break;
  case 's':
    ss_button=1;
    break;
  case 'I':
    init_cap_globes();
    break;
  default:
    break;
  }
}

void monitoring_window(){
  int fake_argc=3;
  char *fake_argv[]={"a", "-display", ":0.0"};
  glutInit(&fake_argc, fake_argv);
  glutInitWindowPosition(100,0);
  glutInitWindowSize(1024, 1024);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutCreateWindow("aaaa");
  glutKeyboardFunc(key0);
  glutDisplayFunc(disp0);
  glutIdleFunc(idle0);
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glutMainLoop();
}

void init_v4l2_stats(){
  init_cam_globes();
  open_device();
  init_device();
  start_streaming();
}
void uninit_v4l2_stats(){
  stop_streaming();
  uninit_device();
  close_device();
}
