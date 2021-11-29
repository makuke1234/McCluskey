SHELL=cmd
CC=gcc
CDEFFLAGS=-std=c2x -m64 -Wall -Wextra -Wpedantic -Wconversion -Wdouble-promotion -Wshadow -Wfree-nonheap-object -Wcast-align -Wunused -Wsign-conversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wformat=2
CDEBFLAGS=-g -O0
CFLAGS=-O3 -Wl,--strip-all,--build-id=none,--gc-sections -fno-ident

SRC=src
TARGET=Lahendaja
OBJD=objd
OBJ=obj

default: debug


$(OBJD):
	mkdir $(OBJD)
$(OBJ):
	mkdir $(OBJ)

ASMSUFFIX=S

C_SRCS=$(wildcard $(SRC)/*.c)
ASM_SRCS=$(wildcard $(SRC)/*.$(ASMSUFFIX))
debug_obj=$(C_SRCS:%.c=%.c.o)
debug_obj+=$(ASM_SRCS:%.$(ASMSUFFIX)=%.$(ASMSUFFIX).o)
release_obj=$(C_SRCS:%.c=%.c.o)
release_obj+=$(ASM_SRCS:%.$(ASMSUFFIX)=%.$(ASMSUFFIX).o)

debug_obj:=$(debug_obj:$(SRC)/%=$(OBJD)/%)
release_obj:=$(release_obj:$(SRC)/%=$(OBJ)/%)

$(OBJD)/%.c.o: $(SRC)/%.c $(OBJD)
	$(CC) $< -c -o $@ $(CDEFFLAGS) $(CDEBFLAGS)
$(OBJD)/%.$(ASMSUFFIX).o: $(SRC)/%.$(ASMSUFFIX) $(OBJD)
	$(CC) $< -c -o $@ $(CDEFFLAGS) $(CDEBFLAGS)

$(OBJ)/%.c.o: $(SRC)/%.c $(OBJ)
	$(CC) $< -c -o $@ $(CDEFFLAGS) $(CFLAGS)
$(OBJ)/%.$(ASMSUFFIX).o: $(SRC)/%.$(ASMSUFFIX) $(OBJ)
	$(CC) $< -c -o $@ $(CDEFFLAGS) $(CFLAGS)


debug: $(debug_obj)
	$(CC) $^ -o deb$(TARGET) $(CDEBFLAGS)


release: $(release_obj)
	$(CC) $^ -o $(TARGET) $(CFLAGS)

clean.o:
	IF EXIST $(OBJD) rd /s /q $(OBJD)
	IF EXIST $(OBJ) rd /s /q $(OBJ)

clean: clean.o
	del deb$(TARGET).exe
	del $(TARGET).exe
