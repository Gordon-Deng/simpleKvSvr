all:utest


CFLAGS = -g -Wall -lpthread
CC = g++ $(CFLAGS)


utest: processpool

processpool:
	$(CC) processpool.cpp testprocesspool.cpp ./taskpool/shm_cqueue.cpp -o mytestprocesspool.out

#Tool command
clean:
	-rm -rf mytestprocesspool.out mytestprocesspool.out.*
