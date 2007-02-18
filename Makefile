DESTDIR=$(prefix)  # Debian controlled
BIN=/usr/bin/
CC = gcc
LIBS = `gtk-config --libs`
INCS = `gtk-config --cflags`
FLAGS = -Wall -c -g

main: nimf
nimf: sysinfo.o settings.o bookmarks.o config.o operations.o panel.o cmdline.o file.o dialogs.o nimf.o gtkaddons.o context.o
	cc nimf.o context.o sysinfo.o panel.o cmdline.o file.o operations.o dialogs.o config.o bookmarks.o settings.o gtkaddons.o -o nimf -g -Wall `gtk-config --cflags --libs` `glib-config --libs gthread` 
panel.o: operations.h file.h panel.c panel.h config.h
	$(CC) $(FLAGS) $(INCS) panel.c
nimf.o: nimf.c nimf.h panel.h config.h bookmarks.h operations.h dialogs.h settings.h
	$(CC) $(FLAGS) $(INCS) nimf.c
file.o: file.c file.h
	$(CC) $(FLAGS) $(INCS) file.c
operations.o: operations.c panel.h operations.h file.h dialogs.h gtkaddons.h
	$(CC) $(FLAGS) $(INCS) operations.c
dialogs.o: dialogs.c dialogs.h
	$(CC) $(FLAGS) $(INCS) dialogs.c
gtkaddons.o: gtkaddons.c gtkaddons.h
	$(CC) $(FLAGS) $(INCS) gtkaddons.c
config.o: config.c config.h
	$(CC) $(FLAGS) $(INCS) config.c
bookmarks.o: bookmarks.c bookmarks.h
	$(CC) $(FLAGS) $(INCS) bookmarks.c
settings.o: settings.c settings.h
	$(CC) $(FLAGS) $(INCS) settings.c
cmdline.o: cmdline.c cmdline.h
	$(CC) $(FLAGS) $(INCS) cmdline.c
sysinfo.o: sysinfo.c sysinfo.h
	$(CC) $(FLAGS) $(INCS) sysinfo.c
context.o: context.c context.h
	$(CC) $(FLAGS) $(INCS) context.c
clean:
	-rm nimf
	-rm *.o
install:
	install -d $(DESTDIR)$(BIN)
	install ./nimf $(DESTDIR)$(BIN)
