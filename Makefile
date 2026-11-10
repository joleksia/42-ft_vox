# ========

# root directory of the makefile
MK_ROOT	= $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

# ========

# compiler
CC = cc

# compiler flags
CFLAGS = -Wall -Wextra -Werror -std=c99 -ggdb3

# linker files
LFLAGS = -l:libSDL2.so \
		 -l:libm.so

# linker directories
LDFLAGS = # ...

# include directories
IFLAGS = -I $(MK_ROOT)glld.h	\
		 -I $(MK_ROOT)algebra.h	\
		 -I $(MK_ROOT)noise.h	\
		 -I $(MK_ROOT)/inc		\

# preprocessor definitions
DFLAGS = -DMESA_DEBUG 

# ========

OBJS = $(SRCS:.c=.o)
SRCS = $(MK_ROOT)src/deps.c				\
	   $(MK_ROOT)src/ft_vox.c			\
	   $(MK_ROOT)src/ft_vox-shader.c	\
	   $(MK_ROOT)src/ft_vox-chunk.c		\
	   $(MK_ROOT)src/ft_vox-world.c		\
	   $(MK_ROOT)src/ft_vox-camera.c	\

# ========

TARGET = $(MK_ROOT)ft_vox

# ========

.PHONY : all

all : $(TARGET)

.PHONY : clean

clean :
	rm -f $(TARGET)
	rm -f $(OBJS)

# ========

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LFLAGS)

$(OBJS) : %.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $< $(IFLAGS)

# ========
