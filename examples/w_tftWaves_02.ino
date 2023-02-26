/*

В этой версии добавлено чтение звука с аналогового пина,
от звука меняется цвет волны.

превью https://youtu.be/eytcnHIURFc

*/

#include <Arduino_GFX_Library.h>
Arduino_DataBus *bus = create_default_Arduino_DataBus();
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


float ellipseWave(int x, int r1, int r2, float scale) {
  // две полуоси эллипса и острота пиков scale 0..1
  x = abs(x);
  x = x % (r1 * 2) - r1;
  float xs = x * scale;
  float y = sqrt((1 - (xs * xs) / (r1 * r1)) * (r2 * r2));
  return y;
}


// аналоговый пин для чтения звука
int analogPin = A0;
int amp = 0;
float armax = 0;


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
  background(0);
}


void loop() {
  /***************************************
    Начало блока служебных функций
  **************************************/
  resetTransRot();
  /***************************************
     Конец блока служебных функций
   **************************************/

  rotate(0.057); // поворот чтоб экран меньше мигал

  // чернота
  stroke(0);
  for (int i = 0; i < 200; i++) { //
    //    point(random(width), random(height));
    int x = random(width);
    int y = random(height);
    line(x, y, x, y - 20);
  }

  // анализ звука
  // слушаем микрофон (с усилком, нужно хотя бы вольт-полтора, с диодом против переполюсовки)
  // между одним из каналов (средний контакт на джеке) и землёй стоит резюк примерно 600кОм, чтоб не фонило
  // ещё один резюк ом на 330 (подбор) стоит между 5V и землёй для того, чтобы павербанк не считал нагрузку нулевой и не отрубался =)
  amp = analogRead(analogPin);
  if (amp > armax)
    armax = amp;
  armax *= 0.99;
  float amp_rel = amp / armax;
  //  Serial.println(amp_rel); ////
  int c_a = int(255 * amp_rel + 100);

  // волны
  float sh = millis() * .01;
  stroke(c_a, 87, 51);
  //    stroke(255, 87, 51); //// оранжевый
  for (int i = 0; i < width; i+=2) { //
    float x = i;
    int nmax = 20;
    float y = ellipseWave(x + sh, 17, nmax, 1);
    //    point(x, y + height * .5 - 20); //// отрисовка линии
    int n = int(nmax - y) - 10;
    n = constrain(n, 0, nmax);
    for (int j = 0; j < n; j++) {
      //      point(x, random(height));
      int yt = random(height);
      line(x, yt, x, yt - 20);
    }
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
