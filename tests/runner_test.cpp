#include "../runner_scene.h"
#include <cassert>
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>

struct Pixels {
  std::array<unsigned char,128*128> pixels{};
  unsigned char color=255;
  void setDrawColor(int c){color=c?255:0;}
  void drawPixel(int x,int y) {
    assert(x>=0 && x<128 && y>=69 && y<128);
    pixels[y*128+x]=color;
  }
  void drawHLine(int x,int y,int w){for(int i=0;i<w;++i)drawPixel(x+i,y);}
  void drawVLine(int x,int y,int h){for(int i=0;i<h;++i)drawPixel(x,y+i);}
  void drawFrame(int x,int y,int w,int h){drawHLine(x,y,w);drawHLine(x,y+h-1,w);drawVLine(x,y,h);drawVLine(x+w-1,y,h);}
  void drawBox(int x,int y,int w,int h){for(int j=0;j<h;++j)drawHLine(x,y+j,w);}
  void drawDisc(int x,int y,int r){for(int dy=-r;dy<=r;++dy)for(int dx=-r;dx<=r;++dx)if(dx*dx+dy*dy<=r*r)drawPixel(x+dx,y+dy);}
  void drawCircle(int x,int y,int r){
    int a=r,b=0,err=1-r;
    while(a>=b){for(int sx:{-1,1})for(int sy:{-1,1}){drawPixel(x+sx*a,y+sy*b);drawPixel(x+sx*b,y+sy*a);}++b;if(err<0)err+=2*b+1;else {--a;err+=2*(b-a)+1;}}
  }
  void drawLine(int x,int y,int x2,int y2){
    int dx=abs(x2-x),sx=x<x2?1:-1,dy=-abs(y2-y),sy=y<y2?1:-1,err=dx+dy;
    for(;;){drawPixel(x,y);if(x==x2 && y==y2)break;int e=2*err;if(e>=dy){err+=dy;x+=sx;}if(e<=dx){err+=dx;y+=sy;}}
  }
  void write(const std::string& path){std::ofstream out(path,std::ios::binary);out<<"P5\n128 128\n255\n";out.write(reinterpret_cast<const char*>(pixels.data()),pixels.size());}
};

int main(int argc,char** argv) {
  assert(ilr::runnerFrame(0,0).x==80);
  assert(ilr::runnerFrame(0,1500).x>80);
  assert(ilr::runnerFrame(0,3000).x==94);
  assert(ilr::runnerFrame(0,11999).x==94);
  assert(ilr::runnerFrame(0,12000).x==80);
  ilr::Date defaultTarget;
  assert(ilr::parseDate(ilr::DEFAULT_TARGET_DATE,defaultTarget));
  assert(defaultTarget.year==2026 && defaultTarget.month==11 && defaultTarget.day==17);
  int previousX=0;
  for(int days=17;days>=0;--days){const auto s=ilr::runnerFrame(days,0);assert(s.x>=previousX);previousX=s.x;}
  for(int days:{-3,0,1,2,8,17,30,9999})for(uint32_t ms=0;ms<24000;ms+=19) {
    auto s=ilr::runnerFrame(days,ms);
    assert(s.lift>=0 && s.lift<=18);
    assert(s.arrived==(days<=0));
    if(s.arrived)assert(s.lift==0 && s.obstacleX==-100);
    if(!s.arrived && s.obstacleX>=3 && s.obstacleX<=94 &&
       s.obstacleX-1<=s.x+4 && s.obstacleX+7>=s.x-4)assert(s.lift>9);
    for(int hour:{0,6,11,12,23}) {
      Pixels canvas;ilr::drawRunnerScene(canvas,s,hour,int(ms/19%60),ms);
      if(s.arrived)assert(canvas.pixels[(ilr::GROUND_Y-17)*128+s.x]==255);
    }
  }
  for(int days:{0,8})for(uint32_t ms:{0xffffff00u,0xffffffffu}){Pixels c;ilr::drawRunnerScene(c,ilr::runnerFrame(days,ms),23,59,ms);}
  if(argc==2) {
    std::filesystem::create_directories(argv[1]);
    for(int frame=0;frame<60;++frame){uint32_t ms=frame*90;Pixels c;ilr::drawRunnerScene(c,ilr::runnerFrame(17,ms),10,10,ms);c.write(std::string(argv[1])+"/frame_"+std::to_string(frame)+".pgm");}
    Pixels c;ilr::drawRunnerScene(c,ilr::runnerFrame(0,1400),10,10,1400);c.write(std::string(argv[1])+"/arrived.pgm");
    for(int frame=0;frame<134;++frame){uint32_t ms=frame*90;Pixels finish;ilr::drawRunnerScene(finish,ilr::runnerFrame(0,ms),10,10,ms);finish.write(std::string(argv[1])+"/finish_"+std::to_string(frame)+".pgm");}
  }
  std::cout<<"Runner progress, jump clearance, milestones, pixel bounds, timer rollover: passed\n";
}
