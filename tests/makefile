# ---variables---
INCLUDE = ../include
SRC = ../src
LIB = ../lib
EXE = main

# ---dependencies---
SLD2 = libsdl2-2.0-0 libsdl2-dbg libsdl2-dev libsdl2-image-2.0-0 \
	libsdl2-image-dbg libsdl2-image-dev

# ---Rule Structure---
CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -pedantic
# THIS BELOW INCLUDRES AND LDLIBS ON XML CAN BE REMOVED
INCLUDES = -I $(INCLUDE) `sdl2-config --cflags`
LIBS = 
LDLIBS = $(LIBS) `sdl2-config --libs`
LDFLAGS = -L$(LIB)

COMPILE.c = $(CC) $(CFLAGS) $(INCLUDES) $(CPPFLAGS) $(TARGET_ARCH) -c

# ---Object files---
OBJS = emulator.o cartridge.o bus.o cpu.o op.o ram.o io.o stack.o ui.o \
	transfer.o interrupts.o timer.o ppu.o dma.o
.DELETE_ON_ERROR: $(OBJS)

# ---vpath---
vpath %.c $(LIB)
vpath %.h $(INCLUDE)

# ---commands---
RM = rm -rf
CP = cp

# ---targets---
all: DEPENDENCIES $(EXE)

fast: CFLAGS = -g -Wextra -Werror -pedantic -Ofast
fast: all

DEPENDENCIES:
	$(call install_dependency, $(SDL2))
	
$(EXE): $(EXE).o $(OBJS)

# include dependencies created below
include $(subst .o,.d,$(EXE).o $(OBJS))

# trick to create dependencies, see make book page 33
%.d: %.c
	$(CC) -M $(INCLUDES) $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	$(RM) $@.$$$$

.PHONY: clean
clean:
	$(RM) *.[oda]
	$(RM) $(EXE)

# ---functions---
define install_dependency
	@echo "####################"
	@echo "Checking if package '$1' is installed"
	@echo "####################"
	@if dpkg -s $1; then \
	    echo "####################"; \
	    echo "package '$1' found"; \
	    echo "####################"; \
	else \
	    echo "package '$1' not installed"; \
	    read -p "install? (y/n)" confirm; \
	    if [ $$confirm = y ]; then \
		sudo apt install $1; \
	    fi \
	fi
endef
