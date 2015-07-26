#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include<GL/glut.h>
#include<GL/glu.h>
#include<GL/gl.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define CAM_WIDTH (320)
#define CAM_HEIGHT (240)
//#define SLEEP 1

struct buffer {
  void   *start;
  size_t  length;
};

static char             dev_name[]="/dev/video0";
static int              fd = -1;
struct buffer          *buffers;
static unsigned int     n_buffers;

struct cam_globes{
  unsigned char endstate;
  GLubyte y_bits[1024][1024][3];
} cam_globes;

static void init_globes(){
  int i,j,k;
  cam_globes.endstate=0;
  for(i=0;i<1024;i++){
    for(j=0;j<1024;j++){
      for(k=0;k<3;k++){
	cam_globes.y_bits[i][j][k]=0;
      }
    }
  }

}

static void errno_exit(const char *s)
{
  fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
  exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg)
{
  int r;

  do {
	r = ioctl(fh, request, arg);
  } while (-1 == r && EINTR == errno);

  return r;
}

static int read_frame(void)
{
  struct v4l2_buffer buf;
  //unsigned int i;

       
  CLEAR(buf);

  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
	switch (errno) {
	case EAGAIN:
	  return 0;

	case EIO:
	  /* Could ignore EIO, see spec. */

	  /* fall through */

	default:
	  errno_exit("VIDIOC_DQBUF");
	}
  }

  assert(buf.index < n_buffers);

  if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
	errno_exit("VIDIOC_QBUF");

  return 1;
}

static void cap_one_frame(void)
{
  for (;;) {
    fd_set fds;
    struct timeval tv;
    int r;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    /* Timeout. */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    r = select(fd + 1, &fds, NULL, NULL, &tv);

    if (-1 == r) {
      if (EINTR == errno)
		continue;
      errno_exit("select");
    }

    if (0 == r) {
      fprintf(stderr, "select timeout\n");
      exit(EXIT_FAILURE);
    }

    if (read_frame())
      break;
    /* EAGAIN - continue select loop. */
  }

}


static void stop_streaming(void)
{
  enum v4l2_buf_type type;
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
    errno_exit("VIDIOC_STREAMOFF");
}

static void start_streaming(void)
{
  unsigned int i;
  enum v4l2_buf_type type;
 
  for (i = 0; i < n_buffers; ++i) {
	struct v4l2_buffer buf;

	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = i;

	if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
	  errno_exit("VIDIOC_QBUF");
  }
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
	errno_exit("VIDIOC_STREAMON");

}

static void uninit_device(void)
{
  unsigned int i;

  for (i = 0; i < n_buffers; ++i)
    if (-1 == munmap(buffers[i].start, buffers[i].length))
      errno_exit("munmap");
  free(buffers);
}

static void init_read(unsigned int buffer_size)
{
  buffers = calloc(1, sizeof(*buffers));

  if (!buffers) {
	fprintf(stderr, "Out of memory\n");
	exit(EXIT_FAILURE);
  }

  buffers[0].length = buffer_size;
  buffers[0].start = malloc(buffer_size);

  if (!buffers[0].start) {
	fprintf(stderr, "Out of memory\n");
	exit(EXIT_FAILURE);
  }
}

static void init_mmap(void)
{
  struct v4l2_requestbuffers req;

  CLEAR(req);

  req.count = 1;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
	if (EINVAL == errno) {
	  fprintf(stderr, "%s does not support "
			  "memory mapping\n", dev_name);
	  exit(EXIT_FAILURE);
	} else {
	  errno_exit("VIDIOC_REQBUFS");
	}
  }

  if (0 && req.count < 2) {
	fprintf(stderr, "Insufficient buffer memory on %s\n",
			dev_name);
	exit(EXIT_FAILURE);
  }

  buffers = calloc(req.count, sizeof(*buffers));

  if (!buffers) {
	fprintf(stderr, "Out of memory\n");
	exit(EXIT_FAILURE);
  }

  for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
	struct v4l2_buffer buf;

	CLEAR(buf);

	buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory      = V4L2_MEMORY_MMAP;
	buf.index       = n_buffers;

	if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
	  errno_exit("VIDIOC_QUERYBUF");

	buffers[n_buffers].length = buf.length;
	buffers[n_buffers].start =
	  mmap(NULL /* start anywhere */,
		   buf.length,
		   PROT_READ | PROT_WRITE /* required */,
		   MAP_SHARED /* recommended */,
		   fd, buf.m.offset);

	if (MAP_FAILED == buffers[n_buffers].start)
	  errno_exit("mmap");
  }
}

static void init_device(void)
{
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  unsigned int min;

  if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
	if (EINVAL == errno) {
	  fprintf(stderr, "%s is no V4L2 device\n",
			  dev_name);
	  exit(EXIT_FAILURE);
	} else {
	  errno_exit("VIDIOC_QUERYCAP");
	}
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
	fprintf(stderr, "%s is no video capture device\n",
			dev_name);
	exit(EXIT_FAILURE);
  }

  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
	fprintf(stderr, "%s does not support streaming i/o\n",
			dev_name);
	exit(EXIT_FAILURE);
  }

  CLEAR(cropcap);

  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c = cropcap.defrect; /* reset to default */

	if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
	  switch (errno) {
	  case EINVAL:
		/* Cropping not supported. */
		break;
	  default:
		/* Errors ignored. */
		break;
	  }
	}
  } else {
	/* Errors ignored. */
  }

  CLEAR(fmt);

  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  fmt.fmt.pix.width       = CAM_WIDTH;
  fmt.fmt.pix.height      = CAM_HEIGHT;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

  if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
    errno_exit("VIDIOC_S_FMT");

  /* Buggy driver paranoia. */
  min = fmt.fmt.pix.width * 2;
  if (fmt.fmt.pix.bytesperline < min)
	fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if (fmt.fmt.pix.sizeimage < min)
	fmt.fmt.pix.sizeimage = min;
	
  init_mmap();
}

static void close_device(void)
{
  if (-1 == close(fd))
	errno_exit("close");

  fd = -1;
}

static void open_device(void)
{
  struct stat st;

  if (-1 == stat(dev_name, &st)) {
	fprintf(stderr, "Cannot identify '%s': %d, %s\n",
			dev_name, errno, strerror(errno));
	exit(EXIT_FAILURE);
  }

  if (!S_ISCHR(st.st_mode)) {
	fprintf(stderr, "%s is no device\n", dev_name);
	exit(EXIT_FAILURE);
  }

  fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

  if (-1 == fd) {
	fprintf(stderr, "Cannot open '%s': %d, %s\n",
			dev_name, errno, strerror(errno));
	exit(EXIT_FAILURE);
  }
}

int dead_line=10;

static void *cap_loop(){
  while(cam_globes.endstate==0){
    cap_one_frame();
  }
}

static void capturing_thread(pthread_t* cap_thread){
  if(pthread_create(cap_thread, NULL, cap_loop, NULL) != 0)
    perror("pthread_create()");
}

static void image_processing(){
  int i, j, k, x0, x1;
  int y, u, v, r=0, g=0, b=0;

  

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

int main(){
  pthread_t cap_thread;
  
  init_globes();

  open_device();
  init_device();
  start_streaming();
  capturing_thread(&cap_thread);
  
  monitoring_window();

  pthread_join(cap_thread, NULL);
  stop_streaming();
  uninit_device();
  close_device();
  return 0;
}
