#include "includes.h"

#include "osd.h"
#include "layout_emacs.h"

#include <X11/keysym.h>



// terminal/leaf command handlers...

void handleShowTime() {
  showOSD("");
}


// Main handler
void handleKeyPressEvent(XKeyEvent e) {
  // printf("Key pressed %i\n", e.state);
  // printf("Key pressed %i\n", XLookupKeysym(&e, 1));

    switch (XLookupKeysym(&e, 0)) {
    case XK_1:
    case XK_2:
    case XK_3:
    case XK_4:
    case XK_5:
    case XK_6:
    case XK_7:
    case XK_8:
    case XK_9:
    case XK_0:
      showOSD("Moving to some other virtual desktop...");
      break;
      
    case XK_t:
      handleShowTime();
      break;

    case XK_a:
      showOSD("Split vertically (NOOP)");
      break;

    case XK_w:
      showOSD("Split horizontally (NOOP)");
      break;

    case XK_Tab:
      showOSD("Next layout window");
      lmfocusnext();
      break;

    case XK_k:
      showOSD("Kill layout window (NOOP)");
      break;

    case XK_b:
      showOSD("Choose XWindow for this layout window (NOOP)");
      break;

    case XK_d:
      printf("Dumping debugging info...\n");
      lmdebug();
      break;
      
    case XK_p:
      printf("Positioning windows again...\n");
      lmarrange();
      break;

    }
}
