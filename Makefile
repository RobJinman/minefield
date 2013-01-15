NAME = demo
CC = g++
CFLAGS = -std=c++0x -Wall -g -DDEBUG
INCL = -I../Dodge/Dodge/include
LIBS = -L../Dodge/Dodge/lib -L/usr/lib -lDodge -lX11 -lGLESv2 -lEGL -lpnglite -lz -lBox2D
OBJS = Application.o \
	Item.o \
	main.o \
	Mine.o \
	NumericTile.o \
	Player.o \
	Soil.o

all: $(OBJS)
	$(CC) $(OBJS) -o $(NAME) $(LIBS)

$(OBJS): %.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCL) $< -o $@

clean:
	rm -f $(NAME)
	rm -f *.o
