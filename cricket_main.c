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
    }
  }
  fclose(fp);
  globes->endstate=0;
  globes->criX=245;
  globes->criY=245;
  globes->objX=-100;
  globes->objY=250;
  globes->mode=0;
}

int main(int argc, char **argv){

  
  init_globes();

  if(fork()){   // capturing and processing
    pthread_t cap_thread, proc_thread, env_thread;
    int options;

    while((options = getopt(argc, argv, "c"))!=-1){
      switch(options){
      case 'c':
	globes->mode=CALIBRATION;
	break;
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
