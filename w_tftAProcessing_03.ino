
/*

  Подключим TFT экран 128x160 точек к Arduino Nano
  и воспроизведём пару-тройку функций языка Processing.

  # Библиотека и железо

  ## Библиотека:
  Tools -- Manage libraries, искать "GFX for various displays" ссылка: https://github.com/moononournation/Arduino_GFX
  я использовал версию 1.3.1

  ## Железо:
  1. TFT экран 1,77' 128x160 -- https://sl.aliexpress.ru/p?key=zWNNtHg магазин SAMIORE Store https://aliexpress.ru/store/1022067?spm=a2g0o.store_pc_home.pcShopHead_12144492.0
  2. Arduino Nano -- https://sl.aliexpress.ru/p?key=ph3atbG магазин S+ S+ S+ Store https://sl.aliexpress.ru/p?key=yb3atko
  3. Макетная плата 170 точек -- https://sl.aliexpress.ru/p?key=3H3at97 магазин тот же
  4. Кабель 20 см male-female (подадобится 8 проводов) -- https://sl.aliexpress.ru/p?key=gz3at7b магазин тот же

  ## Распиновка:
  На экране |  ..  | На arduino nano |
       1               GND            Минус
       2    VIN        5V             Питание
       3    SCK SCL    D13            Синхронизация
       4    MOSI SDA   D11            Данные
       5    RES RESET  D7             Сброс дисплея
       6    RS DC      D8             Выбор передача данных или команд
       7    CS SS      D9             Выбор экрана (если их несколько)
       8    VDD33      3v3            Подсветка экрана
       -    MISO       -


  # Описание функций и констант

  ## Привычные функции и константы языка Processing:

  frameRate
  // задаётся напрямую константой, не функцией

  width
  // ширина экрана

  height
  // высота экрана

  translate()
  translate(x, y)
  // на вход int

  rotate()
  rotate(angle)
  // на вход float

  stroke()
  stroke(gray)
  stroke(v1, v2, v3)
  // на вход int

  line()
  line(x1, y1, x2, y2)
  // в отличие от Processing на входе int

  point()
  point(x, y)
  // на вход int

  background()
  background(gray)
  background(v1, v2, v3)
  // на вход int

  ## Прочие функции и константы

  deg
  // deg = PI / 180
  // удобная константа для перевода градусов в радианы
  // например: alpha = 45*deg

  randInt()
  randInt(a)
  randInt(a, b)
  // возвращает целое  случайное число в диапазоне 0..a или a..b

*/


#include <Arduino_GFX_Library.h>
Arduino_DataBus *bus = create_default_Arduino_DataBus();


// Значения ниже - выбирать из списка плат https://github.com/moononournation/Arduino_GFX/wiki/Display-Class#st7735-lcd
// мне подошла первая 1.8" REDTAB 128x160
Arduino_GFX *gfx = new Arduino_ST7735(bus, 7 /* RST */, 0 /* rotation */);


/***************************************
   Начало блока служебных функций
 **************************************/
float frameRate = 15; //
int framedelay = int(1 / frameRate * 1000); // ожидание в конце loop
int frameCount = 0;

int width, height;
float deg = PI / 180;

int modx, mody; // сдвиги координат для translate
float modangle; // угол поворота для rotate

void resetTransRot() {
  /*сброс переносов и поворотов в начале каждого кадра,
    разместить в начале loop()*/
  modx = 0;
  mody = 0;
  modangle = 0;
}

void translate(int x, int y) {
  modx += x;
  mody += y;
}

void rotate(float angle) {
  modangle += angle;
}

int rcx(int ox, int oy, int px, int py, float angle) {
  /*rotate coords X*/
  return int(ox + cos(angle) * (px - ox) - sin(angle) * (py - oy));
}
int rcy(int ox, int oy, int px, int py, float angle) {
  /*rotate coords Y*/
  return int(oy + sin(angle) * (px - ox) + cos(angle) * (py - oy));
}

uint16_t strokecolor = GREEN; // цвет по умолчанию
void stroke(int r, int g = -1, int b = -1) {
  /*цвет точек и линий*/
  if (g == -1) {
    g = r;
    b = r;
  }
  strokecolor = gfx->color565(b, g, r); // да, порядок не rgb
}

void line(int x0, int y0, int x1, int y1) {
  /*линия с учётом rotate и translate*/
  int x0r, y0r, x1r, y1r;
  x0r = rcx(modx, mody, x0 + modx, y0 + mody, modangle);
  y0r = rcy(modx, mody, x0 + modx, y0 + mody, modangle);
  x1r = rcx(modx, mody, x1 + modx, y1 + mody, modangle);
  y1r = rcy(modx, mody, x1 + modx, y1 + mody, modangle);
  gfx->drawLine(x0r, y0r, x1r, y1r, strokecolor);
}

void point(int x0, int y0) {
  /*точка с учётом rotate и translate*/
  int x0r, y0r;
  x0r = rcx(modx, mody, x0 + modx, y0 + mody, modangle);
  y0r = rcy(modx, mody, x0 + modx, y0 + mody, modangle);
  gfx->drawPixel(x0r, y0r, strokecolor);
}

void background(int r, int g = -1, int b = -1) {
  if (g == -1) {
    g = r;
    b = r;
  }
  uint16_t color = gfx->color565(b, g, r);
  gfx->fillScreen(color);
}

int randInt(int b, int a = 0) {
  /*случайное целое число*/
  return int(random(min(a, b), max(a, b)));
}
/**************************************
  Конец блока служебных функций
**************************************/

int n_count;

void setup() {
  /***************************************
     Начало блока служебных функций
   **************************************/
  gfx->begin();
  gfx->fillScreen(BLACK);
  width = gfx->width();
  height = gfx->height();
  Serial.begin(9600); //// для отладки
  /***************************************
     Конец блока служебных функций
   **************************************/

  n_count = randInt(2, 12);

  background(255, 170, 0);
  for (int i = 0; i < height; i++) {
    stroke(0);
    line(0, i, width, i);
    delay(12);
  }

}

void loop() {
  /***************************************
    Начало блока служебных функций
  **************************************/
  resetTransRot();
  /***************************************
     Конец блока служебных функций
   **************************************/

  if (frameCount % (15 * 33) == 0) {
    n_count = randInt(2, 22);
    background(0);
  }

  translate(width * .5, height * .5);
  float a = frameCount * 2 * deg;
  stroke(255);

  for (int n = 0; n < n_count; n++) {
    float a_n = a + TWO_PI / n_count * n;
    rotate(a_n);
    point(sin(a / 3) * 100 + 10, 0);
    rotate(-a_n);
  }

  /***************************************
    Начало блока служебных функций
  **************************************/
  frameCount ++;
  delay(framedelay);
  /***************************************
     Конец блока служебных функций
   **************************************/
}
