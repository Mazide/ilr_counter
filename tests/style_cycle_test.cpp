#include "../style_cycle.h"
#include <cassert>
#include <iostream>

int main() {
  ilr::StyleCycle cycle;
  // Connecting and synchronizing cannot consume the first screen's time.
  assert(!cycle.update(95000));
  for(unsigned round=0;round<3;++round)for(unsigned i=0;i<12;++i) {
    unsigned step=round*12+i;
    uint32_t start=95000+step*20000;
    assert(cycle.style==i);
    assert(!cycle.update(start+19999));
    assert(cycle.elapsed(start+19999)==19999);
    assert(cycle.update(start+20000));
    assert(cycle.style==(i+1)%12);
    assert(cycle.elapsed(start+20000)==0);
  }
  // A manual selection gets a full twenty seconds before advancing.
  cycle.select(9,1000000);
  assert(!cycle.update(1019999));assert(cycle.style==9);
  assert(cycle.update(1020000));assert(cycle.style==10);
  // Wraparound of millis(), and returning from a long setup portal.
  cycle.select(11,0xfffffff0u);
  assert(!cycle.update(uint32_t(0xfffffff0u+19999u)));
  assert(cycle.update(uint32_t(0xfffffff0u+20000u)));assert(cycle.style==0);
  cycle.select(5,0);assert(cycle.update(600000));assert(cycle.style==6);
  assert(!cycle.update(619999));
  const uint8_t expected[]={0,1,0,2,3,4,5,0,6,7,8,9,10,11};
  for(unsigned i=0;i<14;++i)assert(ilr::StyleCycle::fromLegacy(i)==expected[i]);
  assert(ilr::StyleCycle::fromLegacy(255)==0);
  std::cout<<"12-screen cycle: 20s boundaries, full loops, manual selection, rollover, legacy selection passed\n";
}
