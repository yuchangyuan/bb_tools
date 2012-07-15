objs = cpu_info.o disp.o

CFLAGS = -g -O2 $(shell pkg-config --cflags cairo)
LDFLAGS = $(shell pkg-config --libs cairo)

all: cpu_info


cpu_info: $(objs)
	gcc $(LDFLAGS) -o $@ $(objs)

cpu_info.o: cpu_info.c
	gcc -c $(CFLAGS) $<

disp.o: disp.c disp.h
	gcc -c $(CFLAGS) $<

# %.o: %.c
#	gcc -c $(CFLAGS) $<

clean:
	rm *.o cpu_info

