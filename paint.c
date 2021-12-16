#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h> // for error catch

// Structure for canvas
typedef struct {
    int width;
    int height;
    char **canvas;
    char pen;
} Canvas;

typedef struct command Command;
struct command {
  char *str;
  size_t bufsize;
  Command *next;
};

typedef struct {
    Command *begin;
} History;

// functions for Canvas type
Canvas *init_canvas(int width, int height, char pen);
void reset_canvas(Canvas *c);
void print_canvas(Canvas *c);
void free_canvas(Canvas *c);

// display functions
void rewind_screen(unsigned int line);
void clear_command(void);
void clear_screen(void);

// enum for interpret_command results
typedef enum res{ EXIT, NORMAL, COMMAND, UNKNOWN, ERROR} Result;

// function for linear list
Command *push_front(History *his, const char* str, const int bufsize);
Command *push_back(History *his, const char* str, const int bufsize);
Command *pop_back(History *his);

int max(const int a, const int b);
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1);
Result interpret_command(const char *command, History *his, Canvas *c, int bufsize);
void save_history(const char *filename, History *his);


int main(int argc, char **argv)
{
    //for history recording
    const int max_history = 5;
    const int bufsize = 1000;

    History his = (History){.begin = NULL};
    
    int width;
    int height;
    if (argc != 3){
      fprintf(stderr,"usage: %s <width> <height>\n",argv[0]);
      return EXIT_FAILURE;
    } else{
      char *e;
      long w = strtol(argv[1],&e,10);
      if (*e != '\0'){
          fprintf(stderr, "%s: irregular character found %s\n", argv[1],e);
          return EXIT_FAILURE;
      }
      long h = strtol(argv[2],&e,10);
      if (*e != '\0'){
          fprintf(stderr, "%s: irregular character found %s\n", argv[2],e);
          return EXIT_FAILURE;
      }
      width = (int)w;
      height = (int)h;    
    }
    char pen = '*';
    
    char buf[bufsize];

    Canvas *c = init_canvas(width,height, pen);

    printf("\n"); // required especially for windows env
    
    while (1 == 1) {
      print_canvas(c);
      printf("> ");
      if(fgets(buf, bufsize, stdin) == NULL) break;
      
      const Result r = interpret_command(buf, &his, c, bufsize);
      if (r == EXIT) break;   
      if (r == NORMAL) {
          push_back(&his, buf, bufsize);
      }
      
      rewind_screen(2); // command results
      clear_command(); // command itself
      rewind_screen(height+2); // rewind the screen to command input
	
    }
    
    clear_screen();
    free_canvas(c);
    
    return 0;
}

Canvas *init_canvas(int width,int height, char pen)
{
  Canvas *new = (Canvas *)malloc(sizeof(Canvas));
  new->width = width;
  new->height = height;
  new->canvas = (char **)malloc(width * sizeof(char *));

  char *tmp = (char *)malloc(width*height*sizeof(char));
  memset(tmp, ' ', width*height*sizeof(char));
  for (int i = 0 ; i < width ; i++){
    new->canvas[i] = tmp + i * height;
  }
  
  new->pen = pen;
  return new;
}

void reset_canvas(Canvas *c)
{
  const int width = c->width;
  const int height = c->height;
  memset(c->canvas[0], ' ', width*height*sizeof(char));
}


void print_canvas(Canvas *c)
{
  const int height = c->height;
  const int width = c->width;
  char **canvas = c->canvas;
  
  // 上の壁
  printf("+");
  for (int x = 0 ; x < width ; x++)
    printf("-");
  printf("+\n");

  // 外壁と内側
  for (int y = 0 ; y < height ; y++) {
      printf("|");
      for (int x = 0 ; x < width; x++){
	  const char c = canvas[x][y];
	  putchar(c);
      }
      printf("|\n");
  }
  
  // 下の壁
  printf("+");
  for (int x = 0 ; x < width ; x++)
      printf("-");
  printf("+\n");
  fflush(stdout);
}

void free_canvas(Canvas *c)
{
  free(c->canvas[0]); //  for 2-D array free
  free(c->canvas);
  free(c);
}

void rewind_screen(unsigned int line)
{
    printf("\e[%dA",line);
}

void clear_command(void)
{
    printf("\e[2K");
}

void clear_screen(void)
{
    printf("\e[2J");
}


int max(const int a, const int b)
{
  return (a > b) ? a : b;
}
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1)
{
  const int width = c->width;
  const int height = c->height;
  char pen = c->pen;
  
  const int n = max(abs(x1 - x0), abs(y1 - y0));
  c->canvas[x0][y0] = pen;
  for (int i = 1; i <= n; i++) {
    const int x = x0 + i * (x1 - x0) / n;
    const int y = y0 + i * (y1 - y0) / n;
    if ( (x >= 0) && (x< width) && (y >= 0) && (y < height))
      c->canvas[x][y] = pen;
  }
}

void save_history(const char *filename, History *his)
{
  const char *default_history_file = "history.txt";
  if (filename == NULL)
    filename = default_history_file;
  
  FILE *fp;
  if ((fp = fopen(filename, "w")) == NULL) {
    fprintf(stderr, "error: cannot open %s.\n", filename);
    return;
  }

  Command *p = his->begin;
  while(p->next != NULL) {
    fprintf(fp, "%s", p->str);
    p = p->next;
  }
  fprintf(fp, "%s", p->str);
    
  fclose(fp);
}

Result interpret_command(const char *command, History *his, Canvas *c, int bufsize)
{
  char buf[bufsize];
  strcpy(buf, command);
  buf[strlen(buf) - 1] = 0; // remove the newline character at the end
  
  const char *s = strtok(buf, " ");
  
  // The first token corresponds to command
  if (strcmp(s, "line") == 0) {
    int p[4] = {0}; // p[0]: x0, p[1]: y0, p[2]: x1, p[3]: x1 
    char *b[4];
    for (int i = 0 ; i < 4; i++){
        b[i] = strtok(NULL, " ");
        if (b[i] == NULL){
          clear_command();
          printf("the number of point is not enough.\n");
          return ERROR;
        }
    }
    for (int i = 0 ; i < 4 ; i++){
        char *e;
        long v = strtol(b[i],&e, 10);
        if (*e != '\0'){
          clear_command();
          printf("Non-int value is included.\n");
          return ERROR;
        }
        p[i] = (int)v;
    }
    
    draw_line(c,p[0],p[1],p[2],p[3]);
    clear_command();
    printf("1 line drawn\n");
    return NORMAL;
  }
  
  if (strcmp(s, "save") == 0) {
    s = strtok(NULL, " ");
    save_history(s, his);
    clear_command();
    printf("saved as \"%s\"\n",(s==NULL)?"history.txt":s);
    return COMMAND;
  }
  
  if (strcmp(s, "undo") == 0) {
    reset_canvas(c);

    pop_back(his);
    Command *p = his->begin;
    while(p->next != NULL) {
      interpret_command(p->str, his, c, bufsize);
      p = p->next;
    }
    interpret_command(p->str, his, c, bufsize);

    clear_command();
    printf("undo!\n");
    return COMMAND;
  }
  
  if (strcmp(s, "quit") == 0) {
    return EXIT;
  }
  clear_command();
  printf("error: unknown command.\n");
  return UNKNOWN;
}

Command *push_front(History *his, const char *str, const int bufsize) {
  Command *p = (Command *)malloc(sizeof(Command));
  char *s = (char *)malloc(strlen(str) + 1);
  strcpy(s, str);

  *p = (Command){.str = s, .bufsize = bufsize, .next = his->begin};
  his->begin = p;

  return p;
}

Command *push_back(History *his, const char *str, const int bufsize)
{
  Command *p = his->begin;
  if (p == NULL) { 
    return push_front(his, str, bufsize);
  }

  while(p->next != NULL) {
    p = p->next;
  }
  
  //ヒープ領域にメモリを確保
  Command *q = (Command *)malloc(sizeof(Command));
  char *s = (char *)malloc(strlen(str) + 1);
  strcpy(s, str);
  
  *q = (Command){.str = s, .bufsize = bufsize, .next = NULL};
  p->next = q;
  
  return q;
}

Command *pop_front(History *his){
    Command *p = his->begin;
    
    if (p != NULL){
      his->begin = p->next;
      p->next = NULL;
      // free(p->str)
	    // free(p);
    }
    
    return p;
}


Command *pop_back(History *his)
{
    Command *p = his->begin;
    Command *q = NULL;
    
    if (p == NULL) return NULL;
    
    while (p->next != NULL){
      q = p;
      p = p->next;
    }
    
    if ( q == NULL ) return pop_front(his);
    // 新たな末尾の設定
    if ( q != NULL) q->next = NULL;
    
    return p;

    free(p->str);
    free(p);
}