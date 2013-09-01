cprs=compress.c list.c
etr=extract.c

CFLAGS=-Wall -g

all:compress extract

compress:$(cprs:.c=.o)
	gcc $(CFLAGS) $^ -o $@

extract:$(etr:.c=.o)
	gcc $(CFLAGS) $^ -o $@

%.o:%.c
	gcc $(CFLAGS) -c $< -o $@
cleanall:
	-rm *.o compress extract
clean:
	-rm *.o
.PHONY:clean all

