CC = gcc
CFLAGS = -O2 -Wall -Wno-unused-result -g
CPPFLAGS = -I.
LDFLAGS =

TARGET = kabuki_config_gen

OBJS = kabuki_config_gen.o

all: $(TARGET)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FLAGS) -c $<


kabuki_config_gen: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@


kabuki_config_gen.c: title_jpn_utf8.txt title_jpn_sjis.txt


clean:
	rm -f *.o *~ $(TARGET)


