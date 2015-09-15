#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include<GL/glut.h>
#include<GL/glu.h>
#include<GL/gl.h>

#ifndef MY_GLOBES
#define MY_GLOBES

#define RIGHT 1
#define TOP 2
#define LEFT 3
#define BOTTOM 4
#define DOTS_SIZE 64

#define CALIBRATION 1
#define CALI_FWWW 2
#define CALI_FBWW 3
#define CALI_FWWB 4
#define CALI_FBWB 5
#define CALI_ACT 6

// global variables shared among processes
struct globes{
  ////////////////////////
  // physical parametes //
  ////////////////////////
  double wall_disp_width;
  double wall_disp_height;
  double wall_bezel_right; // following to the display's relative coordinate
  double wall_bezel_left;
  double wall_bezel_top;
  double wall_bezel_bottom;
  double floor_disp_width;
  double floor_disp_height;
  double floor_bezel_right;
  double floor_bezel_left;
  double floor_bezel_top;
  double floor_bezel_bottom;
  double eye_height;
  ////////////////////////
  ////////////////////////

  unsigned char endstate;
  double criX;
  double criY;
  double criA;
  double objX;
  double objY;
  unsigned char mode;

} *globes;

void init_globes();

void init_cap_globes();
void init_v4l2_stats();
void uninit_v4l2_stats();
void capturing_thread(pthread_t*);
void processing_thread(pthread_t*);
void environment_thread(pthread_t*);
void monitoring_window();
void floor_graphics(int, int, char*);
void wall_graphics(unsigned char, int, int, char*);

#endif
