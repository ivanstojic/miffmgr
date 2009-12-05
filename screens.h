#define BORDER_WIDTH 10
#define BORDER_PIXEL 0xff0000

#define BUFFER_WINDOW 0;
#define BUFFER_BUFFER 1;

#define DIRECTION_RIGHT 0;
#define DIRECTION_DOWN 1;

typedef struct {
  void *content;
  int content_type;
  
  void *next;
  int next_type;

  int next_direction;
} Buffer;


typedef struct {
  int screenNumber;
  int width, height;
  int offsetX, offsetY;

  Buffer rootBuffer;
  
  //  GPtrArray *windows;
} MiffScreen;

