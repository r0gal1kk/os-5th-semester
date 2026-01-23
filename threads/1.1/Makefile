TARGET = thread_c
SRCS = src/thread_c.c

CC=gcc
RM=rm
CFLAGS= -g -Wall -Wextra -fsanitize=address -fsanitize=leak
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET}

${TARGET}: ${SRCS}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS} ${LIBS} -o ${TARGET}

clean:
	${RM} -f *.o ${TARGET}
