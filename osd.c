#include "includes.h"

void showOSD(char *msg) {
  if (!fork()) {
    execl("/home/ivans/Projects/miffmgr/test/osd", "osd", msg, NULL);
  }
}

