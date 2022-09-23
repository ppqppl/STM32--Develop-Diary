.PHONY: clean
CC = gcc
BIN = main.out
OBJS = main.o add1.o sub1.o

${BIN}: ${OBJS}
  @echo "start compiling……"
  @echo $(CC)
  $(cc) -o ${BIN} ${OBJS}
  @echo "compile done"


