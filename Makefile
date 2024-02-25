CC = gcc
CFLAGS = -O2 -Wall -Wno-unused-result -g
CPPFLAGS = -I.
LDFLAGS =

TARGET = kabuki_config_gen kabuki_config_gen_sjis

OBJS = kabuki_config_gen.o
OBJS_SJIS = kabuki_config_gen_sjis.o

all: $(TARGET)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FLAGS) -c $<

kabuki_config_gen_sjis.o: kabuki_config_gen.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -DUSE_SJIS $(FLAGS) -c $< -o $@

kabuki_config_gen: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

kabuki_config_gen_sjis: $(OBJS_SJIS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

kabuki_config_gen.c: title_jpn_utf8.txt title_jpn_sjis.txt

title_jpn_sjis.txt: title_jpn_utf8.txt
	iconv -f UTF-8 -t SJIS $< > $@

title_jpn_utf8.txt: title_jpn.txt
	sed -e 's/^\([^,]*\), \(.*\)$$/{\1, "\2"},/' $< > $@


clean:
	rm -f *.o *~ $(TARGET) title_jpn_utf8.txt title_jpn_sjis.txt


