#pragma once
#include <stdint.h>

namespace ilr {
struct StyleCycle {
  static constexpr uint8_t COUNT=12;
  static constexpr uint32_t INTERVAL_MS=20000;
  uint8_t style=0;
  uint32_t since=0;
  bool started=false;

  static uint8_t fromLegacy(uint8_t oldStyle) {
    // Preserve the selected scene from the former 14-screen firmware.
    // Removed departure/tree selections return to the London scene.
    const uint8_t mapping[]={0,1,0,2,3,4,5,0,6,7,8,9,10,11};
    return oldStyle<sizeof(mapping)?mapping[oldStyle]:0;
  }

  void select(uint8_t value,uint32_t now) {
    style=value%COUNT;since=now;started=true;
  }
  bool update(uint32_t now) {
    if(!started){since=now;started=true;return false;}
    if(uint32_t(now-since)<INTERVAL_MS)return false;
    select(style+1,now);return true;
  }
  uint32_t elapsed(uint32_t now) const {return uint32_t(now-since);}
};
}
