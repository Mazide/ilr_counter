#include "../countdown_styles.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>

int main(int argc,char** argv) {
  ilr::CountdownStyle s;
  for(int style=0;style<2;++style)for(int days:{-1,0,1,7,17,27,99,100,9999,30000}) {
    std::string previous;int changes=0;
    for(uint32_t ms=0;ms<24000;ms+=97) {
      s.render(style,days,{2026,11,17},ms);
      std::string frame;
      for(auto& row:s.cells) {
        assert(row[32]==0);
        for(int x=0;x<32;++x){assert(ilr::validCell(uint8_t(row[x])));frame+=row[x];}
      }
      assert(frame.find("17.11.2026")!=std::string::npos);
      if(frame!=previous)++changes;
      previous=frame;
    }
    if(days>0)assert(changes>3);
  }
  for(int style=0;style<2;++style)s.render(style,27,{2026,11,17},0xffffffffu);
  if(argc==2) {
    std::ofstream out(argv[1]);out<<"[";
    for(int style=0;style<2;++style) {
      if(style)out<<",";
      out<<"[";
      for(int f=0;f<120;++f){
        if(f)out<<",";
        out<<"[";
        s.render(style,27,{2026,11,17},f*100);
        for(int y=0;y<21;++y){
          if(y)out<<",";
          out<<"\"";
          char utf8[65];ilr::rowUTF8(s.cells[y],utf8,sizeof(utf8));
          for(int x=0;utf8[x];++x){char c=utf8[x];if(c=='\\'||c=='\"')out<<'\\';out<<c;}
          out<<"\"";
        }
        out<<"]";
      }
      out<<"]";
    }
    out<<"]";
  }
  std::cout<<"Two styles: valid text cells, dates, day ranges, animation and timer rollover passed\n";
}
