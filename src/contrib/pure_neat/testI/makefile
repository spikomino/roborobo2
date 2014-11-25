
ROOT =../../../../


INCLUDE = -I $(ROOT)/include/contrib/
CCFLAGS = -Wall -std=c++0x -g
LDFLAGS = 
CC      = g++

OBJ = ../gene.o ../genome.o ../link.o ../network.o ../nnode.o

TARGET = test 
TARGET2 = test2

all: $(TARGET) $(TARGET2)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CCFLAGS) $(INCLUDE)

$(TARGET): $(OBJ) test.o
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET2): $(OBJ) test2.o
	$(CC) $(LDFLAGS) -o $@ $^
