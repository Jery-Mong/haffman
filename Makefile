cprs=compress.c list.c
etr=extract.c

CFLAGS=-Wall

all:compress extract

compress:$(cprs:.c=.o)
	gcc $(CFLAGS) $^ -o $@

extract:$(etr:.c=.o)
	gcc $(CFLAGS) $^ -o $@

%.o:%.c
	gcc $(CFLAGS) -c $< -o $@
clean:
	-rm *.o compress extract

.PHONY:clean all

