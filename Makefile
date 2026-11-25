# ========

# root directory of the makefile
MK_ROOT	= $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

# ========

# compiler
CC = cc

# compiler flags
CFLAGS = -Wall -Wextra -Werror -std=c99 -ggdb3 -O0 -fsanitize=address

# linker files
LFLAGS = -l:libSDL2.so 		\
		 -l:libm.so			\

# linker directories
LDFLAGS = # ...

# include directories
IFLAGS = -I $(MK_ROOT)glld.h	\
		 -I $(MK_ROOT)algebra.h	\
		 -I $(MK_ROOT)noise.h	\
		 -I $(MK_ROOT)stb		\
		 -I $(MK_ROOT)inc		\

# preprocessor definitions
DFLAGS = # ... 

# ========

OBJS = $(SRCS:.c=.o)
SRCS = $(MK_ROOT)src/deps.c				\
	   $(MK_ROOT)src/ft_vox.c			\
	   $(MK_ROOT)src/ft_vox-state.c		\
	   $(MK_ROOT)src/ft_vox-window.c	\
	   $(MK_ROOT)src/ft_vox-input.c		\
	   $(MK_ROOT)src/ft_vox-shader.c	\
	   $(MK_ROOT)src/ft_vox-sampler.c	\
	   $(MK_ROOT)src/ft_vox-entity.c	\
	   $(MK_ROOT)src/ft_vox-thread.c	\
	   $(MK_ROOT)src/ft_vox-camera.c	\
	   $(MK_ROOT)src/ft_vox-chunk.c		\
	   $(MK_ROOT)src/ft_vox-world.c		\
	   $(MK_ROOT)src/ft_vox-worldgen.c	\
	   $(MK_ROOT)src/ft_vox-block.c		\

SRCS += $(MK_ROOT)src/blocks/ft_vox-air.c	\
		$(MK_ROOT)src/blocks/ft_vox-stone.c	\
		$(MK_ROOT)src/blocks/ft_vox-grass.c	\
		$(MK_ROOT)src/blocks/ft_vox-dirt.c	\
		$(MK_ROOT)src/blocks/ft_vox-water.c	\

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
