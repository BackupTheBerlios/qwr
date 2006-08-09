SRC= event.cpp connection.cpp timev.cpp scheduler.cpp simpleConnection.cpp \
	threadConnection.cpp descConnection.cpp task.cpp itask.cpp \
	qThreadConnection.cpp qTask.cpp qDescConnection.cpp

CC=g++
FLAGS= -O0 -g -Wall -pthread -I/usr/include/cc++2 -I/usr/lib/qt3/include -DQT_THREAD_SUPPORT
LDFLAGS = 
#VPATH=src_backend global

STRIP=strip
INSTALL=install

all: esystem

%.o:%.cpp
	$(CC) $(FLAGS) $< -c

esystem: $(SRC:.cpp=.o)

clean:
	@rm -f *.o *~ *.d

.PHONY: all esystem clean
