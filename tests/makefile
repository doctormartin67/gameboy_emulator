# ---variables---
INCLUDE = ../include
SRC = ../src
LIB = ../lib
EXE = main

# ---dependencies---

# ---Rule Structure---
CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -pedantic
# THIS BELOW INCLUDRES AND LDLIBS ON XML CAN BE REMOVED
INCLUDES = -I $(INCLUDE) 
LIBS = 
LDLIBS = $(LIBS) 
LDFLAGS = -L$(LIB)

COMPILE.c = $(CC) $(CFLAGS) $(INCLUDES) $(CPPFLAGS) $(TARGET_ARCH) -c

# ---Object files---
OBJS = cartridge.o bus.o cpu.o op.o ram.o stack.o
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
