#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <time.h>
#include <esp_sntp.h>
#include "countdown_core.h"
#include "runner_scene.h"
#include "countdown_styles.h"
#include "concepts/ten_concepts.h"
#include "style_cycle.h"
#include "display_quotes.h"

constexpr uint8_t SDA_PIN=4, SCL_PIN=5, SETUP_BUTTON_PIN=9;
constexpr const char* AP_NAME="ILR-Countdown";
constexpr const char* AP_PASSWORD="17daystogo";
U8G2_SH1107_PIMORONI_128X128_F_HW_I2C oled(U8G2_R0,U8X8_PIN_NONE,SCL_PIN,SDA_PIN);
WiFiManager wm;
WiFiManagerParameter dateInput("ilrdate","Дата ILR (ГГГГ-ММ-ДД, с годом)","",10,
                             "placeholder='2026-11-17' pattern='[0-9]{4}-[0-9]{2}-[0-9]{2}' required");
ilr::Date target;
ilr::CountdownConcept asciiStyle;
// London, dissolve/hourglass, and nine animated scenes.
ilr::StyleCycle styleCycle;
const char* const STYLE_NAMES[]={"Лондон","Исчезновение","Песочные часы",
  "Горизонт событий","Ночной поезд","Маяк","Портал","Отлив",
  "Сигнал","Винил","Вершина","Созвездие"};
static_assert(sizeof(STYLE_NAMES)/sizeof(STYLE_NAMES[0])==ilr::StyleCycle::COUNT,"Style names");
char targetText[11]={};
bool targetValid=false, screenReady=false, previousWifi=false, buttonHeld=false;
volatile bool timeSynced=false;
uint32_t buttonSince=0, frameAt=0, screenAttempt=0, portalAt=0, infoUntil=0;
bool portalTimed=false, badDate=false;
bool celebrating=false;
uint32_t celebrationSince=0;
constexpr uint32_t PORTAL_MS=10*60*1000;

void selectStyle(uint8_t style) {
  styleCycle.select(style,millis());
  Preferences prefs;prefs.begin("ilr-countdown",false);
  prefs.putUChar("scene",styleCycle.style);prefs.end();
  Serial.printf("Экран %u: %s (по 20 с)\n",styleCycle.style,STYLE_NAMES[styleCycle.style]);
}

void centered(int y,const char* s,const uint8_t* font=u8g2_font_5x8_t_cyrillic) {
  oled.setFont(font);oled.setFontPosTop();
  oled.drawUTF8((128-int(oled.getUTF8Width(s)))/2,y,s);
}
void drawQuote(int y,uint8_t style) {
  const char* quote=ilr::displayQuote(style);
  oled.setFont(u8g2_font_4x6_t_cyrillic);oled.setFontPosTop();
  oled.drawUTF8((128-int(oled.getUTF8Width(quote)))/2,y,quote);
}
void message(const char* a,const char* b,const char* c="",const char* d="") {
  oled.clearBuffer();centered(15,a);centered(43,b);centered(70,c);
  centered(91,d);oled.sendBuffer();
}
bool findScreen() {
  for(uint8_t a:{uint8_t(0x3C),uint8_t(0x3D)}) {
    Wire.beginTransmission(a);
    if(Wire.endTransmission()!=0)continue;
    oled.setI2CAddress(a<<1);oled.setBusClock(400000);oled.begin();
    oled.setPowerSave(0);oled.setContrast(100);
    Serial.printf("OLED 0x%02X готов\n",a);return true;
  }
  Serial.println("Нет OLED: проверьте SDA4 SCL5 3V3 GND");return false;
}
void saveDate() {
  ilr::Date candidate;
  const char* value=dateInput.getValue();
  if(!ilr::parseDate(value,candidate)) {
    badDate=true;Serial.println("Неверная дата; прежняя сохранена. Формат ГГГГ-ММ-ДД, 1970..2099.");return;
  }
  Preferences store;store.begin("ilr-countdown",false);
  store.putString("date",value);store.end();
  target=candidate;targetValid=true;badDate=false;
  snprintf(targetText,sizeof(targetText),"%s",value);
  Serial.printf("Дата ILR сохранена: %s (Лондон)\n",targetText);
}
void openSetup() {
  if(wm.getConfigPortalActive())return;
  wm.stopWebPortal();
  dateInput.setValue(targetText,10);
  wm.startConfigPortal(AP_NAME,AP_PASSWORD);
  portalAt=millis();portalTimed=true;
}
void onTimeSync(struct timeval*) { timeSynced=true; }

void drawCountdown(const tm& local,time_t epoch,uint32_t now) {
  const ilr::Date today{local.tm_year+1900,local.tm_mon+1,local.tm_mday};
  const int days=ilr::remaining(today,target);
  if(days<=0 && !celebrating)celebrationSince=now;
  celebrating=days<=0;
  if(styleCycle.update(now))Serial.printf("Экран %u: %s\n",styleCycle.style,STYLE_NAMES[styleCycle.style]);
  const uint8_t displayStyle=celebrating?0:styleCycle.style;
  const uint32_t animationMs=celebrating?uint32_t(now-celebrationSince):styleCycle.elapsed(now);
  if(displayStyle) {
    if(displayStyle<=2)asciiStyle.render(displayStyle-1,days,target,animationMs);
    else asciiStyle.renderConcept(displayStyle-3,days,target,animationMs);
    oled.clearBuffer();oled.setFont(u8g2_font_4x6_t_cyrillic);oled.setFontPosTop();
    char row[ilr::CountdownStyle::W*2+1];
    for(int y=0;y<ilr::CountdownStyle::H;++y) {
      ilr::rowUTF8(asciiStyle.cells[y],row,sizeof(row));
      oled.drawUTF8(0,1+y*6,row);
    }
    drawQuote(115,displayStyle);
    oled.sendBuffer();return;
  }
  char dateLabel[20];ilr::label(target,dateLabel,sizeof(dateLabel));
  oled.clearBuffer();
  centered(2,"ДО ПОДАЧИ ILR");
  const int64_t seconds=ilr::remainingSeconds(epoch,target);
  char digits[24];snprintf(digits,sizeof(digits),"%lld",static_cast<long long>(seconds));
  const uint8_t* fonts[]={u8g2_font_logisoso32_tn,u8g2_font_logisoso24_tn,u8g2_font_logisoso20_tn};
  const uint8_t* chosen=fonts[2];
  for(auto f:fonts){oled.setFont(f);if(oled.getStrWidth(digits)<=122){chosen=f;break;}}
  centered(14,digits,chosen);
  centered(50,"СЕКУНД");
  centered(61,dateLabel);
  const auto runner=ilr::runnerFrame(days,animationMs);
  ilr::drawRunnerScene(oled,runner,local.tm_hour,local.tm_min,animationMs);
  const int marks=ilr::completedMarks(days);
  for(int i=0;i<17;++i) {
    const int x=5+i*7;
    if(i<marks)oled.drawBox(x,125,5,3);else oled.drawFrame(x,125,5,3);
  }
  // Heartbeat remains independent of the game scene.
  if(local.tm_sec%2==0)oled.drawPixel(124,2);
  if(WiFi.status()!=WL_CONNECTED) {
    oled.drawHLine(1,2,5);oled.drawHLine(1,4,3);
  }
  oled.sendBuffer();
}

void setup() {
  Serial.begin(115200);pinMode(SETUP_BUTTON_PIN,INPUT_PULLUP);
  Wire.begin(SDA_PIN,SCL_PIN,100000);Wire.setTimeOut(25);
  screenReady=findScreen();
  if(screenReady)message("ОТСЧЁТ ДО ILR","ПОДКЛЮЧЕНИЕ", "BOOT 3 с: настройки");
  Preferences store;store.begin("ilr-countdown",true);
  String saved=store.getString("date",ilr::DEFAULT_TARGET_DATE);
  styleCycle.style=store.getUChar("scene",ilr::StyleCycle::fromLegacy(store.getUChar("style",0)));
  if(styleCycle.style>=ilr::StyleCycle::COUNT)styleCycle.style=0;
  store.end();
  targetValid=ilr::parseDate(saved.c_str(),target);
  if(targetValid)snprintf(targetText,sizeof(targetText),"%s",saved.c_str());
  dateInput.setValue(targetText,10);
  WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(true);
  wm.setDebugOutput(false);wm.setConfigPortalBlocking(false);wm.setConnectTimeout(12);
  wm.setTitle("Отсчёт до ILR");
  wm.setConnectRetries(1);wm.addParameter(&dateInput);wm.setSaveParamsCallback(saveDate);
  wm.setAPCallback([](WiFiManager*){portalAt=millis();portalTimed=true;});
  std::vector<const char*> menu={"wifi","info","close"};
  wm.setMenu(menu); // Date is shown alongside Wi-Fi credentials on the same form.
  sntp_set_time_sync_notification_cb(onTimeSync);
  configTzTime(ilr::LONDON_TZ,"time.cloudflare.com","pool.ntp.org","time.google.com");
  wm.autoConnect(AP_NAME,AP_PASSWORD);
  if(!targetValid && !wm.getConfigPortalActive())openSetup();
  Serial.println("ILR: 12 экранов по 20 с; n следующий, 0..9/A..B выбор, ? состояние, c настройки. BOOT: следующий; 3 с: настройки.");
}

void loop() {
  wm.process();
  uint32_t now=millis();
  const bool down=digitalRead(SETUP_BUTTON_PIN)==LOW;
  if(down && !buttonSince)buttonSince=now;
  if(down && !buttonHeld && now-buttonSince>=3000){buttonHeld=true;openSetup();}
  if(!down && buttonSince) {
    if(!buttonHeld) {
      selectStyle(styleCycle.style+1);
    }
    buttonSince=0;buttonHeld=false;
  }
  while(Serial.available()) {
    const char c=Serial.read();
    if(c=='c')openSetup();
    if(c>='0' && c<='9')selectStyle(c-'0');
    if(c>='A' && c<='B')selectStyle(10+c-'A');
    if(c=='n')selectStyle(styleCycle.style+1);
    if(c=='i')infoUntil=now+5000;
    if(c=='?')Serial.printf("style=%u name=%s auto=1 interval=20s date=%s synced=%d wifi=%d ip=%s\n",styleCycle.style,STYLE_NAMES[styleCycle.style],targetText,timeSynced,WiFi.status()==WL_CONNECTED,WiFi.localIP().toString().c_str());
  }
  if(portalTimed && wm.getConfigPortalActive() && now-portalAt>=PORTAL_MS){wm.stopConfigPortal();portalTimed=false;}
  const bool online=WiFi.status()==WL_CONNECTED;
  if(online && !previousWifi) {
    Serial.printf("Подключено; настройки: http://%s/param\n",WiFi.localIP().toString().c_str());
    // Start NTP after IP acquisition as well; retain a valid clock during outages.
    configTzTime(ilr::LONDON_TZ,"time.cloudflare.com","pool.ntp.org","time.google.com");
  }
  if(online && !wm.getConfigPortalActive() && !wm.getWebPortalActive())wm.startWebPortal();
  previousWifi=online;
  if(!screenReady) {
    if(now-screenAttempt>=3000){screenAttempt=now;screenReady=findScreen();}
    delay(2);return;
  }
  if(now-frameAt<90){delay(2);return;}frameAt=now;
  if(wm.getConfigPortalActive()) {
    message("НАСТРОЙКА WI-FI",AP_NAME,AP_PASSWORD,"192.168.4.1");return;
  }
  if(badDate) {message("НЕВЕРНАЯ ДАТА","ГГГГ-ММ-ДД", "Укажите год", "BOOT 3 с: настройки");return;}
  if(!targetValid) {message("ЗАДАЙТЕ ДАТУ ILR","BOOT 3 с: настройки","Укажите год");return;}
  if(infoUntil && int32_t(infoUntil-now)>0) {
    String ip=online?WiFi.localIP().toString():"Нет Wi-Fi";
    message("НАСТРОЙКИ",ip.c_str(),"Откройте /param","BOOT 3 с: Wi-Fi");return;
  }
  time_t epoch=time(nullptr);tm local{};
  if(!timeSynced || epoch<1704067200 || !localtime_r(&epoch,&local)) {
    message("ВРЕМЯ ЛОНДОНА",online?"СИНХРОНИЗАЦИЯ":"ПОДКЛЮЧЕНИЕ WI-FI", "Ожидание точного времени", "BOOT 3 с: настройки");return;
  }
  infoUntil=0;
  drawCountdown(local,epoch,now);
}
