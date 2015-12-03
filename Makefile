CC		= gcc
CFLAGS		= -O2
OPENGLLIBS	= -lglut -lGLU -lGL -lm
V4L2LIBS	= -lv4l2
PTHREADLIBS	= -lpthread
OPENCVCFLAGS = `pkg-config --cflags opencv`
OPENCVLIBS   = `pkg-config --libs opencv`

all:cricket_main

.SUFFIXES:.c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

v4l2_mmap_test:v4l2_mmap_test.o
	$(CC) $^ -o $@ $(OPENGLLIBS) $(V4L2LIBS) $(PTHREADLIBS) -g

cricket_main.o:cricket_header.h
capture.o:cricket_header.h v4l2_mmap.h
graphics.o:cricket_header.h
environment.o:cricket_header.h
cricket_main:cricket_main.o capture.o graphics.o environment.o
	$(CC) $^ -o $@ $(OPENGLLIBS) $(V4L2LIBS) $(PTHREADLIBS)
	
