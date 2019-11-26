all:utest

INSTDIR = ./out

# Options for development
  CFLAGS = -g -Wall -lpthread

#pthread 线程
#pcre    正则表达式
#aio     异步IO

# Options for release
# CFLAGS = -O -Wall

CC = g++ $(CFLAGS)

# app vars
FFILES = $(wildcard ./*.cpp , ./*.h)
FILES  = $(notdir $(FFILES))

utest: 
	$(CC) *.cpp -o a.out

#Tool command
clean:
	-rm -rf a.out a.out.*
files:
	@echo '$(FILES)'
