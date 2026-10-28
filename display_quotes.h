#pragma once
#include <stdint.h>

namespace ilr {
// Short excerpts from Krovostok, "Potochnoe". Nine quoted words in total.
// https://www.shazam.com/ru-ru/song/1556756135/
inline const char* displayQuote(uint8_t style) {
  return style%2 ? "Лениво крутим педали" : "Ну да, мы живём в потоке";
}
}
