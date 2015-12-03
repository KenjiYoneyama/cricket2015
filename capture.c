#include "cricket_header.h"
#include "v4l2_mmap.h"
#include<math.h>
#include<string.h>

//#define CAP_FPS
//#define PRC_FPS

#define WIDTH_MIN 88
#define WIDTH_MAX 546
#define HEIGHT_MIN 16
#define HEIGHT_MAX 474


int dead_line=10;
unsigned char ss_button=0;

struct cap_globes{
  unsigned char red_thresh;
  unsigned char green_thresh;

  int filter[CAM_HEIGHT*CAM_WIDTH][2];
  int filter_n;

  int height_min, height_max, width_min, width_max;
  
  FILE *fp;

} *cap_globes;

void init_cap_globes(){
  FILE *fp;
  char line[255];
  int i,j;
  double params;

  time_t t;
  char fname[255];

  cap_globes=(struct cap_globes*)malloc(sizeof(struct cap_globes));

  if((fp=fopen("cap_settings.txt", "r"))==NULL){
    printf("FOE:cap_settings.txt\n");
    exit(EXIT_FAILURE);
  }
  while(fscanf(fp, "%s%lf", line, &params) != EOF){
    if(strcmp("RED_THRESH", line)==0){
      cap_globes->red_thresh=params;
    }else if(strcmp("GREEN_THRESH", line)==0){
      cap_globes->green_thresh=params;
    }
  }
  fclose(fp);

  if(globes->filter){
    cap_globes->filter_n=0;
    if((fp=fopen("filter.txt", "r"))==NULL){
      printf("FOE:filter.txt\n");
      exit(EXIT_FAILURE);
    }
    cap_globes->width_min=CAM_WIDTH/2;
    cap_globes->width_max=CAM_WIDTH/2;
    cap_globes->height_min=CAM_HEIGHT/2;
    cap_globes->height_min=CAM_HEIGHT/2;
    while(fscanf(fp, "%d %d", &j, &i) != EOF){
      cap_globes->filter[cap_globes->filter_n][0]=i;
      cap_globes->filter[cap_globes->filter_n][1]=j;
      cap_globes->filter_n++;
      /*
      if(i<cap_globes->width_min) cap_globes->width_min=i;
      if(i>cap_globes->width_max) cap_globes->width_max=i;
      if(j<cap_globes->height_min) cap_globes->height_min=j;
      if(i>cap_globes->height_max) cap_globes->height_max=j;
      */
    }
    fclose(fp);
    cap_globes->width_min=WIDTH_MIN;
    cap_globes->width_max=WIDTH_MAX;
    cap_globes->height_min=HEIGHT_MIN;
    cap_globes->height_max=HEIGHT_MAX;
  }else{
    cap_globes->filter_n=0;
    for(j=0;j<CAM_HEIGHT;j++){
      for(i=0;i<CAM_WIDTH;i++){
	cap_globes->filter[cap_globes->filter_n][0]=i;
	cap_globes->filter[cap_globes->filter_n][1]=j;
	cap_globes->filter_n++;
      }
    }
    cap_globes->width_min=0;
    cap_globes->width_max=CAM_WIDTH;
    cap_globes->height_min=0;
    cap_globes->height_min=CAM_HEIGHT;
  }
  /*
  printf("width_min:%d\n", cap_globes->width_min);
  printf("width_max:%d\n", cap_globes->width_max);
  printf("height_min:%d\n", cap_globes->height_min);
  printf("height_max:%d\n", cap_globes->height_max);
  */
  if(globes->record){
    t = time(NULL);
    strftime(fname, sizeof(fname), "./record/%Y-%m-%d-%H:%M:%S.txt", localtime(&t));
    printf("%s\n", fname);
    if((cap_globes->fp=fopen(fname, "w"))==NULL){
      printf("FOE: recording file\n");
      exit(EXIT_FAILURE);
    }
  }
}

void uninit_cap_globes(){
  if(globes->record){
    fclose(cap_globes->fp);
  }
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

void my_erosion(unsigned char out_map[CAM_HEIGHT][CAM_WIDTH],
		int out_list[CAM_HEIGHT*CAM_WIDTH][2], int *out_n,
		unsigned char in_map[CAM_HEIGHT][CAM_WIDTH],
		int in_list[CAM_HEIGHT*CAM_WIDTH][2], int in_n){
  int pix_n, i, j;
  for(j=0;j<CAM_HEIGHT;j++){
    for(i=0;i<CAM_WIDTH;i++){
      out_map[j][i]=0;
    }
  }
  *out_n=0;
  for(pix_n=0;pix_n<in_n;pix_n++){
    i=in_list[pix_n][0];
    j=in_list[pix_n][1];
    if(i==0 || i==CAM_WIDTH-1 || j==0 || j==CAM_HEIGHT-1
       || in_map[j-1][i]==0
       || in_map[j][i-1]==0
       || in_map[j][i+1]==0
       || in_map[j+1][i]==0
       || in_map[j-1][i-1]==0
       || in_map[j-1][i+1]==0
       || in_map[j+1][i-1]==0
       || in_map[j*1][i+1]==0)
      continue;
    else{
      out_map[j][i]=1;
      out_list[(*out_n)][0]=i;
      out_list[(*out_n)][1]=j;
      (*out_n)++;
    }
  }
}

void my_dilation(unsigned char out_map[CAM_HEIGHT][CAM_WIDTH],
		int out_list[CAM_HEIGHT*CAM_WIDTH][2], int *out_n,
		unsigned char in_map[CAM_HEIGHT][CAM_WIDTH],
		int in_list[CAM_HEIGHT*CAM_WIDTH][2], int in_n){
  int pix_n, i, j;
  for(j=0;j<CAM_HEIGHT;j++){
    for(i=0;i<CAM_WIDTH;i++){
      out_map[j][i]=0;
    }
  }
  *out_n=0;
  for(pix_n=0;pix_n<in_n;pix_n++){
    i=in_list[pix_n][0];
    j=in_list[pix_n][1];

    if(out_map[j][i]==0){
      out_map[j][i]=1;
      out_list[(*out_n)][0]=i;
      out_list[(*out_n)][1]=j;
      (*out_n)++;
    }
    if(i!=0 && out_map[j][i-1]==0){
      out_map[j][i-1]=1;
      out_list[(*out_n)][0]=i-1;
      out_list[(*out_n)][1]=j;
      (*out_n)++;
    }
    if(i!=CAM_WIDTH-1 && out_map[j][i+1]==0){
      out_map[j][i+1]=1;
      out_list[(*out_n)][0]=i+1;
      out_list[(*out_n)][1]=j;
      (*out_n)++;
    }
    if(j!=0 && out_map[j-1][i]==0){
      out_map[j-1][i]=1;
      out_list[(*out_n)][0]=i;
      out_list[(*out_n)][1]=j-1;
      (*out_n)++;
    }
    if(j!=CAM_HEIGHT-1 && out_map[j+1][i]==0){
      out_map[j+1][i]=1;
      out_list[(*out_n)][0]=i;
      out_list[(*out_n)][1]=j+1;
      (*out_n)++;
    }
    if(j!=0 && i!=0 && out_map[j-1][i-1]==0){
      out_map[j-1][i-1]=1;
      out_list[(*out_n)][0]=i-1;
      out_list[(*out_n)][1]=j-1;
      (*out_n)++;
    }
    if(j!=0 && i!=CAM_WIDTH-1 && out_map[j-1][i+1]==0){
      out_map[j-1][i+1]=1;
      out_list[(*out_n)][0]=i+1;
      out_list[(*out_n)][1]=j-1;
      (*out_n)++;
    }
    if(j!=CAM_HEIGHT-1 && i!=0 && out_map[j+1][i-1]==0){
      out_map[j+1][i-1]=1;
      out_list[(*out_n)][0]=i-1;
      out_list[(*out_n)][1]=j+1;
      (*out_n)++;
    }
    if(j!=CAM_HEIGHT-1 && i!=CAM_WIDTH-1 && out_map[j+1][i+1]==0){
      out_map[j+1][i+1]=1;
      out_list[(*out_n)][0]=i+1;
      out_list[(*out_n)][1]=j+1;
      (*out_n)++;
    }
  }
}

int fill_pond(unsigned char out_map[CAM_HEIGHT][CAM_WIDTH],
	      int out_list[CAM_HEIGHT*CAM_WIDTH][2], int *out_n,
	      unsigned char in_map[CAM_HEIGHT][CAM_WIDTH],
	      int i, int j){

  if(i<0 || i>=CAM_WIDTH || j<0 || j>=CAM_HEIGHT
     || in_map[j][i]==0 || out_map[j][i]==1){
    return 0;
  }

  out_map[j][i]=1;

  return 1;
  printf("%d\n", *out_n);


  out_list[*(out_n)][0]=i;
  out_list[*(out_n)][1]=j;
  *(out_n)=*(out_n)+1;
  return 1 
    + fill_pond(out_map, out_list, out_n, in_map, i-1, j) 
    + fill_pond(out_map, out_list, out_n, in_map, i+1, j)
    + fill_pond(out_map, out_list, out_n, in_map, i, j-1)
    + fill_pond(out_map, out_list, out_n, in_map, i, j+1);

}

void largest_pond(unsigned char out_map[CAM_HEIGHT][CAM_WIDTH],
		int out_list[CAM_HEIGHT*CAM_WIDTH][2], int *out_n,
		unsigned char in_map[CAM_HEIGHT][CAM_WIDTH],
		int in_list[CAM_HEIGHT*CAM_WIDTH][2], int in_n){
  
  int pix_n, i, j, ii, jj, filling_n, m, max_n, max_m;
  
  unsigned char filling_map[CAM_HEIGHT][CAM_WIDTH];
  
  unsigned char pond_maps[4][CAM_HEIGHT][CAM_WIDTH];
  int pond_lists[4][CAM_HEIGHT*CAM_WIDTH][2];
  int pond_ns[4];
  
  for(j=0;j<CAM_HEIGHT;j++){
    for(i=0;i<CAM_WIDTH;i++){
      filling_map[j][i]=0;
    }
  }
  for(m=0;m<4;m++){
    for(j=0;j<CAM_HEIGHT;j++){
      for(i=0;i<CAM_WIDTH;i++){
	pond_maps[m][j][i]=0;
      }
    }
    pond_ns[m]=0;
  }
  
    
  for(pix_n=0;pix_n<in_n;pix_n++){
    i=in_list[pix_n][0];
    j=in_list[pix_n][1];
    if(filling_map[j][i]==0){
      
      fill_pond(pond_maps[0], pond_lists[0], &(pond_ns[0]), in_map, i, j);
      for(filling_n=0;filling_n<pond_ns[0];filling_n++){
	i=pond_lists[0][filling_n][0];
	j=pond_lists[0][filling_n][1];
	filling_map[j][i]=1;
      
      }
      
    }
  }
  max_n=0;
  max_m=0;
  for(m=0;m<4;m++){
    if(pond_ns[m]>max_n){
      max_n=pond_ns[m];
      max_m=m;
    }
  }
  for(j=0;j<CAM_HEIGHT;j++){
    for(i=0;i<CAM_WIDTH;i++){
      out_map[j][i]=pond_maps[max_m][j][i];
    }
  }
  *out_n=0;
  for(pix_n=0;pix_n<pond_ns[max_m];pix_n++){
    out_list[*out_n][0]=pond_lists[max_m][*out_n][0];
    out_list[*out_n][1]=pond_lists[max_m][*out_n][1];
    (*out_n)=(*out_n)+1;
  }
  
}

static void image_processing(){
  int pix_n, i, j, k, t, x0, x1;
  int y, u, v, r=0, g=0, b=0;
  unsigned char* yuyv=(unsigned char*)buffers->start;
  unsigned char nred_map[CAM_HEIGHT][CAM_WIDTH]={{}};  // 'not' red points
  unsigned char green_map[CAM_HEIGHT][CAM_WIDTH]={{}}; // green points
  unsigned char tmp_map[CAM_HEIGHT][CAM_WIDTH]; // for smoothing
  int nred_list[CAM_HEIGHT*CAM_WIDTH][2];   // 'not' red points
  int green_list[CAM_HEIGHT*CAM_WIDTH][2];  // green points
  int tmp_list[CAM_HEIGHT*CAM_WIDTH][2];
  int nred_n=0, green_n=0, tmp_n;
  FILE *fp, *fp_cal;
  static int count_ss=0;
  char line[255];
  unsigned char loc_ss_button=0;

  static int wb_list[7][CAM_HEIGHT][CAM_WIDTH]={0};
  static int wb_count=1500;

  double criX=0, criY=0, criA;
  double marX=0, marY=0;
  double s11=0, s20=0, s02=0;
  static double pre_s[10][3]={{},{},{},{},{},{},{},{},{},{}};
  double sin_av=0, cos_av=0;

  struct timeval rc_t, st_t, pr_t;
  static unsigned char first_frame=1;
  double max_g=0;
  int max_g_i, max_g_j;

  if(first_frame && globes->record){
    first_frame=0;
    gettimeofday(&st_t, NULL); // start time
  }

  if(ss_button) loc_ss_button=1;
  if(loc_ss_button==1){
    sprintf(line, "ScreenShot%d.txt", count_ss++);
    fp=fopen(line, "w");
  }
  for(pix_n=0;pix_n<cap_globes->filter_n;pix_n++){
    i=cap_globes->filter[pix_n][0];
    j=cap_globes->filter[pix_n][1];

    y=yuyv[(j*CAM_WIDTH+i)*2];
    u=yuyv[((j*CAM_WIDTH+i)/2)*4+1]-128;  // even:Yuyv odd:yuYv
    v=yuyv[((j*CAM_WIDTH+i)/2)*4+3]-128;
    /*
    r=(1.0*(y-16)/219
       +1.602*v/224)*255;       // red, the color of the floor and dots
    */
    r=y+1.40*v;
    /*
    g=(1.0*(y-16)/219
       -0.344*u/224
       -0.714*v/224)*255;       // green, marker
    */
    /*
    g=(1.0*(y-16)/219
       -0.34*u/224
       +0.9*v/224)*255;
    */
    g=4.0*y-0.34*u-0.714*v;
    /*    b=(1.0*(y-16)/219
       +1.77*u/224)*255;        // blue, marker
    */
    r=(r>255)?255:(r<0)?0:r; 
    g=(g>255)?255:(g<0)?0:g; 
    //    b=(b>255)?255:(b<0)?0:b;
    switch(globes->mode){
    case CALIBRATION:
      if(i==0 && j==0 && !(--wb_count)){
	printf("CALI_FWWW\n");
	wb_count=1500;
	globes->mode=CALI_FWWW;
      }
      break;
    case CALI_FWWW:
      if(i==0 && j==0 && !(--wb_count)){
	printf("CALI_FBWW\n");
	wb_count=1500;
	globes->mode=CALI_FBWW;
      }
      if(wb_count==10) wb_list[0][j][i]=y;
      break;
    case CALI_FBWW:
      if(i==0 && j==0 && !(--wb_count)){
	printf("CALI_FWWB\n");
	wb_count=1500;
	globes->mode=CALI_FWWB;
      }
      if(wb_count==10) wb_list[1][j][i]=y;
      break;
    case CALI_FWWB:
      if(i==0 && j==0 && !(--wb_count)){
	printf("CALI_ACT\n");
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

    if(r>cap_globes->red_thresh){
      nred_map[j][i]=0;
      green_map[j][i]=0;
    }else{
      nred_map[j][i]=1;
      nred_list[nred_n][0]=i;
      nred_list[nred_n][1]=j;
      nred_n++;
      if(g>max_g){
	max_g_i=i;
	max_g_j=j;
	max_g=g;
      }
      if(g>cap_globes->green_thresh){
	green_map[j][i]=1;
	green_list[green_n][0]=i;
	green_list[green_n][1]=j;
	green_n++;
      }else{
	green_map[j][i]=0;
      }	
    }
    /*
    cam_globes.y_bits[j][i][0]=r;
    cam_globes.y_bits[j][i][1]=g;
    cam_globes.y_bits[j][i][2]=b;
    
    cam_globes.y_bits[j+400][i+700][0]=b;
    cam_globes.y_bits[j+400][i+700][1]=b;
    cam_globes.y_bits[j+400][i+700][2]=b;
    */
    if(loc_ss_button==1){
      fprintf(fp, "%d %d %d\n", y, u, v);
    }

  }
  // closing
  my_dilation(tmp_map, tmp_list, &tmp_n, nred_map, nred_list, nred_n);
  my_erosion(nred_map, nred_list, &nred_n, tmp_map, tmp_list, tmp_n); 
  
  // opening
  my_erosion(tmp_map, tmp_list, &tmp_n, nred_map, nred_list, nred_n);
  my_dilation(nred_map, nred_list, &nred_n, tmp_map, tmp_list, tmp_n);
  
  // largest pond
  //  largest_pond(nred_map, nred_list, &nred_n, tmp_map, tmp_list, tmp_n);
  
  // closing
  my_dilation(tmp_map, tmp_list, &tmp_n, green_map, green_list, green_n);
  my_erosion(green_map, green_list, &green_n, tmp_map, tmp_list, tmp_n); 

  // opening  
  my_erosion(tmp_map, tmp_list, &tmp_n, green_map, green_list, green_n);
  my_dilation(green_map, green_list, &green_n, tmp_map, tmp_list, tmp_n);

  // select the largest mass of dots
  /*
  largest_pond(tmp_map, tmp_list, &tmp_n, nred_map, nred_list, nred_n);
  memcpy(nred_map, tmp_map, (CAM_WIDTH*CAM_HEIGHT));
  memcpy(nred_list, tmp_list, (CAM_WIDTH*CAM_HEIGHT)*2);
  nred_n=tmp_n;
  */

  for(pix_n=0;pix_n<nred_n;pix_n++){
    i=nred_list[pix_n][0];
    j=nred_list[pix_n][1];
    criX+=i;
    criY+=j;
  }
  criX = (nred_n>0) ? 1.0*criX/nred_n : CAM_WIDTH;
  criY = (nred_n>0) ? 1.0*criY/nred_n : CAM_HEIGHT;

  for(pix_n=0;pix_n<nred_n;pix_n++){
    i=nred_list[pix_n][0]-criX;
    j=nred_list[pix_n][1]-criY;
    s11+=i*j;
    s20+=j*j;
    s02+=i*i;
  }
  int true_green_n=0;
  for(pix_n=0;pix_n<green_n;pix_n++){
    i=green_list[pix_n][0];
    j=green_list[pix_n][1];
    if(nred_map[j][i]){
      marX+=i;
      marY+=j;
      true_green_n++;
    }
  }
  marX = (true_green_n>0) ? 1.0*marX/true_green_n : CAM_WIDTH;
  marY = (true_green_n>0) ? 1.0*marY/true_green_n : CAM_HEIGHT;

  for(i=0;i<9;i++){
    pre_s[i][0]=pre_s[i+1][0];
    pre_s[i][1]=pre_s[i+1][1];
    pre_s[i][2]=pre_s[i+1][2];
  }
  pre_s[9][0]=s11;
  pre_s[9][1]=s02;
  pre_s[9][2]=s20;
  s11=0;
  s02=0;
  s20=0;
  for(i=0;i<9;i++){
    s11+=pre_s[i][0];
    s02+=pre_s[i][1];
    s20+=pre_s[i][2];
  }
  s11=1.0*s11/10;
  s02=1.0*s02/10;
  s20=1.0*s20/10;

  criA=(atan2(2*s11, s02-s20))/2.0;
  if(globes->marker){
    if((criX*cos(criA)+criY*sin(criA))
       -(marX*cos(criA)+marY*sin(criA))>0){
      criA+=M_PI;
    }
  }else{
    if(cos((-criA)-globes->criA)<0){
      if(criA>0) criA-=M_PI;
      else criA+=M_PI;
    }
  }

  /*
  for(i=0;i<9;i++){
    pre_angle[i]=pre_angle[i+1];
  }
  pre_angle[9]=criA;

  for(i=0;i<10;i++){
    sin_av+=sin(pre_angle[i]);
    cos_av+=cos(pre_angle[i]);
  }
  sin_av=1.0*sin_av/10;
  cos_av=1.0*cos_av/10;
  criA=atan2(sin_av, cos_av);
  */
  //printf("%f %f\n", marX, marY);
  // copy to globes
  if(globes->black){
    globes->criX=245;
    globes->criY=245;
    globes->criA=0;
  }else{
    globes->criX=globes->floor_bezel_left+(criX-cap_globes->width_min)*globes->floor_disp_width/(cap_globes->width_max-cap_globes->width_min)+globes->eye_pos*cos(-criA);
    globes->criY=globes->floor_bezel_bottom+globes->floor_disp_height-(criY-cap_globes->height_min)*globes->floor_disp_height/(cap_globes->height_max-cap_globes->height_min)+globes->eye_pos*sin(-criA);
    globes->criA=-criA;
  }
  // write to recording file
  if(globes->record){
    gettimeofday(&pr_t, NULL); // present time
    timersub(&pr_t, &st_t, &rc_t);
    fprintf(cap_globes->fp, "%f %f %f %f %f %f\n", rc_t.tv_sec+rc_t.tv_usec*0.000001, globes->criX, globes->criY, globes->criA, globes->objX, globes->objY);
  }

  // show on monitoring window
  
  for(pix_n=0;pix_n<cap_globes->filter_n;pix_n++){
    i=cap_globes->filter[pix_n][0];
    j=cap_globes->filter[pix_n][1];

    y=yuyv[(j*CAM_WIDTH+i)*2];
    u=yuyv[((j*CAM_WIDTH+i)/2)*4+1]-128;
    v=yuyv[((j*CAM_WIDTH+i)/2)*4+3]-128;
    
    cam_globes.y_bits[j+500][i+700][0]=u+128;
    cam_globes.y_bits[j+500][i+700][1]=u+128;
    cam_globes.y_bits[j+500][i+700][2]=u+128;

    if(i==(int)marX && j==(int)marY){
      cam_globes.y_bits[j+500][i][0]=0;
      cam_globes.y_bits[j+500][i][1]=0;
      cam_globes.y_bits[j+500][i][2]=0xff;      
    }else if(nred_map[j][i]){
      if(green_map[j][i]){
	cam_globes.y_bits[j+500][i][0]=0;
	cam_globes.y_bits[j+500][i][1]=0xff;
	cam_globes.y_bits[j+500][i][2]=0;
      }else{
	cam_globes.y_bits[j+500][i][0]=0xff;
	cam_globes.y_bits[j+500][i][1]=0;
	cam_globes.y_bits[j+500][i][2]=0;
      }
    }else{
      r=(1.0*(y-16)/219
	 +1.602*v/224)*255;
      r=(r>255)?255:(r<0)?0:r; 

      cam_globes.y_bits[j+500][i][0]=r;
      cam_globes.y_bits[j+500][i][1]=r;
      cam_globes.y_bits[j+500][i][2]=r;
    }
    /*    
    if(green_map[j][i]){
      cam_globes.y_bits[j+500][i][0]=0;
      cam_globes.y_bits[j+500][i][1]=0xff;
      cam_globes.y_bits[j+500][i][2]=0;
    }else{
      g=(1.0*(y-16)/219
	 -0.344*u/224
	 -0.714*v/224)*255;
      g=(g>255)?255:(g<0)?0:g; 

      cam_globes.y_bits[j+400][i+350][0]=g;
      cam_globes.y_bits[j+400][i+350][1]=g;
      cam_globes.y_bits[j+400][i+350][2]=g;
    }
    */
    /*
    g=(1.0*(y-16)/219
       -0.34*u/224
       +0.9*v/224)*255;
    */
    /*    if((i-max_g_i)*(i-max_g_i)+(j-max_g_j)*(j-max_g_j)<100){
      cam_globes.y_bits[j+1000][i+700][0]=0;
      cam_globes.y_bits[j+1000][i+700][1]=0xff;
      cam_globes.y_bits[j+1000][i+700][2]=0;
      }else*/{
      r=y*2+1.402*(v);
      g=y*2-0.344*(u)-0.714*(v);
      b=y*2+1.77*(u);
      
      r=(r>255)?255:(r<0)?0:r;
      g=(g>255)?255:(g<0)?0:g;
      b=(b>255)?255:(b<0)?0:b;
      
      cam_globes.y_bits[j+1000][i+700][0]=r;
      cam_globes.y_bits[j+1000][i+700][1]=g;
      cam_globes.y_bits[j+1000][i+700][2]=b;
    }
  }
  for(k=-30;k<30;k++){
    cam_globes.y_bits[(int)(criY+sin(criA)*k)+500][(int)(criX+cos(criA)*k)][1]=0xff;
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
  glMatrixMode(GL_MODELVIEW);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glEnable(GL_TEXTURE_2D);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, cam_globes.y_bits);
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

  glPushMatrix();
  {
    glColor3d(0.0, 1.0, 0.0);
    glTranslatef(0.7, -0.3, 0.0);
    glRotatef(90-globes->criA*180/3.14, 0.0, 0.0, 1.0);
    glScalef(0.1, 0.1, 0.1);
    glBegin(GL_TRIANGLES);
    {
      glVertex3d(0.0, 1.0, 0.0);
      glVertex3d(-0.5, -1.0, 0.0);
      glVertex3d(0.5, -1.0, 0.0);
    }
    glEnd();
  }
  glPopMatrix();

  glutSwapBuffers();
}

void idle0(){
  if(globes->endstate==1) exit(0);
  glutPostRedisplay();
}

void key0(unsigned char key, int x, int y){
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
  case 'l':
    globes->looming=LOOMING_SLOW;
    break;
  case 'L':
    globes->looming=LOOMING_FAST;
    break;
  case 'i':
    globes->looming_angle=LOOMING_LEFT;
    break;
  case 'o':
    globes->looming_angle=LOOMING_FRONT;
    break;
  case 'p':
    globes->looming_angle=LOOMING_RIGHT;
    break;
  case 'a':
    if(globes->mode!=A_TRACK) globes->mode=A_TRACK;
    else globes->mode=0;
  default:
    break;
  }
}

void specialkey(int key, int x ,int y){
  switch(key){
  case GLUT_KEY_UP:
    if(globes->criA>=0) globes->criA-=M_PI;
    break;
  case GLUT_KEY_DOWN:
    if(globes->criA<0) globes->criA+=M_PI;
    break;
  case GLUT_KEY_LEFT:
    if(globes->criA>=M_PI/2 || globes->criA<-M_PI/2) globes->criA+=M_PI;
    break;
  case GLUT_KEY_RIGHT:
    if(globes->criA>=-M_PI/2 && globes->criA<M_PI/2) globes->criA+=M_PI;
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
  glutSpecialFunc(specialkey);
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
