#pragma once
#include <stdint.h>
#include <stddef.h>

namespace ilr {
// One byte per OLED cell: printable ASCII, А..я at 0x80..0xbf, Ё/ё at 0xc0/0xc1.
// UTF-8 is decoded before positioning, so Cyrillic letters occupy one cell.
inline uint8_t nextCell(const char*& text) {
  uint8_t a=uint8_t(*text++);
  if(a<128)return a;
  if((a==0xd0 || a==0xd1) && uint8_t(*text)>=0x80 && uint8_t(*text)<=0xbf) {
    uint16_t cp=((a&31)<<6)|(uint8_t(*text++)&63);
    if(cp>=0x410 && cp<=0x44f)return 0x80+cp-0x410;
    if(cp==0x401)return 0xc0;
    if(cp==0x451)return 0xc1;
  }
  return '?';
}
inline int cellLength(const char* text) {
  int n=0;while(*text){nextCell(text);++n;}return n;
}
inline bool validCell(uint8_t cell) {return cell>=32 && cell<=0xc1 && cell!=127;}
inline void rowUTF8(const char* row,char* out,size_t capacity) {
  size_t n=0;
  for(;*row;++row) {
    uint8_t cell=uint8_t(*row);
    if(cell<128){if(n+1>=capacity)break;out[n++]=cell;}
    else {
      uint16_t cp=cell==0xc0?0x401:cell==0xc1?0x451:0x410+cell-0x80;
      if(n+2>=capacity)break;
      out[n++]=0xc0|(cp>>6);out[n++]=0x80|(cp&63);
    }
  }
  if(capacity)out[n]=0;
}
inline const char* dayWord(int days) {
  const int n=days>0?days:0;
  if(n%100>=11 && n%100<=14)return "ДНЕЙ";
  return n%10==1?"ДЕНЬ":n%10>=2 && n%10<=4?"ДНЯ":"ДНЕЙ";
}
}
