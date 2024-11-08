# Daedalus (Version 3.5) File: Makefile
# By Walter D. Pullen, Astara@msn.com, http://www.astrolog.org/labyrnth.htm
#
# IMPORTANT NOTICE: Daedalus and all Maze generation and general
# graphics routines used in this program are Copyright (C) 1998-2024 by
# Walter D. Pullen. Permission is granted to freely use, modify, and
# distribute these routines provided these credits and notices remain
# unmodified with any altered or distributed versions of the program.
# The user does have all rights to Mazes and other graphic output
# they make in Daedalus, like a novel created in a word processor.
#
# First created 12/7/2018.
#
# This Makefile is included only for convenience. One could easily compile
# Daedalus on a Unix system by hand with the command:
# % g++ -c -O *.cpp; g++ -o daedalus *.o -lm
# Generally, all that needs to be done to compile once util.h has been
# edited, is compile each source file, and link them together with the math
# library.
#
NAME = daedalus
OBJS = color.o command.o create.o create2.o create3.o daedalus.o\
 draw.o draw2.o game.o graphics.o inside.o labyrnth.o maze.o solids.o\
 solve.o threed.o util.o

LIBS = -lm -s
CPPFLAGS = -O -Wno-write-strings -Wno-narrowing -Wno-comment
RM = rm -f

daedalus:: $(OBJS)
	g++ -o $(NAME) $(OBJS) $(LIBS)

clean:
	$(RM) $(OBJS) $(NAME)
#