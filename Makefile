CC		= gcc
CFLAGS		= -O2
OPENGLLIBS	= -lglut -lGLU -lGL -lm
V4L2LIBS	= -lv4l2
PTHREADLIBS	= -lpthread

all:cricket_main

.SUFFIXES:.c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

v4l2_mmap_test:v4l2_mmap_test.o
	$(CC) $^ -o $@ $(OPENGLLIBS) $(V4L2LIBS) $(PTHREADLIBS)

cricket_main.o:cricket_header.h
capture.o:cricket_header.h v4l2_mmap.h
graphics.o:cricket_header.h
cricket_main:cricket_main.o capture.o graphics.o
	$(CC) $^ -o $@ $(OPENGLLIBS) $(V4L2LIBS) $(PTHREADLIBS)
	