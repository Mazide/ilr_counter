#pragma once
#include "../countdown_styles.h"

namespace ilr {
// Nine ASCII scenes shared by the device and the concept previews.
struct CountdownConcept:CountdownStyle {
  void line(int x,int y,int x2,int y2,char ink='.') {
    int dx=std::abs(x2-x),sx=x<x2?1:-1,dy=-std::abs(y2-y),sy=y<y2?1:-1,e=dx+dy;
    for(;;){put(x,y,ink);if(x==x2&&y==y2)break;int q=2*e;if(q>=dy){e+=dy;x+=sx;}if(q<=dx){e+=dx;y+=sy;}}
  }
  void countLine(int y,int days) {
    char text[64];snprintf(text,sizeof(text),"%d %s ДО ILR",days>0?days:0,dayWord(days));center(y,text);
  }
  void bigCount(int days,int top,int sy=1) {
    char text[12];snprintf(text,sizeof(text),"%d",days>0?days:0);
    banner(text,(W-(int(strlen(text))*6-1))/2,top,1,sy);
  }
  void horizon(int days,uint32_t ms) {
    center(0,"ГОРИЗОНТ СОБЫТИЙ");
    const float t=ms*.0007f;
    for(int y=3;y<18;++y)for(int x=1;x<31;++x) {
      const float nx=(x-15.5f)/13,ny=(y-10.f)/6;
      const float r=std::sqrt(nx*nx+ny*ny),a=std::atan2(ny,nx);
      if(std::abs(r-.78f)<.16f) {
        int shade=int((std::sin(a*3-t)+1)*2.4f);
        put(x,y,".:-=+#"[shade]);
      } else if(r>1.03f && noise(x,y)>.94f)put(x,y,'.');
    }
    char value[12];snprintf(value,sizeof(value),"%d",days>0?days:0);center(9,value);center(11,dayWord(days));
    put(16+int(12*std::cos(t)),10+int(6*std::sin(t)),'*');
  }
  void train(int days,uint32_t ms) {
    center(0,"НОЧНОЙ ПОЕЗД");countLine(2,days);
    for(int x=0;x<32;++x) {
      const int h=6+int(2*std::sin((x+ms/650.f)*.35f));
      put(x,h,'_');if(x%5==0)put(x,5,'/');
    }
    int left=int(ms/140%58)-25;
    box(left,9,25,7);at(left+1,8,"_______________________");
    for(int i=0;i<4;++i)box(left+2+i*5,10,4,3);
    at(left+7,14,"Л О Н Д О Н");put(left+4,16,'o');put(left+20,16,'o');
    for(int x=0;x<32;++x){put(x,17,'=');put(x,18,(x+ms/100)%4==0?'|':'_');}
  }
  void beacon(int days,uint32_t ms) {
    center(0,"МАЯК");
    const int ray=4+int((std::sin(ms*.0008f)+1)*3);
    line(11,7,30,ray,':');line(11,8,30,ray+3,'.');line(30,ray,30,ray+3,':');
    at(6,4,"  /\\");at(6,5," /__\\");box(7,6,6,3);at(8,7,"[*]");
    line(8,9,5,16,'/');line(11,9,14,16,'\\');at(8,12,"[_]");at(5,17,"__________");
    for(int y=16;y<19;++y)for(int x=0;x<17;++x)if((x+int(ms/180)+y*3)%8<3)put(x,y,'~');
    char n[12];snprintf(n,sizeof(n),"%d",days>0?days:0);
    if(strlen(n)<=2)banner(n,20,10);else at(19,12,n);
    at(22,18,dayWord(days));
  }
  void portal(int days,uint32_t ms) {
    center(0,"ПОРТАЛ");
    box(1,3,30,15);box(5,5,22,13);box(9,6,14,12);
    line(1,3,9,6,'\\');line(30,3,22,6,'/');line(1,17,9,17,'/');line(30,17,22,17,'\\');
    for(int i=0;i<10;++i) {
      const int x=2+int(noise(i,4)*27),y=4+int((ms/180+i*3)%13);
      if(x<8||x>23)put(x,y,i%2?'.':'+');
    }
    bigCount(days,8);put(21,13,'o');center(18,"ДО ILR");
  }
  void tide(int days,uint32_t ms) {
    center(0,"ОТЛИВ");bigCount(days,3,2);
    const int level=6+completedMarks(days)*10/17;
    for(int x=0;x<32;++x) {
      const int y=level+int(std::sin(x*.35f-ms*.002f));
      for(int row=y+1;row<18;++row) {
        if(cells[row][x]=='#')cells[row][x]=':';
        else if((x+row*2+ms/200)%13==0)put(x,row,'.');
      }
      put(x,y,'~');
    }
    char caption[40];snprintf(caption,sizeof(caption),"%s ДО ILR",dayWord(days));center(18,caption);
  }
  void signal(int days,uint32_t ms) {
    center(0,"СИГНАЛ");countLine(2,days);
    bigCount(days,5);
    const int scan=ms/110%40-4;
    for(int y=4;y<13;++y)for(int x=0;x<32;++x)
      if(std::abs(x-scan)<2 && noise(x,int(ms/180)+y)>.5f)put(x,y,".:+*"[(x+y)%4]);
    for(int x=2;x<30;++x) {
      int h=1+int((1+std::sin(x*.7f+ms*.003f)+std::sin(x*.23f-ms*.001f))*1.2f);
      if(h<1)h=1;
      if(h>4)h=4;
      for(int y=0;y<h;++y)put(x,17-y,'|');
    }
  }
  void vinyl(int days,uint32_t ms) {
    center(0,"ВИНИЛ");
    const float t=ms*.001f;
    for(int y=3;y<=17;++y)for(int x=0;x<=22;++x) {
      float nx=(x-11.f)/10,ny=(y-10.f)/6.8f;
      float r=std::sqrt(nx*nx+ny*ny),a=std::atan2(ny,nx);
      if(r<=1 && r>.25f && (int(r*13)%3==0))put(x,y,std::sin(a-t)>.6f?'#':':');
    }
    at(10,10,"ILR");line(28,4,22,11,'/');at(21,11,"[]");
    char n[12];snprintf(n,sizeof(n),"%d",days>0?days:0);at(24,13,n);at(24,15,dayWord(days));
  }
  void summit(int days,uint32_t ms) {
    center(0,"ВЕРШИНА");countLine(2,days);
    const int points[][2]={{0,15},{5,11},{9,13},{16,6},{20,10},{25,5},{31,12}};
    for(int i=0;i<6;++i)line(points[i][0],points[i][1],points[i+1][0],points[i+1][1],i%2?'\\':'/');
    line(25,5,25,3,'|');at(26,3,"=>");
    for(int y=13;y<18;++y)for(int x=0;x<32;++x)if((x+y*3)%11==0)put(x,y,'.');
    const int path[][2]={{3,17},{6,16},{9,17},{12,15},{10,14},{14,13},{17,12},{18,10},{22,10},{23,8},{25,6}};
    for(int i=0;i<10;++i)line(path[i][0],path[i][1],path[i+1][0],path[i+1][1],'.');
    int p=completedMarks(days)*10/17;put(path[p][0],path[p][1],ms/400%2?'o':'*');
  }
  void constellation(int days,uint32_t ms) {
    center(0,"СОЗВЕЗДИЕ");
    const int points[][2]={{3,4},{8,3},{13,4},{18,3},{24,4},{29,6},{28,9},{30,12},{27,15},
                         {24,18},{19,17},{14,18},{9,17},{4,18},{2,14},{3,10},{1,7}};
    int count=completedMarks(days);
    for(int i=0;i<17;++i) {
      if(i<count && i>0)line(points[i-1][0],points[i-1][1],points[i][0],points[i][1],'.');
      const bool pulse=ms/500%3==unsigned(i%3);
      put(points[i][0],points[i][1],i<count?(pulse?'*':'+'):(pulse?'*':'.'));
    }
    bigCount(days,6);center(14,dayWord(days));
  }
  void renderConcept(int mode,int days,Date target,uint32_t ms) {
    for(auto& row:cells){memset(row,' ',W);row[W]=0;}
    switch(mode) {
      case 0:horizon(days,ms);break;case 1:train(days,ms);break;
      case 2:beacon(days,ms);break;case 3:portal(days,ms);break;
      case 4:tide(days,ms);break;case 5:signal(days,ms);break;
      case 6:vinyl(days,ms);break;case 7:summit(days,ms);break;
      default:constellation(days,ms);break;
    }
    memset(cells[19],' ',W);
    char date[20];label(target,date,sizeof(date));center(20,date);
  }
};
}
