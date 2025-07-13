void sprintf_c(char *dst, char *fmt, ...);
void inttostr(char *str, int n);
void puts_c(char **dst, char *str);
void draw_str(int x, int y, char *str, u16 color);
void draw_char(int x, int y, char chr, u16 color);

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8