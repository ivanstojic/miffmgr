#include "includes.h"

#include "screens.h"
#include "miffmgr.h"


void lminit() {
}



void lmarrange() {
  int i;
  for(i=0; i<miffScreenCount; i++) {
    /*    MiffScreen screen = miffScreens[i];

    int px = screen.offsetX;
    int py = screen.offsetY;

    int w = screen.width;
    int h = screen.height;
    
    printf("Arranging windows on screen %i, with geometry %ix%i+%i+%i\n",
	   i,
	   screen.width, screen.height, screen.offsetX, screen.offsetY);
    */
  }
}



void lmdebug() {
  printf("Debugging info...\n");
}


Buffer *lmfindemptybuffer(Buffer *where) {
  return NULL;
}

void lmaddwindow(Window *w, int suppress) {
  
  
  if (!suppress) {
    lmarrange();
  }
}

Window _emacsEmptyWindow() {
  // Fuck GCC and X
  XTextProperty title;
  title.value = (unsigned char *) "miffmgr: empty frame";
  title.encoding = XUTF8StringStyle;
  title.format = 8;
  title.nitems = strlen((char *)title.value);
  
  Window result = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0x104020);
  XSetWMName(dpy, result, &title);
  XMapWindow(dpy, result);

  return result;
}

void lmremovewindow(Window *w) {
  lmarrange();    
}


void lmfocusnext() {
  XSetInputFocus(dpy, NULL, RevertToPointerRoot, CurrentTime);
}

void lmfocuswindow(Window *w) {
  XSetInputFocus(dpy, NULL, RevertToPointerRoot, CurrentTime);
}

