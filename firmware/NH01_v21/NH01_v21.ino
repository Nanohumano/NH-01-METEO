#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <DHT.h>
#include <time.h>
#include <math.h>
#include <Preferences.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>

// =====================================================================
//  NANOHUMANO LABS NH-01  -  v21
//  CONFIGURACION: valores por defecto. La ciudad tambien se puede cambiar
//  desde el portal Wi-Fi o desde la web interna (http://nh-01.local).
// =====================================================================
const char* CIUDAD_DEFECTO = "Andratx";
const float LAT_DEFECTO = 39.5751f, LON_DEFECTO = 2.4214f; // Centro de Andratx (aproximado)
const char* NOMBRE_RED = "NH-01";   // Nombre en la red: http://nh-01.local
const char* CLAVE_OTA = "nh01";     // Clave para actualizar por Wi-Fi (cambiala si quieres)
// IMPORTANTE para la OTA: Herramientas > Partition Scheme > "Minimal SPIFFS (1.9MB APP with OTA)"
// y Flash Size 4MB. Con "Huge APP (No OTA)" todo funciona salvo actualizar por Wi-Fi.
String ciudad = CIUDAD_DEFECTO;     // Se carga de la memoria en setup()
// ---------------------------------------------------------------------
// v16: logo del canal (JPEG 150x150 en base64) y estilo NANOHUMANO LABS
// para el portal Wi-Fi y la web interna. Colores: cian del cerebro y
// naranja del hexagono del logo, sobre azul noche.
// ---------------------------------------------------------------------
#define LOGO_NH_B64 "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"
const char CABECERA_PORTAL[] =
  "<style>"
  "body,body.invert{background:#07131f;color:#e6f1f8;font-family:system-ui,-apple-system,Segoe UI,sans-serif}"
  ".wrap{max-width:420px}"
  ".wrap::before{content:'';display:block;width:140px;height:140px;margin:18px auto 4px;border-radius:50%;"
  "background:url(data:image/jpeg;base64," LOGO_NH_B64 ") center/cover;box-shadow:0 0 26px rgba(63,224,255,.35)}"
  "h1{color:#3fe0ff;text-align:center;letter-spacing:.08em;font-size:1.5em;margin:.2em 0 0}"
  "h1::after{content:'RELOJ Y ESTACION METEOROLOGICA';display:block;color:#ff8a3d;font-size:.45em;letter-spacing:.18em;margin-top:.4em}"
  "h3{color:#ff8a3d;text-align:center}"
  "a{color:#3fe0ff}label{color:#9fc4d4}"
  "input{background:#10273a;color:#fff;border:1px solid #2a4a60;border-radius:8px;padding:8px}"
  "input:focus{outline:none;border-color:#3fe0ff}"
  "button,input[type=submit]{background:#0e5d81;color:#fff;border:1px solid #3fe0ff;border-radius:10px;font-weight:600;letter-spacing:.04em}"
  "button:hover,button:focus{background:#ff8a3d;border-color:#ff8a3d}"
  ".msg{background:#10273a;color:#e6f1f8;border-left-color:#ff8a3d;border-radius:8px}"
  "hr{border-color:#1f3a4d}"
  "</style>";
const char* MENU_PORTAL[] = {"wifi"}; // Solo "Configure WiFi"

float LAT = LAT_DEFECTO, LON = LON_DEFECTO;

// Pin de la retroiluminacion: pin LED del modulo de pantalla -> D13 del ESP32.
// (El modulo lleva su propio transistor Q1, se conecta directo.) -1 = sin control.
const int PIN_BRILLO_PANTALLA = 13;
const float BRILLO_DIA = 100.0f;    // % en modo AUTO entre salida y puesta del sol
const float BRILLO_NOCHE = 25.0f;   // % en modo AUTO de noche
const float MINUTOS_CREPUSCULO = 30.0f; // Bajada/subida gradual alrededor de puesta y salida
// v21: CODIGO DE COLORES SIMPLE - UNA SOLA LUZ cada vez (prevision de HOY):
//   VERDE = buen tiempo (ni lluvia ni viento)
//   ROJO  = viento maximo previsto >= VIENTO_AVISO
//   AZUL  = probabilidad de lluvia >= LLUVIA_AVISO, o esta lloviendo
//   PARPADEO = alerta: azul con tormenta, nieve o lluvia fuerte; rojo con viento >= VIENTO_ALERTA
//   Prioridad: alerta de lluvia > alerta de viento > lluvia > viento > verde
const float VIENTO_AVISO = 25.0f;                    // km/h
const int   LLUVIA_AVISO = 40;                       // %
const float VIENTO_ALERTA = 50.0f;                   // km/h
// =====================================================================

// Color RGB segun la prevision del DIA.
// R=25, G=26, B=32. Mantiene la transicion suave y brillo moderado.
const uint8_t RGB_R=25, RGB_G=26, RGB_B=32;
int codigoMeteoRGB=-1;
int lluviaRGB=-1;        // Probabilidad de lluvia de hoy (copia para el LED)
bool alertaViento=false, alertaLluvia=false;  // Canales que parpadean
float vientoPronosticoRGB=NAN; // Maximo previsto hoy, no viento instantaneo
float rgbActual[3]={0,0,0};
unsigned long ultimoRGB=0;
uint8_t brilloRGB=75; // Porcentaje guardado: 25, 50, 75 o 100
Preferences preferenciasNH;

void iniciarRGB() {
  pinMode(RGB_R, OUTPUT); pinMode(RGB_G, OUTPUT); pinMode(RGB_B, OUTPUT);
  digitalWrite(RGB_R, LOW); digitalWrite(RGB_G, LOW); digitalWrite(RGB_B, LOW);
}
void actualizarRGB() {
  unsigned long ahora=millis();
  if (ahora-ultimoRGB<35) return;
  ultimoRGB=ahora;
  float r=0,g=0,b=0; // Sin prevision: apagado
  alertaViento=false; alertaLluvia=false;
  int c=codigoMeteoRGB;
  if(c>=0) {
    bool tormenta=(c>=95 && c<=99);
    bool nieve=(c>=71 && c<=77)||(c>=85 && c<=86);
    bool lluviaFuerte=(c==65 || c==67 || c==81 || c==82);
    bool precipitacion=(c>=51 && c<=67)||(c>=80 && c<=82)||tormenta||nieve;
    bool hayViento = !isnan(vientoPronosticoRGB) && vientoPronosticoRGB >= VIENTO_AVISO;
    bool vientoFuerte = !isnan(vientoPronosticoRGB) && vientoPronosticoRGB >= VIENTO_ALERTA;
    bool hayLluvia = precipitacion || lluviaRGB >= LLUVIA_AVISO;
    if(tormenta || nieve || lluviaFuerte) { b = 65.0f; alertaLluvia = true; }  // 1. alerta de lluvia
    else if(vientoFuerte)                 { r = 65.0f; alertaViento = true; }  // 2. alerta de viento
    else if(hayLluvia)                    { b = 65.0f; }                       // 3. lluvia
    else if(hayViento)                    { r = 65.0f; }                       // 4. viento
    else                                  { g = 65.0f; }                       // 5. buen tiempo
  }
  const float objetivo[3]={(float)r,(float)g,(float)b};
  for(int i=0;i<3;i++) rgbActual[i]+=(objetivo[i]-rgbActual[i])*0.018f;
  // El porcentaje de brillo (25/50/75/100) regula la luz encendida.
  const float factor=(brilloRGB/100.0f)*(255.0f/65.0f);
  // Parpadeo de alerta, aplicado despues del fundido
  const bool encendido = (millis() % 1000UL) < 700UL;
  const float kR = (alertaViento && !encendido) ? 0.0f : 1.0f;
  const float kB = (alertaLluvia && !encendido) ? 0.0f : 1.0f;
  analogWrite(RGB_R,constrain((int)(rgbActual[0]*factor*kR+0.5f),0,255));
  analogWrite(RGB_G,constrain((int)(rgbActual[1]*factor+0.5f),0,255));
  analogWrite(RGB_B,constrain((int)(rgbActual[2]*factor*kB+0.5f),0,255));
}

#define DHTPIN 27
#define DHTTYPE DHT11
TFT_eSPI tft;
DHT dht(DHTPIN, DHTTYPE);
WiFiManager wm;
bool portalWiFiActivo = false;
unsigned long inicioPortalWiFi = 0;
uint16_t calData[5] = {349, 3425, 415, 3200, 7};
float tempInterior = NAN, humedad = NAN, tempExterior = NAN, humedadExterior = NAN, maxima = NAN, minima = NAN;
int lluvia = -1, codigoMeteo = -1;
float vientoActual = NAN;
int direccionViento = -1;
String salidaSol = "--:--", puestaSol = "--:--";
bool meteoDisponible = false;
int pagina = 0;
float max5[5], min5[5];
int lluvia5[5], codigo5[5];
float vientoMax5[5];
String fecha5[5];
// v15: datos extra (pantalla DETALLE y web)
float sensacionTermica = NAN, uvMaximo = NAN, temperaturaMar = NAN, alturaOlas = NAN, polvo = NAN;
int indiceAire = -1;
float temp24[24]; int lluvia24[24]; int hora24[24]; int horas24 = 0;
float interiorMax = NAN, interiorMin = NAN; int diaMinMax = -1;
unsigned long ultimoExtras = 0; bool extrasOk = false;
WebServer servidorWeb(80);
bool webActiva = false, otaActiva = false;
String ciudadTitulo() {
  String s = ciudad; s.toUpperCase();
  // La fuente solo tiene ASCII: quitamos tildes y enes.
  s.replace("\xC3\x81","A"); s.replace("\xC3\x89","E"); s.replace("\xC3\x8D","I");
  s.replace("\xC3\x93","O"); s.replace("\xC3\x9A","U"); s.replace("\xC3\x91","N");
  s.replace("\xC3\xA1","A"); s.replace("\xC3\xA9","E"); s.replace("\xC3\xAD","I");
  s.replace("\xC3\xB3","O"); s.replace("\xC3\xBA","U"); s.replace("\xC3\xB1","N");
  if (s.length() > 16) s = s.substring(0, 16);
  return s;
}
int diasDisponibles = 0;
unsigned long ultimaHora = 0, ultimoDHT = 0, ultimoWiFi = 0, ultimoMeteo = 0;
String horaAnterior = "", fechaAnterior = "";
const char* dias[] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
const char* meses[] = {"enero", "febrero", "marzo", "abril", "mayo", "junio", "julio", "agosto", "septiembre", "octubre", "noviembre", "diciembre"};

// ---------------------------------------------------------------------
// Brillo de la pantalla: AUTO (segun salida/puesta del sol) o manual.
// modoBrilloPantalla: 0 = AUTO; 25, 50, 75 o 100 = fijo.
// ---------------------------------------------------------------------
uint8_t modoBrilloPantalla = 0;
float brilloPantallaActual = 100.0f;
float brilloPantallaObjetivo = 100.0f;
unsigned long ultimoBrilloPantalla = 0, ultimoCalculoBrillo = 0;

int minutosDesdeTexto(const String& hhmm) {
  if (hhmm.length() < 5 || hhmm[2] != ':') return -1;
  return hhmm.substring(0, 2).toInt() * 60 + hhmm.substring(3, 5).toInt();
}
// v20: hora local sin espera. getLocalTime(&t, 0) podia fallar de forma aleatoria
// y el modo AUTO saltaba al 100 % un instante (parpadeos de brillo).
bool horaLocalYa(struct tm* t) {
  time_t ahora = time(nullptr);
  localtime_r(&ahora, t);
  return t->tm_year > (2016 - 1900);
}
float calcularObjetivoPantalla() {
  if (modoBrilloPantalla != 0) return modoBrilloPantalla;
  struct tm t;
  if (!horaLocalYa(&t)) return brilloPantallaObjetivo;     // Sin hora: mantener el objetivo actual
  int salida = minutosDesdeTexto(salidaSol);
  int puesta = minutosDesdeTexto(puestaSol);
  if (salida < 0 || puesta < 0) return brilloPantallaObjetivo; // Sin datos de sol: mantener
  float ahora = t.tm_hour * 60 + t.tm_min + t.tm_sec / 60.0f;
  const float rango = BRILLO_DIA - BRILLO_NOCHE;
  if (ahora >= salida && ahora < puesta) return BRILLO_DIA; // De dia
  if (ahora >= puesta && ahora < puesta + MINUTOS_CREPUSCULO)       // Anochece poco a poco
    return BRILLO_DIA - rango * (ahora - puesta) / MINUTOS_CREPUSCULO;
  if (ahora >= salida - MINUTOS_CREPUSCULO && ahora < salida)       // Amanece poco a poco
    return BRILLO_NOCHE + rango * (ahora - (salida - MINUTOS_CREPUSCULO)) / MINUTOS_CREPUSCULO;
  return BRILLO_NOCHE;                                      // De noche
}
void aplicarBrilloPantalla() {
  if (PIN_BRILLO_PANTALLA < 0) return;
  analogWrite(PIN_BRILLO_PANTALLA, constrain((int)(brilloPantallaActual * 2.55f + 0.5f), 0, 255));
}
void actualizarBrilloPantalla() {
  unsigned long ahora = millis();
  if (ahora - ultimoCalculoBrillo >= 1000UL || ultimoCalculoBrillo == 0) {
    brilloPantallaObjetivo = calcularObjetivoPantalla();
    ultimoCalculoBrillo = ahora;
  }
  if (ahora - ultimoBrilloPantalla < 35) return;
  ultimoBrilloPantalla = ahora;
  brilloPantallaActual += (brilloPantallaObjetivo - brilloPantallaActual) * 0.06f; // Fundido suave
  aplicarBrilloPantalla();
}

// Fuente Orbitron incluida en el programa (no requiere Free_Fonts.h).
struct OrbGlyph { uint16_t offset; uint8_t w,h; int8_t xoff,yoff; uint8_t advance; };
const uint8_t orbBits13[] PROGMEM = {
  73,36,144,8,82,128,0,0,0,0,0,4,65,152,255,132,65,152,34,31,241,136,51,4,
  64,4,1,7,253,17,68,17,7,248,17,4,65,20,69,255,4,1,0,0,3,128,146,12,
  144,196,140,61,192,31,193,145,24,137,132,72,60,63,130,12,32,2,0,56,4,196,67,68,
  28,64,247,249,72,0,0,0,100,68,68,68,70,66,34,34,34,36,16,41,241,194,128,0,
  0,0,0,16,71,196,16,0,73,0,124,0,0,0,0,8,0,3,2,4,8,24,48,32,
  64,0,127,200,25,7,33,164,100,152,150,19,130,96,79,248,51,180,33,8,66,16,128,127,
  136,8,1,0,32,4,255,144,2,0,64,15,248,127,200,8,1,0,33,252,0,128,16,2,
  64,79,240,3,1,192,208,100,49,16,79,248,4,1,0,64,127,200,1,0,32,7,248,0,
  128,16,2,64,79,248,127,8,1,0,32,7,248,128,144,18,2,64,79,248,255,0,128,64,
  32,16,8,4,2,1,0,128,127,136,9,1,32,39,252,128,144,18,2,64,79,240,127,200,
  9,1,32,36,4,255,128,16,2,0,79,248,64,0,2,64,0,2,72,0,4,51,24,96,
  224,193,127,0,0,127,0,0,65,129,131,12,230,16,255,0,64,32,16,9,252,128,0,0,
  16,0,127,200,9,1,47,37,20,162,151,242,0,64,15,248,127,200,9,1,32,36,4,255,
  144,18,2,64,72,8,127,200,9,1,32,39,252,128,144,18,2,64,79,248,127,200,1,0,
  32,4,0,128,16,2,0,64,15,248,127,200,9,1,32,36,4,128,144,18,2,64,79,248,
  127,208,4,1,0,127,16,4,1,0,64,31,240,127,208,4,1,0,127,16,4,1,0,64,
  16,0,127,200,9,0,32,4,0,131,144,18,2,64,79,248,64,40,5,0,160,23,254,128,
  80,10,1,64,40,4,73,36,146,72,0,64,16,4,1,0,64,16,5,1,64,95,240,64,
  144,100,49,24,124,17,132,33,4,65,144,48,64,8,1,0,32,4,0,128,16,2,0,64,
  15,248,96,55,7,88,212,153,77,20,113,64,20,1,64,20,1,96,78,9,65,44,36,196,
  140,144,146,10,65,200,24,127,200,9,1,32,36,4,128,144,18,2,64,79,248,127,208,20,
  5,1,64,95,244,1,0,64,16,0,127,196,4,64,68,4,64,68,4,64,68,4,64,71,
  255,127,200,9,1,32,36,4,255,144,194,8,65,136,24,127,200,9,0,32,7,248,0,128,
  16,2,64,79,248,127,193,0,64,16,4,1,0,64,16,4,1,0,64,72,9,1,32,36,
  4,128,144,18,2,64,79,248,64,19,1,136,8,96,193,4,12,96,34,1,176,7,0,48,
  0,65,134,135,13,138,17,22,98,100,134,137,5,30,14,24,24,48,16,96,64,196,48,204,
  13,0,192,24,6,129,152,33,136,24,64,76,16,198,9,128,224,28,1,0,32,4,0,128,
  127,192,24,6,1,128,96,24,6,1,128,96,15,248,100,68,68,68,70,64,64,32,48,24,
  8,4,6,3,0,98,34,34,34,38,253,6,12,24,48,96,193,131,252,127,192,0,98,0,
  0,0,0,0,127,0,64,47,244,10,5,2,255,64,32,16,15,228,10,5,2,129,64,160,
  95,192,127,32,16,8,4,2,1,0,254,0,128,64,47,244,10,5,2,129,64,160,95,224,
  127,160,80,47,244,2,1,0,255,121,4,30,65,4,16,65,4,0,127,32,80,40,20,10,
  5,2,255,0,128,79,192,64,32,16,15,228,10,5,2,129,64,160,80,32,64,36,146,73,
  0,8,0,2,8,32,130,8,32,130,11,224,64,32,16,8,100,98,97,224,152,70,33,16,
  192,68,68,68,68,68,112,127,242,16,144,132,132,36,33,33,9,8,72,66,127,32,80,40,
  20,10,5,2,129,127,32,80,40,20,10,5,2,254,127,32,80,40,20,10,5,2,254,64,
  32,16,0,127,160,80,40,20,10,5,2,255,0,128,64,32,126,129,2,4,8,16,32,127,
  32,80,15,224,8,5,2,254,65,4,30,65,4,16,65,7,128,64,160,80,40,20,10,5,
  2,254,192,208,38,24,132,51,4,128,224,48,67,13,12,38,121,137,36,44,208,225,193,134,
  6,8,99,17,13,3,129,193,160,136,134,64,160,80,40,20,10,5,2,255,0,128,79,192,
  127,128,192,192,193,129,129,128,255,100,68,204,68,70,73,36,146,73,36,66,34,34,34,36,
  225,192,0,0,
};
const OrbGlyph orbGlyphs13[] PROGMEM = {
  {0,4,0,0,0,4},
  {0,3,10,0,-10,3},
  {4,5,10,0,-10,5},
  {11,11,10,0,-10,10},
  {25,10,14,0,-12,10},
  {43,13,11,0,-11,13},
  {61,12,10,0,-10,12},
  {76,3,10,0,-10,3},
  {80,4,10,0,-10,4},
  {85,4,10,0,-10,4},
  {90,7,10,0,-10,6},
  {99,6,6,0,-6,6},
  {104,3,3,0,-1,3},
  {106,7,5,0,-5,7},
  {111,3,2,0,-2,3},
  {112,8,10,0,-10,7},
  {122,11,10,0,-10,11},
  {136,5,10,0,-10,5},
  {143,11,10,0,-10,11},
  {157,11,10,0,-10,11},
  {171,10,10,0,-10,9},
  {184,11,10,0,-10,11},
  {198,11,10,0,-10,11},
  {212,9,10,0,-10,9},
  {224,11,10,0,-10,11},
  {238,11,10,0,-10,11},
  {252,3,8,0,-8,3},
  {255,3,11,0,-8,3},
  {260,6,8,0,-8,6},
  {266,8,6,0,-6,8},
  {272,6,8,0,-8,6},
  {278,9,10,0,-10,9},
  {290,11,10,0,-10,11},
  {304,11,10,0,-10,11},
  {318,11,10,0,-10,11},
  {332,11,10,0,-10,11},
  {346,11,10,0,-10,11},
  {360,10,10,0,-10,10},
  {373,10,10,0,-10,9},
  {386,11,10,0,-10,11},
  {400,11,10,0,-10,11},
  {414,3,10,0,-10,3},
  {418,10,10,0,-10,10},
  {431,10,10,0,-10,10},
  {444,11,10,0,-10,10},
  {458,12,10,0,-10,12},
  {473,11,10,0,-10,11},
  {487,11,10,0,-10,11},
  {501,10,10,0,-10,10},
  {514,12,10,0,-10,11},
  {529,11,10,0,-10,11},
  {543,11,10,0,-10,11},
  {557,10,10,0,-10,10},
  {570,11,10,0,-10,11},
  {584,13,10,0,-10,13},
  {601,15,10,0,-10,15},
  {620,11,10,0,-10,11},
  {634,11,10,0,-10,10},
  {648,11,10,0,-10,11},
  {662,4,10,0,-10,4},
  {667,8,10,0,-10,7},
  {677,4,10,0,-10,4},
  {682,7,10,0,-10,6},
  {691,11,2,0,0,11},
  {694,4,11,0,-11,4},
  {700,9,8,0,-8,9},
  {709,9,11,0,-11,9},
  {722,9,8,0,-8,9},
  {731,9,11,0,-11,9},
  {744,9,8,0,-8,9},
  {753,6,11,0,-11,5},
  {762,9,11,0,-8,9},
  {775,9,11,0,-11,9},
  {788,3,11,0,-11,3},
  {793,6,14,-3,-11,3},
  {804,9,11,0,-11,8},
  {817,4,11,0,-11,4},
  {823,13,8,0,-8,13},
  {836,9,8,0,-8,9},
  {845,9,8,0,-8,9},
  {854,9,11,0,-8,9},
  {867,9,11,0,-8,9},
  {880,7,8,0,-8,7},
  {887,9,8,0,-8,9},
  {896,6,11,0,-11,5},
  {905,9,8,0,-8,9},
  {914,10,8,0,-8,10},
  {924,14,8,0,-8,14},
  {938,9,8,0,-8,9},
  {947,9,11,0,-8,9},
  {960,9,8,0,-8,9},
  {969,4,10,0,-10,4},
  {974,3,13,0,-11,3},
  {979,4,10,0,-10,4},
  {984,5,5,0,-5,5},
};
const uint8_t orbBits19[] PROGMEM = {
  102,102,102,102,102,0,70,108,217,176,0,0,0,0,0,0,0,0,0,0,3,12,6,24,
  8,33,255,243,255,224,130,3,12,6,24,8,32,255,249,255,240,195,1,4,6,24,0,1,
  128,3,0,255,243,255,230,24,108,48,24,96,48,192,127,248,127,248,6,24,12,48,24,108,
  48,223,255,159,254,1,128,3,0,0,0,15,128,7,224,49,140,28,99,14,24,199,7,227,
  128,113,192,0,231,0,231,240,113,140,56,99,28,24,198,7,241,0,248,31,248,15,255,3,
  0,192,192,0,48,0,14,0,3,224,1,156,8,97,198,24,29,134,1,225,128,28,127,255,
  207,255,0,102,96,0,0,0,0,0,51,152,198,49,140,99,24,199,24,113,134,49,140,99,
  24,198,51,56,12,6,27,239,241,224,240,204,4,0,0,0,0,0,0,0,12,12,127,127,
  12,12,12,0,0,102,100,0,127,31,192,0,0,0,0,0,0,70,0,0,12,1,0,96,
  24,6,1,128,32,12,3,0,192,48,4,0,128,0,63,252,127,254,96,14,96,30,96,54,
  96,230,97,198,99,134,103,6,108,6,120,6,112,6,127,254,63,252,14,30,62,118,6,6,
  6,6,6,6,6,6,6,6,63,252,127,254,96,6,0,6,0,6,0,6,31,254,127,252,
  96,0,96,0,96,0,96,0,127,254,127,254,63,248,127,252,96,12,0,12,0,12,0,12,
  15,252,31,254,0,6,0,6,0,6,64,6,127,254,63,252,0,112,3,192,31,0,236,7,
  48,48,193,131,12,12,96,51,255,247,255,128,12,0,48,0,192,127,254,127,254,96,0,96,
  0,96,0,96,0,127,252,127,254,0,6,0,6,0,6,96,6,127,254,63,252,63,248,127,
  240,96,0,96,0,96,0,96,0,127,252,127,254,96,6,96,6,96,6,96,6,127,254,63,
  252,127,243,255,128,6,0,48,1,128,12,0,96,3,0,24,0,192,6,0,48,1,128,12,
  63,252,127,254,96,6,96,6,96,6,96,6,127,254,127,254,96,6,96,6,96,6,96,6,
  127,254,63,252,63,252,127,254,96,6,96,6,96,6,96,6,127,254,63,254,0,6,0,6,
  0,6,0,6,127,254,63,252,100,0,0,0,4,96,100,0,0,0,4,102,64,0,1,195,
  199,135,3,1,224,56,7,1,192,0,127,231,254,0,0,0,127,231,254,0,0,0,0,0,
  64,56,14,1,192,112,28,28,60,56,48,16,0,255,243,255,192,6,0,48,1,128,12,0,
  97,255,31,224,192,0,0,0,1,0,12,0,63,252,127,254,96,6,96,6,103,230,103,230,
  108,54,108,54,108,54,103,254,99,254,96,0,127,254,63,254,63,252,127,252,96,6,96,6,
  96,6,96,6,96,6,127,254,127,254,96,6,96,6,96,6,96,6,96,6,127,248,127,252,
  96,12,96,12,96,12,96,12,127,252,127,252,96,6,96,6,96,6,96,6,127,254,127,252,
  63,254,127,252,96,0,96,0,96,0,96,0,96,0,96,0,96,0,96,0,96,0,96,0,
  127,252,63,254,127,252,127,252,96,6,96,6,96,6,96,6,96,6,96,6,96,6,96,6,
  96,6,96,6,127,252,127,252,127,252,255,241,128,3,0,6,0,12,0,31,248,63,248,96,
  0,192,1,128,3,0,7,255,143,255,128,127,253,255,230,0,24,0,96,1,128,7,254,31,
  252,96,1,128,6,0,24,0,96,1,128,0,63,252,127,252,96,6,96,0,96,0,96,0,
  96,0,96,62,96,62,96,6,96,6,96,6,127,254,63,252,96,6,96,6,96,6,96,6,
  96,6,96,6,127,254,127,254,96,6,96,6,96,6,96,6,96,6,96,6,102,102,102,102,
  102,102,102,0,6,0,12,0,24,0,48,0,96,0,192,1,128,3,0,6,0,13,0,27,
  0,55,255,199,255,128,96,12,192,49,128,195,3,6,14,12,24,31,224,63,192,97,192,193,
  129,129,131,1,134,1,140,1,128,96,0,192,1,128,3,0,6,0,12,0,24,0,48,0,
  96,0,192,1,128,3,0,7,255,207,255,192,112,3,156,1,231,128,249,176,118,102,25,153,
  204,102,62,25,135,6,96,193,152,0,102,0,25,128,6,96,1,152,0,96,112,6,112,6,
  120,6,108,6,102,6,103,6,99,6,97,134,96,198,96,102,96,118,96,62,96,30,96,14,
  63,252,127,252,96,6,96,6,96,6,96,6,96,6,96,6,96,6,96,6,96,6,96,6,
  127,252,63,252,127,252,255,249,128,27,0,54,0,108,0,216,1,191,254,127,252,192,1,128,
  3,0,6,0,12,0,0,63,252,63,254,24,1,140,0,198,0,99,0,49,128,24,192,12,
  96,6,48,3,24,1,140,0,199,255,241,255,248,127,252,127,252,96,6,96,6,96,6,96,
  6,96,6,127,252,127,252,96,96,96,112,96,48,96,24,96,12,63,252,127,252,96,6,96,
  0,96,0,96,0,127,248,63,252,0,6,0,6,0,6,96,6,127,254,63,252,127,254,255,
  252,6,0,12,0,24,0,48,0,96,0,192,1,128,3,0,6,0,12,0,24,0,48,0,
  96,6,96,6,96,6,96,6,96,6,96,6,96,6,96,6,96,6,96,6,96,6,96,6,
  127,252,63,252,96,0,204,0,24,192,6,12,1,129,128,48,24,12,3,1,128,48,96,6,
  12,0,99,0,4,192,0,216,0,14,0,1,192,0,96,48,13,128,224,102,7,129,140,27,
  6,48,76,48,67,48,193,140,99,6,33,152,9,134,96,54,13,128,240,60,1,192,240,7,
  1,128,24,6,0,96,12,96,48,96,192,99,128,230,0,248,0,224,1,192,7,192,28,192,
  49,192,193,131,1,140,1,128,96,6,96,24,224,96,193,128,199,0,204,1,240,1,224,1,
  128,3,0,6,0,12,0,24,0,48,0,127,254,127,254,0,14,0,28,0,48,0,224,1,
  192,3,128,7,0,12,0,56,0,112,0,127,254,127,254,115,152,198,49,140,99,24,199,56,
  0,8,1,128,24,1,128,16,3,0,48,3,0,48,2,0,96,6,0,0,243,140,99,24,
  198,49,140,103,120,255,160,40,10,2,128,160,40,10,2,128,160,40,10,2,128,191,224,127,
  254,127,254,48,195,0,0,0,0,0,0,0,0,0,0,127,227,255,128,12,0,96,3,63,
  249,255,204,6,96,51,255,143,252,96,3,0,24,0,192,7,254,63,249,128,204,6,96,51,
  1,152,12,192,102,3,63,249,255,128,63,243,255,152,0,192,6,0,48,1,128,12,0,96,
  3,255,143,252,0,48,1,128,12,0,99,255,63,249,128,204,6,96,51,1,152,12,192,102,
  3,63,248,255,192,63,227,255,152,12,192,102,3,63,249,255,204,0,96,3,255,143,252,63,
  126,96,96,127,126,96,96,96,96,96,96,96,96,96,63,227,255,152,12,192,102,3,48,25,
  128,204,6,96,51,255,143,252,0,96,3,15,248,127,128,96,3,0,24,0,192,7,254,63,
  249,128,204,6,96,51,1,152,12,192,102,3,48,25,128,192,100,0,102,102,102,102,102,96,
  3,1,0,0,0,48,24,12,6,3,1,128,192,96,48,24,12,6,3,63,191,128,96,6,
  0,96,6,0,96,118,6,96,198,24,99,7,224,127,6,56,97,198,14,96,112,97,134,24,
  97,134,24,97,134,24,97,227,128,127,255,143,255,241,131,3,48,96,102,12,12,193,129,152,
  48,51,6,6,96,192,204,24,25,131,3,0,127,227,255,152,12,192,102,3,48,25,128,204,
  6,96,51,1,152,12,63,227,255,152,12,192,102,3,48,25,128,204,6,96,51,255,143,248,
  127,227,255,152,12,192,102,3,48,25,128,204,6,96,51,255,159,248,192,6,0,48,1,128,
  0,63,243,255,152,12,192,102,3,48,25,128,204,6,96,51,255,143,252,0,96,3,0,24,
  0,192,63,223,230,1,128,96,24,6,1,128,96,24,6,0,63,227,255,152,4,192,6,0,
  63,240,127,192,6,0,51,255,143,248,96,96,96,96,127,126,96,96,96,96,96,96,96,126,
  63,96,51,1,152,12,192,102,3,48,25,128,204,6,96,51,255,143,248,96,12,192,24,192,
  97,128,193,131,3,12,3,24,6,96,6,192,7,0,14,0,96,112,54,15,2,32,248,99,
  13,134,49,152,193,152,204,27,12,192,240,120,15,7,128,224,48,6,3,0,96,49,131,6,
  48,27,0,112,3,128,62,1,176,24,193,131,24,12,96,51,1,152,12,192,102,3,48,25,
  128,204,6,96,51,255,143,252,0,96,3,15,248,127,128,127,243,255,128,28,1,128,56,3,
  128,56,3,0,112,3,255,159,252,25,204,99,25,140,49,140,99,140,102,102,102,102,102,102,
  102,102,102,227,12,99,24,99,49,140,102,112,96,126,14,0,0,0,0,0,
};
const OrbGlyph orbGlyphs19[] PROGMEM = {
  {0,5,0,0,0,5},
  {0,4,14,0,-14,4},
  {7,7,14,0,-14,7},
  {20,15,14,0,-14,15},
  {47,15,18,0,-16,15},
  {81,18,15,0,-15,18},
  {115,18,14,0,-14,18},
  {147,4,14,0,-14,4},
  {154,5,14,0,-14,5},
  {163,5,14,0,-14,5},
  {172,9,14,0,-14,9},
  {188,8,9,0,-9,8},
  {197,4,5,0,-2,4},
  {200,10,7,0,-7,10},
  {209,4,2,0,-2,4},
  {210,11,14,0,-14,10},
  {230,16,14,0,-14,16},
  {258,8,14,-1,-14,7},
  {272,16,14,0,-14,16},
  {300,16,14,0,-14,16},
  {328,14,14,0,-14,14},
  {353,16,14,0,-14,16},
  {381,16,14,0,-14,16},
  {409,13,14,0,-14,13},
  {432,16,14,0,-14,16},
  {460,16,14,0,-14,16},
  {488,4,11,0,-11,4},
  {494,4,14,0,-11,4},
  {501,9,11,0,-11,9},
  {514,12,9,0,-9,12},
  {528,9,11,0,-11,9},
  {541,13,14,0,-14,13},
  {564,16,14,0,-14,16},
  {592,16,14,0,-14,16},
  {620,16,14,0,-14,16},
  {648,16,14,0,-14,16},
  {676,16,14,0,-14,16},
  {704,15,14,0,-14,15},
  {731,14,14,0,-14,14},
  {756,16,14,0,-14,16},
  {784,16,14,0,-14,16},
  {812,4,14,0,-14,4},
  {819,15,14,0,-14,15},
  {846,15,14,0,-14,15},
  {873,15,14,0,-14,15},
  {900,18,14,0,-14,18},
  {932,16,14,0,-14,16},
  {960,16,14,0,-14,16},
  {988,15,14,0,-14,15},
  {1015,17,14,0,-14,17},
  {1045,16,14,0,-14,16},
  {1073,16,14,0,-14,16},
  {1101,15,14,0,-14,14},
  {1128,16,14,0,-14,16},
  {1156,19,14,0,-14,19},
  {1190,22,14,0,-14,22},
  {1229,15,14,0,-14,15},
  {1256,15,14,0,-14,15},
  {1283,16,14,0,-14,16},
  {1311,5,14,0,-14,5},
  {1320,11,14,0,-14,10},
  {1340,5,14,0,-14,5},
  {1349,10,14,0,-14,10},
  {1367,16,2,0,0,16},
  {1371,6,15,0,-15,6},
  {1383,13,11,0,-11,13},
  {1401,13,15,0,-15,13},
  {1426,13,11,0,-11,13},
  {1444,13,15,0,-15,13},
  {1469,13,11,0,-11,13},
  {1487,8,15,0,-15,8},
  {1502,13,15,0,-11,13},
  {1527,13,15,0,-15,13},
  {1552,4,15,0,-15,4},
  {1560,9,19,-4,-15,5},
  {1582,12,15,0,-15,12},
  {1605,6,15,0,-15,6},
  {1617,19,11,0,-11,19},
  {1644,13,11,0,-11,13},
  {1662,13,11,0,-11,13},
  {1680,13,15,0,-11,13},
  {1705,13,15,0,-11,13},
  {1730,10,11,0,-11,10},
  {1744,13,11,0,-11,13},
  {1762,8,15,0,-15,8},
  {1777,13,11,0,-11,13},
  {1795,15,11,0,-11,15},
  {1816,20,11,0,-11,20},
  {1844,13,11,0,-11,13},
  {1862,13,15,0,-11,13},
  {1887,13,11,0,-11,13},
  {1905,5,14,0,-14,5},
  {1914,4,18,0,-16,4},
  {1923,5,14,0,-14,5},
  {1932,8,8,0,-8,8},
};
int orbAncho(const String &s, bool grande=false) {
  const OrbGlyph* fuente = grande ? orbGlyphs19 : orbGlyphs13;
  int ancho=0;
  for (unsigned int i=0;i<s.length();i++) {
    uint8_t c=(uint8_t)s[i];
    if(c<32||c>126) c='?';
    OrbGlyph g; memcpy_P(&g, &fuente[c-32], sizeof(g));
    ancho+=g.advance;
  }
  return ancho;
}
void orbTexto(const String &s, int x, int y, uint16_t color, bool grande=false) {
  const OrbGlyph* fuente=grande ? orbGlyphs19 : orbGlyphs13;
  const uint8_t* bits=grande ? orbBits19 : orbBits13;
  int cursor=x;
  const int baseline=y+(grande?21:15);
  for(unsigned int k=0;k<s.length();k++){
    uint8_t c=(uint8_t)s[k];
    if(c<32||c>126)c='?';
    OrbGlyph g; memcpy_P(&g,&fuente[c-32],sizeof(g));
    uint32_t pixel=0;
    for(int row=0;row<g.h;row++){
      int run=-1;
      for(int col=0;col<=g.w;col++){
        bool on=false;
        if(col<g.w){
          uint8_t b=pgm_read_byte(bits+g.offset+(pixel>>3));
          on=(b & (0x80>>(pixel&7)))!=0;
          pixel++;
        }
        if(on && run<0)run=col;
        if(!on && run>=0){
          tft.drawFastHLine(cursor+g.xoff+run,baseline+g.yoff+row,col-run,color);
          run=-1;
        }
      }
    }
    cursor+=g.advance;
  }
}
void orbCentro(const String &s,int cx,int y,uint16_t color,bool grande=false){
  orbTexto(s,cx-orbAncho(s,grande)/2,y,color,grande);
}
void orbDerecha(const String &s,int right,int y,uint16_t color,bool grande=false){
  orbTexto(s,right-orbAncho(s,grande),y,color,grande);
}

void iconoMeteo(int codigo, int x, int y);
int ultimoDiaLunar=-1;
// Diseno adaptado a los 480x320 pixeles reales del ST7796S.
void dibujarBase() {
  // Al limpiar la pantalla, forzar que se vuelva a dibujar la Luna.
  ultimoDiaLunar = -1;
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  orbTexto(ciudadTitulo(), 15, 6, TFT_CYAN, true);
  tft.drawRoundRect(390, 5, 85, 34, 8, TFT_CYAN);
  orbDerecha("WIFI", 468, 12, TFT_WHITE);
  // Panel solar, con iconos grandes y separador vertical.
  tft.fillRoundRect(8, 145, 464, 48, 10, tft.color565(9, 28, 48));
  tft.drawFastVLine(240, 153, 32, TFT_CYAN);
  // Panel de meteorologia.
  tft.fillRoundRect(8, 197, 464, 64, 11, TFT_NAVY);
  orbCentro("EL TIEMPO HOY", 240, 199, TFT_CYAN);
  // Dos tarjetas de temperaturas.
  tft.fillRoundRect(8, 265, 228, 52, 10, TFT_DARKGREY);
  tft.fillRoundRect(244, 265, 228, 52, 10, TFT_DARKGREY);
  orbCentro("INTERIOR", 122, 267, TFT_CYAN);
  orbCentro("EXTERIOR", 358, 267, TFT_CYAN);
}
void iconoViento(int x, int y, uint16_t color);
void dibujarCincoDias() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  orbTexto(ciudadTitulo(), 16, 8, TFT_CYAN, true);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  orbDerecha("VOLVER", 466, 14, TFT_WHITE);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  orbCentro("PREVISION PROXIMOS 5 DIAS", 240, 45, TFT_CYAN);
  if (!meteoDisponible || diasDisponibles == 0) {
    orbCentro("Esperando datos...", 240, 150, TFT_CYAN, true);
    return;
  }
  for (int i = 0; i < diasDisponibles; i++) {
    int y = 70 + i * 48;
    tft.fillRoundRect(12, y, 456, 43, 8, TFT_NAVY);
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    // Fecha API YYYY-MM-DD -> abreviatura del dia + numero
    int anio = fecha5[i].substring(0, 4).toInt();
    int mes = fecha5[i].substring(5, 7).toInt();
    int dia = fecha5[i].substring(8, 10).toInt();
    struct tm diaTm = {};
    diaTm.tm_year = anio - 1900;
    diaTm.tm_mon = mes - 1;
    diaTm.tm_mday = dia;
    diaTm.tm_hour = 12;
    mktime(&diaTm);
    const char* abreviados[] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};
    String diaTexto = String(abreviados[diaTm.tm_wday]) + " " + String(dia);
    orbTexto(diaTexto, 22, y + 11, TFT_WHITE);
    // Iconos pequenos dibujados con las mismas primitivas del programa
    // v15: el icono (46 px) se recorta a la tarjeta (43 px) para que no se salga.
    tft.setViewport(12, y, 456, 43, false);
    iconoMeteo(codigo5[i], 116, y - 2);
    tft.resetViewport();
    orbTexto(String(max5[i], 0) + "/" + String(min5[i], 0) + "C", 184, y + 11, TFT_WHITE);
    orbDerecha(String(lluvia5[i]) + "%", 365, y + 11, TFT_CYAN);
    if (!isnan(vientoMax5[i])) {
      iconoViento(373, y + 11, TFT_YELLOW);
      orbDerecha(String(vientoMax5[i], 0) + "km/h", 463, y + 11, TFT_YELLOW);
    }
  }
}

// =====================================================================
// v15: pantalla DETALLE (grafica de 24 h + 6 tarjetas de datos)
// =====================================================================
String nivelUV(float uv, uint16_t &color) {
  if (uv < 3) { color = TFT_GREEN; return "Bajo"; }
  if (uv < 6) { color = TFT_YELLOW; return "Moder."; }
  if (uv < 8) { color = TFT_ORANGE; return "Alto"; }
  if (uv < 11) { color = TFT_RED; return "M.alto"; }
  color = TFT_MAGENTA; return "Extr.";
}
String nivelAire(int aqi, uint16_t &color) {
  if (aqi <= 20) { color = TFT_GREEN; return "Buena"; }
  if (aqi <= 40) { color = TFT_GREENYELLOW; return "Acept."; }
  if (aqi <= 60) { color = TFT_YELLOW; return "Moder."; }
  if (aqi <= 80) { color = TFT_ORANGE; return "Mala"; }
  if (aqi <= 100) { color = TFT_RED; return "M.mala"; }
  color = TFT_MAGENTA; return "Extr.";
}
void tarjetaDato(int x, int y, const String& titulo, const String& valor, uint16_t colorValor,
                 const String& nota = "", uint16_t colorNota = TFT_LIGHTGREY) {
  tft.fillRoundRect(x, y, 148, 60, 8, TFT_NAVY);
  orbTexto(titulo, x + 8, y + 3, TFT_LIGHTGREY);
  orbTexto(valor, x + 8, y + 28, colorValor, true);
  if (nota.length()) orbDerecha(nota, x + 140, y + 34, colorNota);
}
void dibujarGrafica24h() {
  const int gx = 44, gy = 70, gw = 418, gh = 90;
  tft.fillRoundRect(8, 40, 464, 146, 10, tft.color565(9, 28, 48));
  orbTexto("PROXIMAS 24 HORAS", 18, 43, TFT_CYAN);
  tft.fillRect(300, 51, 14, 3, TFT_CYAN);           orbTexto("TEMP", 318, 43, TFT_CYAN);
  tft.fillRect(372, 47, 10, 10, tft.color565(40, 90, 170)); orbTexto("LLUVIA", 386, 43, tft.color565(110, 160, 230));
  if (horas24 < 2) { orbCentro("Esperando datos...", 240, 110, TFT_YELLOW); return; }
  float tmin = temp24[0], tmax = temp24[0];
  for (int i = 1; i < horas24; i++) { tmin = min(tmin, temp24[i]); tmax = max(tmax, temp24[i]); }
  tmin = floorf(tmin) - 1; tmax = ceilf(tmax) + 1;
  const float paso = (float)gw / horas24;
  tft.drawFastHLine(gx, gy + gh / 2, gw, tft.color565(30, 55, 80)); // Linea guia central
  tft.drawFastHLine(gx, gy + gh, gw, tft.color565(60, 100, 130));    // Base
  // Barras de probabilidad de lluvia
  for (int i = 0; i < horas24; i++) {
    int alto = lluvia24[i] * gh / 100;
    if (alto > 0) tft.fillRect(gx + (int)(i * paso) + 2, gy + gh - alto, max(2, (int)paso - 3), alto, tft.color565(40, 90, 170));
  }
  // Linea de temperatura (doble para que se vea gruesa)
  int px = 0, py = 0;
  for (int i = 0; i < horas24; i++) {
    int x = gx + (int)(i * paso + paso / 2);
    int y = gy + gh - (int)((temp24[i] - tmin) / (tmax - tmin) * gh);
    if (i > 0) { tft.drawLine(px, py, x, y, TFT_CYAN); tft.drawLine(px, py + 1, x, y + 1, TFT_CYAN); }
    px = x; py = y;
  }
  // Escala y horas
  orbDerecha(String(tmax, 0), gx - 6, gy - 6, TFT_LIGHTGREY);
  orbDerecha(String(tmin, 0), gx - 6, gy + gh - 12, TFT_LIGHTGREY);
  for (int i = 0; i < horas24; i += 6)
    orbCentro(String(hora24[i]) + "h", gx + (int)(i * paso + paso / 2), gy + gh + 2, TFT_LIGHTGREY);
}
void dibujarDetalle() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  orbTexto(ciudadTitulo(), 16, 8, TFT_CYAN, true);
  orbDerecha("DETALLE  >", 466, 14, TFT_WHITE);
  dibujarGrafica24h();
  const int c0 = 12, c1 = 166, c2 = 320, f0 = 192, f1 = 256;
  uint16_t col;
  tarjetaDato(c0, f0, "SENSACION", isnan(sensacionTermica) ? String("--") : String(sensacionTermica, 0) + " C", TFT_YELLOW);
  if (isnan(uvMaximo)) tarjetaDato(c1, f0, "INDICE UV", "--", TFT_WHITE);
  else { String n = nivelUV(uvMaximo, col); tarjetaDato(c1, f0, "INDICE UV", String(uvMaximo, 0), col, n, col); }
  tarjetaDato(c2, f0, "AGUA DEL MAR", isnan(temperaturaMar) ? String("--") : String(temperaturaMar, 0) + " C", TFT_CYAN);
  tarjetaDato(c0, f1, "OLAS", isnan(alturaOlas) ? String("--") : String(alturaOlas, 1) + " m", TFT_CYAN);
  if (indiceAire < 0) tarjetaDato(c1, f1, "AIRE", "--", TFT_WHITE);
  else {
    String n = nivelAire(indiceAire, col);
    bool calima = !isnan(polvo) && polvo >= 50.0f;   // Mas de 50 ug/m3 de polvo: calima
    tarjetaDato(c1, f1, calima ? "AIRE / CALIMA" : "AIRE", String(indiceAire), col,
                calima ? String("CALIMA") : n, calima ? TFT_ORANGE : col);
  }
  String interior = isnan(interiorMax) ? String("--") : String(interiorMax, 0) + "/" + String(interiorMin, 0) + "C";
  tarjetaDato(c2, f1, "INT. MAX/MIN", interior, TFT_YELLOW);
}

// Cuarta pantalla: control tactil de RGB y estado real de la conexion Wi-Fi.
void dibujarControlNH() {
  const uint16_t fondo=tft.color565(7,19,33);
  const uint16_t panel=tft.color565(16,39,58);
  tft.fillScreen(fondo);
  tft.setTextDatum(TL_DATUM);
  orbCentro("NANOHUMANO LABS",240,12,TFT_CYAN,true);
  orbCentro("NH-01  /  SYSTEM CONTROL",240,45,TFT_LIGHTGREY);
  tft.drawFastHLine(14,70,452,TFT_DARKCYAN);
  const uint16_t activoColor=tft.color565(14,93,129);
  orbTexto("CONEXION WI-FI",20,76,TFT_CYAN);
  tft.fillRoundRect(14,98,452,52,9,panel);
  tft.setTextColor(TFT_WHITE,panel);
  if(WiFi.status()==WL_CONNECTED) {
    String ssid=WiFi.SSID();
    if(ssid.length()>26) ssid=ssid.substring(0,25)+"...";
    tft.drawString("RED: "+ssid,25,104,2);
    tft.drawString("WEB: http://"+WiFi.localIP().toString(),25,126,2);
    tft.drawString("SENAL: "+String(WiFi.RSSI())+" dBm",265,126,2);
  } else {
    tft.drawString("Wi-Fi desconectado",25,115,2);
  }
  // Fila 1: brillo del LED RGB (4 botones)
  orbTexto("BRILLO LED RGB",20,154,TFT_CYAN);
  const int niveles[4]={25,50,75,100};
  for(int i=0;i<4;i++) {
    int x=14+i*116;
    bool activo=brilloRGB==niveles[i];
    tft.fillRoundRect(x,176,104,40,9,activo?activoColor:panel);
    tft.drawRoundRect(x,176,104,40,9,activo?TFT_CYAN:TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE,activo?activoColor:panel);
    tft.drawCentreString(String(niveles[i])+"%",x+52,183,4);
  }
  // Fila 2: brillo de la pantalla (AUTO + 4 niveles)
  orbTexto("BRILLO PANTALLA",20,220,TFT_CYAN);
  const uint8_t modos[5]={0,25,50,75,100};
  for(int i=0;i<5;i++) {
    int x=14+i*92;
    bool activo=modoBrilloPantalla==modos[i];
    tft.fillRoundRect(x,242,84,40,9,activo?activoColor:panel);
    tft.drawRoundRect(x,242,84,40,9,activo?TFT_CYAN:TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE,activo?activoColor:panel);
    String txt = modos[i]==0 ? String("AUTO") : String(modos[i])+"%";
    tft.drawCentreString(txt,x+42,249,4);
  }
  tft.setTextColor(TFT_LIGHTGREY,fondo);
  tft.drawCentreString("AUTO: baja al ponerse el sol  |  TOCA FUERA PARA VOLVER",240,294,2);
}
void seleccionarBrilloNH(uint8_t nivel) {
  brilloRGB=nivel;
  preferenciasNH.putUChar("brillo",brilloRGB);
  if(pagina==3) dibujarControlNH(); // Tambien se llama desde la web
}
void seleccionarBrilloPantalla(uint8_t modo) {
  modoBrilloPantalla=modo;
  preferenciasNH.putUChar("pantalla",modoBrilloPantalla);
  ultimoCalculoBrillo=0; // Recalcular ya
  if(pagina==3) dibujarControlNH();
}
void mostrarPaginaNH() {
  if(pagina==0) {
    horaAnterior=""; fechaAnterior="";
    dibujarBase(); actualizarReloj(); actualizarInterior();
    actualizarWiFi(); dibujarMeteo();
  } else if(pagina==1) {
    dibujarCincoDias();
  } else if(pagina==2) {
    dibujarDetalle();
  } else {
    dibujarControlNH();
  }
}

// Digitos digitales de siete segmentos, dibujados directamente en TFT.
// No requieren fuentes externas ni nuevas bibliotecas.
void segmentoH(int x, int y, int largo, uint16_t color) {
  tft.fillRoundRect(x, y, largo, 7, 3, color);
}
void segmentoV(int x, int y, int largo, uint16_t color) {
  tft.fillRoundRect(x, y, 7, largo, 3, color);
}
void digitoDigital(char c, int x, int y) {
  const uint8_t patron[10] = {
    0b1111110, 0b0110000, 0b1101101, 0b1111001, 0b0110011,
    0b1011011, 0b1011111, 0b1110000, 0b1111111, 0b1111011
  };
  if (c < '0' || c > '9') return;
  uint8_t p = patron[c - '0'];
  const uint16_t luz = TFT_CYAN;
  const uint16_t apagado = tft.color565(13, 34, 48);
  // Bits: A B C D E F G
  segmentoH(x+7, y, 29, (p & 0b1000000) ? luz : apagado);        // A
  segmentoV(x+36, y+6, 27, (p & 0b0100000) ? luz : apagado);      // B
  segmentoV(x+36, y+39, 27, (p & 0b0010000) ? luz : apagado);     // C
  segmentoH(x+7, y+66, 29, (p & 0b0001000) ? luz : apagado);      // D
  segmentoV(x, y+39, 27, (p & 0b0000100) ? luz : apagado);        // E
  segmentoV(x, y+6, 27, (p & 0b0000010) ? luz : apagado);         // F
  segmentoH(x+7, y+33, 29, (p & 0b0000001) ? luz : apagado);      // G
}
void pintarHoraDigital(const String& hora) {
  const int x = 143, y = 39;
  // Fondo solo del reloj: nunca borra la pantalla entera
  tft.fillRect(110, 37, 260, 80, TFT_BLACK);
  digitoDigital(hora[0], x, y);
  digitoDigital(hora[1], x+48, y);
  tft.fillCircle(x+98, y+24, 4, TFT_CYAN);
  tft.fillCircle(x+98, y+49, 4, TFT_CYAN);
  digitoDigital(hora[3], x+112, y);
  digitoDigital(hora[4], x+160, y);
}
// Iconos compactos de amanecer y atardecer: sin imagenes ni librerias extra.
// Amanecer y atardecer con semicirculo, rayos y lineas del horizonte.
void iconoSolar(int cx, int cy, uint16_t color, bool amanecer) {
  tft.fillCircle(cx, cy + 5, 10, color);
  tft.fillRect(cx - 11, cy + 6, 22, 12, tft.color565(9, 28, 48));
  tft.drawFastHLine(cx - 18, cy + 6, 36, color);
  tft.drawFastHLine(cx - 12, cy + 10, 24, color);
  tft.drawLine(cx, cy - 13, cx, cy - 9, color);
  tft.drawLine(cx - 13, cy - 8, cx - 10, cy - 5, color);
  tft.drawLine(cx + 13, cy - 8, cx + 10, cy - 5, color);
  tft.drawFastHLine(cx - 19, cy, 5, color);
  tft.drawFastHLine(cx + 14, cy, 5, color);
  // La orientacion queda expresada por el color y el rotulo.
}
void dibujarSol() {
  if (pagina != 0) return;
  uint16_t fondo = tft.color565(9, 28, 48);
  tft.fillRoundRect(8, 145, 464, 48, 10, fondo);
  tft.drawFastVLine(240, 153, 32, TFT_CYAN);
  iconoSolar(91, 170, TFT_YELLOW, true);
  orbTexto("SALIDA", 120, 150, TFT_WHITE);
  orbTexto(salidaSol, 120, 169, TFT_YELLOW, true);
  iconoSolar(292, 170, TFT_ORANGE, false);
  orbTexto("PUESTA", 321, 150, TFT_WHITE);
  orbTexto(puestaSol, 321, 169, TFT_ORANGE, true);
}
// Fase lunar aproximada, calculada localmente sin consultas adicionales.
// Referencia: luna nueva del 6 de enero de 2000, 18:14 UTC.
void dibujarLuna(const struct tm &fechaLocal) {
  const double ciclo=29.530588853;
  double dias=difftime(time(nullptr), 947182440) / 86400.0;
  double fase=fmod(dias/ciclo,1.0);
  if (fase<0) fase+=1.0;
  const int cx=65,cy=65,radio=20;
  const uint16_t oscuro=tft.color565(19,35,53);
  const uint16_t claro=tft.color565(211,224,237);
  tft.fillRect(31,40,69,76,TFT_BLACK);
  // Luna creciente iluminada a la derecha, menguante a la izquierda.
  double ang=2.0*PI*fase;
  for (int dy=-radio;dy<=radio;dy++) {
    int ext=(int)sqrt((double)radio*radio-dy*dy);
    double borde=cos(ang)*ext; // v14: corregido (antes luna nueva y llena salian al reves)
    for(int dx=-ext;dx<=ext;dx++){
      bool iluminado=(fase<0.5) ? (dx>=borde) : (dx<=-borde);
      tft.drawPixel(cx+dx,cy+dy,iluminado?claro:oscuro);
    }
  }
  orbCentro("LUNA",65,91,TFT_LIGHTGREY);
}
void actualizarReloj() {
  struct tm ahora;
  if (!getLocalTime(&ahora, 100)) {
    if (horaAnterior == "") {
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.drawCentreString("Sincronizando hora...", 240, 90, 2);
    }
    return;
  }
  char h[16];
  snprintf(h, sizeof(h), "%02d:%02d", ahora.tm_hour, ahora.tm_min);
  String hora = h;
  String fecha = String(dias[ahora.tm_wday]) + " " + String(ahora.tm_mday) + " de " + meses[ahora.tm_mon] + " de " + String(ahora.tm_year + 1900);
  if (ahora.tm_yday != ultimoDiaLunar) { dibujarLuna(ahora); ultimoDiaLunar=ahora.tm_yday; }
  if (hora != horaAnterior) {
    pintarHoraDigital(hora);
    horaAnterior = hora;
  }
  if (fecha != fechaAnterior) {
    tft.fillRect(10, 117, 460, 27, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    orbCentro(fecha, 240, 120, TFT_LIGHTGREY);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    orbDerecha(">", 464, 120, TFT_CYAN);
    dibujarSol();
    fechaAnterior = fecha;
  }
}
// Gota de agua: dibujo directo en la pantalla, sin fuentes ni librerias extra.
void dibujarGota(int cx, int cy, uint16_t color) {
  tft.fillTriangle(cx, cy - 11, cx - 7, cy + 2, cx + 7, cy + 2, color);
  tft.fillCircle(cx, cy + 3, 7, color);
  tft.fillCircle(cx - 2, cy + 2, 2, TFT_WHITE);
}
// v15: lectura separada del dibujo, para llevar maxima y minima del dia
// aunque estes en otra pantalla.
// v17: filtro de mediana de 5 lecturas. Una lectura suelta mala del DHT11
// ya no aparece en pantalla ni cuenta para la maxima y la minima.
float medianaDe(const float* v, int n) {
  float o[5];
  for (int i = 0; i < n; i++) o[i] = v[i];
  for (int i = 1; i < n; i++) { float x = o[i]; int j = i - 1; while (j >= 0 && o[j] > x) { o[j + 1] = o[j]; j--; } o[j + 1] = x; }
  return (n % 2) ? o[n / 2] : (o[n / 2 - 1] + o[n / 2]) / 2.0f;
}
void leerInterior() {
  static float histT[5], histH[5];
  static int nT = 0, pT = 0, nH = 0, pH = 0;
  float tLeida = dht.readTemperature(), hLeida = dht.readHumidity();
  float t = NAN;
  if (!isnan(tLeida) && tLeida > -20 && tLeida < 60) {
    histT[pT] = tLeida; pT = (pT + 1) % 5; if (nT < 5) nT++;
    t = medianaDe(histT, nT);
    tempInterior = t;
  }
  if (!isnan(hLeida) && hLeida >= 0 && hLeida <= 100) {
    histH[pH] = hLeida; pH = (pH + 1) % 5; if (nH < 5) nH++;
    humedad = medianaDe(histH, nH);
  }
  struct tm hoy;
  if (horaLocalYa(&hoy) && hoy.tm_yday != diaMinMax) {
    diaMinMax = hoy.tm_yday; interiorMax = NAN; interiorMin = NAN; // Nuevo dia
  }
  if (!isnan(t) && nT >= 3) {
    if (isnan(interiorMax) || t > interiorMax) interiorMax = t;
    if (isnan(interiorMin) || t < interiorMin) interiorMin = t;
  }
}
void actualizarInterior() {
  leerInterior();
  dibujarInterior();
}
void dibujarInterior() {
  if (pagina != 0) return;
  // Se borra unicamente el contenido inferior, conservando el titulo INTERIOR.
  tft.fillRect(15, 283, 215, 32, TFT_DARKGREY);
  if (!isnan(tempInterior)) {
    orbCentro(String(tempInterior, 1) + " C", 88, 283, TFT_YELLOW, true);
  } else {
    orbCentro("Error DHT", 88, 285, TFT_RED);
  }
  if (!isnan(humedad)) {
    dibujarGota(162, 297, TFT_CYAN);
    orbTexto(String(humedad, 0) + "%", 174, 285, TFT_GREEN);
  }
}
void actualizarWiFi() {
  tft.fillRect(395, 9, 40, 25, TFT_BLACK);
  if (WiFi.status() != WL_CONNECTED) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawCentreString("X", 411, 13, 2);
    return;
  }
  int rssi = WiFi.RSSI(), barras = 1;
  if (rssi >= -55) barras = 4;
  else if (rssi >= -67) barras = 3;
  else if (rssi >= -75) barras = 2;
  uint16_t color = barras >= 3 ? TFT_GREEN : (barras == 2 ? TFT_YELLOW : TFT_RED);
  for (int i = 0; i < 4; ++i) {
    int alto = 5 + i * 5;
    tft.fillRect(396 + i * 10, 33 - alto, 7, alto, i < barras ? color : TFT_DARKGREY);
  }
}
String descripcionTiempo(int c) {
  if (c == 0) return "Despejado";
  if (c == 1) return "Mayormente despejado";
  if (c == 2) return "Parcialmente nublado";
  if (c == 3) return "Nublado";
  if (c == 45 || c == 48) return "Niebla";
  if (c >= 51 && c <= 57) return "Llovizna";
  if (c >= 61 && c <= 67) return "Lluvia";
  if (c >= 71 && c <= 77) return "Nieve";
  if (c == 85 || c == 86) return "Chubascos de nieve";
  if (c >= 80 && c <= 82) return "Chubascos";
  if (c >= 95) return "Tormentas";
  return "Tiempo variable";
}
void nube(int x, int y, uint16_t color) {
  tft.fillCircle(x + 13, y + 16, 9, color);
  tft.fillCircle(x + 27, y + 11, 13, color);
  tft.fillCircle(x + 40, y + 17, 9, color);
  tft.fillRoundRect(x + 5, y + 16, 45, 14, 6, color);
}
void sol(int x, int y) {
  tft.fillCircle(x, y, 11, TFT_YELLOW);
  for (int a = 0; a < 360; a += 45) {
    float rad = a * 0.0174532925f;
    tft.drawLine(x + (int)(15 * cosf(rad)), y + (int)(15 * sinf(rad)),
                 x + (int)(20 * cosf(rad)), y + (int)(20 * sinf(rad)), TFT_ORANGE);
  }
}
void iconoMeteo(int codigo, int x, int y) {
  // Icono de 58 x 46 dentro del panel meteorologico
  if (codigo == 0 || codigo == 1) {
    sol(x + 27, y + 23);
  } else if (codigo == 2) {
    sol(x + 19, y + 17);
    nube(x + 8, y + 12, TFT_LIGHTGREY);
  } else if (codigo == 3) {
    nube(x + 2, y + 8, TFT_LIGHTGREY);
  } else if (codigo == 45 || codigo == 48) {
    nube(x + 2, y + 2, TFT_LIGHTGREY);
    tft.drawFastHLine(x + 4, y + 36, 48, TFT_CYAN);
    tft.drawFastHLine(x + 11, y + 42, 37, TFT_CYAN);
  } else if ((codigo >= 51 && codigo <= 67) || (codigo >= 80 && codigo <= 82)) {
    nube(x + 2, y + 1, TFT_LIGHTGREY);
    for (int i = 0; i < 3; ++i) tft.drawLine(x + 14 + i * 14, y + 33, x + 10 + i * 14, y + 42, TFT_CYAN);
  } else if ((codigo >= 71 && codigo <= 77) || codigo == 85 || codigo == 86) {
    nube(x + 2, y + 1, TFT_LIGHTGREY);
    for (int i = 0; i < 3; ++i) tft.fillCircle(x + 13 + i * 14, y + 39, 2, TFT_WHITE);
  } else if (codigo >= 95) {
    nube(x + 2, y + 1, TFT_LIGHTGREY);
    tft.fillTriangle(x + 30, y + 29, x + 21, y + 41, x + 29, y + 39, TFT_YELLOW);
    tft.fillTriangle(x + 29, y + 39, x + 25, y + 47, x + 38, y + 35, TFT_YELLOW);
  } else {
    nube(x + 2, y + 8, TFT_LIGHTGREY);
  }
}
// Icono de viento: tres corrientes curvas, dibujadas sin fuentes adicionales.
void iconoViento(int x, int y, uint16_t color) {
  tft.drawFastHLine(x, y + 4, 16, color);
  tft.drawLine(x + 16, y + 4, x + 20, y + 2, color);
  tft.drawLine(x + 20, y + 2, x + 23, y + 4, color);
  tft.drawLine(x + 23, y + 4, x + 22, y + 7, color);
  tft.drawFastHLine(x + 3, y + 10, 18, color);
  tft.drawLine(x + 21, y + 10, x + 24, y + 12, color);
  tft.drawLine(x + 24, y + 12, x + 22, y + 15, color);
  tft.drawFastHLine(x, y + 16, 12, color);
  tft.drawLine(x + 12, y + 16, x + 15, y + 18, color);
}
String rumboViento(int grados) {
  if (grados < 0) return "--";
  const char* rumbos[] = {"N", "NE", "E", "SE", "S", "SO", "O", "NO"};
  return String(rumbos[((grados + 22) % 360) / 45]);
}
void dibujarMeteo() {
  // Redibujamos solo el contenido de la tarjeta meteorologica.
  tft.fillRoundRect(12, 220, 456, 38, 6, TFT_NAVY);
  tft.fillRoundRect(252, 283, 212, 31, 5, TFT_DARKGREY);
  if (!meteoDisponible) {
    orbCentro("Esperando meteo...", 240, 226, TFT_YELLOW);
    orbCentro("--.- C", 358, 281, TFT_CYAN, true);
    return;
  }

  // Icono a la izquierda, como en el diseno original.
  iconoMeteo(codigoMeteo, 18, 208);

  // Columna central: estado del cielo y temperaturas.
  String estado = descripcionTiempo(codigoMeteo);
  // Evitar que las descripciones largas invadan la columna del viento.
  while (estado.length() > 1 && orbAncho(estado) > 207) {
    estado.remove(estado.length() - 1);
  }
  orbTexto(estado, 91, 220, TFT_WHITE);
  orbTexto("Max " + String(maxima, 0) + " C  Min " + String(minima, 0) + " C", 91, 241, TFT_CYAN);

  // Separador sutil para mantener alineadas las dos columnas.
  tft.drawFastVLine(310, 221, 33, tft.color565(42, 88, 122));

  // Columna derecha: viento arriba, lluvia abajo.
  if (!isnan(vientoActual)) {
    iconoViento(318, 220, TFT_YELLOW);
    orbTexto(String(vientoActual, 0) + "km/h " + rumboViento(direccionViento), 347, 220, TFT_YELLOW);
  }
  // Icono sencillo de lluvia, dibujado con primitivas TFT_eSPI.
  tft.drawLine(325, 241, 321, 247, TFT_CYAN);
  tft.drawLine(325, 241, 329, 247, TFT_CYAN);
  tft.fillCircle(325, 248, 4, TFT_CYAN);
  orbTexto("Lluvia " + String(lluvia) + "%", 347, 241, TFT_CYAN);

  // Exterior: misma distribucion que INTERIOR (temperatura izquierda, gota y humedad derecha).
  tft.fillRect(251, 283, 214, 32, TFT_DARKGREY);
  if (!isnan(tempExterior)) {
    orbCentro(String(tempExterior, 1) + " C", 324, 283, TFT_CYAN, true);
  }
  if (!isnan(humedadExterior)) {
    dibujarGota(398, 297, TFT_CYAN);
    orbTexto(String(humedadExterior, 0) + "%", 410, 285, TFT_GREEN);
  }
}

bool procesarRespuesta(const String& respuesta) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, respuesta);
  if (err) {
    Serial.print("Error JSON: "); Serial.println(err.c_str());
    return false;
  }
  JsonVariant actual = doc["current"]["temperature_2m"];
  JsonVariant maximo = doc["daily"]["temperature_2m_max"][0];
  JsonVariant minimo = doc["daily"]["temperature_2m_min"][0];
  JsonVariant precipitacion = doc["daily"]["precipitation_probability_max"][0];
  JsonVariant tiempo = doc["daily"]["weather_code"][0];
  if (actual.isNull() || maximo.isNull() || minimo.isNull() || precipitacion.isNull() || tiempo.isNull()) {
    Serial.println("JSON incompleto");
    Serial.println(respuesta.substring(0, 300));
    return false;
  }
  tempExterior = actual.as<float>();
  JsonVariant humedadActual = doc["current"]["relative_humidity_2m"];
  humedadExterior = humedadActual.isNull() ? NAN : humedadActual.as<float>();
  maxima = maximo.as<float>();
  minima = minimo.as<float>();
  lluvia = precipitacion.as<int>();
  codigoMeteo = tiempo.as<int>();
  codigoMeteoRGB = codigoMeteo;
  lluviaRGB = lluvia;
  if (!doc["current"]["wind_speed_10m"].isNull())
    vientoActual = doc["current"]["wind_speed_10m"].as<float>();
  if (!doc["current"]["wind_direction_10m"].isNull())
    direccionViento = doc["current"]["wind_direction_10m"].as<int>();
  // Open-Meteo devuelve horas locales porque se solicita Europe/Madrid.
  const char* salida = doc["daily"]["sunrise"][0] | "";
  const char* puesta = doc["daily"]["sunset"][0] | "";
  if (strlen(salida) >= 16 && strlen(puesta) >= 16) {
    salidaSol = String(salida).substring(11, 16);
    puestaSol = String(puesta).substring(11, 16);
  }
  meteoDisponible = true;
  Serial.printf("Meteo OK: %.1f C, max %.1f, min %.1f, lluvia %d%%, codigo %d\n", tempExterior, maxima, minima, lluvia, codigoMeteo);
  JsonArray fechas = doc["daily"]["time"].as<JsonArray>();
  JsonArray maximos = doc["daily"]["temperature_2m_max"].as<JsonArray>();
  JsonArray minimos = doc["daily"]["temperature_2m_min"].as<JsonArray>();
  JsonArray lluvias = doc["daily"]["precipitation_probability_max"].as<JsonArray>();
  JsonArray codigos = doc["daily"]["weather_code"].as<JsonArray>();
  JsonArray vientos = doc["daily"]["wind_speed_10m_max"].as<JsonArray>();
  diasDisponibles = 0;
  for (int i = 0; i < 5 && i < (int)fechas.size(); i++) {
    if (fechas[i].isNull() || maximos[i].isNull() || minimos[i].isNull() ||
        lluvias[i].isNull() || codigos[i].isNull()) break;
    fecha5[i] = fechas[i].as<String>();
    max5[i] = maximos[i].as<float>();
    min5[i] = minimos[i].as<float>();
    lluvia5[i] = lluvias[i].as<int>();
    codigo5[i] = codigos[i].as<int>();
    vientoMax5[i] = (i < (int)vientos.size() && !vientos[i].isNull()) ? vientos[i].as<float>() : NAN;
    diasDisponibles++;
  }
  vientoPronosticoRGB = diasDisponibles > 0 ? vientoMax5[0] : NAN;
  // v15: sensacion termica, UV y las proximas 24 horas
  sensacionTermica = doc["current"]["apparent_temperature"].isNull() ? NAN : doc["current"]["apparent_temperature"].as<float>();
  uvMaximo = doc["daily"]["uv_index_max"][0].isNull() ? NAN : doc["daily"]["uv_index_max"][0].as<float>();
  JsonArray horasT = doc["hourly"]["time"].as<JsonArray>();
  JsonArray tempsH = doc["hourly"]["temperature_2m"].as<JsonArray>();
  JsonArray lluviasH = doc["hourly"]["precipitation_probability"].as<JsonArray>();
  horas24 = 0;
  for (int i = 0; i < 24 && i < (int)horasT.size(); i++) {
    if (tempsH[i].isNull()) break;
    String hs = horasT[i].as<String>();               // "2026-11-11T23:00"
    hora24[i] = hs.length() >= 13 ? hs.substring(11, 13).toInt() : i;
    temp24[i] = tempsH[i].as<float>();
    lluvia24[i] = lluviasH[i].isNull() ? 0 : lluviasH[i].as<int>();
    horas24++;
  }
  Serial.printf("Dias de prevision recibidos: %d\n", diasDisponibles);
  if (pagina == 0) { dibujarMeteo(); dibujarSol(); }
  else if (pagina == 1) dibujarCincoDias();
  else if (pagina == 2) dibujarDetalle();
  else dibujarControlNH();
  return true;
}
bool consultarURL(const String& url, bool seguro) {
  HTTPClient http;
  http.setTimeout(8000);
  if (seguro) {
    WiFiClientSecure cliente;
    cliente.setInsecure(); // Prototipo: sin verificacion de certificado
    if (!http.begin(cliente, url)) return false;
    int codigo = http.GET();
    Serial.printf("Meteo HTTPS: %d\n", codigo);
    if (codigo < 0) Serial.println(http.errorToString(codigo));
    bool ok = false;
    if (codigo == HTTP_CODE_OK) ok = procesarRespuesta(http.getString());
    else if (codigo > 0) Serial.println(http.getString().substring(0, 200));
    http.end();
    return ok;
  }
  WiFiClient cliente;
  if (!http.begin(cliente, url)) return false;
  int codigo = http.GET();
  Serial.printf("Meteo HTTP: %d\n", codigo);
  if (codigo < 0) Serial.println(http.errorToString(codigo));
  bool ok = false;
  if (codigo == HTTP_CODE_OK) ok = procesarRespuesta(http.getString());
  else if (codigo > 0) Serial.println(http.getString().substring(0, 200));
  http.end();
  return ok;
}
// v15: descarga un JSON por HTTPS (sin verificar certificado, como el resto).
bool descargarJSON(const String& url, JsonDocument& doc) {
  WiFiClientSecure cliente;
  cliente.setInsecure();
  HTTPClient http;
  http.setTimeout(8000);
  if (!http.begin(cliente, url)) return false;
  int codigo = http.GET();
  bool ok = false;
  if (codigo == HTTP_CODE_OK) ok = !deserializeJson(doc, http.getString());
  else Serial.printf("Extra HTTP %d\n", codigo);
  http.end();
  return ok;
}
// Temperatura del agua y olas (API marina) + calidad del aire y polvo (calima).
void consultarExtras() {
  if (WiFi.status() != WL_CONNECTED) return;
  String coords = "latitude=" + String(LAT, 4) + "&longitude=" + String(LON, 4);
  bool algo = false;
  {
    JsonDocument doc;
    // cell_selection=sea: usa el punto de mar mas cercano aunque la ciudad este en tierra.
    if (descargarJSON("https://marine-api.open-meteo.com/v1/marine?" + coords +
                      "&current=wave_height,sea_surface_temperature&cell_selection=sea", doc)) {
      alturaOlas = doc["current"]["wave_height"].isNull() ? NAN : doc["current"]["wave_height"].as<float>();
      temperaturaMar = doc["current"]["sea_surface_temperature"].isNull() ? NAN : doc["current"]["sea_surface_temperature"].as<float>();
      algo = true;
    }
  }
  {
    JsonDocument doc;
    if (descargarJSON("https://air-quality-api.open-meteo.com/v1/air-quality?" + coords +
                      "&current=european_aqi,dust", doc)) {
      indiceAire = doc["current"]["european_aqi"].isNull() ? -1 : doc["current"]["european_aqi"].as<int>();
      polvo = doc["current"]["dust"].isNull() ? NAN : doc["current"]["dust"].as<float>();
      algo = true;
    }
  }
  if (algo) { extrasOk = true; ultimoExtras = millis(); }
  Serial.printf("Extras: mar %.1f C, olas %.1f m, aire %d, polvo %.0f\n", temperaturaMar, alturaOlas, indiceAire, polvo);
  if (pagina == 2) dibujarDetalle();
}
void consultarMeteo() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Meteo: WiFi desconectado");
    return;
  }
  String parametros = "latitude=" + String(LAT, 4) + "&longitude=" + String(LON, 4) +
    "&current=temperature_2m,relative_humidity_2m,wind_speed_10m,wind_direction_10m,apparent_temperature"
    "&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max,weather_code,sunrise,sunset,wind_speed_10m_max,uv_index_max"
    "&hourly=temperature_2m,precipitation_probability&forecast_hours=24"
    "&wind_speed_unit=kmh&timezone=Europe%2FMadrid&forecast_days=5";
  Serial.println("Consultando meteorologia de " + ciudad + "...");
  bool ok = consultarURL("https://api.open-meteo.com/v1/forecast?" + parametros, true);
  if (!ok) {
    Serial.println("HTTPS fallo; probando HTTP...");
    ok = consultarURL("http://api.open-meteo.com/v1/forecast?" + parametros, false);
  }
  if (!ok) { Serial.println("No se pudo obtener la meteorologia"); return; }
  // Mar y aire cambian despacio: cada 30 minutos es suficiente.
  if (!extrasOk || millis() - ultimoExtras >= 1800000UL) consultarExtras();
}
// =====================================================================
// v15: ciudad configurable, web interna y actualizacion por Wi-Fi (OTA)
// =====================================================================
WiFiManagerParameter* paramCiudad = nullptr;
WiFiManagerParameter* paramLat = nullptr;
WiFiManagerParameter* paramLon = nullptr;
bool refrescarMeteo = false;

bool textoEsNumero(const String& t) {
  bool digito = false;
  for (unsigned int i = 0; i < t.length(); i++) {
    char c = t[i];
    if (isdigit(c)) digito = true;
    else if (c != '.' && c != '-' && c != '+' && c != ' ') return false;
  }
  return digito;
}
bool guardarCiudad(String nombre, const String& latTxt, const String& lonTxt) {
  nombre.trim();
  if (nombre.length() == 0 || !textoEsNumero(latTxt) || !textoEsNumero(lonTxt)) return false;
  float la = latTxt.toFloat(), lo = lonTxt.toFloat();
  if (la < -90 || la > 90 || lo < -180 || lo > 180) return false;
  if (nombre.length() > 20) nombre = nombre.substring(0, 20);
  ciudad = nombre; LAT = la; LON = lo;
  preferenciasNH.putString("ciudad", ciudad);
  preferenciasNH.putFloat("lat", LAT);
  preferenciasNH.putFloat("lon", LON);
  // Datos viejos fuera y nueva consulta en el loop.
  meteoDisponible = false; diasDisponibles = 0; horas24 = 0; extrasOk = false;
  sensacionTermica = uvMaximo = temperaturaMar = alturaOlas = polvo = NAN; indiceAire = -1;
  refrescarMeteo = true;
  Serial.printf("Nueva ciudad: %s (%.4f, %.4f)\n", ciudad.c_str(), LAT, LON);
  return true;
}
void guardarParametrosPortal() {
  if (paramCiudad && paramLat && paramLon)
    guardarCiudad(paramCiudad->getValue(), paramLat->getValue(), paramLon->getValue());
}
void prepararParametrosPortal() {
  static char bufLat[16], bufLon[16];
  snprintf(bufLat, sizeof(bufLat), "%.4f", LAT);
  snprintf(bufLon, sizeof(bufLon), "%.4f", LON);
  paramCiudad = new WiFiManagerParameter("ciudad", "Ciudad (nombre en pantalla)", ciudad.c_str(), 20);
  paramLat = new WiFiManagerParameter("lat", "Latitud (ej. 39.5751)", bufLat, 12);
  paramLon = new WiFiManagerParameter("lon", "Longitud (ej. 2.4214)", bufLon, 12);
  wm.addParameter(paramCiudad);
  wm.addParameter(paramLat);
  wm.addParameter(paramLon);
  wm.setSaveParamsCallback(guardarParametrosPortal);
}

String htmlSeguro(String t) {
  t.replace("&", "&amp;"); t.replace("<", "&lt;"); t.replace(">", "&gt;"); t.replace("\"", "&quot;");
  return t;
}
String dato(float v, int dec, const char* unidad) {
  return isnan(v) ? String("--") : String(v, dec) + unidad;
}
String paginaWeb() {
  String h;
  h.reserve(14000);
  h += F("<!DOCTYPE html><html lang='es'><head><meta charset='utf-8'>"
         "<meta name='viewport' content='width=device-width,initial-scale=1'><title>NH-01</title><style>"
         "body{font-family:system-ui,sans-serif;background:#07131f;color:#e6f1f8;margin:0;padding:16px;max-width:560px;margin:auto}"
         "h1{color:#3fe0ff;margin:0 0 2px;font-size:26px;letter-spacing:.04em}small{color:#8aa}"
         ".top{display:flex;align-items:center;gap:14px;margin-bottom:4px}"
         ".top img{width:72px;height:72px;border-radius:50%;box-shadow:0 0 18px rgba(63,224,255,.3)}"
         ".g{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin:14px 0}"
         ".c{background:#10273a;border-radius:10px;padding:10px}.c b{display:block;font-size:22px;color:#fff}"
         ".c span{font-size:12px;color:#8ab}h2{font-size:15px;color:#ff8a3d;margin:18px 0 6px;letter-spacing:.06em;text-transform:uppercase}"
         "a.b{display:inline-block;padding:9px 13px;margin:3px;border-radius:8px;background:#10273a;color:#fff;text-decoration:none}"
         "a.on{background:#0e5d81;outline:1px solid #3fe0ff}"
         "input{width:100%;box-sizing:border-box;padding:8px;margin:3px 0 8px;border-radius:6px;border:0}"
         "button{padding:10px 16px;border:0;border-radius:8px;background:#0e5d81;color:#fff;font-size:15px}"
         "a.b:hover,button:hover{background:#ff8a3d}.c{border-left:3px solid #3fe0ff}"
         "</style></head><body>");
  h += F("<div class='top'><img alt='NANOHUMANO LABS' src='data:image/jpeg;base64," LOGO_NH_B64 "'><div>");
  h += "<h1>" + htmlSeguro(ciudad) + "</h1><small>NANOHUMANO LABS &middot; NH-01 &middot; ";
  struct tm t;
  if (horaLocalYa(&t)) { char b[24]; snprintf(b, sizeof(b), "%02d:%02d", t.tm_hour, t.tm_min); h += b; }
  h += " &middot; <a href='/' style='color:#3fe0ff'>actualizar</a></small></div></div><div class='g'>";
  auto carta = [&](const String& t1, const String& v) { h += "<div class='c'><span>" + t1 + "</span><b>" + v + "</b></div>"; };
  carta("Exterior", dato(tempExterior, 1, " &deg;C"));
  carta("Sensaci&oacute;n t&eacute;rmica", dato(sensacionTermica, 0, " &deg;C"));
  carta("Hoy m&aacute;x / m&iacute;n", meteoDisponible ? String(maxima, 0) + " / " + String(minima, 0) + " &deg;C" : String("--"));
  carta("Cielo", meteoDisponible ? descripcionTiempo(codigoMeteo) : String("--"));
  carta("Lluvia hoy", lluvia >= 0 ? String(lluvia) + " %" : String("--"));
  carta("Viento", isnan(vientoActual) ? String("--") : String(vientoActual, 0) + " km/h " + rumboViento(direccionViento));
  carta("Interior", dato(tempInterior, 1, " &deg;C") + " &middot; " + dato(humedad, 0, " %"));
  carta("Interior m&aacute;x / m&iacute;n", isnan(interiorMax) ? String("--") : String(interiorMax, 0) + " / " + String(interiorMin, 0) + " &deg;C");
  carta("&Iacute;ndice UV", dato(uvMaximo, 0, ""));
  carta("Agua del mar", dato(temperaturaMar, 0, " &deg;C") + " &middot; olas " + dato(alturaOlas, 1, " m"));
  carta("Calidad del aire", indiceAire < 0 ? String("--") : String(indiceAire) + ((!isnan(polvo) && polvo >= 50) ? " &middot; CALIMA" : ""));
  carta("Salida / puesta", salidaSol + " / " + puestaSol);
  h += "</div><h2>Brillo LED RGB</h2>";
  const int niveles[4] = {25, 50, 75, 100};
  for (int i = 0; i < 4; i++)
    h += "<a class='b" + String(brilloRGB == niveles[i] ? " on" : "") + "' href='/led?v=" + String(niveles[i]) + "'>" + String(niveles[i]) + "%</a>";
  h += "<h2>Brillo pantalla</h2>";
  const int modos[5] = {0, 25, 50, 75, 100};
  for (int i = 0; i < 5; i++)
    h += "<a class='b" + String(modoBrilloPantalla == modos[i] ? " on" : "") + "' href='/pantalla?v=" + String(modos[i]) + "'>" +
         (modos[i] == 0 ? String("AUTO") : String(modos[i]) + "%") + "</a>";
  h += "<h2>Ciudad</h2><form action='/ciudad'>Nombre<input name='nombre' maxlength='20' value='" + htmlSeguro(ciudad) +
       "'>Latitud<input name='lat' value='" + String(LAT, 4) + "'>Longitud<input name='lon' value='" + String(LON, 4) +
       "'><button>Guardar</button></form><p><small>Coordenadas: b&uacute;scalas en Google Maps (clic derecho sobre el sitio).</small></p>";
  h += "</body></html>";
  return h;
}
void webRedirigir() { servidorWeb.sendHeader("Location", "/"); servidorWeb.send(303); }
void configurarWeb() {
  servidorWeb.on("/", []() { servidorWeb.send(200, "text/html; charset=utf-8", paginaWeb()); });
  servidorWeb.on("/led", []() {
    int v = servidorWeb.arg("v").toInt();
    if (v == 25 || v == 50 || v == 75 || v == 100) seleccionarBrilloNH(v);
    webRedirigir();
  });
  servidorWeb.on("/pantalla", []() {
    int v = servidorWeb.arg("v").toInt();
    if (v == 0 || v == 25 || v == 50 || v == 75 || v == 100) seleccionarBrilloPantalla(v);
    webRedirigir();
  });
  servidorWeb.on("/ciudad", []() {
    if (guardarCiudad(servidorWeb.arg("nombre"), servidorWeb.arg("lat"), servidorWeb.arg("lon"))) {
      if (pagina != 3) mostrarPaginaNH();
      webRedirigir();
    } else {
      servidorWeb.send(400, "text/html; charset=utf-8",
        "<meta charset='utf-8'><p>Datos no v&aacute;lidos. La latitud va de -90 a 90 y la longitud de -180 a 180.</p><a href='/'>Volver</a>");
    }
  });
  servidorWeb.onNotFound(webRedirigir);
}
void configurarOTA() {
  ArduinoOTA.setHostname("nh-01");          // En el Arduino IDE: Puerto -> nh-01
  ArduinoOTA.setPassword(CLAVE_OTA);
  ArduinoOTA.onStart([]() {
    tft.fillScreen(TFT_BLACK);
    orbCentro("ACTUALIZANDO...", 240, 110, TFT_CYAN, true);
    tft.drawRoundRect(88, 160, 304, 22, 6, TFT_CYAN);
  });
  ArduinoOTA.onProgress([](unsigned int hecho, unsigned int total) {
    if (total) tft.fillRect(91, 163, (int)((uint64_t)hecho * 298 / total), 16, TFT_CYAN);
  });
  ArduinoOTA.onEnd([]() { orbCentro("LISTO. REINICIANDO...", 240, 200, TFT_GREEN); });
  ArduinoOTA.onError([](ota_error_t e) { orbCentro("ERROR AL ACTUALIZAR", 240, 200, TFT_RED); });
}
// Arranca web y OTA en cuanto hay Wi-Fi (y las vuelve a arrancar tras el portal).
void atenderRed() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (!otaActiva) {
    ArduinoOTA.begin();
    MDNS.addService("http", "tcp", 80);
    otaActiva = true;
    Serial.println("OTA y web: http://nh-01.local  o  http://" + WiFi.localIP().toString());
  }
  if (!webActiva) { servidorWeb.begin(); webActiva = true; }
  ArduinoOTA.handle();
  servidorWeb.handleClient();
}

// Portal no bloqueante: permite cancelar tocando la TFT.
void cerrarPortalWiFi() {
  wm.stopConfigPortal();
  portalWiFiActivo = false;
  wm.setConfigPortalBlocking(true); // Restaurar modo habitual para futuros arranques.
  pagina = 0;
  mostrarPaginaNH();
  ultimoMeteo = millis() - 60000UL;
}
// Pantalla de ayuda integrada para configurar la red desde un movil.
// conBotonVolver=false en el primer arranque (portal bloqueante, sin tactil).
void dibujarAyudaWiFi(bool conBotonVolver) {
  tft.fillScreen(TFT_BLACK);
  tft.drawRoundRect(9, 7, 462, 305, 10, TFT_CYAN);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("NANOHUMANO LABS", 240, 28, 4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("NH-01  |  CONFIGURAR WI-FI", 240, 66, 2);
  tft.drawFastHLine(30, 86, 420, TFT_DARKGREY);

  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("1. Abre Wi-Fi en tu movil.", 30, 99, 2);
  tft.drawString("2. Conectate a: NH-01", 30, 126, 2);
  tft.drawString("3. Abre en el navegador:", 30, 153, 2);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("192.168.4.1", 66, 178, 4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("4. Elige tu red, la clave y (opcional) tu ciudad.", 30, 218, 2);

  tft.fillRoundRect(27, 264, 426, 35, 7, TFT_DARKGREY);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_DARKGREY);
  if (conBotonVolver) tft.drawString("TOCA PARA VOLVER AL RELOJ", 240, 281, 2);
  else tft.drawString("SIN CONFIGURAR: ARRANCA SIN WI-FI EN 3 MIN", 240, 281, 2);
  tft.setTextDatum(TL_DATUM);
}
void alAbrirPortal(WiFiManager* gestor) {
  dibujarAyudaWiFi(portalWiFiActivo);
}
void configurarWiFi() {
  // El portal usa su propia web en el puerto 80: paramos la nuestra mientras tanto.
  if (webActiva) { servidorWeb.stop(); webActiva = false; }
  dibujarAyudaWiFi(true);
  wm.setConfigPortalTimeout(180);
  wm.setConfigPortalBlocking(false);
  portalWiFiActivo = true;
  inicioPortalWiFi = millis();
  wm.startConfigPortal("NH-01");
}
void setup() {
  preferenciasNH.begin("nh01",false);
  ciudad=preferenciasNH.getString("ciudad",CIUDAD_DEFECTO);
  LAT=preferenciasNH.getFloat("lat",LAT_DEFECTO);
  LON=preferenciasNH.getFloat("lon",LON_DEFECTO);
  brilloRGB=preferenciasNH.getUChar("brillo",75);
  if(brilloRGB!=25 && brilloRGB!=50 && brilloRGB!=75 && brilloRGB!=100) brilloRGB=75;
  modoBrilloPantalla=preferenciasNH.getUChar("pantalla",0);
  if(modoBrilloPantalla!=0 && modoBrilloPantalla!=25 && modoBrilloPantalla!=50 &&
     modoBrilloPantalla!=75 && modoBrilloPantalla!=100) modoBrilloPantalla=0;
  iniciarRGB();
  Serial.begin(115200);
  delay(500);
  dht.begin();
  tft.init();
  aplicarBrilloPantalla(); // Arranca al 100% hasta tener hora y datos del sol
  if (PIN_BRILLO_PANTALLA < 0) Serial.println("Aviso: sin pin de retroiluminacion, brillo de pantalla fijo");
  tft.setRotation(1);
  tft.setTouch(calData);
  tft.setTextDatum(TL_DATUM);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawCentreString("NANOHUMANO LABS", 240, 82, 4);
  tft.drawRoundRect(160, 123, 160, 61, 12, TFT_CYAN);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("NH-01", 240, 136, 4);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawCentreString("INICIANDO SISTEMA...", 240, 212, 2);
  WiFi.mode(WIFI_STA);
  wm.setConfigPortalTimeout(180);
  wm.setAPCallback(alAbrirPortal); // Muestra las instrucciones si no hay red guardada
  prepararParametrosPortal();      // v15: ciudad y coordenadas en el portal
  // v16: portal con el estilo y el logo de NANOHUMANO LABS
  wm.setTitle("NH-01");
  wm.setClass("invert");
  wm.setCustomHeadElement(CABECERA_PORTAL);
  wm.setMenu(MENU_PORTAL, 1);
  if (wm.autoConnect("NH-01")) {
    Serial.print("WiFi conectado: "); Serial.println(WiFi.localIP());
  } else Serial.println("WiFi no conectado");
  configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org", "time.google.com");
  dibujarBase();
  actualizarReloj();
  actualizarInterior();
  actualizarWiFi();
  dibujarMeteo();
  ultimaHora = ultimoDHT = ultimoWiFi = millis();
  configurarWeb();
  configurarOTA();
  consultarMeteo();
  ultimoMeteo = millis();
}
void loop() {
  // Mantener el portal Wi-Fi activo sin bloquear la pantalla tactil.
  if (portalWiFiActivo) {
    actualizarBrilloPantalla();
    if (wm.process()) {          // Se ha conectado a la nueva red
      Serial.println("WiFi configurado desde el portal");
      cerrarPortalWiFi();
      return;
    }
    uint16_t tx=0, ty=0;
    if (tft.getTouch(&tx, &ty)) {
      while (tft.getTouch(&tx, &ty)) delay(25);
      cerrarPortalWiFi();
      delay(180);
    } else if (millis() - inicioPortalWiFi >= 180000UL) {
      cerrarPortalWiFi();
    }
    actualizarRGB();
    delay(20);
    return;
  }
  actualizarRGB();
  actualizarBrilloPantalla();
  atenderRed();   // v15: web interna y actualizaciones por Wi-Fi
  if (refrescarMeteo) { refrescarMeteo = false; consultarMeteo(); ultimoMeteo = millis(); }
  unsigned long ahora = millis();
  if (pagina == 0 && ahora - ultimaHora >= 1000UL) { actualizarReloj(); ultimaHora = millis(); }
  if (ahora - ultimoDHT >= 5000UL) { leerInterior(); dibujarInterior(); ultimoDHT = millis(); }
  if (pagina == 0 && ahora - ultimoWiFi >= 5000UL) { actualizarWiFi(); ultimoWiFi = millis(); }
  unsigned long intervalo = meteoDisponible ? 600000UL : 60000UL;
  if (ahora - ultimoMeteo >= intervalo) { consultarMeteo(); ultimoMeteo = millis(); }
  uint16_t x = 0, y = 0;
  if (tft.getTouch(&x, &y)) {
    if (pagina == 0 && x >= 390 && y <= 45) {
      configurarWiFi();
    } else if (pagina == 3 && y >= 176 && y <= 216 && x >= 14 && x < 466) {
      int indice=(x-14)/116;
      if (indice>=0 && indice<4 && x < 14+indice*116+104) {
        const uint8_t niveles[4]={25,50,75,100};
        seleccionarBrilloNH(niveles[indice]);
      }
    } else if (pagina == 3 && y >= 242 && y <= 282 && x >= 14 && x < 466) {
      int indice=(x-14)/92;
      if (indice>=0 && indice<5 && x < 14+indice*92+84) {
        const uint8_t modos[5]={0,25,50,75,100};
        seleccionarBrilloPantalla(modos[indice]);
      }
    } else if (pagina == 3 && y > 150 && y < 290) {
      // Toque entre botones: no cambiar de pagina por accidente.
    } else {
      pagina=(pagina+1)%4;
      mostrarPaginaNH();
    }
    // Esperar a soltar el dedo para evitar cambios repetidos
    while (tft.getTouch(&x, &y)) delay(25);
    delay(180);
  }
  delay(20);
}
