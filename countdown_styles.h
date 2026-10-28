#pragma once
#include <cmath>
#include <cstring>
#include <cstdio>
#include <initializer_list>
#include "countdown_core.h"
#include "ascii_font.h"
#include "text_grid.h"

namespace ilr {
struct CountdownStyle {
  static constexpr int W=32,H=21;
  char cells[H][W+1];
  void put(int x,int y,char c){if(x>=0&&x<W&&y>=0&&y<H)cells[y][x]=c;}
  void at(int x,int y,const char* s){for(int i=0;*s;++i)put(x+i,y,char(nextCell(s)));}
  void center(int y,const char* s){at((W-cellLength(s))/2,y,s);}
  static float noise(int x,int y) {
    uint32_t n=uint32_t(x)*374761393u+uint32_t(y)*668265263u;
    n=(n^(n>>13))*1274126177u;return float(n^(n>>16))/4294967296.f;
  }
  void banner(const char* word,int left,int top,int sx=1,int sy=1,char ink='#') {
    for(int n=0;word[n];++n)for(int y=0;y<7;++y)for(int x=0;x<5;++x)
      if(asciiInk(word[n],x,y))for(int yy=0;yy<sy;++yy)for(int xx=0;xx<sx;++xx)
        put(left+n*6*sx+x*sx+xx,top+y*sy+yy,ink);
  }
  void box(int left,int top,int width,int height) {
    for(int x=left;x<left+width;++x){put(x,top,'-');put(x,top+height-1,'-');}
    for(int y=top;y<top+height;++y){put(left,y,'|');put(left+width-1,y,'|');}
    for(int x:{left,left+width-1})for(int y:{top,top+height-1})put(x,y,'+');
  }
  void dissolve(int days,uint32_t ms) {
    center(0,"ИСЧЕЗНОВЕНИЕ / ILR");
    char number[12];snprintf(number,sizeof(number),"%d",days>0?days:0);
    // Stable count -> dust -> ILR -> dust. Never suggest a fictitious day change.
    const uint32_t tick=ms%12000;
    const bool second=tick>=6000;
    const char* word=second?"ILR":number;
    const int sx=strlen(word)<=2?2:1,sy=2;
    const int left=(W-(int(strlen(word))*6-1)*sx)/2;
    const uint32_t local=tick%6000;
    for(int n=0;word[n];++n)for(int y=0;y<7;++y)for(int x=0;x<5;++x) {
      if(!asciiInk(word[n],x,y))continue;
      for(int yy=0;yy<sy;++yy)for(int xx=0;xx<sx;++xx) {
        int cx=left+n*6*sx+x*sx+xx,cy=3+y*sy+yy;
        const float h=noise(cx,cy),drift=std::sin(h*40);
        if(local<1000) {
          const float p=float(local)/1000;
          if(p>h*.4f)put(cx+int(drift*(1-p)*5),cy-int((1-p)*4),p>.8f?'#':p>.45f?':':'.');
        } else if(local<4000)put(cx,cy,'#');
        else {
          const float p=float(local-4000)/2000;
          const float edge=.12f+h*.35f+n*.04f;
          if(p<edge)put(cx,cy,'#');
          else if(p<edge+.42f) {
            const float q=(p-edge)/.42f;
            put(cx+int(drift*q*5),cy+int(q*q*4),q<.3f?'*':q<.7f?':':'.');
          }
        }
      }
    }
    char caption[40];snprintf(caption,sizeof(caption),"%s ДО ILR",dayWord(days));
    center(18,days>0?caption:"ДАТА НАСТУПИЛА");
  }
  void hourglass(int days,uint32_t ms) {
    center(0,"ПЕСОЧНЫЕ ЧАСЫ");
    for(int x=0;x<=14;++x){put(x,3,x==0||x==14?'+':'-');put(x,17,x==0||x==14?'+':'-');}
    for(int y=4;y<=9;++y){int half=10-y;put(7-half,y,'\\');put(7+half,y,'/');}
    for(int y=11;y<=16;++y){int half=y-10;put(7-half,y,'/');put(7+half,y,'\\');}
    const int remaining=days>17?17:days<0?0:days;
    const int top=(remaining*6+16)/17,bottom=6-top;
    for(int y=10-top;y<=9;++y){int half=10-y;for(int x=8-half;x<7+half;++x)put(x,y,(x+y)%2?'#':':');}
    for(int y=17-bottom;y<=16;++y){int half=y-10;for(int x=8-half;x<7+half;++x)put(x,y,(x+y)%2?'#':':');}
    if(days>0)put(7,10+ms/150%6,ms/150%2?':':'.');
    char number[12];snprintf(number,sizeof(number),"%d",days>0?days:0);
    if(strlen(number)<=2)banner(number,20,6);else at(19,9,number);
    at(21,15,dayWord(days));at(20,17,"ДО ILR");
  }
  void render(int style,int days,Date target,uint32_t ms) {
    for(auto& row:cells){memset(row,' ',W);row[W]=0;}
    if(style==0)dissolve(days,ms);else hourglass(days,ms);
    // The UTF-8 quote is drawn separately on this row by the OLED adapter.
    memset(cells[19],' ',W);
    char date[20];label(target,date,sizeof(date));center(20,date);
  }
};
}
