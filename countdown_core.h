#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <ctime>

namespace ilr {
constexpr const char* LONDON_TZ = "GMT0BST,M3.5.0/1,M10.5.0/2";
constexpr const char* DEFAULT_TARGET_DATE = "2026-11-17";
struct Date { int year=0, month=0, day=0; };
inline bool leap(int y) { return y%4==0 && (y%100!=0 || y%400==0); }
inline int monthDays(int y,int m) {
  const int n[]={31,28,31,30,31,30,31,31,30,31,30,31};
  return m<1||m>12 ? 0 : n[m-1]+(m==2 && leap(y));
}
inline bool valid(Date d) {
  return d.year>=1970 && d.year<=2099 && d.month>=1 && d.month<=12 &&
         d.day>=1 && d.day<=monthDays(d.year,d.month);
}
inline bool parseDate(const char* s,Date& out) {
  if(!s || std::strlen(s)!=10 || s[4]!='-' || s[7]!='-') return false;
  for(int i=0;i<10;++i) if(i!=4 && i!=7 && (s[i]<'0'||s[i]>'9'))return false;
  Date d{(s[0]-'0')*1000+(s[1]-'0')*100+(s[2]-'0')*10+s[3]-'0',
         (s[5]-'0')*10+s[6]-'0',(s[8]-'0')*10+s[9]-'0'};
  if(!valid(d))return false;
  out=d;return true;
}
inline int32_t dayNumber(Date d) {
  // Calendar arithmetic, independent of 23/25-hour DST transition days.
  int32_t result=0;
  for(int y=1970;y<d.year;++y) result+=leap(y)?366:365;
  for(int m=1;m<d.month;++m) result+=monthDays(d.year,m);
  return result+d.day-1;
}
inline int remaining(Date today,Date target) { return dayNumber(target)-dayNumber(today); }
// The application sets TZ to LONDON_TZ before calling this function.
inline int64_t remainingSeconds(std::time_t now,Date target) {
  if(!valid(target))return 0;
  std::tm midnight{};
  midnight.tm_year=target.year-1900;midnight.tm_mon=target.month-1;
  midnight.tm_mday=target.day;midnight.tm_isdst=-1;
  const std::time_t end=std::mktime(&midnight);
  const int64_t seconds=static_cast<int64_t>(end)-static_cast<int64_t>(now);
  return seconds>0?seconds:0;
}
inline int completedMarks(int days) { return days>=17?0:days<=0?17:17-days; }
inline void label(Date d,char* dst,size_t size) {
  if(!valid(d)){std::snprintf(dst,size,"НЕТ ДАТЫ");return;}
  std::snprintf(dst,size,"%02d.%02d.%04d",d.day,d.month,d.year);
}
}
