
BIN := status1234

CC := cc


SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

_SRC := sblock.c block/stime.c block/swifi.c block/saudio.c block/smic.c block/sbattery.c block/ssysinfo.c block/squotes.c \
		main.c sbar.c util.c

SRC := $(addprefix $(SRC_DIR)/, $(_SRC))


INCLUDE := -I$(SRC_DIR) -I/usr/include -I/usr/X11R6/include
LIB := -L/usr/lib -lc -L/usr/X11R6/lib -lX11 -lm -lasound

CFLAGS := -g -O2 -pedantic -Wall -Wextra ${INCLUDE}
LDFLAGS := -g ${LIB}


INSTALL_PREFIX := /usr
