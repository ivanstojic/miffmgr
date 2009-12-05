CCOPT=-g -Wall -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -funsigned-char  
LIBS=-lX11 -lXinerama -lXrandr -lglib-2.0  

# Stop editing here :-p

miffmgr: *.c *.h
	gcc $(CCOPT) $(LIBS) -c osd.c
	gcc $(CCOPT) $(LIBS) -c command.c
	gcc $(CCOPT) $(LIBS) -c layout_emacs.c
	gcc $(CCOPT) $(LIBS) -c miffmgr.c
	gcc $(CCOPT) $(LIBS) -o miffmgr miffmgr.o layout_emacs.o command.o osd.c

fakexinerama: 
	gcc -O2 -Wall tools/Xinerama.c -fPIC -o libXinerama.so.1.0 -shared
