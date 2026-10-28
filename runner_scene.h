#pragma once
#include <cstdint>
#include <initializer_list>
#include "countdown_core.h"

namespace ilr {
constexpr int GROUND_Y=119;
struct RunnerFrame {
  int x, obstacleX, lift, stride, obstacleKind;
  bool arrived;
};
inline RunnerFrame runnerFrame(int days,uint32_t ms) {
  const bool arrived=days<=0;
  // Repeat the approach and fireworks; keep the runner visible beside the tower.
  const uint32_t finishMs=ms%12000;
  const int x=arrived?80+int((finishMs<3000?finishMs:3000)*14/3000):16+completedMarks(days)*4;
  const int distance=62-int((ms%2700)*86/2700);
  int lift=0;
  if(!arrived && distance>-24 && distance<24)lift=18*(576-distance*distance)/576;
  return {x,arrived?-100:x+distance,lift,int(ms/110%4),int(ms/2700%2),arrived};
}

// Shared by the actual U8g2 display and the host preview/test pixel canvas.
template<class Canvas>
void drawRunnerScene(Canvas& c,const RunnerFrame& s,int hour,int minute,uint32_t ms) {
  // Distant clouds drift more slowly than the bridge traffic and pavement.
  for(int i=0;i<2;++i) {
    const int cx=8+(i*41+84-int(ms/800%84))%84;
    const int cy=i?73:79;
    c.drawHLine(cx-4,cy+2,11);
    c.drawLine(cx-5,cy+1,cx-4,cy-1);c.drawHLine(cx-4,cy-1,3);
    c.drawLine(cx-2,cy-1,cx,cy-4);c.drawLine(cx,cy-4,cx+3,cy-3);
    c.drawVLine(cx+3,cy-2,2);c.drawHLine(cx+3,cy,3);
    c.drawLine(cx+6,cy,cx+7,cy+2);
  }
  // Westminster Bridge, behind the foreground pavement and runner.
  c.drawHLine(0,103,96);c.drawHLine(0,105,96);
  for(int x=3;x<96;x+=8)c.drawVLine(x,101,3);
  for(int left:{0,32,64}) {
    c.drawLine(left+3,116,left+7,111);c.drawLine(left+7,111,left+12,108);
    c.drawHLine(left+12,108,9);
    c.drawLine(left+20,108,left+25,111);c.drawLine(left+25,111,left+29,116);
    c.drawVLine(left+1,106,11);c.drawVLine(left+30,106,11);
  }
  // Double-decker, moving independently across the distant bridge.
  const int busX=3+int(ms/180%70);
  c.drawFrame(busX,88,21,12);c.drawHLine(busX+2,87,16);
  for(int i=0;i<4;++i)c.drawBox(busX+2+i*4,90,2,2);
  for(int i=0;i<3;++i)c.drawBox(busX+2+i*4,95,2,2);
  c.drawFrame(busX+16,94,3,6);
  c.drawDisc(busX+4,101,1);c.drawDisc(busX+16,101,1);
  // Big Ben: spire, roof, clock, stonework and doorway.
  c.drawLine(109,76,109,79);
  c.drawLine(109,79,103,84);c.drawLine(109,79,115,84);
  c.drawHLine(102,84,15);c.drawFrame(102,85,15,12);
  c.drawCircle(109,91,4);
  const int hand[12][2]={{0,-4},{2,-3},{3,-2},{4,0},{3,2},{2,3},
                        {0,4},{-2,3},{-3,2},{-4,0},{-3,-2},{-2,-3}};
  c.drawLine(109,91,109+hand[(minute/5)%12][0],91+hand[(minute/5)%12][1]);
  c.drawLine(109,91,109+hand[hour%12][0]/2,91+hand[hour%12][1]/2);
  c.drawFrame(103,97,13,23);c.drawHLine(101,100,17);
  c.drawVLine(106,103,12);c.drawVLine(112,103,12);
  c.drawFrame(108,115,3,5);
  if(s.arrived && ms%12000>=3000) {
    // Three staggered bursts: launch, expansion, then falling sparks.
    const int rays[8][2]={{0,-10},{7,-7},{10,0},{7,7},{0,10},{-7,7},{-10,0},{-7,-7}};
    for(int i=0;i<3;++i) {
      const uint32_t phase=(ms%12000-3000+i*700)%2200;
      const int cx=16+i*34,cy=82+(i%2)*4;
      if(phase<450) {
        const int y=107-int(phase*25/450);
        c.drawVLine(cx,y,3);
      } else if(phase<1800) {
        const int age=int(phase-450),radius=2+age*8/1350;
        for(const auto& ray:rays) {
          const int px=cx+ray[0]*radius/10;
          const int py=cy+ray[1]*radius/10+age*age/450000;
          if(age<900 || (age/100+i)%2==0)
            c.drawLine(cx+ray[0]*(radius-2)/10,cy+ray[1]*(radius-2)/10+age*age/450000,px,py);
        }
      }
    }
  }
  c.drawHLine(0,GROUND_Y+1,128);
  // Moving pavement makes the automatic runner read as a tiny platformer.
  for(int i=0;i<4;++i) {
    const int px=(i*29+128-int(ms/65%128))%128;
    if(px<=121)c.drawHLine(px,GROUND_Y+3,6);
  }
  if(!s.arrived && s.obstacleX>=3 && s.obstacleX<=94) {
    c.setDrawColor(0);c.drawBox(s.obstacleX-2,GROUND_Y-10,12,11);c.setDrawColor(1);
    if(s.obstacleKind==0) {
      c.drawFrame(s.obstacleX,GROUND_Y-7,7,7);
      c.drawHLine(s.obstacleX+2,GROUND_Y-9,3);
      c.drawVLine(s.obstacleX+2,GROUND_Y-8,2);
      c.drawVLine(s.obstacleX+4,GROUND_Y-8,2);
    } else {
      c.drawFrame(s.obstacleX-1,GROUND_Y-8,9,4);
      c.drawLine(s.obstacleX,GROUND_Y-7,s.obstacleX+2,GROUND_Y-5);
      c.drawLine(s.obstacleX+4,GROUND_Y-7,s.obstacleX+6,GROUND_Y-5);
      c.drawVLine(s.obstacleX,GROUND_Y-4,4);
      c.drawVLine(s.obstacleX+6,GROUND_Y-4,4);
    }
  }
  const int x=s.x, feet=GROUND_Y-s.lift;
  int lines[8][4]={};int count=0;
  auto segment=[&](int x1,int y1,int x2,int y2){
    lines[count][0]=x1;lines[count][1]=y1;lines[count][2]=x2;lines[count++][3]=y2;
  };
  segment(x,feet-14,x,feet-8);
  if(s.arrived && s.x>=94) {
    segment(x,feet-12,x-4,feet-9);
    segment(x,feet-12,x+4,feet-15);
    segment(x+4,feet-15,x+4+(ms/300%2?2:-1),feet-19);
    segment(x,feet-8,x-3,feet);segment(x,feet-8,x+3,feet);
  } else {
    const int leg=s.lift?4:(s.stride==0?5:s.stride==2?-5:1);
    segment(x,feet-8,x+leg,feet-4);segment(x+leg,feet-4,x+leg-2,feet);
    segment(x,feet-8,x-leg,feet-1);
    segment(x,feet-12,x-leg,feet-9);segment(x,feet-12,x+leg,feet-14);
  }
  // One-pixel dark outline separates the foreground sprite from the bridge.
  c.setDrawColor(0);c.drawDisc(x,feet-17,3);
  for(int i=0;i<count;++i)for(int dx=-1;dx<=1;++dx)for(int dy=-1;dy<=1;++dy)
    c.drawLine(lines[i][0]+dx,lines[i][1]+dy,lines[i][2]+dx,lines[i][3]+dy);
  c.setDrawColor(1);c.drawDisc(x,feet-17,2);
  for(int i=0;i<count;++i)c.drawLine(lines[i][0],lines[i][1],lines[i][2],lines[i][3]);
}
}
