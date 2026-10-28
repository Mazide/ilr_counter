// Host-only adapter: export the actual U8g2 4x6 glyphs used by the OLED.
#include <stdio.h>
#include <string.h>
#include "u8g2.h"
static unsigned char pixels[24];
void u8g2_DrawHVLine(u8g2_t* u,u8g2_uint_t x,u8g2_uint_t y,u8g2_uint_t len,uint8_t dir) {
  for(unsigned i=0;i<len;++i) {
    int px=x,py=y;
    if(dir==0)px+=i;else if(dir==1)py+=i;else if(dir==2)px-=i;else py-=i;
    if(px>=0 && px<4 && py>=0 && py<6)pixels[py*4+px]=u->draw_color?1:0;
  }
}
int main(void) {
  u8g2_t u;memset(&u,0,sizeof(u));u.draw_color=1;
  u.user_x1=u.pixel_buf_width=4;u.user_y1=u.pixel_buf_height=6;
  u8g2_SetFont(&u,u8g2_font_4x6_tf);u8g2_SetFontPosTop(&u);u8g2_SetFontMode(&u,1);
  puts("[");
  for(int c=32;c<=126;++c) {
    memset(pixels,0,sizeof(pixels));u8g2_DrawGlyph(&u,0,0,c);
    unsigned bits=0;for(int p=0;p<24;++p)bits|=((unsigned)pixels[p])<<p;
    printf("%u%s",bits,c==126?"":",");
  }
  puts("]");return 0;
}
