#include "../text_grid.h"
#include <cassert>
#include <cstring>
#include <iostream>

int main() {
  for(const char* text:{"", "ASCII / ILR 2026", "ГОРИЗОНТ СОБЫТИЙ", "Ёжик и ёлка", "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", "абвгдеёжзийклмнопрстуфхцчшщъыьэюя"}) {
    char cells[80]={},out[160]={};const char* p=text;int i=0;
    while(*p){cells[i]=ilr::nextCell(p);assert(ilr::validCell(uint8_t(cells[i])));++i;}
    assert(ilr::cellLength(text)==i);
    ilr::rowUTF8(cells,out,sizeof(out));assert(!strcmp(text,out));
    for(size_t cap=1;cap<strlen(out)+2;++cap) {
      char clipped[160];memset(clipped,0x55,sizeof(clipped));
      ilr::rowUTF8(cells,clipped,cap);
      assert(strlen(clipped)<cap && clipped[cap]==0x55);
      const char* cursor=clipped;while(*cursor)assert(ilr::nextCell(cursor)!='?');
    }
  }
  assert(!strcmp(ilr::dayWord(1),"ДЕНЬ"));
  assert(!strcmp(ilr::dayWord(2),"ДНЯ"));
  assert(!strcmp(ilr::dayWord(5),"ДНЕЙ"));
  for(int n:{0,11,12,13,14,111,112})assert(!strcmp(ilr::dayWord(n),"ДНЕЙ"));
  for(int n:{21,31,101})assert(!strcmp(ilr::dayWord(n),"ДЕНЬ"));
  for(int n:{22,23,24,102})assert(!strcmp(ilr::dayWord(n),"ДНЯ"));
  std::cout<<"Russian cells: complete alphabet roundtrip, UTF-8 clipping and day plurals passed\n";
}
