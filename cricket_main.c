#include "cricket_header.h"
void init_globes(){
  int psize = getpagesize();
  globes = mmap(0, (sizeof(struct globes)/psize+1)*psize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);

  FILE *fp;
  char line[255];
  double params;

  if ((fp=fopen("physical_parameters.txt", "r"))==NULL){
    printf("FOE:physical_parameters.txt\n");
    exit(EXIT_FAILURE);
  }
  while(fscanf(fp, "%s%lf", line, &params) != EOF){
    if(strcmp("WALL_DISP_WIDTH", line)==0){
      globes->wall_disp_width=params;
    }else if(strcmp("WALL_DISP_HEIGHT", line)==0){
      globes->wall_disp_height=params;
    }else if(strcmp("WALL_BEZEL_RIGHT", line)==0){
      globes->wall_bezel_right=params;
    }else if(strcmp("WALL_BEZEL_LEFT", line)==0){
      globes->wall_bezel_left=params;
    }else if(strcmp("WALL_BEZEL_TOP", line)==0){
      globes->wall_bezel_top=params;
    }else if(strcmp("WALL_BEZEL_BOTTOM", line)==0){
      globes->wall_bezel_bottom=params;
    }else if(strcmp("FLOOR_DISP_WIDTH", line)==0){
      globes->floor_disp_width=params;
    }else if(strcmp("FLOOR_DISP_HEIGHT", line)==0){
      globes->floor_disp_height=params;
    }else if(strcmp("FLOOR_BEZEL_RIGHT", line)==0){
      globes->floor_bezel_right=params;
    }else if(strcmp("FLOOR_BEZEL_LEFT", line)==0){
      globes->floor_bezel_left=params;
    }else if(strcmp("FLOOR_BEZEL_TOP", line)==0){
      globes->floor_bezel_top=params;
    }else if(strcmp("FLOOR_BEZEL_BOTTOM", line)==0){
      globes->floor_bezel_bottom=params;
    }else if(strcmp("EYE_HEIGHT", line)==0){
      globes->eye_height=params;
    }else if(strcmp("EYE_POS", line)==0){
      globes->eye_pos=params;
    }else if(strcmp("OBJ_HOME_DISTANCE", line)==0){
      globes->obj_home_distance=params;
    }else if(strcmp("OBJ_VELOCITY", line)==0){
      globes->obj_velocity=params;
    }else if(strcmp("OBJ_SIZE", line)==0){
      globes->obj_size=params;
    }else if(strcmp("OBJ_ANGLE", line)==0){
      globes->obj_angle=params;
    }
  }
  fclose(fp);
  globes->endstate=0;
  globes->criX=245;
  globes->criY=245;
  globes->objX=-100;
  globes->objY=250;
  globes->objZ=0;
  globes->mode=0;
  globes->filter=0;
  globes->col=0;
  globes->disp_update[0]=0;
  globes->disp_update[1]=0;
  globes->disp_update[2]=0;
  globes->disp_update[3]=0;
  globes->record=0;
  globes->render=OBJECT_OFF;
  globes->looming=LOOMING_OFF;
  globes->looming_angle=LOOMING_FRONT;
  globes->black=0;
  globes->marker=0;
}

int main(int argc, char **argv){

  
  init_globes();

  if(fork()){   // capturing and processing
    pthread_t cap_thread, proc_thread, env_thread;
    int options;

    while((options = getopt(argc, argv, "acrfbm"))!=-1){
      switch(options){
      case 'a':
	globes->mode=A_TRACK;
	printf("ANTENNA_TRACKING_MODE\n");
	break;
      case 'c':
	globes->mode=CALIBRATION;
	printf("CALIBRATION_MODE\n");
	break;
      case 'f':
	globes->filter=1;
	printf("FILTER_MODE\n");
	break;
      case 'r':
	globes->record=1;
	printf("RECORDING_MODE\n");
	break;
      case 'b':
	globes->black=1;
	break;
	printf("BLACK_MODE\n");
      case 'm':
	globes->marker=1;
	break;
	printf("MARKER_MODE\n");
      default:
	break;
      }
    }

    init_cap_globes();
    init_v4l2_stats();

    capturing_thread(&cap_thread);
    processing_thread(&proc_thread);
    environment_thread(&env_thread);
    monitoring_window();

    pthread_join(cap_thread, NULL);
    uninit_v4l2_stats();
    uninit_cap_globes();
  }else if(fork()){     // floor
    floor_graphics(1920, 1920, ":0.5");
  }else if(fork()){     // walls
    wall_graphics(RIGHT, 1920, 1080, ":0.1");
  }else if(fork()){
    wall_graphics(TOP, 1920, 1080, ":0.2");
  }else if(fork()){
    wall_graphics(LEFT, 1920, 1080, ":0.3");
  }else if(fork()){
    wall_graphics(BOTTOM, 1920, 1080, ":0.4");
  }
  return 0;
}
