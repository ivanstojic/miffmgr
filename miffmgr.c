/*
  miffmgr is written by Ivan Stojic <ivans@ordecon.com>, 2008.

  It is heavily based upon the insight given to me by TinyWM,
  written by Nick Welch in 2005. Thank you Nick!
*/

#include "includes.h"

// My own includes
#include "screens.h"
#include "miffmgr.h"
#include "command.h"
#include "osd.h"

#include "layout_emacs.h"


// Basic Xlib params...
Display *dpy;
Window root;


// Xinerama properties
int xineramaEventBase, xineramaErrorBase;
Bool hasXinerama;

XineramaScreenInfo *xineramaScreens;
int xineramaScreenCount;


// Xrandr properties
int xrandrEventBase, xrandrErrorBase;
Bool hasXrandr;


// This screen section represents the pick of one of the data returned by
// either Xrandr or the fallback option -- Xinerama
int miffScreenCount;
MiffScreen *miffScreens;


// Xinerama handling
void checkAndInitXinerama() {
  hasXinerama = XineramaQueryExtension(dpy, &xineramaEventBase, &xineramaErrorBase);
  
  if (hasXinerama) {
    printf("Enumerating Xinerama screens...\n");

    xineramaScreens = XineramaQueryScreens(dpy, &xineramaScreenCount);

    int i;
    for(i=0; i<xineramaScreenCount; i++) {
    }
  }
}



// Xrandr handling
void checkAndInitXrandr() {
  hasXrandr = XRRQueryExtension(dpy, &xrandrEventBase, &xrandrErrorBase);

  printf("FIXME: Playing dumb and not chechking Xrandr for screens.\n");
}



// Pick the source for our internal screen representation
void initMiffScreens() {
  if (0) {
    // This branch will occupy logic for screen initialization from Xrandr

  } else if (hasXinerama) {
    // Import screens from Xinerama
    printf("\nI will use screen info and geometry as given to me by Xinerama\n");

    miffScreens = malloc(sizeof(MiffScreen) * xineramaScreenCount);
    miffScreenCount = xineramaScreenCount;
    
    int i;
    for(i=0; i<xineramaScreenCount; i++) {
      miffScreens[i].screenNumber = i+1;

      miffScreens[i].width = xineramaScreens[i].width;
      miffScreens[i].height = xineramaScreens[i].height;
      
      miffScreens[i].offsetX = xineramaScreens[i].x_org;
      miffScreens[i].offsetY = xineramaScreens[i].y_org;
    }
    
    
  } else {
    // Init screens from Xlib geometry
    printf("\nNo Xinerama or Xrandr, assuming a single screen.\n");
  }
  
  /*
  printf("\nIniting window lists...\n");

  int i;
  for(i=0; i<miffScreenCount; i++) {
        miffScreens[i].windows = g_ptr_array_new();
	}*/
}



float presenceWeight(MiffScreen s, XWindowAttributes w) {
  XRectangle srect;

  srect.x = s.offsetX;
  srect.y = s.offsetY;
  srect.width = s.width;
  srect.height = s.height;
  
  Region sregion;
  sregion = XCreateRegion();
  XUnionRectWithRegion(&srect, sregion, sregion);
  
  // Do they intersect?
  int result = XRectInRegion(sregion, w.x, w.y, w.width, w.height);

  if (RectangleIn == result) {
    return 1;
  } else if (RectanglePart == result) {
    return 0.5;
  } else {
    return 0;
  }
}

// ...
void printWindowName(Window *w, char *fmt) {
  XTextProperty name;
  XGetWMName(dpy, *w, &name);
  
  printf(fmt, name.value);
}


// Determine the screen for window
MiffScreen *screenForWindow(Window *w) {
  XWindowAttributes wndattrs;

  int index = -1;
  float presence = -1;
  
  if (XGetWindowAttributes(dpy, *w, &wndattrs)) {
    int i;
    for(i=0; i<miffScreenCount; i++) {
      float t = presenceWeight(miffScreens[i], wndattrs);

      if (t > presence) {
	presence = t;
	index = i;
      }
    }
  }

  if (-1 != presence) {
    return &(miffScreens[index]);
  } else {
    return NULL;
  }
}


// On which screen is the mouse?
int screenForPointer() {
  return 0;
}

// On which virtual is a screen?
int virtualForScreen(int s) {
  return 0;
}

// ...
Window focusedWindow() {
  Window w;
  int r;

  XGetInputFocus(dpy, &w, &r);

  if (w != root && w != None && w != PointerRoot) {
    printWindowName(&w, "focusedWindow() responds with %s\n");
    return w;
  } else {
    return None;
  }
}


// handle new window
void wmMapWindow(Window *w, XEvent *ev, int suppress) {
  //  MiffScreen *current = screenForWindow(w);
  XTextProperty name;
  XGetWMName(dpy, *w, &name);

  XWindowAttributes window_attrs;

  if (XGetWindowAttributes(dpy, *w, &window_attrs)) {
    if (window_attrs.map_state == IsViewable) {
      // Window is viewable, that means we should handle pointer crossing...
      XSelectInput(dpy, *w, PointerMotionMask);
      
      if (!ev || (ev->type == MapNotify && !ev->xmap.override_redirect)) {
	// The window is both viewable, and isn't asking for redirection override,
	// that means we're free to play with it's visual properties: handling the size,
	// position, borders...

	// First the border width
	XWindowChanges bw;
	bw.border_width = BORDER_WIDTH;
	XConfigureWindow(dpy, *w, CWBorderWidth, &bw);

	// Then the color...
	XSetWindowAttributes bp;
	bp.border_pixel = BORDER_PIXEL;
	XChangeWindowAttributes(dpy, *w, CWBorderPixel, &bp);
	
	XSetInputFocus(dpy, *w, RevertToPointerRoot, CurrentTime);
	lmaddwindow(w, suppress);
      }
    }
    
  } else {
    printf("ERROR: failed to get window attributes!\n");
  }
}



// handle unmapping/destroying windows
void wmRemoveWindow(Window *w, XEvent *ev) {
  lmremovewindow(w);
}

//
void wmPointerMoving(XEvent *ev) {
  if (ev->xmotion.x > 0 && ev->xmotion.x && ev->xmotion.y > 0) {
    lmfocuswindow(&ev->xcrossing.window);
  }

}

// Window handling and positioning
void enumerateExistingWindows() {
  Window root_return, parent_return, *children_return;
  unsigned int children_count;

  if (XQueryTree(dpy, root, &root_return, &parent_return, &children_return, &children_count)) {
    printf("\nXQueryTree succeeded, with %i child windows. Faking map events for preexisting windows...\n", children_count);

    int i;

    for(i=0; i<children_count; i++) {
      wmMapWindow(&children_return[i], NULL, TRUE);
      printf("\n\n\n");
    }
  }

  XFree(children_return);

  lmarrange();
}


// Grabs+selects on root
void initGrabsAndSelects() {
  XSelectInput(dpy, root, SubstructureNotifyMask);
  
  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("1")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("2")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("3")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("4")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("5")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("6")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("7")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("8")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("9")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("0")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);
  
  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("T")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("A")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("W")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("Tab")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("K")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("B")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);  

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("D")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);  

  XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("P")), Mod1Mask, root,
	   True, GrabModeAsync, GrabModeAsync);  
}

// Main event loop
void runMainLoop() {
  XEvent ev;

  while (TRUE) {
    XNextEvent(dpy, &ev);

    //printf("Got event %i\n", ev.type);

    if(ev.type == KeyPress) {
      handleKeyPressEvent(ev.xkey);

    } else if (ev.type == MotionNotify) {
      wmPointerMoving(&ev);
	
    } else if (ev.type == UnmapNotify) {
      // Window is being unmapped (hidden)
      wmRemoveWindow(&(ev.xunmap.window), &ev);

    } else if (ev.type == DestroyNotify) {
      // Window is being destroyed
      wmRemoveWindow(&(ev.xunmap.window), &ev);

    } else if (ev.type == MapNotify) {
      // A window is being mapped
      wmMapWindow(&(ev.xmap.window), &ev, FALSE);
    }
  }  
}


// Initialization and setting up housekeeping
int main(int argc, char* argv[]) {
  if(!(dpy = XOpenDisplay(0x0))) return 1;
  root = DefaultRootWindow(dpy);

  printf("\nXlib geometry: %ix%i\n", DisplayWidth(dpy, 0), DisplayHeight(dpy,0));
  
  checkAndInitXinerama();
  checkAndInitXrandr();
  initMiffScreens();

  // This initializes the data structures used by the layout manager
  lminit();

  // There might be some windows which were created before the window manager, so
  // synchronize the status of the layout managers by simulating XMapEvent for those
  // windows
  enumerateExistingWindows();

  initGrabsAndSelects();
  
  showOSD("Welcome to miffmgr");
  runMainLoop();

  // Like tears in the rain... time to die.
  return 0;
}
