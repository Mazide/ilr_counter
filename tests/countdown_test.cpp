#include "../countdown_core.h"
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>

int main() {
  ilr::Date d{2000,1,1};
  for(const char* s:{"","2026-1-17","2026-02-29","2026-10-32","2026-13-01","2026-00-01","2026-10-00","2026-10-17x","2026/10/17","202a-10-17"}) {
    assert(!ilr::parseDate(s,d));assert(d.year==2000 && d.month==1 && d.day==1);
  }
  assert(ilr::parseDate("2028-02-29",d));assert(d.day==29);
  assert(ilr::remaining({2026,10,20},{2026,11,6})==17);
  assert(ilr::remaining({2026,10,20},{2026,10,17})==-3);
  assert(ilr::remaining({2026,10,17},{2026,10,17})==0);
  assert(ilr::remaining({2026,12,31},{2027,1,1})==1);
  assert(ilr::remaining({2028,2,28},{2028,3,1})==2);
  assert(ilr::completedMarks(30)==0 && ilr::completedMarks(17)==0);
  assert(ilr::completedMarks(1)==16 && ilr::completedMarks(0)==17 && ilr::completedMarks(-2)==17);
  setenv("TZ",ilr::LONDON_TZ,1);tzset();
  auto midnight=[](int y,int m,int d){tm t{};t.tm_year=y-1900;t.tm_mon=m-1;t.tm_mday=d;t.tm_isdst=-1;return mktime(&t);};
  assert(midnight(2026,3,30)-midnight(2026,3,29)==23*3600);
  assert(midnight(2026,10,26)-midnight(2026,10,25)==25*3600);
  assert(ilr::remainingSeconds(midnight(2026,3,29),{2026,3,30})==23*3600);
  assert(ilr::remainingSeconds(midnight(2026,10,25),{2026,10,26})==25*3600);
  const auto deadline=midnight(2026,11,17);
  assert(ilr::remainingSeconds(deadline-2,{2026,11,17})==2);
  assert(ilr::remainingSeconds(deadline-1,{2026,11,17})==1);
  assert(ilr::remainingSeconds(deadline,{2026,11,17})==0);
  assert(ilr::remainingSeconds(deadline+1,{2026,11,17})==0);
  assert(ilr::remainingSeconds(midnight(2026,10,23),{2026,11,17})==25*86400+3600);
  assert(ilr::remainingSeconds(midnight(2026,10,23),{2099,1,1})>INT32_MAX);
  assert(ilr::remaining({2026,3,29},{2026,3,30})==1);
  assert(ilr::remaining({2026,10,25},{2026,10,26})==1);
  // London date can already be tomorrow while UTC is still yesterday.
  tm utc{};utc.tm_year=126;utc.tm_mon=6;utc.tm_mday=1;utc.tm_hour=23;utc.tm_min=30;
  time_t epoch=timegm(&utc);tm uk{};localtime_r(&epoch,&uk);
  assert(uk.tm_mday==2 && uk.tm_hour==0);
  assert(ilr::remaining({uk.tm_year+1900,uk.tm_mon+1,uk.tm_mday},{2026,7,3})==1);
  for(int y=2024;y<=2030;++y) for(int m=1;m<=12;++m)for(int day=1;day<=ilr::monthDays(y,m);++day) {
    ilr::Date a{y,m,day};ilr::Date b=a;
    if(++b.day>ilr::monthDays(b.year,b.month)){b.day=1;if(++b.month>12){b.month=1;++b.year;}}
    assert(ilr::remaining(a,b)==1);
  }
  std::cout<<"Calendar, validation, rollover and London DST tests passed\n";
}
