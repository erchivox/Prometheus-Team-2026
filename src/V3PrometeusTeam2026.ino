//acuerdate que este codigo tiene 4 segundos para deteccion de una nueva linea
// advertencia : toma 2 tasas de cafe, guarda una copia de seguridad y cuidate del bug que solo pasa a las 
// 2am, de resto suerte. 
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <Adafruit_BNO055.h>

#include <ESP32Servo.h>
#include <ColorConverterLib.h>
#include <VL53L1X.h>   // Pololu VL53L1X 
#include <VL53L0X.h>   // Pololu VL53L0X (sensores diagonales)
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>
#define SerialBT Serial

// --- Configuración de NeoPixels ---
#define NEOPIXEL_PIN 14   // ¡CÁMBIALO al pin de datos que vayas a usar!
#define NUMPIXELS 15       // ¡CÁMBIALO a la cantidad exacta de LEDs que tenga tu tira/módulo!

// ==================== MODO DE PRUEBA: SIMULAR VUELTA YA MEMORIZADA ====================
// Comenta esta línea para volver al comportamiento normal (memorización real en pista).
//#define MODO_PRUEBA_MANIOBRA_OPTIMIZADA

// ==================== TEST: SIMULAR LÍNEA POR COMANDO (A = azul, N = naranja) ====================
// Comenta esta línea cuando termines las pruebas.
//#define TEST_LINEA_SIMULADA

// Crear el objeto de la tira LED
Adafruit_NeoPixel tiraLED(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// --- Pines XSHUT para ToF ---
#define XSHUT_FRONTAL 18
#define XSHUT_TRASERO 17
#define XSHUT_IZQ     5
#define XSHUT_DER     16
#define TOF_MAX_MM_VALIDO 1500   // lecturas >= a esto (mm) se devuelven como 999 (igual que antes). El VL53L1X llega a ~4000

#define XSHUT_DIAG_IZQ 19   // VL53L0X diagonal izquierdo
#define XSHUT_DIAG_DER 23   // VL53L0X diagonal derecho
#define TOF0_MAX_MM_VALIDO 800   // >= esto (mm) se devuelve como 999 (igual que el límite de 80cm que tenía el Sharp)

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_1X);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);
//Servo direccion;
// (direccion se declara más abajo, junto a las constantes del servo)

#define I2C2_SDA 32
#define I2C2_SCL 33
TwoWire I2CSegundo = TwoWire(1);

// --- Direcciones I2C ToF ---
#define ADDR_FRONTAL 0x30
#define ADDR_TRASERO 0x31
#define ADDR_IZQ     0x32
#define ADDR_DER     0x33

#define ADDR_DIAG_IZQ 0x34
#define ADDR_DIAG_DER 0x35

// --- Objetos ---
VL53L1X tofF;
VL53L1X tofT;
VL53L1X tofI;
VL53L1X tofD;

VL53L0X tofDI;   // diagonal izquierdo
VL53L0X tofDD;   // diagonal derecho

// --- Variables BNO055 ---
float yaw_offset = 0.0f;

// --- Configuración de Pines del Motor (Puente H) ---
const int MOTOR_IN1 = 26; // ¡Corregido! Estaba en conflicto con el pin 27 del interruptor
const int MOTOR_IN2 = 25; // Pin de dirección
const int MOTOR_ENA = 27; // Pin PWM para la velocidad

// --- Variables de Velocidad (rango de 0 a 255) ---
int velocidadBase = 150;//208;//255;      // Velocidad normal para avanzar
int velocidadRetroceso = 120;//160;// 190;// 170;//200; // Velocidad para echar hacia atrás

int velocidadMedia = 130;//180;      // v96: avance del primer tramo de comandos dobles (ventana de detección de objeto)

int velocidadLenta = 90;//140;//180;           // Velocidad reducida para avanzar 
int velocidadRetrocesoLento = 100;//140;//150;// 100;//160;  // Velocidad reducida para retroceder

int velocidadGiroCarril = 150;                  // velocidad mientras gira hacia el carril (antes iba a velocidadBase = 180). Menor = más lecturas del ToF frontal antes de llegar al umbral
const float UMBRAL_ANGULO_ENDEREZADO = 20.0;    // ° de tolerancia para dar por terminado el contragiro (antes 8.0)

int velocidadMaxima = 170;//240;    // Velocidad máxima para tramos de solo avance (C02 / C07)

// ===== v103: VELOCIDAD DEL AVANCE HASTA TOF (aquí es donde pruebas) =====
const int MO_VELOCIDAD_AVANCE_TOF = 110;  // PWM del avance (velocidadBase = 150, velocidadLenta = 100). Más bajo = más lento
const int MO_PWM_ARRANQUE_MIN     = 100;  // PWM desde el que el carro ya se mueve. Si arranca a tirones o no arranca, ajústalo


// ==================== RAMPA DE ARRANQUE PROGRESIVO ====================
// Evita el golpe/tambaleo al arrancar o al invertir el sentido del motor,
// subiendo el PWM gradualmente hacia el valor objetivo en vez de aplicarlo
// de una sola vez.
int pwmActualMotor = 0;          // PWM que está realmente aplicado al motor en este instante
int pwmObjetivoMotor = 0;        // PWM al que se quiere llegar (velocidadBase, velocidadLenta, etc.)
bool sentidoActualEsAvance = true; // true = avanzando, false = retrocediendo (para saber si hay que invertir IN1/IN2 antes de rampear)

// Cuánto sube/baja el PWM en cada paso de la rampa, y cada cuántos ms se aplica un paso.
// Con INCREMENTO_RAMPA=10 y INTERVALO_RAMPA_MS=5, pasar de 0 a 208 tarda ~104ms — ajusta
// estos dos valores para hacerlo más suave (baja INCREMENTO o sube INTERVALO) o más
// rápido (al revés), sin tocar ningún otro punto del código.
const int INCREMENTO_RAMPA_PWM = 40;//10
const unsigned long INTERVALO_RAMPA_MS = 4;//5
unsigned long tUltimoPasoRampa = 0;

// --- Configuración de Canales PWM Independientes para ESP32 ---
const int canalMotor  = 4;
const int canalBuzzer = 5;
const int canalLED = 6;// Reservado por si quieres atenuar el LED luego

const int freqMotor = 1000;
const int resolucionPWM = 8;  // Resolución de 8 bits (0-255)

//const int MOTOR_RETROCESO_PIN = 19;  // Pin para motor de retroceso
//const int MOTOR_PIN = 15;
//const int MOTOR_RETROCESO_PIN_LENTO = 25;  // Pin para motor de retroceso
//const int MOTOR_PIN_LENTO = 26;
const int SERVO_PIN = 13;
// Declaraciones de pines (usar pines analógicos como digitales)
#define BUZZER_PIN  4  // Pin analógico A1 usado como digital para buzzer

// esta línea cerca del inicio del archivo, con las otras variables globales
bool primerGiroRealizado = false; // Flag para controlar el primer giro especial
bool forzarReinicioGiro = false;  // Fuerza el reseteo de procesarEstadoGirandoHaciaCarril() aunque el estado no cambie (evita que un re-giro con el mismo estado se salte la Fase 1)
//******************** colores 
// Variables para la detección del color azul
const int HUE_AZUL_MIN = 190;
const int HUE_AZUL_MAX = 240;
const float SATURACION_AZUL_MIN = 0.60;//0.40;
const float VALOR_AZUL_MIN = 0.40;
const float VALOR_AZUL_MAX = 0.80;
const float PROPORCION_AZUL_MIN = 0.95;//1.2;//1.66
const int CONFIRMAR_AZUL_TOLERANCIA = 1;
const int CONFIRMAR_AZUL_DELAY = 1;


const int HUE_NARANJA_MIN = 340;
const int HUE_NARANJA_MAX = 48;
const float SATURACION_NARANJA_MIN = 0.25; // Bajado a 0.25 para tolerar descoloramiento por brillo reflectedo
const float VALOR_NARANJA_MIN = 0.25;   // Permite detectar sombra/naranja oscuro
const float VALOR_NARANJA_MAX = 1.00;   // SUBIDO A 1.00: El blanco/brillo de la pista ya no bloqueará la lectura

const int CONFIRMAR_NARANJA_TOLERANCIA = 1;
const int CONFIRMAR_NARANJA_DELAY = 1;
//*****888*******************************


const int INTERRUPTOR_PIN = 34;  // Pin digital para el interruptor (usa pin con interrupt)

// Variables de estado del sistema
bool sistemaIniciado = false;
bool interruptorAnterior = false;
unsigned long tiempoDebounce = 0;
const unsigned long DEBOUNCE_DELAY = 50;

// ===== CONFIGURACIÓN DE SERVO (escala firmada desde el centro) =====
// 0 = recto | NEGATIVO = izquierda | POSITIVO = derecha
// >>> SOLO TOCA ESTAS TRES LÍNEAS AL CALIBRAR <<<
const int SERVO_CENTRO_FISICO = 88;   // 86 ángulo real del servo con las ruedas rectas
const int LIMITE_IZQ = -39;           // máximo hacia la IZQUIERDA (negativo)
const int LIMITE_DER =  44;           // máximo hacia la DERECHA (positivo)
//33 152
// Alias que usa todo el código (no tocar)
const int SERVO_CENTER   = 0;
const int SERVO_CENTRADO = 0;
const int SERVO_MIN      = LIMITE_IZQ;
const int SERVO_MAX      = LIMITE_DER;
const int SERVO_LEFT     = LIMITE_IZQ;
const int SERVO_RIGHT    = LIMITE_DER;

// Valor especial "no cambiar la dirección" (antes era -1, que ahora es un ángulo válido)
const float SERVO_SIN_COMANDO = 999.0;

// Recorrido disponible a cada lado del centro (asimétrico)
const int SERVO_SPAN_IZQ = -LIMITE_IZQ;   // 30
const int SERVO_SPAN_DER =  LIMITE_DER;   // 40

// Giro "casi a fondo"
const int SERVO_MARGEN_TOPE = 5;
const int SERVO_LEFT_SUAVE  = LIMITE_IZQ + SERVO_MARGEN_TOPE;  // -25
const int SERVO_RIGHT_SUAVE = LIMITE_DER - SERVO_MARGEN_TOPE;  //  35

// Giros orientados a objeto (52% del recorrido izq / 33% del der)
const int SERVO_OrientadoObjeto_LEFT = -(int)(SERVO_SPAN_IZQ * 0.52f);  // -15
const int SERVO_OrientadoObjeto_RIGH =  (int)(SERVO_SPAN_DER * 0.33f);  //  13

// Límites de corrección PROPORCIONAL (fracción del recorrido de cada lado)
const int SERVO_CORR_PARED_IZQ    = (int)(SERVO_SPAN_IZQ * 0.55f); // ~16 (PID de pared)
const int SERVO_CORR_PARED_DER    = (int)(SERVO_SPAN_DER * 0.80f); // ~32
const int SERVO_CORR_CENTRADO_IZQ = (int)(SERVO_SPAN_IZQ * 0.50f); // ~15 (PID de centrado)
const int SERVO_CORR_CENTRADO_DER = (int)(SERVO_SPAN_DER * 0.70f); // ~28
const int MO_SERVO_CORR_MAX_IZQ   = (int)(SERVO_SPAN_IZQ * 0.35f); // ~10 (rumbo suave)
const int MO_SERVO_CORR_MAX_DER   = (int)(SERVO_SPAN_DER * 0.45f); // ~18

// Protección: si pones valores incoherentes, no compila
static_assert(LIMITE_IZQ < 0 && LIMITE_DER > 0, "LIMITE_IZQ debe ser negativo y LIMITE_DER positivo");
static_assert(SERVO_CENTRO_FISICO + LIMITE_IZQ >= 0 && SERVO_CENTRO_FISICO + LIMITE_DER <= 180, "Fuera del rango 0-180 del servo");

// Último valor firmado enviado al servo (para el monitor serial)
int direccionActual = SERVO_CENTRADO;

// Servo con límite duro: cualquier write() se recorta a [LIMITE_IZQ, LIMITE_DER]
class ServoLimitado : public Servo {
public:
  void write(int angulo) {
    direccionActual = constrain(angulo, LIMITE_IZQ, LIMITE_DER);
    Servo::write(SERVO_CENTRO_FISICO + direccionActual);
  }
};
ServoLimitado direccion;


int contadorEntradaEstadoFinal = 0;

// Contador de líneas
int contadorLineas = 0;
int limiteLineas = 12;

int16_t ax, ay, az, gx, gy, gz;

float anguloZ = 0.0;
float pitch = 0.0;          // Ángulo Pitch (inclinación adelante/atrás)
float roll = 0.0;           // Ángulo Roll (inclinación lateral)

float anguloObjetivoGlobal = 0.0; // angulo recto para el giro


  float distanciaDerecha= 0 ;
    float distanciaIzquierda = 0 ;

const unsigned long TIEMPO_RETROCESO_MAXIMO = 1000;      // 1000Mayor tiempo para el giro más cerrado
const bool HABILITAR_RETROCESO_GIRO_CERRADO = false;   // true = vuelve el retroceso cuando está muy pegado a la pared

const unsigned long TIEMPO_RETROCESO_ALTO = 0;        // 800Tiempo intermdio
const unsigned long TIEMPO_RETROCESO_MEDIO_ALTO = 0;  //600 Menor tiempo


float offsetGz = 0.0;
float offsetGx = 0.0; // Offset del giroscopio en X (Roll)
float offsetGy = 0.0; // Offset del giroscopio en Y (Pitch)

float umbralGiro = 1.0;
float umbralReposo = 0.5;
// Control de tiempo
unsigned long lastYawPrint = 0;
const unsigned long YAW_INTERVAL = 300;

unsigned long lastTime, startTime, timeoutGiro = 2000;


// ===== VARIABLES DE CONTROL DE DIRECCIÓN =====
bool corrigiendo = false;           // Estado de corrección activa
unsigned long tiempoInicioCorreccion = 0;
const unsigned long TIEMPO_MAX_CORRECCION = 4000; // 3 segundos máximo corrigiendo
const unsigned long TIEMPO_AVANCE_REINTENTO = 400; // 400ms de avance para reintentar
const unsigned long TIEMPO_ESTABILIZACION = 200;  // 200ms para estabilizar antes de verificar

float anguloObjetivo = 0.0;         // Ángulo objetivo dinámico
const float TOLERANCIA_ANGULO = 8.0; //4 antes Tolerancia de ±15° para considerar "recto"
const float TOLERANCIA_ANGULO_CORRECCION_LINEA_ANTES_DEL_GIRO = 10.0;//
// ===== ESTADOS DE CORRECCIÓN =====
enum EstadoCorreccion {
  RECTO,                  // Vehículo va recto (0° ± margen)
  CORRIGIENDO_IZQUIERDA,  // Corrigiendo hacia la izquierda
  CORRIGIENDO_DERECHA,    // Corrigiendo hacia la derecha
  ESTABILIZANDO           // Estabilizando después de corrección
};

EstadoCorreccion estadoActualCorreccion = RECTO;
bool correccionCompletada = false;  // Flag para indicar que la corrección terminó

// Variables para control de centrado
bool enGiro = false;
unsigned long tiempoFinGiro = 0;
const unsigned long tiempoEsperaPosgiro = 0;
//const int anguloCorreccionMax = 50;
// anguloCorreccionMax reemplazado por SERVO_CORR_CENTRADO_IZQ / _DER (bloque de servo)
const unsigned long intervaloCentrado = 100;
unsigned long ultimoTiempoCentrado = 0;

// Variables para pausar centrado durante detección
bool pausarCentrado = false;
unsigned long tiempoInicioDeteccion = 0;
const unsigned long tiempoPausaCentrado = 100;

// Variables para detección de líneas
bool lineaNaranjaDetectada = false;
unsigned long tiempoUltimaDeteccion = 0;

// ==================== PAUSA NO BLOQUEANTE PRE-GIRO ====================
bool giroPendiente = false;          // true = hay un giro esperando ejecutarse
bool giroPendienteEsIzquierda = false; // true = azul (izq), false = naranja (der)
unsigned long tiempoInicioPausaPreGiro = 0;
const unsigned long DURACION_PAUSA_PRE_GIRO = 150; // ms, ajustable (100-300 sugerido)

// ==================== MANIOBRA OPTIMIZADA (post-memorización, contadorLineas >= 5) ====================
// A partir de que ya se memorizó la vuelta completa, en vez de hacer el giro brusco de 90°
// en la esquina, se ejecuta una maniobra más rápida: retroceso corto / avance hasta pared +
// cruce directo hacia el nuevo ángulo objetivo, terminando en MANTENIENDO_CARRIL.
bool maniobraOptimizadaPendiente = false;      // true = hay una maniobra optimizada esperando su pausa pre-maniobra (igual que giroPendiente)
bool maniobraOptimizadaEsIzquierda = false;    // sentido de la línea que se acaba de detectar (igual semántica que giroPendienteEsIzquierda)
unsigned long tiempoInicioPausaPreManiobraOptimizada = 0;

// Sub-fases internas de MANIOBRA_OPTIMIZADA (no bloqueante, un solo hilo de ejecución)
enum FaseManiobraOptimizada {
  MO_ENDEREZANDO,        // Enderezar el vehículo (si estaba desviado) antes de iniciar la maniobra
  MO_RETROCESO_CORTO,     // Retroceso corto con servo centrado (caso carril interior)
  MO_RETROCESO_LARGO,     // Retroceso ~2s con servo centrado + pausa (caso carril exterior)
  MO_ESPERANDO_PAUSA_RETROCESO, // Pausa no bloqueante después del retroceso, antes de cruzar
  MO_AVANZANDO_HASTA_TOF,       // Avanza manteniendo el ángulo previo hasta detectar 40cm al frente
  MO_ESPERANDO_PAUSA_TOF,       // Pausa no bloqueante tras alcanzar los 40cm, antes de cruzar
  MO_CRUZANDO,             // Cruce activo hacia el nuevo ángulo objetivo (servo a fondo hacia el lado)
  MO_FINALIZADA            // Maniobra completada, lista para pasar a MANTENIENDO_CARRIL
};
FaseManiobraOptimizada faseManiobraOptimizada = MO_ENDEREZANDO;

// Datos calculados una sola vez al iniciar la maniobra (para no recalcular en cada ciclo)
bool  mo_giroIzquierda = false;        // true = la línea recién detectada fue azul (izquierda)
bool  mo_carrilAnteriorInterno = false; // true = el tramo que se acaba de completar terminó en carril interno
bool  mo_usarPatronRetrocesoAvance = false; // true = "retroceder un poco y cruzar" (según tabla), false = "avanzar hasta 40cm y cruzar"
float mo_anguloPrevio = 0.0;           // Ángulo objetivo que se traía ANTES de detectar la línea (para el patrón de avance-hasta-ToF)
float mo_anguloNuevo = 0.0;            // Ángulo objetivo nuevo (rotado 90° según sentido), al que se cruza
unsigned long mo_tiempoInicioFase = 0; // Marca de tiempo de inicio de la fase actual (para retrocesos/pausas no bloqueantes)
unsigned long mo_tiempoInicioEnderezado = 0; // Marca de tiempo al ENTRAR a MO_ENDEREZANDO, para medir cuánto tardó corrigiendo
unsigned long mo_duracionRetrocesoObjetivo = 0; // Duración de retroceso ya ajustada (restando el tiempo que tardó el enderezado previo)
String mo_comandoSiguiente = "";       // Comando del próximo tramo (para saber si termina en carril1 o carril2 objetivo)
bool mo_esComandoExterior = false;     // true = el comando ANTERIOR (el que se completó) fue de carril exterior -> usa retroceso largo (2s) y +2.5s en tiempos

// Duraciones de la maniobra optimizada (ajustables)
const unsigned long MO_DURACION_PAUSA_PRE_MANIOBRA   = 150;   // ms, misma duración que la pausa pre-giro normal, para asentar antes de arrancar
const unsigned long MO_TIEMPO_RETROCESO_CORTO        = 400;   //300 ms, "retroceder un poco" (carril interior)
const unsigned long MO_TIEMPO_RETROCESO_LARGO        = 1400;//1700 2000;  // ms, "retroceder 2 seg" (carril exterior)
const unsigned long MO_TIEMPO_RETROCESO_E_EXT_A_EXT = 300;   //500 ms, retroceso cuando el anterior es exterior y el siguiente es "E" que entra por exterior
bool mo_retrocesoEExtAExtActivo = false;                     // true = usar retroceso de 500ms y saltarse el avance hasta ToF
const unsigned long MO_TIEMPO_RETROCESO_E_EXT_A_INT = 1000;   // ms, retroceso cuando el anterior es "E" exterior y el siguiente entra por carril interior
bool mo_retrocesoEExtAIntActivo = false;                      // true = usar el retroceso reducido en esta maniobra

const unsigned long MO_PAUSA_TRAS_RETROCESO          = 120;   // ms, pausa no bloqueante después de cualquier retroceso, antes de cruzar
const unsigned long MO_PAUSA_TRAS_TOF                = 120;   // ms, pausa no bloqueante tras alcanzar los 40cm, antes de cruzar

// ===== v103: RETROCESO exterior->interior: lento, manteniendo ángulo y parando por ToF frontal =====
const bool  MO_EXTINT_ACTIVO = true;                  // false = vuelve al retroceso por tiempo de antes
const float MO_EXTINT_DIST_FRONTAL_PARAR_CM = 73.0;   // se detiene cuando el ToF frontal marca MÁS que esto y se cruza
const int   MO_EXTINT_VELOCIDAD_RETROCESO = 90;       // PWM del retroceso (antes 120). Más bajo = más lento
const int   MO_EXTINT_CONFIRMACIONES = 2;             // lecturas seguidas > distancia para parar
const unsigned long MO_EXTINT_TIMEOUT_RETROCESO_MS = 3000; // seguridad: si el ToF nunca confirma, para a este tiempo
const bool  MO_EXTINT_999_CUENTA_LEJOS = true;        // leerToF devuelve 999 si está fuera de rango: true = cuenta como "lejos"
// Corrección de rumbo en reversa (el volante va invertido respecto a avanzar)
const bool  MO_EXTINT_INVERTIR = true;                // si en pruebas se desvía MÁS al retroceder, ponlo en false
const float MO_EXTINT_ZONA_MUERTA_GRADOS = 1.0f;      // dentro de este error deja el volante recto
const float MO_EXTINT_SATURA_GRADOS = 10.0f;          // error con el que usa el máximo giro
const int   MO_EXTINT_SERVO_MAX = 14;                 // máximo giro del volante en este retroceso
bool mo_esTramoExteriorAInterior = false;
int  mo_extIntConfirmaciones = 0;

// ===== v103: ENDEREZADO DEL SERVO EN 2 PASOS =====
// Paso 1: el servo se queda a N grados del centro, del lado hacia donde se cruzaba.
// Paso 2: recién entonces se centra del todo.
const int MO_SERVO_GRADOS_PASO_INTERMEDIO = 10;            // ° de separación del centro en el paso 1
const unsigned long MO_PASO_INTERMEDIO_MS = 100;           // ms que dura cada paso (sube si el servo no alcanza a llegar)
int mo_endPaso = 0;                                        // 0=sin iniciar, 1=en +-10°, 2=centrando, 3=listo
unsigned long mo_endPasoInicio = 0;



// ===== v102: ESQUIVE LATERAL SUAVE durante MO_AVANZANDO_HASTA_TOF =====
const bool  MO_HABILITAR_ESQUIVE_LATERAL_AVANCE = true;  // false = vuelve al comportamiento anterior
const float MO_ESQUIVE_DIST_ACTIVA_CM      = 30.0;  // lateral menor a esto = hay objeto, empieza a alejarse
const float MO_ESQUIVE_KP_GRADOS_POR_CM    = 0.5;   // grados de rumbo por cada cm que está más cerca del límite
const float MO_ESQUIVE_MAX_OFFSET_GRADOS   = 8.0;   // tope del desvío del ángulo objetivo
const float MO_ESQUIVE_TOLERANCIA_GRADOS   = 2.0;   // tolerancia estricta mientras esquiva (la normal es 8° y anularía el desvío)

// ===== v102: EMERGENCIA interior->exterior con Sharp diagonal del MISMO lado del giro =====
const bool  MO_HABILITAR_EMERGENCIA_INT_EXT = true;     // <<< false = desactiva TODA esta función
// v103: true = en el cruce, la emergencia (Sharp) retrocede con el servo RECTO en interior->exterior y exterior->exterior.
//       false = vuelve al comportamiento anterior (servo cruzado).
const bool  MO_EMERG_CRUCE_SERVO_RECTO = true;

const float MO_EMERG_INT_EXT_DIST_CM        = 15.0;     // distancia que dispara la emergencia
const unsigned long MO_EMERG_INT_EXT_RETROCESO_MS = 200; // duración del retroceso
const unsigned long MO_EMERG_INT_EXT_COOLDOWN_MS  = 300; // espera mínima entre emergencias
const int   MO_EMERG_INT_EXT_MAX_REPETICIONES = 3;      // tope por maniobra (evita ciclos infinitos)
bool mo_emergIntExtActiva = false;
unsigned long mo_emergIntExtInicio = 0;
unsigned long mo_emergIntExtUltima = 0;
int  mo_emergIntExtContador = 0;

const float MO_TOLERANCIA_ANGULO_ENDEREZADO = 5.0; // v103: antes 4.0. Si el IMU no logra entrar en rango (timeouts), súbelo a 3.0
const int   MO_VELOCIDAD_ENDEREZADO = 80;          // v103: PWM del retroceso SOLO durante el enderezado (antes 120). Si el carro no se mueve, sube a 100

// v103: corrección de rumbo mientras RETROCEDE (el servo va invertido respecto a ir hacia adelante)
const bool  MO_RETRO_INVERTIR_CORRECCION = true;   // si en pruebas se desvía MÁS, ponlo en false
const float MO_RETRO_ZONA_MUERTA_GRADOS = 1.5f;    // dentro de este error deja el servo recto
const float MO_RETRO_SATURA_GRADOS      = 10.0f;   // error a partir del cual usa el máximo giro
const int   MO_RETRO_SERVO_MAX          = 14;      // máximo giro del servo (en unidades de servo) durante el retroceso

const float          MO_DISTANCIA_FRONTAL_OBJETIVO_CM = 45.0; //40 cm, distancia frontal a la que se detiene el patrón "avanzar y cruzar"

const float          MO_DISTANCIA_FRONTAL_OBJETIVO_E_EXTERIOR_CM = 70.0; // cm, para comandos con "E" que empiezan en carril exterior
float                mo_distanciaFrontalObjetivoActual = MO_DISTANCIA_FRONTAL_OBJETIVO_CM; // distancia ToF vigente en esta maniobra

const unsigned long MO_EXTRA_TIEMPO_CARRIL_EXTERIOR  = 100.0;//1500;  // ms, +2.5s para MANTENIENDO_CARRIL en tramos de carril exterior
const unsigned long MO_EXTRA_TIEMPO_CARRIL_EXTERIOR_DETECCION  = 0.0;//1500;  // ms, +2.5s para espera de detección de línea en tramos de carril exterior

bool mo_extenderEsperaDeteccionActivo = false;     // true mientras el +2.5s de espera de detección de línea debe seguir aplicado (tramo de carril exterior en curso)

const unsigned long MO_TIEMPO_MINIMO_MANTENIENDO_INTERNO_A_INTERNO = 1000; // ms, reducido para tramos interno->interno, donde se llega más rápido a la línea y el piso base la deja pasar sin contar
const unsigned long MO_TIEMPO_ESPERA_DETECCION_INTERNO_A_INTERNO   = 2000; // ms, reducido junto con el anterior (son un OR: si uno queda alto, el otro no es suficiente). Ajusta este valor según lo que veas en pista.

bool mo_esTramoInteriorAInterior = false; // true durante toda la maniobra si el tramo es interior->interior: usado para velocidad reducida y vigilancia ToF frontal en MO_CRUZANDO

// ===== v96: confirmación de "objeto pasado" con sensor lateral contrario =====
// ===== v96: confirmación de "objeto pasado" con sensor lateral contrario =====
bool mo_confirmarObjetoActivo = false;   // true = el cambio de carril del comando doble espera a detectar el objeto
// ===== v101: confirmación de entrada al carril interior (sensor opuesto 60-80cm) =====
bool ent_activa = false;                       // true = fase "antes de entrar al carril" en curso
unsigned long ent_tInicio = 0;                 // inicio de la fase (para el timeout)
int  ent_contEntro = 0;                        // lecturas válidas consecutivas del sensor del carril objetivo
float ent_ultimaDistOpuesta = 999.0;           // última lectura del sensor opuesto (debug)
unsigned long ent_ultimoPrint = 0;

const float ENT_DIST_MIN_OPUESTA_CM = 85.0;    // 60 rango deseado del sensor OPUESTO
const float ENT_DIST_MAX_OPUESTA_CM = 100.0; //80
const float ENT_DIST_ENTRO_CM = 40.0;          // sensor del carril objetivo <= esto = "ya entró" (ajústalo)
const int   ENT_MUESTRAS_ENTRO = 2;            // lecturas seguidas para confirmar la entrada
const float ENT_KP_GRADOS_POR_CM = 0.6;        // grados de rumbo por cada cm fuera del rango
const float ENT_MAX_OFFSET_GRADOS = 12.0;      // tope de corrección de rumbo
const unsigned long ENT_TIMEOUT_MS = 4000;     // seguridad: si no entra en este tiempo, se desactiva
// Lectura compartida del sensor OPUESTO (la usan la detección de objeto y la entrada al carril): 1 sola lectura por ciclo
bool  ent_lecturaOpuestaValida = false;
float ent_lecturaOpuesta = 999.0;

int  mo_objFase = 0;                     // 0=esperando zona libre, 1=esperando objeto, 2=objeto al lado, 3=pasó (avance extra)
int  mo_objCont = 0;                     // contador de lecturas consecutivas de la fase actual
unsigned long mo_objTiempoPaso = 0;      // momento en que se confirmó que pasó el objeto

bool mo_objRetrocediendo = false;        // true = retroceso (no bloqueante) por objeto no detectado
bool mo_objRetrocesoTerminado = false;   // true = el retroceso ya terminó, falta liberar el cambio de carril
unsigned long mo_objInicioRetroceso = 0;

// ── v96: parada + retroceso tras confirmar "objeto pasado" ──
const unsigned long MO_OBJ_PAUSA_ANTES_RETROCESO_MS = 100; // motor DETENIDO este tiempo antes de retroceder (0 = sin pausa)
const unsigned long MO_OBJ_RETROCESO_PASO_MS = 0;        // retroceso tras pasar el objeto (0 = solo detenerse, sin retroceso)
unsigned long mo_objPausaMs = 0;          // pausa (motor detenido) de la secuencia actual
unsigned long mo_objDuracionTotalMs = 0;  // duración total de la secuencia actual (pausa + retroceso)

const float MO_OBJ_DIST_OBJETO_CM = 45.0;  //25 menos que esto = hay un objeto al lado
const float MO_OBJ_HISTERESIS_CM  = 3.0;   // para decir "ya pasó" hay que superar DIST_OBJETO + histéresis
const int   MO_OBJ_MUESTRAS_LIBRE  = 1;    // lecturas >= 25cm seguidas ANTES de armar (evita que una pared cercana cuente como objeto)
const int   MO_OBJ_MUESTRAS_OBJETO = 1;    // lecturas < 25cm seguidas para confirmar el objeto
const int   MO_OBJ_MUESTRAS_PASO   = 2;    // lecturas largas seguidas para confirmar que ya pasó
const unsigned long MO_OBJ_AVANCE_EXTRA_MS = 0; // avance extra tras confirmar el paso (0 = cambiar de carril de inmediato)
const unsigned long MO_OBJ_TIMEOUT_DETECCION_MS = 2500; // sin detectar el objeto en este tiempo -> retroceso + cambio de carril
const unsigned long MO_OBJ_RETROCESO_TIMEOUT_MS = 1800; // duración del retroceso si no detectó el objeto
const float MO_OBJ_DIST_MAX_CM = 150.0;    // tope: lecturas inválidas/999 se toman como "lejos"

const unsigned long MO_OBJ_PAUSA_INICIAL_MS = 300; // motor DETENIDO al entrar a MANTENIENDO_CARRIL (ya enderezado), antes de reiniciar el avance
const bool MO_OBJ_AVANZAR_LENTO = true;            // true = reinicia el avance con velocidadLenta mientras busca el objeto
// ===== v98: detección de objeto también en las PRIMERAS vueltas (comandos que llegan por ESPERANDO_COMANDO_CARRIL) =====
// Regla: si el tramo ANTERIOR terminó en carril INTERIOR -> se queda por tiempo (como antes).
//        Si terminó en EXTERIOR -> el cambio de carril del comando doble espera a detectar el objeto.
// Cuando NO se conoce el tramo anterior (no hay comando memorizado previo):
const bool MO_OBJ_ACTIVAR_SIN_ANTERIOR  = true;   // línea 1 (el tramo previo fue el inicio). false = se queda por tiempo
const bool MO_OBJ_ACTIVAR_ESQUIVE_INICIAL = false; // comando recibido ANTES de la primera línea (contadorLineas < 1): individual, se queda por tiempo

// Tiempo extra al timeout de detección cuando el comando NO hizo GIRANDO_HACIA_CARRIL (omitioGiro = true).
// Como ahora avanza más lento, necesita más tiempo para llegar a ver/pasar el objeto. Ajusta a tu gusto.
const unsigned long MO_OBJ_TIMEOUT_EXTRA_OMITIDO_MS = 1500;
unsigned long mo_objOffsetTimeoutMs = 0;                  // tiempo acumulado en emergencias (se suma al timeout)
const bool MO_OBJ_REINICIAR_FASE_EN_EMERGENCIA = false;    // true = tras una emergencia vuelve a esperar zona libre (descarta lecturas del retroceso)
// --- debug de la detección de objeto ---
const unsigned long MO_OBJ_DEBUG_INTERVALO_MS = 0; // imprime una línea [OBJ] cada X ms (0 = cada muestra)
unsigned long mo_objUltimoDebug = 0;
unsigned long mo_objUltimaMuestra = 0;
int   mo_objNumMuestras = 0;
float mo_objMinVisto = 999.0;
float mo_objMaxVisto = 0.0;

bool mo_siguienteEntraPorExterior = false;       // true = el comando siguiente COMIENZA en carril exterior (ext->ext e int->ext)
bool mo_retrocesoPostCrucePendiente = false;     // true = int->ext con comando DOBLE: retroceder tras terminar el cruce
const unsigned long MO_TIEMPO_RETROCESO_POST_CRUCE = 100; // ms, retroceso tras el cruce, siguiendo el ángulo objetivo
// --- Cambios v96 ---

const float MO_DISTANCIA_EMERGENCIA_TOF_FRONTAL = 8.0; // cm, umbral del ToF frontal para activar la emergencia durante MO_CRUZANDO (solo interior->interior)

// Tras una maniobra optimizada el carro puede quedar lejos de la pared que debe seguir.
// Mientras este modo esté activo, la corrección por distancia se limita para que no
// supere al control de ángulo y el carro no se desvíe hacia la pared.
bool mo_aproximacionLimitadaActiva = false;
const float MO_CORR_DISTANCIA_MAX_APROXIMACION = 10.0;   // máx. de servo por distancia (normal = correccionMaxima = 40)
const float MO_ERROR_DISTANCIA_FIN_APROXIMACION = 15.0;  // cm: al bajar de esto, vuelve al control normal

// ==================== EMERGENCIA POR SHARP DURANTE EL CRUCE (MO_CRUZANDO) ====================
// Igual en espíritu al retroceso de emergencia que ya existe en GIRANDO_HACIA_CARRIL/
// ENDEREZANDO_CARRIL, pero no bloqueante e integrado a la máquina de fases de la maniobra
// optimizada. Vigila el Sharp del lado CONTRARIO al que se está cruzando (para no llevarse
// la pared/objeto exterior de la curva), y si detecta algo muy cerca: detiene, centra el
// servo, retrocede un poco, y luego retoma el cruce -- repitiendo cuantas veces haga falta.
enum FaseEmergenciaCruceMO {
  MO_EMERG_INACTIVA,          // sin emergencia en curso, cruce normal
  MO_EMERG_RETROCEDIENDO,     // retrocediendo tras haber centrado el servo
  MO_EMERG_PAUSA_ANTES_REANUDAR // pausa breve antes de volver a girar el servo y avanzar
};
FaseEmergenciaCruceMO mo_faseEmergenciaCruce = MO_EMERG_INACTIVA;
unsigned long mo_tiempoInicioEmergenciaCruce = 0;

// Dos casos distintos de forzado (antes combinados en una sola bandera). Se separan porque
// el retroceso forzado en interior->interior usa el servo hacia el lado CONTRARIO al cruce
// (mejor acomodo en espacio muy estrecho), mientras que exterior->interior sigue igual que
// cualquier otra emergencia (servo centrado).
bool mo_emergenciaForzadaPendienteInteriorInterior = false; // true = forzar retroceso con servo CONTRARIO, caso interior->interior
bool mo_emergenciaForzadaPendienteExteriorInterior = false; // true = forzar retroceso con servo centrado (igual que siempre), caso exterior->interior
bool mo_emergenciaForzadaYaEjecutada = false; // evita repetir el forzado más de una vez por maniobra
const bool MO_HABILITAR_EMERGENCIAS_FORZADAS = false;   // true = vuelven las emergencias arbitrarias por tiempo durante el cruce

bool mo_emergenciaActivaEsForzadaInteriorInterior = false; // true mientras la emergencia EN CURSO es la forzada interior->interior (decide el servo del retroceso)
// --- v96: servo CRUZADO (contrario al cruce) durante el retroceso de emergencia ---
bool mo_esTramoExteriorAExterior = false;        // true durante toda la maniobra si el tramo es exterior->exterior
bool mo_emergenciaServoContrarioActiva = false;  // true mientras la emergencia EN CURSO retrocede con el servo contrario al cruce

// --- v97: interior->exterior: SOLO la 1ra emergencia por Sharp retrocede con servo CRUZADO; las demás centrado ---
bool mo_esTramoInteriorAExterior = false;          // true durante toda la maniobra si el tramo es interior->exterior
bool mo_emergenciaCruzadaIntExtYaUsada = false;    // true una vez que ya se usó el servo cruzado en esta maniobra
// --- v98: exterior->exterior con comando siguiente "E": 1 emergencia ARBITRARIA con servo cruzado; las de Sharp van rectas ---
bool mo_esTramoExtExtConE = false;
const unsigned long MO_TIEMPO_FORZAR_EMERGENCIA_CRUCE_EXT_EXT = 700; // ms desde el inicio del cruce (ajústalo a tu gusto)

const unsigned long MO_TIEMPO_FORZAR_EMERGENCIA_CRUCE = 700; // ms, tiempo desde el inicio de MO_CRUZANDO para forzar la emergencia una vez -- SOLO caso exterior->interior
const unsigned long MO_TIEMPO_FORZAR_EMERGENCIA_CRUCE_INT_INT = 800; // ms, más corto, SOLO caso interior->interior (servo contrario)

const unsigned long MO_DURACION_RETROCESO_EMERGENCIA = 600; // ms, usado por emergencia por SHARP y forzado exterior->interior
const unsigned long MO_DURACION_RETROCESO_EMERGENCIA_INT_INT = 500; // ms, independiente, SOLO para el forzado interior->interior (servo contrario)
// ── NUEVO: solo caso exterior->interior (emergencia forzada/arbitraria) ──
// Retroceso arbitrario SOLO para giro a la IZQUIERDA (derecha sigue usando MO_DURACION_RETROCESO_EMERGENCIA = 600)
const unsigned long MO_DURACION_RETROCESO_EMERGENCIA_EXT_INT_IZQ = 500; // ms  <-- AJUSTA AQUÍ el tiempo
// Umbral (°) al que se da por terminado el cruce. Más grande = el carro se queda MÁS lejos del ángulo objetivo.
// TOLERANCIA_ANGULO normal es 8.0, así que usa un valor mayor a 8.
const float MO_UMBRAL_FIN_CRUCE_EXT_INT_IZQ = 21.0; // °  <-- AJUSTA AQUÍ (giro izquierda)
const float MO_UMBRAL_FIN_CRUCE_EXT_INT_DER = 16.0; // °  <-- AJUSTA AQUÍ (giro derecha). Pon 8.0 si quieres dejarlo igual que antes

const unsigned long MO_PAUSA_ANTES_DE_RETROCEDER_EMERGENCIA = 150; // ms, para centrar el servo antes de retroceder
const unsigned long MO_PAUSA_ANTES_DE_REANUDAR_EMERGENCIA = 250;   // ms, para que el servo llegue a su posición de cruce antes de avanzar
const unsigned long MO_COOLDOWN_EMERGENCIA_CRUCE = 300; // ms, evita reactivar la emergencia inmediatamente tras resolverla
unsigned long mo_ultimoRetrocesoEmergenciaCruce = 0;

bool esperandoCambioCarril = false; // v104: true mientras EVALUANDO_POSICION espera recto/lateral libre -> avance lento

// ===== v104: interior->interior: 1ra emergencia retroceso LARGO, las demás CORTO. Se resetea en cada maniobra nueva =====
const unsigned long MO_RETROCESO_EMERG_INT_INT_LARGO_MS = 400; // 1ra emergencia de la maniobra  <-- AJUSTA AQUÍ
const unsigned long MO_RETROCESO_EMERG_INT_INT_CORTO_MS = 200; // 2da, 3ra, ... emergencia         <-- AJUSTA AQUÍ
int mo_contadorEmergenciasIntInt = 0;
unsigned long mo_duracionEmergIntIntActual = 0; // 0 = sin override (usa la lógica de siempre)


//tiempo de espera para deteccion de linea correcta, tambien se activa cuando inicia girando a carril.
const unsigned long TIEMPO_ESPERA_DETECCION_BASE = 4500;//4500   4000 5000
const unsigned long TIEMPO_MINIMO_MANTENIENDO_PARA_DETECTAR_BASE = 1500; // 2 segundos

// Estas dos ahora son variables (no const): la maniobra optimizada les suma
// MO_EXTRA_TIEMPO_CARRIL_EXTERIOR cuando el tramo que se acaba de armar en MANTENIENDO_CARRIL
// viene de un comando de carril exterior, para no confundir la línea con la que se cruzó al girar.
unsigned long tiempoEsperaDeteccion = TIEMPO_ESPERA_DETECCION_BASE;
unsigned long TIEMPO_MINIMO_MANTENIENDO_PARA_DETECTAR = TIEMPO_MINIMO_MANTENIENDO_PARA_DETECTAR_BASE;

// Flag para forzar tiempo de giro a 500ms al evadir obstáculo frontal
//Flag para controlar el inicio del cooldown tras un comando de carril
//bool iniciarCooldownCarril = false;

// Variables para direccionalidad
enum SentidoGiro {
  INDEFINIDO,
  IZQUIERDA,  // Detectó azul primero
  DERECHA     // Detectó naranja primero
};

SentidoGiro sentidoVehiculo = INDEFINIDO;
bool primeraDeteccion = true;

// ==================== VARIABLES PID PARA GIROS ====================

// Variables para controlador PID de giros
struct PIDController {
  float kp;           // Ganancia proporcional
  float ki;           // Ganancia integral
  float kd;           // Ganancia derivativa
  float error_anterior;
  float integral;
  float integral_max; // Límite para evitar windup
  unsigned long tiempo_anterior;
};

// Configuración del PID para giros
PIDController pidGiro = {
  .kp = 2.5,          // Ganancia proporcional - ajusta respuesta principal
  .ki = 0.8,          // Ganancia integral - elimina error en estado estacionario
  .kd = 0.4,          // Ganancia derivativa - reduce oscilaciones
  .error_anterior = 0,
  .integral = 0,
  .integral_max = 30, // Límite para integral windup
  .tiempo_anterior = 0
};

// Variables adicionales para control PID
float anguloObjetivoActual = 0;
bool giroEnProceso = false;
const float TOLERANCIA_PID = 5.0;      // Tolerancia más estricta con PID

// ==================== VARIABLES PID PARA CENTRADO ====================

// Variables para controlador PID de centrado
struct PIDCentrado {
  float kp;           // Ganancia proporcional
  float ki;           // Ganancia integral  
  float kd;           // Ganancia derivativa
  float error_anterior;
  float integral;
  float integral_max; // Límite para evitar windup
  unsigned long tiempo_anterior;
  bool inicializado;  // Para manejar primera ejecución
};

// Configuración del PID para centrado
PIDCentrado pidCentrado = {
  .kp = 2.0,          // Ganancia proporcional - respuesta principal al error
  .ki = 0.0,          // Ganancia integral - elimina error residual
  .kd = 1.2,          // Ganancia derivativa - reduce oscilaciones
  .error_anterior = 0,
  .integral = 0,
  .integral_max = 20, // Límite para integral windup
  .tiempo_anterior = 0,
  .inicializado = false
};

// Variables adicionales para control PID del centrado
const float TOLERANCIA_CENTRADO_PID = 5.0;    // Tolerancia más estricta
const float ERROR_MAXIMO_CENTRADO = 30.0;     // Error máximo esperado
const float FACTOR_SUAVIZADO = 0.7;           // Para suavizar cambios bruscos

// Buffer para comandos serie
String comandoRecibido = "";
  bool comandoListo = false;

// ==================== SISTEMA DE MEMORIZACIÓN DE COMANDOS ====================
const int TOTAL_TRAMOS_PISTA = 4;      // La pista tiene 4 tramos/comandos que se repiten
String comandosMemorizados[TOTAL_TRAMOS_PISTA] = {" ", " ", " ", " "};
int cantidadComandosMemorizados = 0;   // Cuántos comandos distintos llevamos guardados (0..4)
bool memorizacionCompleta = false;     // true cuando ya tenemos los 4 comandos guardados


//-------variables de evasion de objetos--------------

// Estados del sistema integrado
enum EstadoSistema {
  ESTADO_NORMAL,
  ESTADO_DETENCION_FINAL,
  ESPERANDO_COMANDO_CARRIL,
  EVALUANDO_POSICION,
  GIRANDO_HACIA_CARRIL,
  ENDEREZANDO_CARRIL,
  MANTENIENDO_CARRIL,
  ESTADO_CORRECCION_LATERAL,
  MANIOBRA_OPTIMIZADA        // NUEVO: maniobra rápida de esquina a partir de la vuelta memorizada (contadorLineas >= 5)
};

// Variables para recepción serial optimizada (del código de esquive)
const int BUFFER_SIZE = 32;
char buffer[BUFFER_SIZE];
int bufferIndex = 0;

// Variables para parsing ultra rápido
char colorDetectado = 'N';
int distanciaObjeto = 0;
char orientacionObjeto = 'C';
 
// Variables de estado
EstadoSistema estadoActual = ESTADO_NORMAL;
char colorObjetivo = 'N';
unsigned long tiempoEsquive = 0;
int etapaEsquive = 0;
char lastOrientation = 'X';

// ============================================================
// CORRECCIÓN GLOBAL DE CALIBRACIÓN DE SENSORES ToF
// ============================================================
// Si un sensor empieza a desviarse (ej: marca +5cm de más de lo real),
// ajusta SU offset aquí. Se aplica automáticamente en leerToF()
// a TODO el código que use ese sensor (umbrales, carriles, acomodo lateral, etc).
//
// Cómo calibrar: coloca el robot a una distancia real conocida (ej: 20cm)
// de una pared, lee lo que reporta el sensor, y resta:
//   offset = lectura_sensor - distancia_real
// Si el sensor marca 25cm cuando la distancia real es 20cm -> offset = +5.0
//
// El offset se RESTA de la lectura cruda, así que:
//   offset positivo -> el sensor "acercará" sus lecturas (corrige sobre-medición)
//   offset negativo -> el sensor "alejará" sus lecturas (corrige sub-medición)
const float OFFSET_TOF_FRONTAL  = 0.0;  // sensor 'F'
const float OFFSET_TOF_TRASERO  = 0.0;  // sensor 'T'
const float OFFSET_TOF_IZQ      = 0.0;  //5 sensor 'I'
const float OFFSET_TOF_DER      = 0.0;  //5 sensor 'D'  <-- el que te afectó en C07E

const float OFFSET_TOF_DIAG_IZQ = 0.0;  // sensor 'X' (VL53L0X diagonal izquierdo)
const float OFFSET_TOF_DIAG_DER = 0.0;  // sensor 'Y' (VL53L0X diagonal derecho)

const int ANGULO_CORRECCION_SHARP_IZQ = SERVO_LEFT;
const int ANGULO_CORRECCION_SHARP_DER = SERVO_RIGHT;

// Variables para recordar último color detectado
char ultimoColorDetectado = 'N';           // Último color válido recibido
unsigned long tiempoUltimoColor = 0;       // Timestamp del último color detectado
const unsigned long TIEMPO_MEMORIA_COLOR = 0; // 4 segundos de memoria

// Variables para función de emergencia
bool emergenciaActivada = false;
unsigned long tiempoUltimaEmergencia = 0;
const unsigned long TIEMPO_ESPERA_EMERGENCIA = 2000; // Evitar activaciones múltiples
char colorEmergencia = 'N';

// Variables para interrupción de cruce
volatile bool interrupcionCruce = false;
volatile char comandoInterrupcion = 'N';
bool cruceEnProceso = false;
//------------------- Fin de variables de esquive--------------
 
  const int MAX_INVALIDAS_REFERENCIA_LATERAL = 3; // lecturas inválidas seguidas antes de dejar de confiar en el valor viejo del filtro

// ==================== CONFIGURACIÓN Y VARIABLES SHARP ====================

// Pines para sensores Sharp diagonales
const int SHARP_DIAGONAL_IZQ_PIN = 35;  // Pin analógico para sensor Sharp izquierdo
const int SHARP_DIAGONAL_DER_PIN = 34;  // Pin analógico para sensor Sharp derecho

const float DISTANCIA_EMERGENCIA_SHARP_PARED = 16.0; // Distancia en cm para activar el retroceso de emergencia
const unsigned long TIEMPO_RETROCESO_SHARP_PARED = 700;  //800 Tiempo de retroceso en ms (ajustable)

 //CONSTANTES PARA CORRECCIÓN EN PARED *****
const float DISTANCIA_MINIMA_SHARP_PARED = 12.0; // Distancia en cm para activar la corrección
//const int ANGULO_CORRECCION_SHARP_IZQ = 180;     // Ángulo para girar a la izquierda (alejarse de la pared derecha)
//const int ANGULO_CORRECCION_SHARP_DER = 0;      // Ángulo para girar a la derecha (alejarse de la pared izquierda)
unsigned long tiempoUltimaEmergenciaSharpDer = 0;
unsigned long tiempoUltimaEmergenciaSharpIzq = 0;
const unsigned long COOLDOWN_EMERGENCIA_SHARP = 2000; // 3 segundos para considerar una detección como "reciente"
const unsigned long DURACION_AVANCE_CORRECTIVO = 800; // 400ms de avance

// Variables para esquive diagonal por sensores Sharp
bool esquiveDiagonalActivo = false;
unsigned long tiempoInicioEsquiveDiagonal = 0;
unsigned long tiempoUltimaDeteccionSharp = 0;
char direccionEsquiveDiagonal = 'N'; // 'I' = izquierda, 'D' = derecha
int etapaEsquiveDiagonal = 0;

// Configuración para sensores Sharp diagonales
const float DISTANCIA_SHARP_EMERGENCIA = 15.0;     // 10cm de detección
const unsigned long TIEMPO_CRUCE_DIAGONAL = 500;  // Tiempo de cruce diagonal
const unsigned long TIEMPO_ESPERA_SHARP = 100;     // Evitar detecciones múltiples

// Sistema de prioridades para gestión de emergencias
enum PrioridadEmergencia {
  PRIORIDAD_NORMAL,
  PRIORIDAD_RETROCESO,  // Máxima prioridad
  PRIORIDAD_ESQUIVE     // Prioridad normal
};
PrioridadEmergencia prioridadActual = PRIORIDAD_NORMAL;

// Variables mejoradas para retroceso de emergencia
const int DISTANCIA_RETROCESO_EMERGENCIA = 10;    // Distancia mínima para activar retroceso (cm)
const unsigned long TIEMPO_RETROCESO = 2000;     // Tiempo de retroceso en ms
const unsigned long TIMEOUT_RETROCESO = 2500;    // Timeout de seguridad para retroceso
bool retrocesoPorEmergencia = false;    

// --- Confirmación anti-spike para TOF frontal en MANTENIENDO_CARRIL ---
unsigned long tiempoInicioDeteccionFrontal = 0; // Momento en que se detectó el primer obstáculo
bool deteccionFrontalActiva = false;             // Flag: ¿hay una detección en curso?
const unsigned long TIEMPO_CONFIRMACION_FRONTAL = 80; // ms continuos bajo el umbral para confirmar (ajustable)

// Variables de control temporal
unsigned long tiempoInicioRetroceso = 0;          
unsigned long tiempoUltimaVerificacionRetroceso = 0; 
const unsigned long INTERVALO_VERIFICACION_RETROCESO = 100; // Verificación cada 100ms
const unsigned long TIEMPO_ESPERA_POST_RETROCESO = 100;     // Tiempo de espera después del retroceso
bool motorRetrocesoConfigurado = false;  // controlar configuración de motores

bool retrocesoOrientadoActivo = false;
bool enFaseContragiro = false; // true cuando está ejecutando el contragiro de enderezado
bool omitioGiro = false; // true cuando saltó directo a MANTENIENDO sin hacer giro
unsigned long tiempoInicioRetrocesoOrientado = 0;
const unsigned long TIEMPO_RETROCESO_ORIENTADO = 700; // 1 segundo (modificable)
int direccionRetroceso = SERVO_CENTRADO;

// ==================== ESTRUCTURA DE CONTROL LATERAL ====================
struct ControlDistanciaLateral {
  enum Estado {
    IDLE,
    MOVIENDO_DERECHA,
    MOVIENDO_IZQUIERDA,
    CORRIGIENDO_ANGULO_DERECHA,
    CORRIGIENDO_ANGULO_IZQUIERDA,
    RETROCEDIENDO_ORIENTADO,     
    CORRIGIENDO_POST_RETROCESO,   
    PAUSA_CENTRADO_PRE_RETROCESO,
    RETROCEDIENDO_FINAL
    };     
  Estado estadoActual = IDLE;
  unsigned long tiempoInicioEstado = 0;

  
  float anguloObjetivo = 0.0;
  bool funcionActiva = false; 
  unsigned long ultimaVerificacion = 0;
  bool esCorreccionLarga = false; //  Flag para saber qué maniobra ejecutar
  char direccionManiobra = 'N';   // 'D' o 'I' para recordar la dirección

  // Parámetros de control distancia acomodo lateral
  //variables modificables dependiendo de la condicion que se le asignen pero por ahora estan por defecto.
  float UMBRAL_MIN = 35.0;//38//35   // Ya no es const: se puede modificar según el tramo/comando
  float UMBRAL_MAX = 45.0; //50

  // Valores originales/por defecto (para poder restaurar cuando no aplique la condición especial)
  const float UMBRAL_MIN_DEFAULT_IZQUIERDA = 33.0;//35
  const float UMBRAL_MAX_DEFAULT_IZQUIERDA = 45.0;//55

    // Valores por defecto específicos para sentido DERECHA (ver aplicarUmbralesSegunComando)
  const float UMBRAL_MIN_DEFAULT_DERECHA = 33.0;
  const float UMBRAL_MAX_DEFAULT_DERECHA = 45.0;

  // Valores alternativos PARA COMANDOS DE ESTACIONAMIENTO DONDE SE AVANCE ALEJADO DE LA PARED SIN CRUZAR." (ver aplicarUmbralesSegunComando)
  const float UMBRAL_MIN_ALT = 10.0;//35
  const float UMBRAL_MAX_ALT = 45.0;//50.0; //55

  unsigned long tiempoCruceActual = 1000;
  
  const float DISTANCIA_TRASERA_OBJETIVO = 14.0;//10.0; // cm
  const float TOLERANCIA_ANGULAR = 3.0; 
  const unsigned long INTERVALO_VERIFICACION = 1000;
};

ControlDistanciaLateral controlLateral_PostGiro; // Usamos un nuevo nombre para no confundir con el existente

// Variables para control de retroceso post-giro
unsigned long tiempoInicioEvaluacion = 0;

// ============== CONSTANTES DE ÁNGULOS ==============

// Ángulos de giro modificables para cada sentido
float giroObjetivoAzul = 70.0;    // Giro a la izquierda para línea azul
float giroObjetivoNaranja = 50.0; // Giro a la derecha para línea naranja (modificable)

const float GIRO_MAXIMO = 70.0;      // Giro máximo - obstáculo muy cerca
/*const float GIRO_ALTO = 60.0;        // Giro alto - obstáculo cerca
const float GIRO_MEDIO_ALTO = 50.0;  // Giro medio-alto
const float GIRO_MEDIO = 50.0;       // Giro medio
const float GIRO_MEDIO_BAJO = 50.0;  // Giro medio-bajo
const float GIRO_BAJO = 50.0;        // Giro bajo - obstáculo lejano*/
const float GIRO_MINIMO = 60.0;//30.0 Giro mínimo - obstáculo muy lejano

// Rango de interpolación específico para giro a la DERECHA (línea naranja).
// GIRO_MAXIMO / GIRO_MINIMO de arriba siguen aplicando para la IZQUIERDA (línea azul).
const float GIRO_MAXIMO_DERECHA = 50.0; // Giro máximo - obstáculo muy cerca (sentido derecha)
const float GIRO_MINIMO_DERECHA = 40.0; // Giro mínimo - obstáculo muy lejano (sentido derecha)

float anguloObjetivoGiroGlobal = 0;

// Variable para distancia mínima para terminar el retroceso post giro
float DISTANCIA_MINIMA_RETROCESO = 10.0; // 15cm
const unsigned long TIMEOUT_RETROCESO_MAXIMO = 4000;    // Timeout de seguridad de retroceso orientado
// Variable global para medir tiempo de retroceso durante corrección
unsigned long tiempoTotalRetroceso = 0;
bool huboRetrocesoEnCorreccion = false;

// Variables para el giro de apertura
bool giroAperturaActivo = false;
bool giroAperturaCompletado = false;
unsigned long tiempoInicioApertura = 0;
const unsigned long DURACION_APERTURA_MAXIMA = 550; //450 Pegado a la pared: necesita más recorrido para separarse
const unsigned long DURACION_APERTURA_ALTA   = 450;//550
const unsigned long DURACION_APERTURA_MEDIA  = 0;
const unsigned long DURACION_APERTURA_BAJA   = 0;
const unsigned long DURACION_APERTURA_MINIMA = 0;  // Ya hay espacio de sobra: apertura mínima
bool esGiroApertura = false;
unsigned long duracionAperturaActual = DURACION_APERTURA_MINIMA; // Variable para almacenar la duración del giro actualbool esGiroApertura = false;
bool direccionGiroAperturaIzquierda = false;
bool ultimoGiroFueApertura = false; // Congela si el giro que originó el retroceso actual fue de apertura
const bool HABILITAR_AVANCE_ESPECIAL_IZQ = false;   // true = vuelve el avance de 600 ms (caso especial apertura + IZQUIERDA)

bool sistemaActivo = false;  // Estado del sistema (activo/inactivo)

// Variable para controlar giro antes de estacionamiento
bool giroParaEstacionamiento = false;

bool esUltimoTramoFinal = false;   // Se activa al reproducir el comando post-línea-12; hace que el próximo MANTENIENDO_CARRIL avance 2s y se detenga para siempre.
bool detencionPermanenteActiva = false; // Una vez true, ignora color y cualquier sensor: el robot ya no vuelve a moverse ni a leer líneas.
unsigned long tiempoInicioUltimoAvance = 0; // Marca el inicio del avance final de 2s.

// tiempo de finalizacion del codigo para avance final
unsigned long tiempoInicio = millis();
//unsigned long duracionCiclo = 3000; // 5 segundos en milisegundos

unsigned long tiempoInicioDetencionFinal = 0;


// Variables globales para el control de arranque del motor
unsigned long tiempoInicioSistema = 0;
bool motorArrancado = false;
const unsigned long DELAY_ARRANQUE_MOTOR = 1000; // 1 segundo en milisegundos

static unsigned long tiempoInicioAvance = 0;

// Variable para pausa de prueba (configurable)
static unsigned long tiempoInicioPausa = 0;

// Configuración para control angular prioritario
#define UMBRAL_ANGULO_MAX 15.0      // Umbral máximo de desviación angular (±20°)
#define TOLERANCIA_ANGULO_FINO 2.0  // Tolerancia para considerar ángulo correcto
#define FACTOR_CORRECCION_ANGULAR 0.8  // Factor fuerte para corrección angular
#define FACTOR_CORRECCION_DISTANCIA 0.3  // Factor menor para distancia cuando hay error angular


// ==================== CONFIGURACIÓN DEL SISTEMA DE CARRILES ====================
struct SistemaCarriles {
  // Definición de carriles
  float distanciaCarril1;
  float distanciaCarril2;  
  float toleranciaCarril;
  float umbralCambio;
  
  // Parámetros de control
  float velocidadGiro;
  unsigned long tiempoGiro;
  unsigned long tiempoGiroCarril1;
  unsigned long tiempoGiroCarril2;
  float toleranciaAngulo;
  
  // Variables de control
  int carrilObjetivo;
  char ladoControl;
  unsigned long tiempoInicioEstado;
  float direccionGiro;
  bool necesitaCambio;
  bool esComandoC21;
  bool esComandoC14; 
  bool esComandoC15;
  bool esComandoC20;
  bool esComandoC26; 
  bool esComandoC33;
  bool esComandoC27;
  bool esComandoC32;

  bool esComandoC36;

  //individuales
  bool esComandoC02;
  bool esComandoC08;

  bool esComandoC07;
  bool esComandoC01;
    bool esComandoC01ModoDistancia; // C01 con sentido IZQUIERDA: tiempo de giro variable por ToF derecho
    bool esComandoC07ModoDistancia;
    bool esComandoC02ModoDistancia;
    bool esComandoC08ModoDistancia;
  
};

SistemaCarriles carriles = {
  .distanciaCarril1 = 20.0,
  .distanciaCarril2 = 20.0,
  .toleranciaCarril = 3.0,
  .umbralCambio = 1.0,
  .velocidadGiro = 45.0,
  .tiempoGiro = 1500,
  .tiempoGiroCarril1 = 1300,//1800
  .tiempoGiroCarril2 = 1300,//1500
  .toleranciaAngulo = 5.0,
  .carrilObjetivo = 0,
  .ladoControl = 'D',
  .tiempoInicioEstado = 0,
  .direccionGiro = 0,
  .necesitaCambio = false,
  .esComandoC21 = false,
  .esComandoC14 = false,
  .esComandoC15 = false,
  .esComandoC20 = false,
  .esComandoC26 = false,
  .esComandoC33 = false,
  .esComandoC27 = false,
  .esComandoC32 = false,
  .esComandoC36 = false,

  //individuales
  .esComandoC02 = false,
  .esComandoC08 = false,

  .esComandoC07 = false,
  .esComandoC01 = false,
    .esComandoC01ModoDistancia = false,
    .esComandoC07ModoDistancia = false,
    .esComandoC02ModoDistancia = false,
    .esComandoC08ModoDistancia = false


  
};
int ultimoCarrilActivo = 0; 
// Variables de tiempo y monitoreo de carriles
unsigned long ultimaActualizacion = 0;
const unsigned long INTERVALO_CONTROL = 50;
unsigned long ultimoReporte = 0;
const unsigned long INTERVALO_REPORTE = 300;


const unsigned long INTERVALO_maxAngulo = 200;//500  // 2000 ms = 2 segundos

unsigned long ultimaImpresion_maxAngulo = 0;

// Estructura para el control lateral
struct FiltroDistancia {
  float historial[5];
  int indice;
  bool inicializado;
  float ultimaDistanciaValida;
  float umbralCambioMaximo;
  int invalidasConsecutivas; // lecturas seguidas inválidas o rechazadas (el valor devuelto es "viejo")
  float candidatoSalto;      // v99: última lectura rechazada por salto grande (para confirmarla)
  bool  hayCandidatoSalto;   // v99: true si hay un candidato esperando confirmación

  void inicializar() {
    for (int i = 0; i < 5; i++) historial[i] = 0;
    indice = 0;
    inicializado = false;
    ultimaDistanciaValida = 0;
    umbralCambioMaximo = 50.0;
    invalidasConsecutivas = 0;
    candidatoSalto = 0;
    hayCandidatoSalto = false;
  }
  
  float filtrar(float nuevaLectura) {
    if (nuevaLectura <= 0 || nuevaLectura > 150) {
      invalidasConsecutivas++;
      hayCandidatoSalto = false; // v99: una lectura inválida rompe la confirmación
      return ultimaDistanciaValida;
    }
    if (!inicializado || abs(nuevaLectura - ultimaDistanciaValida) <= umbralCambioMaximo) {
      historial[indice] = nuevaLectura;
      indice = (indice + 1) % 5;
      if (!inicializado) {
        for (int i = 0; i < 5; i++) historial[i] = nuevaLectura;
        inicializado = true;
      }
      float suma = 0;
      for (int i = 0; i < 5; i++) suma += historial[i];
      ultimaDistanciaValida = suma / 5.0;
      invalidasConsecutivas = 0;
      hayCandidatoSalto = false;
      return ultimaDistanciaValida;
    } else {
      // v99: salto grande. Se rechaza la primera vez (posible pico), pero si la
      // lectura siguiente confirma el mismo valor nuevo (±15cm), se ACEPTA y el filtro se reinicia.
      if (hayCandidatoSalto && abs(nuevaLectura - candidatoSalto) <= 15.0) {
        for (int i = 0; i < 5; i++) historial[i] = nuevaLectura;
        indice = 0;
        ultimaDistanciaValida = nuevaLectura;
        invalidasConsecutivas = 0;
        hayCandidatoSalto = false;
        return ultimaDistanciaValida;
      }
      candidatoSalto = nuevaLectura;
      hayCandidatoSalto = true;
      invalidasConsecutivas++;
      return ultimaDistanciaValida;
    }
  }
};

struct ControlLateral {
  float distanciaObjetivo;
  float toleranciaDistancia;
  float toleranciaAngulo;
  float anguloObjetivo;
  float anguloUmbral;
  float kpDistancia, kiDistancia, kdDistancia;
  float correccionMaxima;
  float factorGiroIzquierda;
  float factorGiroDerecha; 
  float errorDistanciaAnterior;
  float integralDistancia;
  float integralMaxDistancia;
  char ladoControl;
  FiltroDistancia filtroIzquierda;
  FiltroDistancia filtroDerecha;
};

ControlLateral controlLateral = {
  .toleranciaDistancia = 3.0,
  .toleranciaAngulo = 3.0,//algulo aceptado como recto
  .anguloObjetivo = 0.0,
  .anguloUmbral = 20.0, //angulo de desviasion maxima para empezar correccion agresiva
  .kpDistancia = 1.5, //3.5 5.0,//4.5
  .kiDistancia = 0.0,
  .kdDistancia =0.8,// 1.2,
  .correccionMaxima = 40.0,
  .factorGiroIzquierda = 1.0,//0.8,//1.0
  .factorGiroDerecha = 1.0,
  .errorDistanciaAnterior = 0,
  .integralDistancia = 0,
  .integralMaxDistancia = 20.0,
  .ladoControl = 'D'
};

// ============================================================
// DISTANCIAS DE CARRIL - VALORES GLOBALES POR SITUACIÓN
// ============================================================
// Estas constantes agrupan las distancias que se repiten en
// distintos comandos que hacen lo mismo (mismo sentido de giro,
// mismo tipo de maniobra). Si necesitas recalibrar una situación,
// cambia el valor aquí y se aplica a TODOS los comandos que la usan.
//
// Nomenclatura:
//   CERCA_PARED_xxx    -> el robot se acerca/pega más a esa pared
//   LEJOS_PARED_xxx     -> el robot mantiene más distancia de esa pared (carril "extendido")


const float DIST_CARRIL1_CERCA_PARED_DER   = 13.0;  //14 17🔴 C02,C04,C06,C18,C24,C30,C08,C10,C38,C36 -> distanciaCarril1
const float DIST_CARRIL1_LEJOS_PARED_DER   = 32.0;  //35🥊 C08,C10,C38,C36,C02,C04,C06,C18,C24,C30 (E ) -> distanciaCarril1

const float DIST_CARRIL2_CERCA_PARED_IZQ   = 10.0;  // 16 17 22.0; 🟢C07,C09,C13,C37,C19,C31,C01,C03,C05,C25 (IZQUIERDA) -> distanciaCarril2
const float DIST_CARRIL2_LEJOS_PARED_IZQ   = 35.0;  //40 🐸C01,C03,C05,C25,C07,C09,C13,C37,C19,C31 (E) -> distanciaCarril2

const float DIST_CARRIL1_ALEJANDOSE_PARED_DER   = 16.0;  // 📛C02,C04,C06,C18,C24,C30,C08,C10,C38,C36 -> distanciaCarril1
const float DIST_CARRIL2_ALEJANDOSE_PARED_IZQ   = 16.0;  // 20 ❎ C02,C04,C06,C18,C24,C30,C08,C10,C38,C36 -> distanciaCarril1

// DISTANCIAS DE CARRIL - VALORES GLOBALES POR SITUACIÓN
// ============================================================

unsigned long ultimaActualizacionf = 0;
const unsigned long INTERVALO_CONTROLf = 50;
unsigned long ultimoReportef = 0; // Añade esta línea
const unsigned long INTERVALO_REPORTEf = 300; // Añade esta línea (imprime ~3 veces/seg)

// ===== VARIABLES PARA MONITOR LEGIBLE DE CARRIL =====
volatile float dbgDistCruda = 0;      // lectura ToF lateral sin filtrar
volatile float dbgDistFiltrada = 0;   // lo que el filtro entrega al control
volatile float dbgDistObjetivo = 0;   // distancia que debería mantener
volatile float dbgErrDist = 0;        // filtrada - objetivo
volatile int   dbgInvalidas = 0;      // lecturas inválidas/rechazadas seguidas
volatile bool  dbgSinRef = false;     // true = no hay referencia lateral válida
volatile unsigned long dbgTDist = 0;  // última vez que corrió el control lateral

volatile float dbgHue = 0, dbgSat = 0, dbgVal = 0;
volatile char  dbgColor = '?';        // B=blanco A=azul N=naranja P=piso/otro
volatile unsigned long dbgTLectura = 0; // última vez que el sensor de color leyó
volatile bool  dbgPuedeDetectar = true; // false = lee pero no cuenta líneas (espera)

// --- Configuración de Tareas Multi-núcleo (FreeRTOS) ---

TaskHandle_t TareaColorHandle = NULL;



// ==================== TEMPORIZADOR NO BLOQUEANTE PARA LEDS ====================
unsigned long tiempoInicioLedTemporal = 0;
unsigned long duracionLedTemporal = 0;
bool ledTemporalActivo = false;

// Tiempo mínimo que el vehículo avanza en MANTENIENDO_CARRIL antes del cambio C21
const unsigned long TIEMPO_AVANCE_MANTENIENDO_C21 = 500;//600
const unsigned long TIEMPO_AVANCE_MANTENIENDO_C21_OMITIDO = TIEMPO_AVANCE_MANTENIENDO_C21 + 1500; // 2000
// Variantes de TIEMPO_AVANCE_MANTENIENDO_C21 según el sentido de giro del vehículo
const unsigned long TIEMPO_AVANCE_MANTENIENDO_C21_IZQUIERDA = 1000;//700
const unsigned long TIEMPO_AVANCE_MANTENIENDO_C21_DERECHA = 700;

unsigned long TIEMPO_AVANCE_MANTENIENDO_C14_IZQUIERDA = 0;
unsigned long TIEMPO_AVANCE_MANTENIENDO_C14_DERECHA = 300;
const unsigned long TIEMPO_AVANCE_MANTENIENDO_C14_OMITIDO = 200 + 1500; // 1700

const unsigned long TIEMPO_AVANCE_MANTENIENDO_C15 = 600;
const unsigned long TIEMPO_AVANCE_MANTENIENDO_C15_OMITIDO = TIEMPO_AVANCE_MANTENIENDO_C15 + 1500; // 2100

const unsigned long TIEMPO_AVANCE_MANTENIENDO_C20 = 500;
const unsigned long TIEMPO_AVANCE_MANTENIENDO_C20_OMITIDO = TIEMPO_AVANCE_MANTENIENDO_C20 + 1500; // 2000

const unsigned long TIEMPO_AVANCE_MANTENIENDO_C26 = 0;
const unsigned long TIEMPO_AVANCE_MANTENIENDO_C26_OMITIDO = TIEMPO_AVANCE_MANTENIENDO_C26 + 1500; // 1500

const unsigned long TIEMPO_AVANCE_MANTENIENDO_C27 = 500;
const unsigned long TIEMPO_AVANCE_MANTENIENDO_C27_OMITIDO = TIEMPO_AVANCE_MANTENIENDO_C27 + 1500; // 2000

const unsigned long TIEMPO_AVANCE_MANTENIENDO_C32 = 400;
const unsigned long TIEMPO_AVANCE_MANTENIENDO_C32_OMITIDO = TIEMPO_AVANCE_MANTENIENDO_C32 + 1500; // 1900

const unsigned long TIEMPO_AVANCE_MANTENIENDO_C33 = 400;
const unsigned long TIEMPO_AVANCE_MANTENIENDO_C33_OMITIDO = TIEMPO_AVANCE_MANTENIENDO_C33 + 1500; // 1900
// Flag para indicar que el siguiente MANTENIENDO_CARRIL es el final
bool esperandoMantenimientoFinal = false;
unsigned long tiempoInicioMantenimientoFinal = 0;
const unsigned long DURACION_MANTENIMIENTO_FINAL = 2000; // 2 segundos

// ==================== AJUSTE TIEMPO MANTENIENDO EN COMANDOS DOBLES (viene de carril INTERNO) ====================
const unsigned long MO_MANT_DOBLE_NORMAL_MS    = 500; // 100 sin emergencia en el cruce
const unsigned long MO_MANT_DOBLE_EMERG_IZQ_MS = 600; // con emergencia, sentido izquierda
const unsigned long MO_MANT_DOBLE_EMERG_DER_MS = 500; // con emergencia, sentido derecha

bool mo_huboEmergenciaEnCruce = false;          // true si hubo retroceso de emergencia durante MO_CRUZANDO
bool mo_ajusteManteniendoDobleActivo = false;   // true = aplicar ajuste al comando doble en curso
bool mo_manteniendoDobleEsEmergencia = false;
unsigned long mo_valorManteniendoDobleMs = 0;
// ===================== exterior carril========================
const unsigned long MO_MANT_DOBLE_EXT_EXT_EMERG_EXTRA_MS = 200; // exterior -> exterior (comando doble) con emergencia en el cruce
bool mo_sumarManteniendoDobleActivo = false;                    // true = sumar +200ms al manteniendo del comando doble en curso


// ===== v103: AJUSTES DEL ACOMODO LATERAL (aquí pruebas) =====
const int   AC_VELOCIDAD_ENDEREZADO = 85;    // PWM al enderezar el ángulo (antes 100 vía avanzarLento). Más bajo = más lento
const int   AC_VELOCIDAD_RETROCESO_FINAL = 90; // PWM del retroceso final recto (antes 100 vía retrocederLento)
const float AC_ANG_SATURA = 20.0f;           // ° de error a partir de los cuales el volante va a fondo
const float AC_SERVO_MIN_FRAC = 0.35f;       // fracción mínima del volante cerca del objetivo (evita pasarse)
// Corrección de rumbo mientras retrocede (el volante va invertido respecto a avanzar)
const bool  AC_RETRO_INVERTIR = true;        // si en pruebas se desvía MÁS al retroceder, ponlo en false
const float AC_RETRO_ZONA_MUERTA = 1.0f;     // ° dentro de los cuales deja el volante recto
const float AC_RETRO_SATURA = 10.0f;         // ° de error para el máximo giro en reversa
const int   AC_RETRO_SERVO_MAX = 14;         // máximo giro del volante en reversa

void mo_registrarEmergenciaIntInt() {
  if (!mo_esTramoInteriorAInterior) {
    mo_duracionEmergIntIntActual = 0;
    return;
  }
  mo_contadorEmergenciasIntInt++;
  mo_duracionEmergIntIntActual = (mo_contadorEmergenciasIntInt == 1)
                                   ? MO_RETROCESO_EMERG_INT_INT_LARGO_MS
                                   : MO_RETROCESO_EMERG_INT_INT_CORTO_MS;
  Serial.println("🔁 Emergencia int->int #" + String(mo_contadorEmergenciasIntInt) +
                 " -> retroceso " + String(mo_duracionEmergIntIntActual) + "ms");
}

// Usa mo_tiempoInicioFase: durante los primeros MO_PASO_INTERMEDIO_MS queda a +-10°, luego centra.
void mo_servoEnderezadoDosPasos() {
  if (millis() - mo_tiempoInicioFase < MO_PASO_INTERMEDIO_MS) {
    direccion.write(mo_giroIzquierda ? -MO_SERVO_GRADOS_PASO_INTERMEDIO : MO_SERVO_GRADOS_PASO_INTERMEDIO);
  } else {
    direccion.write(SERVO_CENTER);
  }
}

// Misma función que en vuelta libre
void escribirDireccion(int angulo) {
   direccion.write(angulo); 
}

// Convierte un ángulo de la ESCALA VIEJA (0-180, centro 75) a la nueva
float servoDesdeEscalaVieja(float v) {
   if (v <= 75.0f) return  (75.0f - v) / 75.0f * SERVO_SPAN_DER;    // derecha = positivo
  return -(v - 75.0f) / 105.0f * SERVO_SPAN_IZQ;                   // izquierda = negativo
}

unsigned long ajustarTiempoManteniendoDoble(unsigned long base) {
  if (mo_sumarManteniendoDobleActivo) return base + MO_MANT_DOBLE_EXT_EXT_EMERG_EXTRA_MS; // exterior->exterior con emergencia: +200ms
  if (!mo_ajusteManteniendoDobleActivo) return base;
  if (mo_manteniendoDobleEsEmergencia) return mo_valorManteniendoDobleMs;      // emergencia: valor fijo (más alto)
  return (base < mo_valorManteniendoDobleMs) ? base : mo_valorManteniendoDobleMs; // normal: nunca sube, solo reduce
}
// Función para lanzar un destello sin bloquear el código
void encenderLedTemporal(uint8_t r, uint8_t g, uint8_t b, unsigned long duracion) {
  colorVehiculo(r, g, b);
  tiempoInicioLedTemporal = millis();
  duracionLedTemporal = duracion;
  ledTemporalActivo = true;
}

// Función que vigila si ya es hora de apagar el destello
void actualizarLedTemporal() {
  if (ledTemporalActivo && (millis() - tiempoInicioLedTemporal >= duracionLedTemporal)) {
    // Si estamos en modo de espera, debe volver a morado. Si no, se apaga.
    if (estadoActual == ESPERANDO_COMANDO_CARRIL) {
      colorVehiculo(128, 0, 255); // Morado
    } else {
      apagarVehiculo();
    }
    ledTemporalActivo = false;
  }
}
// ==============================================================================
// ==================== CONTROL DE NEOPIXELS ====================
// Función para poner todos los LEDs del mismo color de golpe
void colorVehiculo(uint8_t r, uint8_t g, uint8_t b) {
  for(int i = 0; i < tiraLED.numPixels(); i++) {
    tiraLED.setPixelColor(i, tiraLED.Color(r, g, b));
  }
  tiraLED.show();
}

// Función para apagar los LEDs
void apagarVehiculo() {
  tiraLED.clear();
  tiraLED.show();
}
// ==============================================================

// ==============================================================
// Espera bloqueante con parpadeo de un color mientras se reintenta
// un sensor que falló en la inicialización. No detiene el robot
// para siempre: cada "intentos" ciclos vuelve a llamar a tu función
// de init (pasada como puntero a función) hasta que retorne true.
bool reintentarSensor(bool (*funcionInit)(), uint8_t r, uint8_t g, uint8_t b, const char* nombreSensor) {
  bool ok = funcionInit();
  unsigned long ultimoParpadeo = millis();
  bool ledEncendido = false;

  while (!ok) {
    // Parpadeo no bloqueante del color del sensor que está fallando
    if (millis() - ultimoParpadeo >= 300) {
      ultimoParpadeo = millis();
      ledEncendido = !ledEncendido;
      if (ledEncendido) colorVehiculo(r, g, b);
      else apagarVehiculo();
    }

    // Reintenta cada ~1 segundo aprox (cada 3 parpadeos), sin bloquear demasiado
    static unsigned long ultimoReintento = 0;
    if (millis() - ultimoReintento >= 1000) {
      ultimoReintento = millis();
      SerialBT.print(F("Reintentando "));
      SerialBT.print(nombreSensor);
      SerialBT.println(F("..."));
      ok = funcionInit();
    }
  }

  apagarVehiculo();
  SerialBT.print(nombreSensor);
  SerialBT.println(F(" OK (tras reintentos)"));
  return true;
}

// Función independiente que se ejecuta continuamente en el Núcleo 0
void TareaDeteccionColor(void * pvParameters) {
  SerialBT.print(F("Control del sensor de color iniciado en el núcleo: "));
  SerialBT.println(xPortGetCoreID());

  while (1) {
    // Ejecuta la lectura del sensor solo si el interruptor activó el sistema
    if (sistemaActivo) {
      detectarColor();
    }
    
    // Retraso obligatorio de FreeRTOS en milisegundos (ej. 40ms)
    // Permite liberar el núcleo para tareas en segundo plano del ESP32
    vTaskDelay(pdMS_TO_TICKS(10)); 
  }
}

// ==================== FUNCIONES DE CONTROL DE MOTOR ====================
void detenerMotor() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  ledcWrite(MOTOR_ENA, 0);
  pwmActualMotor = 0;
  pwmObjetivoMotor = 0;
}

void avanzarMotor(int velocidad = velocidadBase) {
  // Si el motor estaba retrocediendo (o parado con IN2 activo), hay que
  // frenar el PWM a 0 antes de invertir el sentido físico, para no forzar
  // el puente H con una inversión brusca de corriente.
  if (!sentidoActualEsAvance && pwmActualMotor > 0) {
    ledcWrite(MOTOR_ENA, 0);
    pwmActualMotor = 0;
  }
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  sentidoActualEsAvance = true;
  pwmObjetivoMotor = velocidad; // La rampa (actualizarRampaMotor) se encargará de llegar aquí
}

void retrocederMotor(int velocidad = velocidadRetroceso) {
  if (sentidoActualEsAvance && pwmActualMotor > 0) {
    ledcWrite(MOTOR_ENA, 0);
    pwmActualMotor = 0;
  }
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  sentidoActualEsAvance = false;
  pwmObjetivoMotor = velocidad;
}

void avanzarLento() {
  // Reutiliza la lógica de avanzar, pero le inyecta la velocidad reducida
  avanzarMotor(velocidadLenta); 
}

void retrocederLento() {
  // Reutiliza la lógica de retroceder, pero le inyecta la velocidad reducida
  retrocederMotor(velocidadRetrocesoLento); 
}

// v96: true si el comando activo es DOBLE (C21, C27, C15, C33, C14, C32, C20, C26 y sus variantes E)
bool esComandoDobleActivo() {
  return carriles.esComandoC21 || carriles.esComandoC14 || carriles.esComandoC15 ||
         carriles.esComandoC20 || carriles.esComandoC32 || carriles.esComandoC33 ||
         carriles.esComandoC26 || carriles.esComandoC27;
}

void avanzarRapido() {
  // Reutiliza la lógica de avanzar, pero le inyecta la velocidad máxima
//  avanzarMotor(velocidadMaxima);
  // v96: en comandos dobles NO usar velocidad máxima, usar la velocidad base
  avanzarMotor(esComandoDobleActivo() ? velocidadBase : velocidadMaxima);
  }

// Debe llamarse en CADA vuelta del loop() principal. Sube o baja pwmActualMotor
// hacia pwmObjetivoMotor de forma gradual y no bloqueante.
void actualizarRampaMotor() {
  if (pwmActualMotor == pwmObjetivoMotor) return; // ya llegó, no hacer nada

  unsigned long ahora = millis();
  if (ahora - tUltimoPasoRampa < INTERVALO_RAMPA_MS) return; // aún no toca dar el siguiente paso
  tUltimoPasoRampa = ahora;

  if (pwmActualMotor < pwmObjetivoMotor) {
    pwmActualMotor = min(pwmActualMotor + INCREMENTO_RAMPA_PWM, pwmObjetivoMotor);
  } else {
    pwmActualMotor = max(pwmActualMotor - INCREMENTO_RAMPA_PWM, pwmObjetivoMotor);
  }
  ledcWrite(MOTOR_ENA, pwmActualMotor);
}

// Retroceso compartido usado por "SHARP en corrección" y "ToF frontal".
// Detiene, centra el servo, retrocede con rampa durante 'duracionMs' actualizando
// el giro en vivo, y vuelve a detener. Al terminar, el servo NO se mueve todavía:
// eso lo decide cada llamador porque el lado del contragiro es distinto en cada caso.
void retrocesoCompartidoPorObstaculo(unsigned long duracionMs) {
  detenerMotor();
  direccion.write(SERVO_CENTER);
  delay(200);

  retrocederMotor();
  unsigned long t0 = millis();
  while (millis() - t0 < duracionMs) {
    actualizarGiro();
    actualizarRampaMotor();
    delay(10);
  }
  detenerMotor();
}

// Retroceso corto y bloqueante usado SOLO al cerrar el segundo giro de C14 (DERECHA)
// y C21 (IZQUIERDA), para recuperar la línea si el contragiro terminó habiéndola pasado.
void retrocesoCortoPostContragiro() {
  
  SerialBT.println("↩️ Retroceso corto post-contragiro (recuperar línea antes de MANTENIENDO_CARRIL)...");
  detenerMotor();
  direccion.write(SERVO_CENTRADO);
  delay(80);
  retrocederLento();
  unsigned long t0 = millis();

  if (carriles.esComandoC07 || carriles.esComandoC02){
    while (millis() - t0 < 400) {
    actualizarGiro(); // mantener el yaw actualizado durante el retroceso
    actualizarRampaMotor();
    delay(10);
    }
  }else{
    while (millis() - t0 < 700) {
    actualizarGiro(); // mantener el yaw actualizado durante el retroceso
    actualizarRampaMotor();
    delay(10);
      }
     }
  
  
  detenerMotor();
  delay(80);
}


// =======================================================================
// Inicia un VL53L1X (arranca en 0x29), le cambia la dirección y lo deja en modo largo / 50 ms.
// Devuelve false si el sensor no responde (para usarla con reintentarSensor).
bool iniciarToF(VL53L1X &s, uint8_t direccion) {
  s.setTimeout(500);
  if (!s.init()) return false;
  s.setAddress(direccion);
  s.setDistanceMode(VL53L1X::Long);          // hasta ~4 m
  s.setMeasurementTimingBudget(50000);       // 50 ms por medida (igual que antes)
  return true;
}

// ===== VL53L0X (diagonales) =====
bool iniciarToF_L0X(VL53L0X &s, uint8_t addr) {
  s.setTimeout(500);
  if (!s.init()) return false;
  s.setAddress(addr);
  s.setMeasurementTimingBudget(20000);  // 20 ms por medida
  s.startContinuous();                  // modo continuo: leer casi no bloquea el loop
  return true;
}

// 'X' = diagonal izquierdo | 'Y' = diagonal derecho. Devuelve cm, o 999.0 si no hay lectura válida.
float leerToF_L0X(char sensor) {
    if (sensor == 'X' || sensor == 'Y') return leerToF_L0X(sensor);

  VL53L0X* s;
  float offset;
  if (sensor == 'X')      { s = &tofDI; offset = OFFSET_TOF_DIAG_IZQ; }
  else if (sensor == 'Y') { s = &tofDD; offset = OFFSET_TOF_DIAG_DER; }
  else return 999.0;

  uint16_t mm = s->readRangeContinuousMillimeters();
  if (s->timeoutOccurred()) return 999.0;
  if (mm >= TOF0_MAX_MM_VALIDO) return 999.0;   // fuera de rango (el VL53L0X devuelve ~8190)
  return (mm / 10.0) - offset;
}

float leerToF(char sensor) {
  VL53L1X* s;
  float offset;
  switch (sensor) {
    case 'F': s = &tofF; offset = OFFSET_TOF_FRONTAL; break;
    case 'T': s = &tofT; offset = OFFSET_TOF_TRASERO; break;
    case 'I': s = &tofI; offset = OFFSET_TOF_IZQ;     break;
    case 'D': s = &tofD; offset = OFFSET_TOF_DER;     break;
    default:  return 999.0;
  }

  // Medición única bloqueante (hace lo mismo que el rangingTest anterior)
  uint16_t mm = s->readSingle(true);
  if (s->timeoutOccurred()) return 999.0;

  // Estados válidos: 0 = valida, 3 = valida (objeto muy cerca, recortada), 6 = valida sin chequeo de wrap
  uint8_t st = (uint8_t)s->ranging_data.range_status;
  bool valida = (st == 0 || st == 3 || st == 6);

  if (!valida || mm >= TOF_MAX_MM_VALIDO) {
    return 999.0;
  }
  return (mm / 10.0) - offset;
}

// ==================== FILTRO ANTI-SPIKE PARA TOF ====================
// Confirma una lectura del ToF usando un buffer deslizante.
// NO bloquea: cada llamada agrega 1 muestra. Devuelve true solo si
// 'minimoConfirmaciones' de las últimas 'ventana' muestras son válidas
// y están dentro del umbral.
// Usar una instancia por sensor (o resetear entre usos distintos).
struct FiltroToF {
  static const int VENTANA = 4;       // Cuántas muestras guarda el buffer
  float muestras[VENTANA];
  int indice = 0;
  int totalMuestras = 0;

  void reset() {
    indice = 0;
    totalMuestras = 0;
    for (int i = 0; i < VENTANA; i++) muestras[i] = 999.0;
  }

  // Agrega una muestra y retorna cuántas de las últimas VENTANA
  // están por debajo del umbral (y son > 0 y < 999).
  int agregarYContar(float valor, float umbral) {
    muestras[indice] = valor;
    indice = (indice + 1) % VENTANA;
    if (totalMuestras < VENTANA) totalMuestras++;

    int confirmadas = 0;
    for (int i = 0; i < totalMuestras; i++) {
      if (muestras[i] > 0 && muestras[i] < 999.0 && muestras[i] <= umbral) {
        confirmadas++;
      }
    }
    return confirmadas;
  }
};


unsigned long calcularDuracionAperturaPorDistancia(float distanciaSensor) {
  if (distanciaSensor < 25.0)      return DURACION_APERTURA_MINIMA; // Muy cerca: apertura mínima
  else if (distanciaSensor < 35.0) return DURACION_APERTURA_BAJA;
  else if (distanciaSensor < 60.0) return DURACION_APERTURA_MEDIA;
  else if (distanciaSensor < 70.0) return DURACION_APERTURA_ALTA;
  else                              return DURACION_APERTURA_MAXIMA; // Lejos de la pared: apertura máxima
}

// Mide ambos sensores laterales con el vehículo detenido un instante,
// promediando varias muestras, para tomar la decisión de apertura
// con un dato confiable y no con una lectura ruidosa al vuelo.
void medirDistanciasLateralesPrecisas(float &outIzq, float &outDer) {
  detenerMotor();
  delay(60); // pequeña pausa para que el ToF se estabilice

  float sumaIzq = 0, sumaDer = 0;
  int validasIzq = 0, validasDer = 0;
  const int NUM_MUESTRAS = 4;

  for (int i = 0; i < NUM_MUESTRAS; i++) {
    float dI = leerToF('I');
    float dD = leerToF('D');
    if (dI > 0 && dI < 200) { sumaIzq += dI; validasIzq++; }
    if (dD > 0 && dD < 200) { sumaDer += dD; validasDer++; }
    delay(15);
  }

  avanzarMotor(); // reanuda el avance inmediatamente tras medir

  outIzq = (validasIzq > 0) ? (sumaIzq / validasIzq) : 999.0;
  outDer = (validasDer > 0) ? (sumaDer / validasDer) : 999.0;
}

// ==================== FUNCIONES DE CARRIL ====================
float obtenerDistanciaObjetivo() {
  if (carriles.carrilObjetivo == 1) {
    return carriles.distanciaCarril1;
  } else {
    return carriles.distanciaCarril2;
  }
}

char obtenerLadoControlPorCarril() {
  if (carriles.carrilObjetivo == 1) {
    return 'D';  // Carril 1 siempre usa sensor derecho
  } else {
    return 'I';  // Carril 2 siempre usa sensor izquierdo
  }
}

String obtenerNombreEstado(EstadoSistema estado) { // 1. Cambiado el tipo de parámetro
  switch (estado) {
    // Estados generales
    case ESTADO_NORMAL: return "ESTADO_NORMAL";
    case ESTADO_DETENCION_FINAL: return "ESTADO_DETENCION_FINAL";
    
    // Estados de carril (usando los nombres de EstadoSistema)
    case ESPERANDO_COMANDO_CARRIL: return "ESPERANDO_COMANDO_CARRIL";
    case EVALUANDO_POSICION: return "EVALUANDO_POSICION";
    case GIRANDO_HACIA_CARRIL: return "GIRANDO_HACIA_CARRIL";
    case ENDEREZANDO_CARRIL: return "ENDEREZANDO_CARRIL";
    case MANTENIENDO_CARRIL: return "MANTENIENDO_CARRIL";
    case ESTADO_CORRECCION_LATERAL: return "ESTADO_CORRECCION_LATERAL"; 
    case MANIOBRA_OPTIMIZADA: return "MANIOBRA_OPTIMIZADA";
    
    default: return "DESCONOCIDO";
  }
}

void cambiarEstado(EstadoSistema nuevoEstado) { // 1. Cambiado el tipo de parámetro
  // Si se pide (re)entrar a GIRANDO_HACIA_CARRIL, forzar el reinicio interno de esa función
  // AUNQUE el estado actual ya sea GIRANDO_HACIA_CARRIL. Esto cubre el caso de comandos dobles
  // (C14/C21) cuyo segundo tramo puede solicitar este estado estando ya en él, lo que antes
  // hacía que se saltara por completo la Fase 1 (el giro inicial) por quedar tiempoInicioEstado
  // desactualizado.
  if (nuevoEstado == GIRANDO_HACIA_CARRIL) {
    forzarReinicioGiro = true;
  }

  if (estadoActual != nuevoEstado) { // 2. Se usa la variable global estadoActual
    SerialBT.println("🔄 Cambio estado: " + obtenerNombreEstado(estadoActual) + 
                  " → " + obtenerNombreEstado(nuevoEstado));
    // ==========================================
    // Control de luz de estado Morada
    if (nuevoEstado == ESPERANDO_COMANDO_CARRIL) {
      colorVehiculo(128, 0, 255); // Morado
    } else {
      apagarVehiculo(); // Asegura que se apague al salir del estado
    }
    // ==========================================
    //*********
    // Si entramos en el estado de mantener carril, guardamos cuál es el carril activo.
    // Esta será nuestra "memoria" para cuando se detecte una línea.
    if (nuevoEstado == MANTENIENDO_CARRIL) {
      ultimoCarrilActivo = carriles.carrilObjetivo;
      SerialBT.println("✍️ Último carril activo actualizado a: " + String(ultimoCarrilActivo));
    }

        // v101: cualquier cambio de estado cancela la fase; se rearma solo si venimos del giro hacia un carril INTERIOR
    ent_activa = false;
    ent_contEntro = 0;
    if (nuevoEstado == MANTENIENDO_CARRIL && estadoActual == GIRANDO_HACIA_CARRIL &&
        sentidoVehiculo != INDEFINIDO && !esUltimoTramoFinal) {
      bool carrilInterior = (sentidoVehiculo == IZQUIERDA && carriles.carrilObjetivo == 2) ||
                            (sentidoVehiculo == DERECHA   && carriles.carrilObjetivo == 1);
      if (carrilInterior) {
        ent_activa = true;
        ent_tInicio = millis();
        SerialBT.println("🎯 ENTRADA CARRIL INTERIOR: fase armada (sensor opuesto " +
                         String(ENT_DIST_MIN_OPUESTA_CM, 0) + "-" + String(ENT_DIST_MAX_OPUESTA_CM, 0) + "cm, velocidad media)");
      }
    }

    //******
    estadoActual = nuevoEstado; // 3. Se actualiza la variable global
    carriles.tiempoInicioEstado = millis();
    
    // Resetear variables de control del mantenimiento de pared
        mo_objRetrocediendo = false;      // v96: seguridad, nunca quedar en reversa al cambiar de estado
    mo_objRetrocesoTerminado = false;
    controlLateral.integralDistancia = 0;
    controlLateral.errorDistanciaAnterior = 0;

    controlLateral.filtroIzquierda.inicializar();
    controlLateral.filtroDerecha.inicializar();
       // Reset flag C21 si se cambia manualmente de estado
    if (nuevoEstado == ESPERANDO_COMANDO_CARRIL) { // 4. Usar el nombre de EstadoSistema
      mo_confirmarObjetoActivo = false; // v96
      carriles.esComandoC21 = false;
      carriles.esComandoC14 = false;      
      carriles.esComandoC15 = false;
      carriles.esComandoC20 = false; 
      carriles.esComandoC26 = false;
      carriles.esComandoC33 = false; 
      carriles.esComandoC27 = false; 
      carriles.esComandoC32 = false;
      carriles.esComandoC02 = false;
      carriles.esComandoC08 = false;
      carriles.esComandoC36 = false;
      carriles.esComandoC07 = false;
      carriles.esComandoC01 = false;
      SerialBT.println("reseteo de los esComandoC */*****  ********* *******");

      omitioGiro = false; // Resetear flag al volver a esperar comando
      
      // Limpiar buffers de comunicación para asegurar que se espere un comando NUEVO.
      SerialBT.println("Buffers de comunicación limpiados. Esperando nuevo comando...");

      while (Serial.available() > 0) {
        Serial.read(); // Lee y descarta cada byte del búfer USB
      }
      while (Serial.available() > 0) { 
        Serial.read(); //  
      }
    }
  }
}

// ==================== MÁQUINA DE ESTADOS DE CARRIL ====================
float procesarEstadoEsperandoComando() {
  // Sistema detenido esperando comando
  return (float)SERVO_CENTER;
}

float procesarEstadoEvaluandoPosicion() {
  
  float distanciaIzq = leerToF('I');
  float distanciaDer = leerToF('D');
  
  // Determinar qué sensor usar para el control principal según 'sentidoVehiculo'
  char ladoControlEvaluacion = (sentidoVehiculo == DERECHA) ? 'I' : 'D';
  float distanciaControl = (ladoControlEvaluacion == 'I') ? distanciaIzq : distanciaDer;

  SerialBT.println(" Sin obstáculos detectados, evaluando posición normal");
  
  if (distanciaControl < 0 || distanciaControl > 300) {
    SerialBT.println("⚠ No se puede leer distancia, manteniendo posición");
    //cambiarEstado(ESPERANDO_COMANDO_CARRIL);
    return (float)SERVO_CENTER;
  }
  
  // =========================================================================
  // NUEVO ANÁLISIS: Verificar si el robot YA ESTÁ en el carril objetivo
  // dependiendo de si el carril representa la distancia corta o larga
  // según el sentido de giro.
  // =========================================================================
  
  float MIN_DISTANCIA_CORTA = 0.0;
  float MAX_DISTANCIA_CORTA = 25.0;//25.0;
  float MIN_DISTANCIA_LARGA = 65.0;
  float MAX_DISTANCIA_LARGA = 83.0;

  bool yaEnCarrilObjetivo = false;
  //estos cambios para que nunca se salte el giro despues del acomodo
  if (carriles.esComandoC21) {
    MIN_DISTANCIA_CORTA = 0.0;
    MAX_DISTANCIA_CORTA = 1.0;
    
    MIN_DISTANCIA_LARGA = 82.0;
    MAX_DISTANCIA_LARGA = 83.0;

   }
   if (carriles.esComandoC14) {
    MIN_DISTANCIA_CORTA = 0.0;
    MAX_DISTANCIA_CORTA = 1.0;
    
    MIN_DISTANCIA_LARGA = 82.0;
    MAX_DISTANCIA_LARGA = 83.0;

   }
  if (carriles.esComandoC08) {
    MIN_DISTANCIA_CORTA = 0.0;
    MAX_DISTANCIA_CORTA = 45.0;
    
    MIN_DISTANCIA_LARGA = 65.0;
    MAX_DISTANCIA_LARGA = 83.0;

   }
  if (carriles.esComandoC33) {
    MIN_DISTANCIA_CORTA = 0.0;
    MAX_DISTANCIA_CORTA = 45.0;
    
    MIN_DISTANCIA_LARGA = 65.0;
    MAX_DISTANCIA_LARGA = 83.0;

   }
  if (carriles.esComandoC26) {
    MIN_DISTANCIA_CORTA = 0.0;
    MAX_DISTANCIA_CORTA = 45.0;
    
    MIN_DISTANCIA_LARGA = 65.0;
    MAX_DISTANCIA_LARGA = 83.0;

   }
  if (carriles.esComandoC01) {
    MIN_DISTANCIA_CORTA = 0.0;
    MAX_DISTANCIA_CORTA = 55.0;//MODFICADO POR PROBLEMA DE MEDICION DE DISTANCIA DEL SENSOR MIDE MAS DE LO QUE ES 
    
    MIN_DISTANCIA_LARGA = 65.0;
    MAX_DISTANCIA_LARGA = 85.0;

   }

  if (carriles.carrilObjetivo == 1) {
    if (sentidoVehiculo == IZQUIERDA) {
      // Pared a la derecha -> Carril 1 es la distancia corta
      if (distanciaControl >= MIN_DISTANCIA_CORTA && distanciaControl <= MAX_DISTANCIA_CORTA) {
          yaEnCarrilObjetivo = true;
      }
    } else if (sentidoVehiculo == DERECHA) {
      // Pared a la izquierda -> Carril 1 es la distancia larga
      if (distanciaControl >= MIN_DISTANCIA_LARGA && distanciaControl <= MAX_DISTANCIA_LARGA) {
          yaEnCarrilObjetivo = true;
      }
    }
  } else if (carriles.carrilObjetivo == 2) {
    
    if (sentidoVehiculo == IZQUIERDA) {
      // Pared a la derecha -> Carril 2 es la distancia larga
      if (distanciaControl >= MIN_DISTANCIA_LARGA && distanciaControl <= MAX_DISTANCIA_LARGA) {
          yaEnCarrilObjetivo = true;
      }
    } else if (sentidoVehiculo == DERECHA) {
      // Pared a la izquierda -> Carril 2 es la distancia corta
      if (distanciaControl >= MIN_DISTANCIA_CORTA && distanciaControl <= MAX_DISTANCIA_CORTA) {
          yaEnCarrilObjetivo = true;
      }
    }
    
  }

  // DEBUG: para confirmar si el falso "ya en carril" se está disparando en C14/C21
  if (yaEnCarrilObjetivo && (carriles.esComandoC14 || carriles.esComandoC21)) {
    SerialBT.println("🐛 DEBUG: yaEnCarrilObjetivo=true detectado en comando doble (C14=" +
                     String(carriles.esComandoC14) + " C21=" + String(carriles.esComandoC21) +
                     ") | Sensor: " + String(ladoControlEvaluacion) +
                     " Dist: " + String(distanciaControl, 1) + "cm");
  }
  // Si ya estamos dentro del rango del carril, saltamos el giro
  //if (yaEnCarrilObjetivo && !carriles.esComandoC14 && carriles.esComandoC21 && carriles.esComandoC20) {
  if (yaEnCarrilObjetivo && !carriles.esComandoC14 && !carriles.esComandoC21 && (carriles.esComandoC20 || carriles.esComandoC08|| carriles.esComandoC33 ||carriles.esComandoC01||carriles.esComandoC26)) {
    SerialBT.println("✅ El vehículo ya está en el Carril " + String(carriles.carrilObjetivo) + 
                     " (Sensor " + String(ladoControlEvaluacion) + ": " + String(distanciaControl, 1) + "cm).");
    SerialBT.println(">> OMITIENDO GIRO: Pasando directo a MANTENIENDO_CARRIL.");
    
    // Actualizar el lado de control para el mantenimiento de pared
    controlLateral.ladoControl = ladoControlEvaluacion;
    
    omitioGiro = true; // Marcar que no hubo giro previo
    cambiarEstado(MANTENIENDO_CARRIL);
    return (float)SERVO_CENTER;
  }
  // =========================================================================

  // --- SI NO ESTÁ EN EL CARRIL, SE PROCEDE CON EL GIRO NORMAL ---
  float distanciaObjetivo = obtenerDistanciaObjetivo();
  float diferencia = distanciaControl - distanciaObjetivo;
  
  SerialBT.println(" Carril " + String(carriles.carrilObjetivo) + 
                " | Sensor: " + String(ladoControlEvaluacion) +
                " | Distancia actual: " + String(distanciaControl, 1) + 
                "cm | Objetivo: " + String(distanciaObjetivo, 1) + 
                "cm | Diferencia: " + String(diferencia, 1) + "cm");

  // ===== v104: antes de iniciar el cambio de carril, el carro debe estar RECTO y el ToF LATERAL del lado del giro LIBRE =====
  // Carril 1 -> ToF derecho ('D') | Carril 2 -> ToF izquierdo ('I')
  {
    const bool  EXIGIR_RECTO_Y_LIBRE   = true;   // false = desactiva todo este bloque
    const float TOL_RECTO_GRADOS       = 8.0;    // qué tan recto debe estar respecto al ángulo objetivo
    const float LATERAL_LIBRE_MIN_CM   = 25.0;   // menos que esto en el lateral del giro = "hay algo cerca"
    const unsigned long ESPERA_MAX_MS  = 600;   // tope de seguridad: si no se cumple, gira igual

    if (EXIGIR_RECTO_Y_LIBRE) {
      esperandoCambioCarril = false;

      if (!primerGiroRealizado) anguloObjetivo = 0.0;
      else calcularAnguloObjetivoMantener();

      float difAng = calcularDiferenciaAngular(anguloZ, anguloObjetivo); // actual - objetivo
      bool  recto = fabs(difAng) <= TOL_RECTO_GRADOS;

      float dLateralGiro = (carriles.carrilObjetivo == 1) ? distanciaDer : distanciaIzq;
      bool  lateralLibre = !(dLateralGiro > 0 && dLateralGiro < LATERAL_LIBRE_MIN_CM); // 999/inválido = libre

      bool agotado = (millis() - carriles.tiempoInicioEstado) > ESPERA_MAX_MS;

      if ((!recto || !lateralLibre) && !agotado) {
        static unsigned long ultimoLogEsperaGiro = 0;
        if (millis() - ultimoLogEsperaGiro > 150) {
          SerialBT.println("⏳ Esperando para cambiar de carril | recto=" + String(recto) +
                           " (dif " + String(difAng, 1) + "°) | lateral " +
                           String(carriles.carrilObjetivo == 1 ? "D" : "I") + "=" +
                           String(dLateralGiro, 1) + "cm libre=" + String(lateralLibre));
          ultimoLogEsperaGiro = millis();
        }
        // Seguir avanzando y enderezar por ángulo (misma ley que el fallback de MANTENIENDO_CARRIL)
        esperandoCambioCarril = true; // el loop usa avanzarLento() mientras espera
        float corrRecto = constrain(2.5 * (-difAng),
                                    -(float)SERVO_CORR_PARED_DER,
                                    (float)SERVO_CORR_PARED_IZQ);
        return constrain((float)SERVO_CENTER - corrRecto, (float)SERVO_MIN, (float)SERVO_MAX);
      }
      if ((!recto || !lateralLibre) && agotado) {
        SerialBT.println("⚠️ Espera agotada (" + String(ESPERA_MAX_MS) + "ms): se inicia el cambio de carril igual.");
      }
    }
  }
  
  // Determinar la dirección del giro basado únicamente en el carril objetivo.
  if (carriles.carrilObjetivo == 1) {
    // Para ir al carril 1, SIEMPRE giramos a la DERECHA.
    carriles.direccionGiro = 1.0; 
  } else { 
    // Para ir al carril 2, SIEMPRE giramos a la IZQUIERDA.
    carriles.direccionGiro = -1.0;
  }

  // Actualizar el lado de control según el carril
  controlLateral.ladoControl = ladoControlEvaluacion;
  
  SerialBT.println(" Iniciando giro hacia carril " + String(carriles.carrilObjetivo) + 
                " (sensor: " + String(ladoControlEvaluacion) + 
                ", dirección: " + String(carriles.direccionGiro > 0 ? "derecha" : "izquierda") + ")");
                
  cambiarEstado(GIRANDO_HACIA_CARRIL);
  
  return (float)SERVO_CENTER;
}

// Tiempo máximo de seguridad esperando que el ToF frontal confirme (si nunca confirma, se pasa al contragiro igual)
const unsigned long TIMEOUT_GIRO_ESPERA_TOF_MS = 4000;

// ¿Este giro es el PRIMERO del comando? (los comandos individuales solo tienen uno)
// C21/C27/C15/C33 empiezan en carril 1; C14/C32/C20/C26 empiezan en carril 2.
bool esPrimerGiroDelComando() {
  bool empiezaCarril1 = carriles.esComandoC21 || carriles.esComandoC27 || carriles.esComandoC15 || carriles.esComandoC33;
  bool empiezaCarril2 = carriles.esComandoC14 || carriles.esComandoC32 || carriles.esComandoC20 || carriles.esComandoC26;
  if (empiezaCarril1) return carriles.carrilObjetivo == 1;
  if (empiezaCarril2) return carriles.carrilObjetivo == 2;
  return true;
}

// true = el contragiro espera la confirmación del ToF frontal (no depende del tiempo)
// false = se mantiene el giro por tiempo (excepciones)
bool giroEsperaTofFrontal() {
  bool primero = esPrimerGiroDelComando();
  if (sentidoVehiculo == IZQUIERDA) {
    if (carriles.esComandoC07) return false;                                     // C07 y C07E
    if (primero && (carriles.esComandoC14 || carriles.esComandoC32)) return false; // 1er giro de C14 y C32E
  } else if (sentidoVehiculo == DERECHA) {
    if (carriles.esComandoC02) return false;                                     // C02 y C02E
    if (primero && (carriles.esComandoC21 || carriles.esComandoC27 || carriles.esComandoC33)) return false; // 1er giro de C21, C33E y C27E
  }
  return true;
}

// Función auxiliar para verificar si el ángulo está dentro del umbral
bool estaEnRangoAngulo(float anguloActual, float anguloObjetivo, float umbral = 3.0) {
  float diferencia = abs(anguloActual - anguloObjetivo);
  
  // Manejar la transición 0°-360°
  if (diferencia > 180.0) {
    diferencia = 360.0 - diferencia;
  }
  
  return diferencia <= umbral;
}
float procesarEstadoGirandoHaciaCarril() {
  // el cooldown ahora se activa inmediatamente en 'procesarComandoCarril'.
  // Variables estáticas para recordar el estado de la interrupción
  static bool sharpInterrupcionActiva = false;
  static bool lineaInterrupcionActiva = false;
  static unsigned long tiempoInicioInterrupcion = 0;
  static bool pausaPorSharpRealizada = false;
  static FiltroToF filtroFrontal;

  // --- EMERGENCIA: control de atasco durante el retorno de ángulo (contragiro) ---
  static unsigned long tiempoInicioRetornoAtascado = 0;   // marca cuándo empezó el intento de retorno actual
  static bool retornoAtascadoEnCurso = false;             // true mientras estamos "contando" un intento continuo
  const unsigned long TIEMPO_LIMITE_RETORNO_ATASCADO = 3000; // 3 segundos

  // Variables estáticas para el control de las fases
  static float anguloGiroFijo = SERVO_CENTER;
  static bool anguloCalculado = false;
  static bool mensajeFase1Mostrado = false;
  static bool mensajeFase2Mostrado = false;
  

  // Resetear el estado cada vez que se entra a este estado de giro.
  // Se dispara por cambio real de tiempoInicioEstado, O por la bandera forzarReinicioGiro
  // (necesaria para comandos dobles como C14/C21 cuyo segundo tramo puede re-solicitar
  // este mismo estado sin que cambiarEstado() haya podido actualizar tiempoInicioEstado).
 static unsigned long ultimoTiempoInicioEstado = 0;
  if (carriles.tiempoInicioEstado != ultimoTiempoInicioEstado || forzarReinicioGiro) {
    bool esGiroForzado = forzarReinicioGiro; // guardamos antes de consumirla, no cambia la lógica siguiente
    forzarReinicioGiro = false; // Consumir la bandera
    sharpInterrupcionActiva = false;
    lineaInterrupcionActiva = false;
    pausaPorSharpRealizada = false;
    anguloCalculado = false; 
    retornoAtascadoEnCurso = false;
    tiempoInicioRetornoAtascado = 0;
    
    mensajeFase1Mostrado = false;
    mensajeFase2Mostrado = false;
    enFaseContragiro = false;
    
    filtroFrontal.reset();
    SerialBT.println("🔄 Nuevo giro iniciado - Sistemas de interrupción reseteados.");

    float anguloArranque = SERVO_CENTER;
    if (carriles.esComandoC21) {
      anguloArranque = (carriles.direccionGiro < 0) ? SERVO_LEFT : SERVO_RIGHT;
    } else if (carriles.esComandoC14) {
      anguloArranque = (carriles.direccionGiro < 0) ? SERVO_LEFT : SERVO_RIGHT;
    } else if (carriles.esComandoC02) {
      anguloArranque = (carriles.direccionGiro < 0) ? SERVO_LEFT : SERVO_RIGHT;
    } else {
      anguloArranque = (carriles.direccionGiro < 0) ? SERVO_LEFT : SERVO_RIGHT;
    }
    detenerMotor();
    direccion.write((int)anguloArranque);
    SerialBT.println("⏳ Pausa de arranque: servo posicionado a " + String((int)anguloArranque) + "° antes de avanzar.");
    delay(300);
    //avanzarMotor();
    avanzarMotor(velocidadGiroCarril);

    // ── El cronómetro del giro arranca AQUÍ, justo cuando el motor empieza a moverse
    // (con la rampa progresiva), no antes del delay(300) del servo. Así tiempoGiroActual
    // sigue representando el tiempo real de movimiento, igual que en la versión sin rampa.
    carriles.tiempoInicioEstado = millis();
    ultimoTiempoInicioEstado = carriles.tiempoInicioEstado;
  }

  unsigned long tiempoEnGiro = millis() - carriles.tiempoInicioEstado;
  unsigned long tiempoGiroActual;
    if (carriles.esComandoC21 && carriles.carrilObjetivo == 1) tiempoGiroActual = 1600;//1600
  else if (carriles.esComandoC21 && carriles.carrilObjetivo == 2) tiempoGiroActual = 1600;//1700;
  
  else if (carriles.esComandoC20 && carriles.carrilObjetivo == 2) tiempoGiroActual = 800;//1800;

  else if (carriles.esComandoC20 && carriles.carrilObjetivo == 1) tiempoGiroActual = 1200;//1800;

  else if (carriles.esComandoC14 && carriles.carrilObjetivo == 1) tiempoGiroActual = 2200;//2000;

  else if (carriles.esComandoC15 && carriles.carrilObjetivo == 1) tiempoGiroActual = 800;//1900;

  else if (carriles.esComandoC15 && carriles.carrilObjetivo == 2) tiempoGiroActual = 1200;//1900;

  //C33 (Rojo, Adelante Izquierda y Verde, Atrás Izquierda)
  //Primer giro
  else if (carriles.esComandoC33 && carriles.carrilObjetivo == 1) tiempoGiroActual = 1000;//1550;
  // retorno
  else if (carriles.esComandoC33 && carriles.carrilObjetivo == 2) tiempoGiroActual = 1500;//2200;
  //C26 (Verde, Adelante Derecha y Rojo, Atrás Derecha)
  // Primer giro verde izquierda
  else if (carriles.esComandoC26 && carriles.carrilObjetivo == 2) tiempoGiroActual = 1000;//1800;
  // retorno rojo izquierda
  else if (carriles.esComandoC26 && carriles.carrilObjetivo == 1) tiempoGiroActual = 1600;//1100;//1600;//1500;
  //C27 (Rojo, Adelante Derecha y Verde, Atrás Derecha)
  else if (carriles.esComandoC27 && carriles.carrilObjetivo == 1) tiempoGiroActual = 1700;//1600;
  else if (carriles.esComandoC27 && carriles.carrilObjetivo == 2) tiempoGiroActual = 1300;// 900 1200//800;//1800;
  //C32 (Verde, Adelante Izquierda y Rojo, Atrás Izquierda
  else if (carriles.esComandoC32 && carriles.carrilObjetivo == 1 ) tiempoGiroActual = 1400;//1100  1600;

  else if (carriles.esComandoC32 && carriles.carrilObjetivo == 2 && sentidoVehiculo == DERECHA) tiempoGiroActual = 1300;//1800;

  else if (carriles.esComandoC08 && carriles.carrilObjetivo == 1) tiempoGiroActual = 1000;//1500;
  
  else if (carriles.esComandoC36 && carriles.carrilObjetivo == 1) tiempoGiroActual = 1500;//2000;
  
  else if (carriles.carrilObjetivo == 1)  {
    tiempoGiroActual = carriles.tiempoGiroCarril1;
    SerialBT.println(" TIEMPO ARBITRARIO CARRIL 1");

    }

  else if (carriles.carrilObjetivo == 2){ 
   tiempoGiroActual = carriles.tiempoGiroCarril2;
    SerialBT.println(" TIEMPO ARBITRARIO CARRIL 2");

    }
  else{ 
    tiempoGiroActual = carriles.tiempoGiro;
    SerialBT.println(" TIEMPO ARBITRARIO CARRIL GIRO");
}
  // LÓGICA DE INTERRUPCIÓN CON FILTRO ANTI-SPIKE
  if (!sharpInterrupcionActiva && !lineaInterrupcionActiva) {
    
    // Buffer deslizante estático para el ToF frontal
    //static FiltroToF filtroFrontal;
    const int CONFIRMACIONES_REQUERIDAS = 2; // De 4 muestras, al menos 3 deben confirmar

    float distanciaFrontalGiro = leerToF('F');

    float distancia_interrupcion_frontal = 40.0;//34 38
    // v104: destino INTERIOR (pared del centro): al girar hacia ella el haz frontal la ve en diagonal
    bool destinoInteriorTOF = (sentidoVehiculo == IZQUIERDA && carriles.carrilObjetivo == 2) ||
                              (sentidoVehiculo == DERECHA   && carriles.carrilObjetivo == 1);

    if (carriles.esComandoC26 && carriles.carrilObjetivo == 1) {
      distancia_interrupcion_frontal = 40.0;//30
    } else if (carriles.esComandoC21 && carriles.carrilObjetivo == 1) {
      distancia_interrupcion_frontal = 40.0;//35
    } else if (destinoInteriorTOF) {
      distancia_interrupcion_frontal = 40.0; // prueba 18-25
    }


    if (carriles.esComandoC26 && carriles.carrilObjetivo == 1) {
      distancia_interrupcion_frontal = 40.0;//30
    } else if (carriles.esComandoC21 && carriles.carrilObjetivo == 1) {
      distancia_interrupcion_frontal = 40.0;//35
    }

    // Solo contar si la lectura es válida (no es 999 ni error de status)
    int confirmaciones = 0;
    // v103: el ToF solo puede interrumpir cuando el giro ya avanzó. Antes de eso se descartan
    // las lecturas (y se limpia el buffer) para que solo cuenten las confirmaciones posteriores.
    const unsigned long MIN_TIEMPO_GIRO_ANTES_TOF_MS = 1000;
    if (tiempoEnGiro < MIN_TIEMPO_GIRO_ANTES_TOF_MS) {
      filtroFrontal.reset();
    } else if (distanciaFrontalGiro > 0 && distanciaFrontalGiro < 999.0) {

      confirmaciones = filtroFrontal.agregarYContar(distanciaFrontalGiro, distancia_interrupcion_frontal);
      SerialBT.print("[ToF F] Lectura: "); SerialBT.print(distanciaFrontalGiro, 1);
      SerialBT.print("cm | Confirmaciones: "); SerialBT.print(confirmaciones);
      SerialBT.print("/"); SerialBT.println(CONFIRMACIONES_REQUERIDAS);
    } else {
      // Lectura inválida (999 o <= 0): NO contar, pero tampoco resetear el buffer
      // (un spike de 999 no debe borrar confirmaciones reales)
      SerialBT.println("[ToF F] Lectura inválida descartada.");
    }

    // v104: confirmación extra con el Sharp diagonal del lado del carril objetivo.
    // Carril 1 -> Sharp DERECHO | Carril 2 -> Sharp IZQUIERDO.
    // El ToF solo interrumpe si el Sharp también ve la pared cerca (o si el ToF está en zona crítica).
    bool sharpConfirmaGiro = true;
    if (confirmaciones >= CONFIRMACIONES_REQUERIDAS) {
      const float SHARP_CONFIRMA_GIRO_CM = 30.0;      // 25 pared del carril objetivo a menos de esto
      const float TOF_CRITICO_SIN_SHARP_CM = 15.0;    // frente muy cerca: interrumpe aunque el Sharp no confirme
      float dSharpGiro = leerSensorSharp(carriles.carrilObjetivo == 1 ? SHARP_DIAGONAL_DER_PIN
                                                                      : SHARP_DIAGONAL_IZQ_PIN);
      bool sharpOkGiro = (dSharpGiro > 0 && dSharpGiro < SHARP_CONFIRMA_GIRO_CM);
      bool tofCriticoGiro = (distanciaFrontalGiro > 0 && distanciaFrontalGiro < TOF_CRITICO_SIN_SHARP_CM);
      sharpConfirmaGiro = sharpOkGiro || tofCriticoGiro;
      if (!sharpConfirmaGiro) {
        static unsigned long ultimoLogSharpGiro = 0;
        if (millis() - ultimoLogSharpGiro > 200) {
          SerialBT.println("🛑 ToF frontal (" + String(distanciaFrontalGiro, 1) + "cm) NO confirmado por Sharp diagonal ("
                           + String(dSharpGiro, 1) + "cm >= " + String(SHARP_CONFIRMA_GIRO_CM, 0) + "). Sigo girando.");
          ultimoLogSharpGiro = millis();
        }
      }
    }
    if (confirmaciones >= CONFIRMACIONES_REQUERIDAS && sharpConfirmaGiro) {
      SerialBT.println(" INTERRUPCIÓN CONFIRMADA POR TOF FRONTAL! (" + String(distanciaFrontalGiro, 1) + "cm). Saltando a corrección.");
      sharpInterrupcionActiva = true;
      tiempoInicioInterrupcion = millis();
      filtroFrontal.reset(); // Limpiar para el próximo giro
    }
    else {
      //miestras esta retornando al angulo objtivo no me detecte colores por un tiempo especifico
        if ((millis() - tiempoUltimaDeteccion > tiempoEsperaDeteccion) || (carriles.esComandoC14 && carriles.carrilObjetivo == 1) || (carriles.esComandoC21 && carriles.carrilObjetivo == 2)|| (carriles.esComandoC07 && contadorLineas == 0) || (carriles.esComandoC02 && contadorLineas == 0)) {
            bool lineaDetectada = false;
            if (sentidoVehiculo == IZQUIERDA) {
                if (confirmarColor(HUE_AZUL_MIN, HUE_AZUL_MAX, SATURACION_AZUL_MIN, VALOR_AZUL_MIN, VALOR_AZUL_MAX, CONFIRMAR_AZUL_TOLERANCIA, CONFIRMAR_AZUL_DELAY)) {
                    lineaDetectada = true;
                    SerialBT.println(" INTERRUPCIÓN POR LÍNEA AZUL! Saltando a la fase de corrección AHORA.");
                }
            }
            else if (sentidoVehiculo == DERECHA) {
                if (confirmarColor(HUE_NARANJA_MIN, HUE_NARANJA_MAX, SATURACION_NARANJA_MIN, VALOR_NARANJA_MIN, VALOR_NARANJA_MAX, CONFIRMAR_NARANJA_TOLERANCIA, CONFIRMAR_NARANJA_DELAY)) {
                    lineaDetectada = true;
                    SerialBT.println(" INTERRUPCIÓN POR LÍNEA NARANJA! Saltando a la fase de corrección AHORA.");
                }
            }
            if (lineaDetectada) {
                lineaInterrupcionActiva = true;
                tiempoInicioInterrupcion = millis();
            }
        }
      }
  }

  // ==================== ====================
  // FASE DE CORRECCIÓN Y ENDEREZADO (Lógica Unificada)
  //if (tiempoEnGiro >= tiempoGiroActual || sharpInterrupcionActiva || lineaInterrupcionActiva) { 
  bool esperarTof = giroEsperaTofFrontal();
  unsigned long tiempoLimiteGiro = esperarTof ? TIMEOUT_GIRO_ESPERA_TOF_MS : tiempoGiroActual;
  if (tiempoEnGiro >= tiempoLimiteGiro || sharpInterrupcionActiva || lineaInterrupcionActiva) {
    // ── SEGURO: Si la desviación respecto al objetivo es pequeña, no hace falta contragiro ──
    if (!primerGiroRealizado) {
        anguloObjetivo = 0.0;
    } else {
        calcularAnguloObjetivoMantener();
    }

    /*const float UMBRAL_CONTRAGIRO = 6.0; // Si está a menos de 12° del objetivo, va directo
    if (estaEnRangoAngulo(anguloZ, anguloObjetivo, UMBRAL_CONTRAGIRO)) {
        SerialBT.println("✅ Desviación pequeña, sin contragiro. Directo a MANTENIENDO_CARRIL.");
        controlLateral.ladoControl = obtenerLadoControlPorCarril();
        cambiarEstado(MANTENIENDO_CARRIL);
        return (float)SERVO_CENTER;*/
        // 2. Verificar si ya hemos llegado al ángulo objetivo
    
    //if (estaEnRangoAngulo(anguloZ, anguloObjetivo, 8.0)) {
      if (!sharpInterrupcionActiva && estaEnRangoAngulo(anguloZ, anguloObjetivo, UMBRAL_ANGULO_ENDEREZADO)){
      SerialBT.println(" Ángulo corregido. Pasando a mantener carril.");
      enFaseContragiro = false;

      // Caso específico: segundo giro de C14 (DERECHA) o C21 (IZQUIERDA) —
      // el contragiro por ángulo puede terminar habiendo pasado la línea.
      // Retroceder un poco antes de entrar a MANTENIENDO_CARRIL para poder detectarla.
      bool esSegundoGiroC14Derecha = carriles.esComandoC14 && sentidoVehiculo == DERECHA && carriles.carrilObjetivo == 1;
      bool esSegundoGiroC21Izquierda = carriles.esComandoC21 && sentidoVehiculo == IZQUIERDA && carriles.carrilObjetivo == 2;

      bool esSegundoGiroC26Derecha = carriles.esComandoC26 && sentidoVehiculo == DERECHA && carriles.carrilObjetivo == 1;
      bool esSegundoGiroC33Izquierda = carriles.esComandoC33 && sentidoVehiculo == IZQUIERDA && carriles.carrilObjetivo == 2;

      /*if (esSegundoGiroC33Izquierda || esSegundoGiroC26Derecha || esSegundoGiroC14Derecha || esSegundoGiroC21Izquierda ||  carriles.esComandoC07 && contadorLineas < 1 || carriles.esComandoC02 && contadorLineas < 1) {
        retrocesoCortoPostContragiro();
      }*/

      controlLateral.ladoControl = obtenerLadoControlPorCarril();
      cambiarEstado(MANTENIENDO_CARRIL);
      return (float)SERVO_CENTER;
      }
    // ─────────────────────────────────────────────────────────────────────────────────────

   // ── VERIFICACIÓN SHARP DENTRO DE LA FASE DE CORRECCIÓN ───────────────
    static unsigned long ultimoRetrocesoSharpCorreccion = 0;
    const unsigned long COOLDOWN_SHARP_CORRECCION = 300; //300 ms de pausa entre retrocesos

    float distSharpCorreccion = 999;
    if (carriles.carrilObjetivo == 1) {
      distSharpCorreccion = leerSensorSharp(SHARP_DIAGONAL_DER_PIN);
    } else {
      distSharpCorreccion = leerSensorSharp(SHARP_DIAGONAL_IZQ_PIN);
    }

    float distanciaLimite = 14.0; //14  Valor por defecto
    if (carriles.esComandoC20) {
      distanciaLimite = 14.0;
    } else if (carriles.esComandoC21) {
      distanciaLimite = 14.0;
    } else if (carriles.esComandoC14) {
      distanciaLimite = 14.0;
    }

    if (distSharpCorreccion <= distanciaLimite && distSharpCorreccion > 0 &&
        millis() - ultimoRetrocesoSharpCorreccion > COOLDOWN_SHARP_CORRECCION) {

      SerialBT.println("⚠️ SHARP en CORRECCIÓN: " + String(distSharpCorreccion, 1) + "cm. Retrocediendo...");

       unsigned long duracion = (carriles.esComandoC21 || carriles.esComandoC14) ? 400 : 400;
      retrocesoCompartidoPorObstaculo(duracion);
      delay(100);

      /*if (carriles.direccionGiro < 0) {
        direccion.write(SERVO_LEFT);
      } else {
        direccion.write(SERVO_RIGHT);
      }*/
      if (carriles.direccionGiro < 0) {
        direccion.write(SERVO_RIGHT);
      } else {
        direccion.write(SERVO_LEFT);
      }
      delay(300);
      avanzarMotor();
      ultimoRetrocesoSharpCorreccion = millis(); // marca el tiempo del último retroceso
            TIEMPO_AVANCE_MANTENIENDO_C14_IZQUIERDA = 300;
            TIEMPO_AVANCE_MANTENIENDO_C14_DERECHA = 300; // Aumentamos el tiempo tras retroceso por SHARP en corrección (independiente del sentido)
      SerialBT.println("▶️ SHARP CORRECCIÓN: Reanudando enderezado.");

      // Este retroceso ya resolvió el atasco: reseteamos el conteo de emergencia
      retornoAtascadoEnCurso = false;
      tiempoInicioRetornoAtascado = 0;
    }
    // ─────────────────────────────────────────────────────────────────────
   // Manejo de la interrupción por ToF Frontal → contragiro directo retrocediendo en algunos casos 
    /*if (sharpInterrupcionActiva && !pausaPorSharpRealizada) {
      SerialBT.println("↩️ TOF FRONTAL: Iniciando contragiro directo...");

      //detencion de motor y aplicacion de retroceso para 
      
              ultimoRetrocesoSharpCorreccion = millis(); // marca el tiempo del último retroceso
              TIEMPO_AVANCE_MANTENIENDO_C14_IZQUIERDA = 300;
              TIEMPO_AVANCE_MANTENIENDO_C14_DERECHA = 300; // Aumentamos el tiempo tras retroceso por TOF frontal (independiente del sentido)

        unsigned long duracion = (carriles.esComandoC21 || carriles.esComandoC14) ? 400 : 400;
        retrocesoCompartidoPorObstaculo(duracion);

        // Girar el servo y esperar a que llegue antes de avanzar
        if (carriles.carrilObjetivo == 1) {
          direccion.write(SERVO_LEFT); // Giro inicial fue derecha, contragiro es izquierda
        } else {
          direccion.write(SERVO_RIGHT);   // Giro inicial fue izquierda, contragiro es derecha
        }

        delay(300); // Pausa para que el servo alcance su posición antes de avanzar

        avanzarMotor(); // Reanudar avance ya con el servo en posición

        pausaPorSharpRealizada = true;*/

    if (sharpInterrupcionActiva && !pausaPorSharpRealizada) {
      SerialBT.println("↩️ TOF FRONTAL: Contragiro directo SIN retroceso...");

      ultimoRetrocesoSharpCorreccion = millis(); // cooldown para que el Sharp de corrección no dispare enseguida
      TIEMPO_AVANCE_MANTENIENDO_C14_IZQUIERDA = 300;
      TIEMPO_AVANCE_MANTENIENDO_C14_DERECHA = 300;

      // SIN RETROCESO: se frena y se pasa directo al enderezado (contragiro)
      detenerMotor();

      // Girar el servo hacia el contragiro y esperar a que llegue antes de avanzar
      if (carriles.carrilObjetivo == 1) {
        direccion.write(SERVO_LEFT);    // Giro inicial fue derecha, contragiro es izquierda
      } else {
        direccion.write(SERVO_RIGHT);   // Giro inicial fue izquierda, contragiro es derecha
      }

      delay(300); // Pausa para que el servo alcance su posición antes de avanzar

      avanzarMotor(); // Reanudar avance ya con el servo en el contragiro

      pausaPorSharpRealizada = true;
        tiempoInicioInterrupcion = millis();
        
        SerialBT.println("▶️ Contragiro activo. FASE A controlará por ángulo.");

        // Este retroceso ya resolvió el atasco: reseteamos el conteo de emergencia
        retornoAtascadoEnCurso = false;
        tiempoInicioRetornoAtascado = 0;
    }

    // 1. Calcular el ángulo objetivo final
    // Protección: durante los primeros 400ms tras la interrupción forzar el contragiro
    // para evitar que un ángulo casual mande a MANTENIENDO antes de cruzar.
    if (sharpInterrupcionActiva && (millis() - tiempoInicioInterrupcion < 400)) {
      return (carriles.carrilObjetivo == 1) ? SERVO_LEFT : SERVO_RIGHT;
    }

    if (!primerGiroRealizado) {
        anguloObjetivo = 0.0;
        if (!mensajeFase2Mostrado) {
            SerialBT.println(">>> PRIMER GIRO: Ángulo de enderezado forzado a 0.0°");
        }
    } else {
        calcularAnguloObjetivoMantener();
    }

    // 2. Verificar si ya hemos llegado al ángulo objetivo
    if (estaEnRangoAngulo(anguloZ, anguloObjetivo, 8.0)) {
      SerialBT.println(" Ángulo corregido. Pasando a mantener carril.");
      enFaseContragiro = false;
      controlLateral.ladoControl = obtenerLadoControlPorCarril();
      cambiarEstado(MANTENIENDO_CARRIL);
      return (float)SERVO_CENTER;
      // ── NUEVO: Detectar si se sobrepasó el ángulo objetivo ──────────────
      // En el contragiro: carril 1 gira a izquierda (anguloZ sube), carril 2 gira a derecha (anguloZ baja)
      float errorContragiro = calcularDiferenciaAngular(anguloZ, anguloObjetivo);
      bool sobrepasado = false;
      if (carriles.carrilObjetivo == 1 && errorContragiro < -5.0) {
          sobrepasado = true; // Iba subiendo y ya pasó por arriba del objetivo
      } else if (carriles.carrilObjetivo == 2 && errorContragiro > 5.0) {
          sobrepasado = true; // Iba bajando y ya pasó por debajo del objetivo
      }

      if (sobrepasado) {
          SerialBT.println("⚠️ Contragiro sobrepasado. Forzando MANTENIENDO_CARRIL.");
          cambiarEstado(MANTENIENDO_CARRIL);
          return (float)SERVO_CENTER;
      }
    } else {
        if (!mensajeFase2Mostrado) {
            SerialBT.println(" Fase 2: Ejecutando contra-giro hasta enderezar.");
            mensajeFase2Mostrado = true;
        }
        enFaseContragiro = true; // Activar velocidad lenta en el loop

        // --- CONTROL DE EMERGENCIA: llevamos cuánto tiempo intentando retornar sin éxito ---
        if (!retornoAtascadoEnCurso) {
          retornoAtascadoEnCurso = true;
          tiempoInicioRetornoAtascado = millis();
        }

        if (millis() - tiempoInicioRetornoAtascado > TIEMPO_LIMITE_RETORNO_ATASCADO) {
          SerialBT.println("🚨 EMERGENCIA: Más de 3s intentando retornar sin éxito. Forzando maniobra de desatasco.");

          // 1. Detener el carro
          detenerMotor();

          // 2. Enderezar la dirección
          direccion.write(SERVO_CENTER);
          delay(200);

          // 3. Retroceder 500ms actualizando el ángulo mientras tanto
          retrocederMotor();
          unsigned long tEmergencia0 = millis();
          while (millis() - tEmergencia0 < 500) {
            actualizarGiro();
            actualizarRampaMotor();
            delay(10);
          }
          detenerMotor();
          delay(100);

          // 4. Reintentar el retorno normal: volvemos a apuntar el servo hacia el contragiro
          if (carriles.carrilObjetivo == 1) {
            direccion.write(SERVO_LEFT);
          } else {
            direccion.write(SERVO_RIGHT);
          }
          delay(300);

          avanzarMotor();

          // Reseteamos el conteo para darle otros 3s de margen al nuevo intento
          retornoAtascadoEnCurso = false;
          tiempoInicioRetornoAtascado = 0;

          SerialBT.println("▶️ EMERGENCIA resuelta. Reintentando retorno normal.");
        }
        // ─────────────────────────────────────────────────────────────────────────────

        if (carriles.carrilObjetivo == 1) {
            return (float)SERVO_LEFT;    // carril 1: contragiro a la IZQUIERDA
        } else {
            return (float)SERVO_RIGHT;   // carril 2: contragiro a la DERECHA
        }
    }
  }
  // ===================== ======================

  // FASE B: GIRO INICIAL 
  else {
    if (!anguloCalculado) {
      if (!mensajeFase1Mostrado) {
        SerialBT.println("🔄 Fase 1: Ejecutando giro inicial por tiempo (cálculo único).");
        mensajeFase1Mostrado = true;
      }
    
       float servoAngleTemporal = 75.0f;// escala VIEJA (se convierte abajo)
 
      if (carriles.esComandoC15) {
        if (carriles.carrilObjetivo == 1) {
          // Para el primer tramo del C15 (hacia carril 1), el giro es a 40 grados.
          servoAngleTemporal = 30.0;//50
          SerialBT.println("   -> Comando C15 (Fase 1/2): Ángulo de giro FIJO a 40°.");
        } else { // carriles.carrilObjetivo == 2
          // Para el segundo tramo (hacia carril 2), el giro es a 140 grados.
          servoAngleTemporal = 120.0;//160
          SerialBT.println("   -> Comando C15 (Fase 2/2): Ángulo de giro FIJO a 140°.");
        }
      } else if (carriles.esComandoC20) { 
        if (carriles.carrilObjetivo == 2) {
          // Para el primer tramo del C20 (hacia carril 2, izq.), el giro es a 140 grados.
          servoAngleTemporal = 140.0;//140
          SerialBT.println("   -> Comando C20 (Fase 1/2): Ángulo de giro FIJO a 140°.");
        } else { // carriles.carrilObjetivo == 1
          // Para el segundo tramo (hacia carril 1, der.), el giro es a 40 grados.
          servoAngleTemporal = 0.0;//40
          SerialBT.println("   -> Comando C20 (Fase 2/2): Ángulo de giro FIJO a 40°.");
        }
      }   else if (carriles.esComandoC26) {
        if (carriles.carrilObjetivo == 2) {
          // Para el primer tramo del C26 (hacia carril 2, izq.), el giro es a 140 grados.
          servoAngleTemporal = 140.0;
          SerialBT.println("   -> Comando C26 (Fase 1/2): Ángulo de giro FIJO a 140°.");
        } else { // carriles.carrilObjetivo == 1
          // Para el segundo tramo (hacia carril 1, der.), el giro es a 20 grados.
          servoAngleTemporal = 0.0;
          SerialBT.println("   -> Comando C26 (Fase 2/2): Ángulo de giro FIJO a 0°.");
         }
       } else if (carriles.esComandoC33) {
        if (carriles.carrilObjetivo == 1) {
          // Para el primer tramo del C33 (hacia carril 1, dere.), el giro es a 40 grados.
          servoAngleTemporal = 20.0;
          SerialBT.println("   -> C33 Comando (Phase 1/2): Ángulo de giro FIJO a 30°");
        } else { // carriles.carrilObjetivo == 2
          //Para el primer tramo del C33 (hacia carril 2, izq.), el giro es a 160 grados.
          servoAngleTemporal = 180.0;
          SerialBT.println("   -> C33 Command (Phase 2/2): Ángulo de giro FIJO a 180°.");
          }
        }
       else if (carriles.esComandoC21) {
        SerialBT.println("   -> Comando C21: Aplicando ángulo de giro reducido.");
        if (carriles.direccionGiro < 0) { // Giro a la IZQUIERDA
          servoAngleTemporal = 180.0; // Ángulo solicitado para la izquierda
          SerialBT.println("       Giro a la IZQUIERDA a 180°.");
        } else { // Giro a la DERECHA
          servoAngleTemporal = 0.0; // Ángulo solicitado para la derecha
          SerialBT.println("       Giro a la DERECHA a 0°.");
        }
      }  else if (carriles.esComandoC14) {
        SerialBT.println("   -> Comando C14: Aplicando ángulo de giro reducido.");
        if (carriles.direccionGiro < 0) { // Giro a la IZQUIERDA
          servoAngleTemporal = 180.0; // Ángulo solicitado para la izquierda
          SerialBT.println("       Giro a la IZQUIERDA a 170°.");
        } else { // Giro a la DERECHA
          servoAngleTemporal = 0.0; // Ángulo solicitado para la derecha
          SerialBT.println("       Giro a la DERECHA a °0.");
        }
      } else if (carriles.esComandoC27) {
        if (carriles.carrilObjetivo == 1) {
          // Para el primer tramo del C27 (hacia carril 1, dere.), el giro es a 20 grados.
          servoAngleTemporal = 0.0;
          SerialBT.println("   -> Comando C27 (Fase 1/2): Ángulo de giro FIJO a 0°.");
        } else { // carriles.carrilObjetivo == 2
          // Para el segundo tramo (hacia carril 2, der.), el giro es a 160 grados.
          servoAngleTemporal = 160.0;
          SerialBT.println("   -> Comando C27 (Fase 2/2): Ángulo de giro FIJO a 160°.");
        } 
      } else if (carriles.esComandoC32) {
        if (carriles.carrilObjetivo == 2) {
          // Para el primer tramo del C32 (hacia carril 2, izq.), el giro es a 160 grados.
          servoAngleTemporal = 170.0;
          SerialBT.println("   -> Comando C32 (Fase 1/2): Ángulo de giro FIJO a 170°.");
        } else { // carriles.carrilObjetivo == 1
          // Para el segundo tramo (hacia carril 1, der.), el giro es a 20 grados.
          servoAngleTemporal = 0.0;
          SerialBT.println("   -> Comando C32 (Fase 2/2): Ángulo de giro FIJO a 0°.");
         }
       }
       else if (carriles.esComandoC02) {
        if (carriles.carrilObjetivo == 1) {
          // rojo derecha
          servoAngleTemporal = 0.0;
          SerialBT.println("   -> C02 Comando : Ángulo de giro FIJO a 0°");
          }
        }
        else if (carriles.esComandoC08 ||carriles.esComandoC36 ) {
        if (carriles.carrilObjetivo == 1) {
          // rojo izquierda
          servoAngleTemporal = 20.0;
          SerialBT.println("   -> C08 Comando : Ángulo de giro FIJO a 20°");
          } 
        }
        else if (carriles.esComandoC01) {
        if (carriles.carrilObjetivo == 2) {
          // rojo izquierda
          servoAngleTemporal = 170.0;
          SerialBT.println("   -> C01 Comando : Ángulo de giro FIJO a 170°");
          } 
        }else if (carriles.esComandoC07) {
        if (carriles.carrilObjetivo == 2) {
          // rojo izquierda
          servoAngleTemporal = 170.0;
          SerialBT.println("   -> C07 Comando : Ángulo de giro FIJO a 170°");
          } 
        }
      else { // Lógica original para todos los demás comandos
          bool calcularAnguloVariable = (carriles.carrilObjetivo == 1 && sentidoVehiculo == DERECHA) ||
                                        (carriles.carrilObjetivo == 2 && sentidoVehiculo == IZQUIERDA); 
          
          if (calcularAnguloVariable) {
              SerialBT.println("--- [Ángulo Variable Activo] ---");
              float distanciaLateral = -1.0; 
              if (carriles.carrilObjetivo == 1) {
                  distanciaLateral = leerToF('I');
                  SerialBT.print("Carril Objetivo 1: Leyendo ToF Izquierdo -> Distancia: ");
                  SerialBT.println(distanciaLateral);
              } else {
                  distanciaLateral = leerToF('D'); 
                  SerialBT.print("Carril Objetivo 2: Leyendo ToF Derecho -> Distancia: ");
                  SerialBT.println(distanciaLateral);
              }
              
              if (distanciaLateral > 0 && distanciaLateral < 100) {
                  float distanciaRestringida = constrain(distanciaLateral, 32.0, 50.0);
                  float ajuste = map(distanciaRestringida, 32.0, 50.0, 0.0, 20.0); 
                  
                  SerialBT.print("Distancia acotada (32-50): ");
                  SerialBT.print(distanciaRestringida);
                  SerialBT.print(" | Ajuste calculado por map: ");
                  SerialBT.println(ajuste);

                  if (carriles.direccionGiro < 0) { 
                      servoAngleTemporal = 170.0 - ajuste; 
                      SerialBT.print("Giro Izquierda. Servo Temporal (170 - ajuste): ");
                      SerialBT.println(servoAngleTemporal);
                  } else { 
                      servoAngleTemporal = 10.0 + ajuste; 
                      SerialBT.print("Giro Derecha. Servo Temporal (10 + ajuste): ");
                      SerialBT.println(servoAngleTemporal);
                  }
              } else {
                  SerialBT.print("Distancia ToF fuera de rango (0-100). Valor obtenido: ");
                  SerialBT.println(distanciaLateral);
                  
                  if (carriles.direccionGiro < 0) {
                      servoAngleTemporal = 170.0;
                      SerialBT.println("Aplicando giro fijo Izquierda: 170.0");
                  } else {
                      servoAngleTemporal = 10.0;
                      SerialBT.println("Aplicando giro fijo Derecha: 10.0");
                  }
              }
          } else {
              SerialBT.println("--- [Giro Fijo Activo] (No cumple condiciones de carril/sentido) ---");
              if (carriles.direccionGiro < 0) {
                  servoAngleTemporal = 170.0;
                  SerialBT.println("Giro Fijo Izquierda: 170.0");
              } else {
                  servoAngleTemporal = 10.0;
                  SerialBT.println("Giro Fijo Derecha: 10.0");
              }
          }
      }

      anguloGiroFijo = constrain(servoDesdeEscalaVieja(servoAngleTemporal), (float)SERVO_MIN, (float)SERVO_MAX);
      anguloCalculado = true;
      
      SerialBT.print(">>> ANGULO FINAL ENVIADO AL SERVO: ");
      SerialBT.println(anguloGiroFijo);
      SerialBT.println("----------------------------------------");

      // ── NUEVO: Girar el servo ANTES de arrancar el motor ──────────────
      // Se escribe el ángulo aquí mismo y se espera a que el servo llegue
      // a su posición antes de que el loop principal active avanzarMotor().
      direccion.write((int)anguloGiroFijo);
      delay(400); // 300ms le da tiempo al servo de posicionarse. Puedes ajustar entre 200-500ms.
      // ──────────────────────────────────────────────────────────────────
    }
    calcularAnguloObjetivoMantener();
   // ── CONTROL DE DESVIACIÓN MÁXIMA DE 45° ─────────────────────────────
    // Calcular cuánto nos hemos desviado desde que inició el giro
    float desviacionActual = 0.0;
    if (carriles.carrilObjetivo == 1) {
      // Giro a la derecha: el ángulo Z baja (se hace negativo o sube hacia 360)
      desviacionActual = calcularDiferenciaAngular(anguloZ, anguloObjetivo);
      desviacionActual = abs(desviacionActual);
    } else {
      // Giro a la izquierda: el ángulo Z sube
      desviacionActual = calcularDiferenciaAngular(anguloZ, anguloObjetivo);
      desviacionActual = abs(desviacionActual);
    }
    SerialBT.print("angulo objetivo por conteo: ");
    SerialBT.print(anguloObjetivo);
    SerialBT.println(" grados************ maximo alcanzado colocando servo 90");
     SerialBT.print("desviacionActual de : ");
     SerialBT.print(desviacionActual);
     SerialBT.println("grados");
     
    
    float MAX_DESVIACION_GIRO = 30.0;//45
     if (carriles.esComandoC07 ||(carriles.esComandoC14 && carriles.carrilObjetivo == 2)) { 
      MAX_DESVIACION_GIRO = 25.0;//30.0;
      SerialBT.print("desviasion por c07 ");

     }if (carriles.esComandoC02 ) { 
      MAX_DESVIACION_GIRO = 30.0;//35
      SerialBT.print("desviasion por c07 ");

     }if (carriles.esComandoC08 ) { 
      MAX_DESVIACION_GIRO = 15.0;
      SerialBT.print("desviasion por c07 ");

     }else if (carriles.esComandoC20|| carriles.esComandoC15 ) { 
      MAX_DESVIACION_GIRO = 15.0;
     }else if(carriles.esComandoC26 && carriles.carrilObjetivo == 1 || carriles.esComandoC32 && carriles.carrilObjetivo == 1){
      MAX_DESVIACION_GIRO = 45.0;
     }else if(carriles.esComandoC32 && carriles.carrilObjetivo == 2){
      MAX_DESVIACION_GIRO = 30.0;
     }else if(carriles.esComandoC26 && carriles.carrilObjetivo == 2){
      MAX_DESVIACION_GIRO = 20.0;
     }else if(carriles.esComandoC27 && carriles.carrilObjetivo == 2){
      MAX_DESVIACION_GIRO = 25.0;
     }else if(carriles.esComandoC14 && carriles.carrilObjetivo == 1){
      MAX_DESVIACION_GIRO = 45.0;//60.0;
     }else if(carriles.esComandoC21 && carriles.carrilObjetivo == 2 && sentidoVehiculo == IZQUIERDA){
      MAX_DESVIACION_GIRO = 45.0;//40
     } else if(carriles.esComandoC21 && carriles.carrilObjetivo == 2 ){
      MAX_DESVIACION_GIRO = 45.0;//40
     }/*else if (carriles.esComandoC21 && carriles.carrilObjetivo == 1) {
      // IZQUIERDA: carril 1 está lejos, necesita ángulo grande
      if (sentidoVehiculo == IZQUIERDA) {
        MAX_DESVIACION_GIRO = 45.0; // Era 35 por defecto — ESTE ERA EL BUG
      } else {
        MAX_DESVIACION_GIRO = 35.0; // DERECHA: mantiene lo que ya funcionaba
      }
    } else if (carriles.esComandoC21 && carriles.carrilObjetivo == 2) {
      MAX_DESVIACION_GIRO = 45.0;
    }*/
    unsigned long ahoraA = millis();
    static unsigned long contadorLoop = 0;
    contadorLoop++;

    if (desviacionActual >= MAX_DESVIACION_GIRO) {
       if (ahoraA - ultimaImpresion_maxAngulo >= INTERVALO_maxAngulo) {
          ultimaImpresion_maxAngulo = ahoraA;

          // Un solo print atómico: no se puede mezclar con otra iteración
          String log = "*** LOOP#" + String(contadorLoop) +
                        " | t=" + String(ahoraA) +
                        " | MAX=" + String(MAX_DESVIACION_GIRO, 2) +
                        " | desviacionActual=" + String(desviacionActual, 2) +
                        " | anguloZ=" + String(anguloZ, 2) +
                        " | anguloObjetivo=" + String(anguloObjetivo, 2) +
                        " ->  CENTRANDO SERVO (centro)";
          SerialBT.println(log);
        }
      return (float)SERVO_CENTER;
    }
    // ────────────────────────────────────────────────────────────────────

    return anguloGiroFijo;
  }
}

float procesarEstadoEnderezandoCarril() {

  
  if (abs(anguloZ) <= carriles.toleranciaAngulo) {
    SerialBT.println("📐 Vehículo enderezado, iniciando mantenimiento de carril");
    cambiarEstado(MANTENIENDO_CARRIL);
    return (float)SERVO_CENTER;
  }
  
  // Control proporcional para enderezar
  float correccionAngulo = -anguloZ * 3.0;  // 2.0Ganancia proporcional
  correccionAngulo = constrain(correccionAngulo, -(float)SERVO_SPAN_DER, (float)SERVO_SPAN_IZQ);
  return constrain((float)SERVO_CENTER - correccionAngulo, (float)SERVO_MIN, (float)SERVO_MAX);

}
float obtenerDistanciaPorSentido(char lado) {
  float lectura;
  if (lado == 'I') {
    lectura = leerToF('I');
    dbgDistCruda = lectura;   // <-- NUEVO
    return controlLateral.filtroIzquierda.filtrar(lectura);
  } else {
    lectura = leerToF('D');
    dbgDistCruda = lectura;   // <-- NUEVO
    return controlLateral.filtroDerecha.filtrar(lectura);
  }
}
// ==================== v96: ¿YA SE PUEDE HACER EL CAMBIO DE CARRIL? ====================
// - Bandera apagada: devuelve la condición por tiempo de siempre (nada cambia).
// - Bandera activa: ignora el tiempo y espera a detectar "larga -> corta -> larga" con el
//   sensor lateral CONTRARIO al carril objetivo (carril 1 -> izquierdo, carril 2 -> derecho).
//   Si pasan MO_OBJ_TIMEOUT_DETECCION_MS sin detectarlo, retrocede y luego libera el cambio.

// ==================== v96: ¿YA SE PUEDE HACER EL CAMBIO DE CARRIL? (NO BLOQUEANTE) ====================
// - Bandera apagada: devuelve la condición por tiempo de siempre (nada cambia).
// - Bandera activa: ignora el tiempo y espera "libre -> objeto (<25cm) -> libre" con el sensor
//   lateral CONTRARIO al carril objetivo (carril 1 -> izquierdo, carril 2 -> derecho).
//   Si pasan MO_OBJ_TIMEOUT_DETECCION_MS sin detectarlo, inicia un retroceso por millis()
//   (lo ejecuta procesarMantenimientoPared + el loop) y al terminar libera el cambio.
void mo_reiniciarDeteccionObjeto() {
    mo_objOffsetTimeoutMs = 0;
    mo_objUltimoDebug = 0;
  mo_objUltimaMuestra = 0;
  mo_objNumMuestras = 0;
  mo_objMinVisto = 999.0;
  mo_objMaxVisto = 0.0;

  mo_objFase = 0;
  mo_objCont = 0;
  mo_objTiempoPaso = 0;
  mo_objRetrocediendo = false;
  mo_objRetrocesoTerminado = false;
}

// v96: durante la secuencia de retroceso, primero detenido (pausa) y luego en reversa.
// La llama el loop() en cada ciclo mientras mo_objRetrocediendo sea true.
void mo_objAplicarMotorRetroceso() {
  if (millis() - mo_objInicioRetroceso < mo_objPausaMs) {
    detenerMotor();
  } else {
    retrocederMotor();
  }
}
// v96: true mientras se busca el objeto (antes de confirmar que pasó y sin secuencia de retroceso)
bool mo_objVentanaDeteccionActiva() {
  return mo_confirmarObjetoActivo && !mo_objRetrocediendo && !mo_objRetrocesoTerminado &&
         mo_objFase < 3 && !enFaseContragiro;
}

// v96: pausa inicial con el motor detenido y luego avance a velocidadMedia.
// Solo se usa en el primer avance del comando doble (mientras busca el objeto).
// Al terminar la ventana (objeto pasado o timeout) el loop vuelve solo a velocidadBase.
// Solo toca el motor: el servo sigue como siempre (ángulo si no hay pared, pared si ya hay referencia).

void mo_objAplicarMotorDeteccion() {
  static unsigned long ultimoInicioAvisado = 0;
  if (ultimoInicioAvisado != carriles.tiempoInicioEstado) {
    ultimoInicioAvisado = carriles.tiempoInicioEstado;
    SerialBT.println("⏸️ Enderezado: PAUSA de " + String(MO_OBJ_PAUSA_INICIAL_MS) + "ms, luego avance a velocidadMedia (" +
                     String(velocidadMedia) + ") leyendo el sensor");
  }
  if (millis() - carriles.tiempoInicioEstado < MO_OBJ_PAUSA_INICIAL_MS) {
    detenerMotor();
    return;
  }
  avanzarMotor(velocidadMedia);
}

// v96: se llama tras una emergencia SHARP en MANTENIENDO_CARRIL (retroceso bloqueante).
// - Suma la duración de la emergencia al timeout: ese tiempo no cuenta como búsqueda del objeto.
// - Descarta lo visto durante el retroceso (el sensor barría paredes/esquinas) y rearma la detección.
void mo_objNotificarEmergencia(unsigned long duracionMs) {
  if (!mo_confirmarObjetoActivo) return;
  mo_objOffsetTimeoutMs += duracionMs;
  if (MO_OBJ_REINICIAR_FASE_EN_EMERGENCIA && mo_objFase < 3) {
    mo_objFase = 0;
    mo_objCont = 0;
  }
  mo_objUltimaMuestra = 0; // evita un Δ gigante en la siguiente línea de debug
  SerialBT.println("🚨 Emergencia SHARP durante la búsqueda: +" + String(duracionMs) +
                   "ms al timeout (acumulado " + String(mo_objOffsetTimeoutMs) + "ms)" +
                   String(MO_OBJ_REINICIAR_FASE_EN_EMERGENCIA ? " | detección REINICIADA" : ""));
}

// ==================== v96: MUESTREO DEL OBJETO (se llama en CADA ciclo de MANTENIENDO_CARRIL) ====================
// Lee el sensor opuesto al carril objetivo UNA sola vez por ciclo y reutiliza el valor (lo invalida procesarMantenimientoPared al inicio de cada ciclo)
float leerToFOpuestoCompartido() {
  if (!ent_lecturaOpuestaValida) {
    char ladoOp = (carriles.carrilObjetivo == 1) ? 'I' : 'D';
    ent_lecturaOpuesta = leerToF(ladoOp);
    ent_lecturaOpuestaValida = true;
  }
  return ent_lecturaOpuesta;
}

// Corre ANTES de los return tempranos de procesarMantenimientoPared (p. ej. sin referencia lateral),
// así lee el sensor contrario desde el primer ciclo, también durante la aproximación.
void mo_actualizarDeteccionObjeto() {
  if (!mo_confirmarObjetoActivo) return;
  if (mo_objRetrocediendo || mo_objRetrocesoTerminado) return; // secuencia de parada/retroceso en curso

  char ladoOpuesto = (carriles.carrilObjetivo == 1) ? 'I' : 'D';
  float lecturaCruda = leerToFOpuestoCompartido(); // v101: lectura compartida con la entrada al carril
  float lectura = lecturaCruda;
  if (lectura > MO_OBJ_DIST_MAX_CM) lectura = MO_OBJ_DIST_MAX_CM;
  if (lectura < 0) lectura = 0;

  // ── estadísticas ──
  mo_objNumMuestras++;
  unsigned long ahoraObj = millis();
  unsigned long dtMuestra = (mo_objUltimaMuestra == 0) ? 0 : (ahoraObj - mo_objUltimaMuestra);
  mo_objUltimaMuestra = ahoraObj;
  if (lectura < mo_objMinVisto) mo_objMinVisto = lectura;
  if (lectura > mo_objMaxVisto) mo_objMaxVisto = lectura;

  // ── línea visual por muestra: 🟢 = libre (>=25cm) | 🔴 = corta (<25cm) ──
  if (ahoraObj - mo_objUltimoDebug >= MO_OBJ_DEBUG_INTERVALO_MS) {
    mo_objUltimoDebug = ahoraObj;
    bool corta = lectura < MO_OBJ_DIST_OBJETO_CM;
    int barras = constrain((int)(lectura / 5.0f), 0, 10);   // 1 bloque = 5cm
    String barra = "";
    for (int i = 0; i < 10; i++) barra += (i < barras) ? "█" : "░";
    String distTxt = (lecturaCruda >= MO_OBJ_DIST_MAX_CM) ? String("lejos") : (String(lecturaCruda, 1) + "cm");
    SerialBT.println(String(corta ? "🔴" : "🟢") + " [" + barra + "] " + distTxt +
                     " | ToF-" + String(ladoOpuesto) +
                     " | fase " + String(mo_objFase) + " c" + String(mo_objCont) +
                     " | t=" + String(ahoraObj - carriles.tiempoInicioEstado) + "ms" +
                     " | Δ" + String(dtMuestra) + "ms #" + String(mo_objNumMuestras));
  }

  switch (mo_objFase) {
    case 0: // Esperando zona libre
      if (lectura >= MO_OBJ_DIST_OBJETO_CM) {
        if (++mo_objCont >= MO_OBJ_MUESTRAS_LIBRE) {
          mo_objFase = 1; mo_objCont = 0;
          SerialBT.println("🟢🟢🟢 DETECCIÓN ARMADA (zona libre) 🟢🟢🟢");
        }
      } else {
        mo_objCont = 0;
      }
      break;

    case 1: // Esperando objeto
      if (lectura < MO_OBJ_DIST_OBJETO_CM) {
        if (++mo_objCont >= MO_OBJ_MUESTRAS_OBJETO) {
          mo_objFase = 2; mo_objCont = 0;
          SerialBT.println("🚨🚨🚨 ¡OBJETO DETECTADO! (" + String(lectura, 1) + "cm) 🚨🚨🚨");
        } else {
          SerialBT.println("🟠 lectura corta " + String(mo_objCont) + "/" + String(MO_OBJ_MUESTRAS_OBJETO) + " (falta confirmar)");
        }
      } else {
        if (mo_objCont > 0) {
          SerialBT.println("⚠️⚠️ OBJETO PERDIDO: solo " + String(mo_objCont) + " lectura(s) corta(s), se necesitan " +
                           String(MO_OBJ_MUESTRAS_OBJETO) + " ⚠️⚠️");
        }
        mo_objCont = 0;
      }
      break;

    case 2: // Objeto al lado, esperando que pase
      if (lectura >= MO_OBJ_DIST_OBJETO_CM + MO_OBJ_HISTERESIS_CM) {
        if (++mo_objCont >= MO_OBJ_MUESTRAS_PASO) {
          mo_objFase = 3;
          mo_objTiempoPaso = millis();
          SerialBT.println("✅✅✅ ¡OBJETO PASADO! ✅✅✅");
        }
      } else {
        mo_objCont = 0;
      }
      break;
  }

  // Paso confirmado: parar y retroceder (o liberar de inmediato si ambos tiempos son 0)
  if (mo_objFase == 3) {
    if (millis() - mo_objTiempoPaso >= MO_OBJ_AVANCE_EXTRA_MS) {
      if (MO_OBJ_PAUSA_ANTES_RETROCESO_MS + MO_OBJ_RETROCESO_PASO_MS == 0) {
        SerialBT.println("➡️ Iniciando cambio de carril tras pasar el objeto.");
        mo_objRetrocesoTerminado = true; // mo_cambioCarrilListo() lo liberará
      } else {
        SerialBT.println("🛑 Objeto pasado: DETENIENDO " + String(MO_OBJ_PAUSA_ANTES_RETROCESO_MS) + "ms" +
                         (MO_OBJ_RETROCESO_PASO_MS > 0 ? String(" + RETROCESO ") + String(MO_OBJ_RETROCESO_PASO_MS) + "ms" : String("")) +
                         " antes del cambio de carril");
        mo_objPausaMs = MO_OBJ_PAUSA_ANTES_RETROCESO_MS;
        mo_objDuracionTotalMs = MO_OBJ_PAUSA_ANTES_RETROCESO_MS + MO_OBJ_RETROCESO_PASO_MS;
        mo_objRetrocediendo = true;
        mo_objInicioRetroceso = millis();
      }
    }
    return;
  }

  // Timeout: no se detectó el objeto -> retroceso (no bloqueante)
if (millis() - carriles.tiempoInicioEstado >= MO_OBJ_TIMEOUT_DETECCION_MS + MO_OBJ_PAUSA_INICIAL_MS + mo_objOffsetTimeoutMs + (omitioGiro ? MO_OBJ_TIMEOUT_EXTRA_OMITIDO_MS : 0UL)) {
       SerialBT.println("❌❌❌ NO SE DETECTÓ EL OBJETO ❌❌❌");
    SerialBT.println("📊 fase_final=" + String(mo_objFase) + " muestras=" + String(mo_objNumMuestras) +
                     " min_visto=" + String(mo_objMinVisto, 1) + "cm max_visto=" + String(mo_objMaxVisto, 1) +
                     "cm (umbral=" + String(MO_OBJ_DIST_OBJETO_CM, 0) + "cm)");
    SerialBT.println("⏪ Retrocediendo " + String(MO_OBJ_RETROCESO_TIMEOUT_MS) + "ms antes del cambio de carril...");
    mo_objPausaMs = 0;
    mo_objDuracionTotalMs = MO_OBJ_RETROCESO_TIMEOUT_MS;
    mo_objRetrocediendo = true;
    mo_objInicioRetroceso = millis();
  }
}

// ¿YA SE PUEDE HACER EL CAMBIO DE CARRIL? (se llama desde los 8 bloques de comandos dobles)
// - Bandera apagada: condición por tiempo de siempre.
// - Bandera activa: solo libera cuando la secuencia (objeto pasado / timeout + retroceso) terminó.
bool mo_cambioCarrilListo(bool condicionPorTiempo) {
  if (!mo_confirmarObjetoActivo) return condicionPorTiempo;

  // v100: avisa UNA vez por tramo que el tiempo ya se cumplió pero se ignora
  static unsigned long tramoAvisado = 0;
  if (condicionPorTiempo && tramoAvisado != carriles.tiempoInicioEstado) {
    tramoAvisado = carriles.tiempoInicioEstado;
    SerialBT.println("🕒 Tiempo cumplido pero IGNORADO -> espero al objeto (fase " + String(mo_objFase) +
                     " | 0=zona libre 1=buscando 2=objeto al lado 3=pasó)");
  }

  if (mo_objRetrocesoTerminado) {
    SerialBT.println("↩️ Secuencia terminada -> haciendo cambio de carril");
    mo_confirmarObjetoActivo = false;
    mo_reiniciarDeteccionObjeto();
    return true;
  }
  return false; // el muestreo y la decisión los hace mo_actualizarDeteccionObjeto() en cada ciclo
}

// ==================== v101: ENTRADA AL CARRIL INTERIOR ====================
// ¿Ya entró al carril? (sensor del carril objetivo con lectura válida y cercana, o el objeto ya pasó)
void ent_evaluarEntrada(float distanciaControl, int invalidasLado) {
  if (!ent_activa) return;

  if (millis() - ent_tInicio > ENT_TIMEOUT_MS) {
    ent_activa = false;
    SerialBT.println("⏱️ ENTRADA: timeout, se vuelve al control normal.");
    return;
  }

  if (mo_confirmarObjetoActivo && (mo_objFase >= 3 || mo_objRetrocediendo || mo_objRetrocesoTerminado)) {
    ent_activa = false;
    SerialBT.println("✅ ENTRADA: objeto pasado, fin de la confirmación.");
    return;
  }

  if (invalidasLado == 0 && distanciaControl > 0 && distanciaControl <= ENT_DIST_ENTRO_CM) {
    if (++ent_contEntro >= ENT_MUESTRAS_ENTRO) {
      ent_activa = false;
      SerialBT.println("✅ ENTRADA: sensor del carril ve la pared a " + String(distanciaControl, 1) + "cm. Control normal.");
    }
  } else {
    ent_contEntro = 0;
  }
}

// Offset de rumbo (grados) según el sensor OPUESTO. Positivo = girar a la izquierda (anguloZ sube).
// 0 si la fase no está activa, si está dentro de 60-80cm o si la lectura no es confiable.
float ent_calcularOffsetAngulo() {
  if (!ent_activa) return 0.0f;

  char ladoOpuesto = (carriles.carrilObjetivo == 1) ? 'I' : 'D';
  float d = leerToFOpuestoCompartido();
  ent_ultimaDistOpuesta = d;
  if (d <= 0 || d >= 150.0) return 0.0f;

  float exceso = 0.0f;
  if (d > ENT_DIST_MAX_OPUESTA_CM)      exceso = d - ENT_DIST_MAX_OPUESTA_CM;  // lejos de la pared opuesta
  else if (d < ENT_DIST_MIN_OPUESTA_CM) exceso = d - ENT_DIST_MIN_OPUESTA_CM;  // pegado a la pared opuesta

  // carril 2 (opuesto = derecho): lejos -> girar a la derecha (offset -) | cerca -> a la izquierda (+)
  // carril 1 (opuesto = izquierdo): lejos -> girar a la izquierda (+)    | cerca -> a la derecha (-)
  float signo = (carriles.carrilObjetivo == 2) ? -1.0f : 1.0f;
  float offset = constrain(signo * ENT_KP_GRADOS_POR_CM * exceso, -ENT_MAX_OFFSET_GRADOS, ENT_MAX_OFFSET_GRADOS);

  if (millis() - ent_ultimoPrint > 300) {
    ent_ultimoPrint = millis();
    SerialBT.println("🎯 ENTRADA | opuesto(" + String(ladoOpuesto) + ")=" + String(d, 1) +
                     "cm | rango " + String(ENT_DIST_MIN_OPUESTA_CM, 0) + "-" + String(ENT_DIST_MAX_OPUESTA_CM, 0) +
                     " | offset rumbo=" + String(offset, 1) + "°");
  }
  return offset;
}

float procesarMantenimientoPared() {
      enFaseContragiro = false; // Resetear para que al entrar en manteniendo vaya a velicidad normal
     ent_lecturaOpuestaValida = false; // v101: nuevo ciclo -> la próxima lectura del sensor opuesto será fresca
    // ── v96: retroceso NO bloqueante por objeto no detectado ──
  // Mientras retrocede se salta todo lo demás (frontal, Sharp, PID) y solo sigue el ángulo objetivo.
  // El motor en reversa lo pone el loop (ver CAMBIO I). actualizarGiro() sigue corriendo en cada ciclo.
  if (mo_objRetrocediendo) {
    if (millis() - mo_objInicioRetroceso >= mo_objDuracionTotalMs) {
      detenerMotor();
      mo_objRetrocediendo = false;
      mo_objRetrocesoTerminado = true; // este mismo ciclo sigue normal y el helper liberará el cambio de carril
    } else {
      return (float)mo_servoRumboSuaveValor(anguloObjetivo);
    }
  }
    // ── v96: muestreo del objeto en CADA ciclo, antes de cualquier return temprano ──
  mo_actualizarDeteccionObjeto();
  // ── 1. VERIFICACIÓN FRONTAL con confirmación anti-spike ──────────────────
  float distanciaFrontal = 999.0; // v97: sin lectura (su único uso está comentado). Ahorra ~50ms por ciclo
  //  float distanciaFrontal = leerToF('F');
  /*if (distanciaFrontal > 0 && distanciaFrontal < 15.0) {
    // Lectura sospechosa: iniciar o continuar la ventana de confirmación
    if (!deteccionFrontalActiva) {
      deteccionFrontalActiva = true;
      tiempoInicioDeteccionFrontal = millis();
      SerialBT.println("⚠️ TOF Frontal: posible obstáculo (" + String(distanciaFrontal, 1) + "cm). Confirmando...");
    }

    // ¿Llevan suficiente tiempo siendo positivas las lecturas?
    if (millis() - tiempoInicioDeteccionFrontal >= TIEMPO_CONFIRMACION_FRONTAL) {
      // ── OBSTÁCULO CONFIRMADO ────────────────────────────────────────────
      deteccionFrontalActiva = false; // Resetear para la próxima vez
      SerialBT.println("!!! OBSTÁCULO FRONTAL (" + String(distanciaFrontal, 1) + "cm) !!!");
      detenerMotor();
      direccion.write(SERVO_CENTER);
      delay(100);

      // Retroceder 
      SerialBT.println("⏪ Retrocediendo por obstáculo frontal...");
      retrocederMotor();
      unsigned long t0 = millis();
      while (millis() - t0 < 700) { actualizarGiro(); actualizarRampaMotor(); delay(10); }
      detenerMotor(); delay(100);

      // Evaluar distancia lateral post-retroceso
      {
        const float UMBRAL_LEJOS_PARED = 1.0;
        float distLateral = 999.0;
        String ladoStr = "";
        if (carriles.carrilObjetivo == 1) {
          distLateral = leerToF('D');
          ladoStr = "DER";
        } else {
          distLateral = leerToF('I');
          ladoStr = "IZQ";
        }
        float distanciaObjetivoCarril = obtenerDistanciaObjetivo();

        SerialBT.print("📏 Post-retroceso | Sensor ");
        SerialBT.print(ladoStr);
        SerialBT.print(": ");
        SerialBT.print(distLateral, 1);
        SerialBT.print("cm | Objetivo: ");
        SerialBT.print(distanciaObjetivoCarril, 1);
        SerialBT.println("cm");

        bool estaLejos = (distLateral > (distanciaObjetivoCarril + UMBRAL_LEJOS_PARED)) ||
                         (distLateral <= 0 || distLateral > 150);

        if (estaLejos) {
          SerialBT.println("🔄 Está LEJOS de la pared. Iniciando giro evasivo hacia el carril...");
          cambiarEstado(GIRANDO_HACIA_CARRIL);
        } else {
          SerialBT.println("✅ Está CERCA de la pared. Solo retroceso fue suficiente. Volviendo a MANTENIENDO_CARRIL.");
          avanzarMotor();
          cambiarEstado(MANTENIENDO_CARRIL);
        }
      }
      return SERVO_SIN_COMANDO;
    }
    // Si aún no se cumplió el tiempo, no hacer nada todavía → retorna el PID normalmente

  } else {
    // La lectura volvió a ser normal: resetear el contador (era un spike)
    if (deteccionFrontalActiva) {
      SerialBT.println("✅ TOF Frontal: falsa alarma, lectura normalizada (" + String(distanciaFrontal, 1) + "cm). Spike ignorado.");
      deteccionFrontalActiva = false;
    }
  }*/

  // ── 2. EMERGENCIA SHARP ────────────────────────────────────────────────────
  if (carriles.carrilObjetivo == 1) {
    float d = leerSensorSharp(SHARP_DIAGONAL_DER_PIN);
    if (d > 0 && d < DISTANCIA_EMERGENCIA_SHARP_PARED) {
      detenerMotor();
      direccion.write(SERVO_RIGHT); 
      delay(100);
      retrocederMotor();
      unsigned long duracionRetroceso = (carriles.esComandoC14 && sentidoVehiculo == DERECHA)
                                            ? 500UL
                                            : TIEMPO_RETROCESO_SHARP_PARED;
      unsigned long t0 = millis();
      while (millis() - t0 < duracionRetroceso) { 
        actualizarGiro(); 
        actualizarRampaMotor();
        delay(10);
       }
      detenerMotor(); 
      direccion.write(SERVO_CENTER); 
      avanzarMotor();
      mo_objNotificarEmergencia(100UL + duracionRetroceso); // v96: delay(100) + retroceso
      return ANGULO_CORRECCION_SHARP_IZQ;
      }
  } else {
    float d = leerSensorSharp(SHARP_DIAGONAL_IZQ_PIN);
    if (d > 0 && d < DISTANCIA_EMERGENCIA_SHARP_PARED) {
      SerialBT.println("🚨 EMERGENCIA SHARP en MANTENIENDO (carril " + String(carriles.carrilObjetivo) +
                       ") d=" + String(d, 1) + "cm yaw=" + String(anguloZ, 1) +
                       " obj=" + String(anguloObjetivo, 1) +
                       " aproxLimitada=" + String(mo_aproximacionLimitadaActiva));
      detenerMotor(); direccion.write(SERVO_LEFT); delay(100);
      retrocederMotor();
      unsigned long duracionRetroceso = (carriles.esComandoC14 && sentidoVehiculo == DERECHA)
                                            ? 500UL
                                            : TIEMPO_RETROCESO_SHARP_PARED;
      unsigned long t0 = millis();
      while (millis() - t0 < duracionRetroceso) { actualizarGiro(); actualizarRampaMotor(); delay(10); }
      detenerMotor(); direccion.write(SERVO_CENTER); avanzarMotor();
      mo_objNotificarEmergencia(100UL + duracionRetroceso); // v96: delay(100) + retroceso
      return ANGULO_CORRECCION_SHARP_DER;
    }
  }

  // ── 3. LECTURA SENSOR LATERAL ──────────────────────────────────────────────
  // Carril 1 → sensor DERECHO | Carril 2 → sensor IZQUIERDO
  char ladoControl = obtenerLadoControlPorCarril(); // 'D' o 'I'
  float distanciaControl = obtenerDistanciaPorSentido(ladoControl);

  // Sin referencia lateral válida (lectura inválida, o el filtro lleva varias lecturas
  // seguidas devolviendo un valor viejo): NO ir recto a ciegas. Mantener el ángulo objetivo.
  int invalidasLado = (ladoControl == 'I') ? controlLateral.filtroIzquierda.invalidasConsecutivas
                                           : controlLateral.filtroDerecha.invalidasConsecutivas;
  ent_evaluarEntrada(distanciaControl, invalidasLado); // v101: ¿ya entró al carril?
  bool sinReferenciaLateral = (distanciaControl <= 0 || distanciaControl > 150) ||
                              (invalidasLado >= MAX_INVALIDAS_REFERENCIA_LATERAL) ||
                              (ent_activa && distanciaControl > ENT_DIST_ENTRO_CM); // v101: antes de entrar, solo ángulo + sensor opuesto
  // ===== datos para el monitor =====
  dbgInvalidas     = invalidasLado;
  dbgSinRef        = sinReferenciaLateral;
  dbgDistFiltrada  = distanciaControl;
  dbgDistObjetivo  = obtenerDistanciaObjetivo();
  dbgErrDist       = distanciaControl - dbgDistObjetivo;
  dbgTDist         = millis();

  float valorServo = 0.0;          // v99
  bool  sinRefFallback = false;    // v99
  if (sinReferenciaLateral) {
    if (!primerGiroRealizado) anguloObjetivo = 0.0;
    else calcularAnguloObjetivoMantener();
    controlLateral.anguloObjetivo = anguloObjetivo;

    float errAngFallback = (anguloObjetivo + ent_calcularOffsetAngulo()) - anguloZ; // v101: offset por sensor opuesto (0 si no aplica)
    if (errAngFallback >  180.0) errAngFallback -= 360.0;
    if (errAngFallback < -180.0) errAngFallback += 360.0;

    float corrFallback = constrain(2.5 * errAngFallback,
                                   -(float)SERVO_CORR_PARED_DER,
                                   (float)SERVO_CORR_PARED_IZQ);

    static unsigned long ultimoPrintSinRef = 0;
    /*if (millis() - ultimoPrintSinRef > 300) {
      SerialBT.println("⚠️ Sin referencia lateral (" + String(ladoControl) + ") -> mantengo ángulo. ErrA:" +
                       String(errAngFallback, 1) + " Srv:" + String(SERVO_CENTER + corrFallback, 0));
      ultimoPrintSinRef = millis();
    }*/
    valorServo = constrain((float)SERVO_CENTER - corrFallback,(float)SERVO_MIN, (float)SERVO_MAX);
    sinRefFallback = true; // v99: NO salir aquí, seguir al paso 8 para que evalúe el cambio de carril
    }

  // ── 4. ÁNGULO OBJETIVO ────────────────────────────────────────────────────
  if (!sinRefFallback) { // v99: pasos 4-6 solo si hay referencia lateral
    if (!primerGiroRealizado) {
    anguloObjetivo = 0.0;
  } else {
    calcularAnguloObjetivoMantener();
  }
  controlLateral.anguloObjetivo = anguloObjetivo;

  // Error de ángulo normalizado [-180, 180]
  // positivo = Yaw < objetivo → robot apunta menos de lo que debería
  // negativo = Yaw > objetivo → robot apunta más de lo que debería
  float errorAngulo = anguloObjetivo - anguloZ;
  if (errorAngulo >  180.0) errorAngulo -= 360.0;
  if (errorAngulo < -180.0) errorAngulo += 360.0;

  // ── 5. ERROR DE DISTANCIA (PID) ────────────────────────────────────────────
  float distanciaObjetivo = obtenerDistanciaObjetivo();
  // positivo = demasiado lejos de la pared, negativo = demasiado cerca
  float errorDistancia = distanciaControl - distanciaObjetivo;

  controlLateral.integralDistancia += errorDistancia;
  controlLateral.integralDistancia = constrain(controlLateral.integralDistancia,
                                               -controlLateral.integralMaxDistancia,
                                               controlLateral.integralMaxDistancia);
  float derivadaDistancia = errorDistancia - controlLateral.errorDistanciaAnterior;
  controlLateral.errorDistanciaAnterior = errorDistancia;

  float correccionDistancia = (controlLateral.kpDistancia * errorDistancia)
                            + (controlLateral.kiDistancia * controlLateral.integralDistancia)
                            + (controlLateral.kdDistancia * derivadaDistancia);

  // Fase de aproximación posterior a una maniobra optimizada: limitar la distancia
  if (mo_aproximacionLimitadaActiva) {
    if (abs(errorDistancia) < MO_ERROR_DISTANCIA_FIN_APROXIMACION) {
      mo_aproximacionLimitadaActiva = false; // ya está cerca: control normal
      SerialBT.println("✅ Aproximación limitada finalizada, control normal.");
    } else {
      correccionDistancia = constrain(correccionDistancia,
                                      -MO_CORR_DISTANCIA_MAX_APROXIMACION,
                                      MO_CORR_DISTANCIA_MAX_APROXIMACION);
    }
  }

  // ── 6. FUSIÓN Y SIGNO ─────────────────────────────────────────────────────
  //
  // CONVENCIÓN SERVO: servo > 90 → gira IZQUIERDA | servo < 90 → gira DERECHA
  //
  // CARRIL 1 (sensor DERECHO, pared a la derecha):
  //   errorAngulo > 0 → Yaw < obj → apunta muy a la derecha → corregir IZQUIERDA → servo+ ✓ directo
  //   errorAngulo < 0 → Yaw > obj → apunta muy a la izquierda → corregir DERECHA → servo- ✓ directo
  //   errorDistancia > 0 → muy lejos de pared derecha → acercarse → girar DERECHA → servo- → invertir
  //   errorDistancia < 0 → muy cerca de pared derecha → alejarse → girar IZQUIERDA → servo+ → invertir
  //
  // CARRIL 2 (sensor IZQUIERDO, pared a la izquierda):
  //   errorAngulo > 0 → mismo razonamiento → directo
  //   errorAngulo < 0 → mismo razonamiento → directo  
  //   errorDistancia > 0 → muy lejos de pared izquierda → acercarse → girar IZQUIERDA → servo+ → directo
  //   errorDistancia < 0 → muy cerca de pared izquierda → alejarse → girar DERECHA → servo- → directo
  //
  // Resumen: corrección de ángulo siempre DIRECTA.
  //          corrección de distancia: INVERTIDA para carril 1, DIRECTA para carril 2.

  const float KP_ANGULO = 2.0;
  const float UMBRAL_ANGULO_DOMINANTE = 15.0; // ° — por encima de esto el ángulo domina

  // ── NUEVO: el peso de la distancia depende de qué tan lejos está del objetivo ──
  // Dentro de la tolerancia (ZONA_DIST_OK) casi manda solo el ángulo.
  // A partir de ZONA_DIST_LEJOS la distancia vuelve a tener su peso normal.
  const float ZONA_DIST_OK    = controlLateral.toleranciaDistancia; // 3.0 cm
  const float ZONA_DIST_LEJOS = 8.0;   // cm  (más alto = el ángulo domina por más tiempo)
  const float PESO_DIST_MIN   = 0.15;  // peso mínimo de la distancia cuando ya está en objetivo

  float pesoAnguloBase = constrain(abs(errorAngulo) / UMBRAL_ANGULO_DOMINANTE, 0.0, 1.0);
  float kDist          = constrain((fabsf(errorDistancia) - ZONA_DIST_OK) /
                                   (ZONA_DIST_LEJOS - ZONA_DIST_OK), 0.0, 1.0);
  float pesoDistMax    = PESO_DIST_MIN + (1.0 - PESO_DIST_MIN) * kDist;
  float pesoDistancia  = fminf(1.0 - pesoAnguloBase, pesoDistMax);
  float pesoAngulo     = 1.0 - pesoDistancia;

  float corrAngulo = KP_ANGULO * errorAngulo; // siempre directo

  float corrDistancia = correccionDistancia;
  if (ladoControl == 'D') {
    corrDistancia = -corrDistancia; // invertir para carril 1
  }
  // carril 2 (ladoControl == 'I'): directo

  float correccionTotal = (pesoAngulo * corrAngulo) + (pesoDistancia * corrDistancia);

  // Factores de asimetría mecánica
  if (correccionTotal > 0) correccionTotal *= controlLateral.factorGiroIzquierda;
  else                     correccionTotal *= controlLateral.factorGiroDerecha;

  // Límites separados por lado, alrededor del centro Ackermann
  correccionTotal = constrain(correccionTotal, -(float)SERVO_CORR_PARED_DER, (float)SERVO_CORR_PARED_IZQ);
  valorServo = constrain((float)SERVO_CENTER - correccionTotal, (float)SERVO_MIN, (float)SERVO_MAX);

  } // v99: fin de if (!sinRefFallback)

  // ── 7. DEBUG ──────────────────────────────────────────────────────────────
  /*static unsigned long ultimoPrint = 0;
  if (millis() - ultimoPrint > 300) {
    SerialBT.print(F("ToF(")); SerialBT.print(ladoControl);
    SerialBT.print(F("):")); SerialBT.print(distanciaControl, 1);
    SerialBT.print(F(" ErrD:")); SerialBT.print(errorDistancia, 1);
    SerialBT.print(F(" ErrA:")); SerialBT.print(errorAngulo, 1);
    SerialBT.print(F(" wA:")); SerialBT.print(pesoAngulo, 2);
    SerialBT.print(F(" Srv:")); SerialBT.println(valorServo, 0);
    ultimoPrint = millis();
  }*/

  // v98: en el tramo final no hay cambio de carril; solo sigue su carril actual hasta detenerse
  if (esUltimoTramoFinal) return valorServo;
  
  // ── 8. TRANSICIONES AUTOMÁTICAS DE COMANDO ───────────────────────────────
  if (carriles.esComandoC21 && carriles.carrilObjetivo == 1) {
    // Calcular cuánto tiempo llevamos en MANTENIENDO_CARRIL
    unsigned long tiempoEnManteniendo = millis() - carriles.tiempoInicioEstado;

    // El tiempo de espera depende del sentido de giro del vehículo
    unsigned long tiempoObjetivoC21 = (sentidoVehiculo == IZQUIERDA)
                                          ? TIEMPO_AVANCE_MANTENIENDO_C21_IZQUIERDA
                                          : TIEMPO_AVANCE_MANTENIENDO_C21_DERECHA;
      tiempoObjetivoC21 = ajustarTiempoManteniendoDoble(tiempoObjetivoC21);

    if (mo_cambioCarrilListo(tiempoEnManteniendo >= tiempoObjetivoC21)) {        // Ya avanzó suficiente, ahora sí proceder al cambio de carril
        SerialBT.println("⏱️ C21: Tiempo en MANTENIENDO_CARRIL cumplido (" + 
                         String(tiempoEnManteniendo) + "ms). Iniciando cambio a carril 2.");
        carriles.carrilObjetivo = 2;
        controlLateral.ladoControl = 'I';
        cambiarEstado(EVALUANDO_POSICION);
        return SERVO_SIN_COMANDO;
    } else {
        // Aún no ha pasado suficiente tiempo, seguir manteniendo
        SerialBT.print("⏳ C21: Avanzando en carril 1 antes del cambio... ");
        SerialBT.print(tiempoEnManteniendo);
        SerialBT.print("/");
        SerialBT.println(tiempoObjetivoC21);
        // No retornar -1: dejar que el PID de pared siga controlando el servo
    }
  }
  if (carriles.esComandoC14 && carriles.carrilObjetivo == 2) {
    // Calcular cuánto tiempo llevamos en MANTENIENDO_CARRIL
    unsigned long tiempoEnManteniendo = millis() - carriles.tiempoInicioEstado;

    // El tiempo de espera depende del sentido de giro del vehículo, igual que en C21
    unsigned long tiempoObjetivoC14 = (sentidoVehiculo == IZQUIERDA)
                                          ? TIEMPO_AVANCE_MANTENIENDO_C14_IZQUIERDA
                                          : TIEMPO_AVANCE_MANTENIENDO_C14_DERECHA;
   tiempoObjetivoC14 = ajustarTiempoManteniendoDoble(tiempoObjetivoC14);
    if (mo_cambioCarrilListo(tiempoEnManteniendo >= tiempoObjetivoC14)) {
        // Ya avanzó suficiente, ahora sí proceder al cambio de carril
        SerialBT.println("⏱️ C14: Tiempo en MANTENIENDO_CARRIL cumplido (" + 
                         String(tiempoEnManteniendo) + "ms). Iniciando cambio a carril 1.");
        carriles.carrilObjetivo = 1; controlLateral.ladoControl = 'D';
        // Reset al valor base tras completar el ciclo (independiente del sentido)
        TIEMPO_AVANCE_MANTENIENDO_C14_IZQUIERDA = 100;
        TIEMPO_AVANCE_MANTENIENDO_C14_DERECHA = 300;
        cambiarEstado(EVALUANDO_POSICION); 
        return SERVO_SIN_COMANDO;
    } else {
        // Aún no ha pasado suficiente tiempo, seguir manteniendo
        SerialBT.print("⏳ C14: Avanzando en carril 2 antes del cambio... ");
        SerialBT.print(tiempoEnManteniendo);
        SerialBT.print("/");
        SerialBT.println(tiempoObjetivoC14);
        // No retornar -1: dejar que el PID de pared siga controlando el servo
    }

  }

  if (carriles.esComandoC15 && carriles.carrilObjetivo == 1) {
     // Calcular cuánto tiempo llevamos en MANTENIENDO_CARRIL
    unsigned long tiempoEnManteniendo = millis() - carriles.tiempoInicioEstado;
      unsigned long tiempoObjetivoC15 = ajustarTiempoManteniendoDoble(TIEMPO_AVANCE_MANTENIENDO_C15);


    if (mo_cambioCarrilListo(tiempoEnManteniendo >= tiempoObjetivoC15)) {
        // Ya avanzó suficiente, ahora sí proceder al cambio de carril
        SerialBT.println("⏱️ C15: Tiempo en MANTENIENDO_CARRIL cumplido (" + 
                         String(tiempoEnManteniendo) + "ms). Iniciando cambio a carril 2.");
        carriles.carrilObjetivo = 2; controlLateral.ladoControl = 'I';
        cambiarEstado(EVALUANDO_POSICION); 
        return SERVO_SIN_COMANDO;
    } else {
        // Aún no ha pasado suficiente tiempo, seguir manteniendo
        SerialBT.print("⏳ C15: Avanzando en carril 1 antes del cambio... ");
        SerialBT.print(tiempoEnManteniendo);
        SerialBT.print("/");
        SerialBT.println(TIEMPO_AVANCE_MANTENIENDO_C15);
        // No retornar -1: dejar que el PID de pared siga controlando el servo
    }

  }
  if (carriles.esComandoC20 && carriles.carrilObjetivo == 2) {
    unsigned long tiempoEnManteniendo = millis() - carriles.tiempoInicioEstado;
    //unsigned long tiempoEsperaC20 = TIEMPO_AVANCE_MANTENIENDO_C20 + (omitioGiro ? 1500UL : 0UL);
      unsigned long tiempoEsperaC20 = ajustarTiempoManteniendoDoble(TIEMPO_AVANCE_MANTENIENDO_C20 + (omitioGiro ? 1500UL : 0UL));
    if (mo_cambioCarrilListo(tiempoEnManteniendo >= tiempoEsperaC20)) {
        // Ya avanzó suficiente, ahora sí proceder al cambio de carril
        SerialBT.println("⏱️ C20: Tiempo en MANTENIENDO_CARRIL cumplido (" + 
                         String(tiempoEnManteniendo) + "ms). Iniciando cambio a carril 1.");
        carriles.carrilObjetivo = 1; controlLateral.ladoControl = 'D';
        //carriles.distanciaCarril1 = 22.0;
        cambiarEstado(EVALUANDO_POSICION); 
        return SERVO_SIN_COMANDO;
    } else {
        // Aún no ha pasado suficiente tiempo, seguir manteniendo
        SerialBT.print("⏳ C20: Avanzando en carril 2 antes del cambio... ");
        SerialBT.print(tiempoEnManteniendo);
        SerialBT.print("/");
        SerialBT.println(tiempoEsperaC20);        // No retornar -1: dejar que el PID de pared siga controlando el servo
    }
  }

  if (carriles.esComandoC32 && carriles.carrilObjetivo == 2) {
    // Calcular cuánto tiempo llevamos en MANTENIENDO_CARRIL
    unsigned long tiempoEnManteniendo = millis() - carriles.tiempoInicioEstado;
      unsigned long tiempoObjetivoC32 = ajustarTiempoManteniendoDoble(TIEMPO_AVANCE_MANTENIENDO_C32);

    if (mo_cambioCarrilListo(tiempoEnManteniendo >= tiempoObjetivoC32)) {

    //if (tiempoEnManteniendo >= TIEMPO_AVANCE_MANTENIENDO_C32) {
        // Ya avanzó suficiente, ahora sí proceder al cambio de carril
        SerialBT.println("⏱️ C32: Tiempo en MANTENIENDO_CARRIL cumplido (" + 
                         String(tiempoEnManteniendo) + "ms). Iniciando cambio a carril 1.");
        carriles.carrilObjetivo = 1; 
        //carriles.distanciaCarril1 = 25.0;
        controlLateral.ladoControl = 'D';
        cambiarEstado(EVALUANDO_POSICION); 
        return SERVO_SIN_COMANDO;
    } else {
        // Aún no ha pasado suficiente tiempo, seguir manteniendo
        SerialBT.print("⏳ C32: Avanzando en carril 2 antes del cambio... ");
        SerialBT.print(tiempoEnManteniendo);
        SerialBT.print("/");
        SerialBT.println(TIEMPO_AVANCE_MANTENIENDO_C32);
        // No retornar -1: dejar que el PID de pared siga controlando el servo
    }
  }
  if (carriles.esComandoC33 && carriles.carrilObjetivo == 1) {
       // Calcular cuánto tiempo llevamos en MANTENIENDO_CARRIL
    unsigned long tiempoEnManteniendo = millis() - carriles.tiempoInicioEstado;

    unsigned long tiempoEsperaC33 = omitioGiro 
    ? TIEMPO_AVANCE_MANTENIENDO_C33_OMITIDO 
    : TIEMPO_AVANCE_MANTENIENDO_C33;
    tiempoEsperaC33 = ajustarTiempoManteniendoDoble(tiempoEsperaC33);
    if (mo_cambioCarrilListo(tiempoEnManteniendo >= tiempoEsperaC33)) {
        // Ya avanzó suficiente, ahora sí proceder al cambio de carril
        SerialBT.println("⏱️ C33: Tiempo en MANTENIENDO_CARRIL cumplido (" + 
                         String(tiempoEnManteniendo) + "ms). Iniciando cambio a carril 1.");
        carriles.carrilObjetivo = 2; 
        //carriles.distanciaCarril1 = 35.0;
        controlLateral.ladoControl = 'D';
        cambiarEstado(EVALUANDO_POSICION); 
        return SERVO_SIN_COMANDO;
      } else {
          // Aún no ha pasado suficiente tiempo, seguir manteniendo
          SerialBT.print("⏳ C33: Avanzando en carril 2 antes del cambio... ");
          SerialBT.print(tiempoEnManteniendo);
          SerialBT.print("/");
          SerialBT.println(tiempoEsperaC33);
          // No retornar -1: dejar que el PID de pared siga controlando el servo
      }
  
  }

  if (carriles.esComandoC26 && carriles.carrilObjetivo == 2) {
       // Calcular cuánto tiempo llevamos en MANTENIENDO_CARRIL
    unsigned long tiempoEnManteniendo = millis() - carriles.tiempoInicioEstado;

    unsigned long tiempoEsperaC26 = omitioGiro 
    ? TIEMPO_AVANCE_MANTENIENDO_C26_OMITIDO 
    : TIEMPO_AVANCE_MANTENIENDO_C26;
    tiempoEsperaC26 = ajustarTiempoManteniendoDoble(tiempoEsperaC26);
   if (mo_cambioCarrilListo(tiempoEnManteniendo >= tiempoEsperaC26)) {
        // Ya avanzó suficiente, ahora sí proceder al cambio de carril
        SerialBT.println("⏱️ C26: Tiempo en MANTENIENDO_CARRIL cumplido (" + 
                         String(tiempoEnManteniendo) + "ms). Iniciando cambio a carril 1.");
        carriles.carrilObjetivo = 1; 
        //carriles.distanciaCarril1 = 15.0;
        controlLateral.ladoControl = 'D';
        cambiarEstado(EVALUANDO_POSICION); 
        return SERVO_SIN_COMANDO;
      } else {
          // Aún no ha pasado suficiente tiempo, seguir manteniendo
          SerialBT.print("⏳ C26: Avanzando en carril 2 antes del cambio... ");
          SerialBT.print(tiempoEnManteniendo);
          SerialBT.print("/");
          SerialBT.println(tiempoEsperaC26);
          // No retornar -1: dejar que el PID de pared siga controlando el servo
      }
  
  }
  
  if (carriles.esComandoC27 && carriles.carrilObjetivo == 1) {
    // Calcular cuánto tiempo llevamos en MANTENIENDO_CARRIL
    unsigned long tiempoEnManteniendo = millis() - carriles.tiempoInicioEstado;
      unsigned long tiempoObjetivoC27 = ajustarTiempoManteniendoDoble(TIEMPO_AVANCE_MANTENIENDO_C27);


    if (mo_cambioCarrilListo(tiempoEnManteniendo >= tiempoObjetivoC27)) {
        // Ya avanzó suficiente, ahora sí proceder al cambio de carril
        SerialBT.println("⏱️ C27: Tiempo en MANTENIENDO_CARRIL cumplido (" + 
                         String(tiempoEnManteniendo) + "ms). Iniciando cambio a carril 2.");
        carriles.carrilObjetivo = 2;
        controlLateral.ladoControl = 'I';
        cambiarEstado(EVALUANDO_POSICION);
        return SERVO_SIN_COMANDO;
    } else {
        // Aún no ha pasado suficiente tiempo, seguir manteniendo
        SerialBT.print("⏳ C27: Avanzando en carril 1 antes del cambio... ");
        SerialBT.print(tiempoEnManteniendo);
        SerialBT.print("/");
        SerialBT.println(TIEMPO_AVANCE_MANTENIENDO_C27);
        // No retornar -1: dejar que el PID de pared siga controlando el servo
    }
  }

  return valorServo;
}


// Avanzando: error>0 (apunta a la izquierda del objetivo) -> volante a la DERECHA; error<0 -> IZQUIERDA.
// Va a fondo si el error es grande y se suaviza al acercarse, para no pasarse del objetivo.
void ac_servoEnderezadoAvance(float objetivo) {
  float err = calcularDiferenciaAngular(anguloZ, objetivo);
  float frac = constrain(abs(err) / AC_ANG_SATURA, AC_SERVO_MIN_FRAC, 1.0f);
  int servo = (err > 0) ? (int)(SERVO_RIGHT * frac) : (int)(SERVO_LEFT * frac);
  direccion.write(servo);
}

// Retrocediendo: error>0 -> volante a la IZQUIERDA (igual que iniciarCorreccion()).
void ac_servoRumboRetroceso(float objetivo) {
  float err = calcularDiferenciaAngular(anguloZ, objetivo);
  if (abs(err) <= AC_RETRO_ZONA_MUERTA) { direccion.write(SERVO_CENTRADO); return; }
  float frac = constrain(err, -AC_RETRO_SATURA, AC_RETRO_SATURA) / AC_RETRO_SATURA;
  int servo = (int)(frac * AC_RETRO_SERVO_MAX);
  if (AC_RETRO_INVERTIR) servo = -servo;
  direccion.write(constrain(servo, SERVO_LEFT, SERVO_RIGHT));
}

// ====================  FUNCIÓN DE CORRECCIÓN LATERAL POST-GIRO ====================
void ejecutarCorreccionDistanciaLateral() {
  if (!controlLateral_PostGiro.funcionActiva) {
    return;
  }

  unsigned long tiempoActual = millis();
  switch (controlLateral_PostGiro.estadoActual) {

    case ControlDistanciaLateral::IDLE: {

      // ── PAUSA DE ESTABILIZACIÓN: Detener y esperar 1s antes de leer ──
      // Se usa tiempoInicioEstado que ya se actualiza cada vez que
      // se entra a este estado, así el timer se resetea solo.
      detenerMotor();
      direccion.write(SERVO_CENTRADO);

      if (millis() - controlLateral_PostGiro.tiempoInicioEstado < 300) {
        SerialBT.println("⏸️ Acomodo lateral: estabilizando 0.3s antes de medir...");
        break; // Salir y volver al loop, el motor ya está detenido
      }
      // ─────────────────────────────────────────────────────────────────

      float distanciaLateral;
      String ladoSensor;

      // 1. MEDIR LA DISTANCIA LATERAL
      if (sentidoVehiculo == DERECHA) {
        distanciaLateral = leerToF('I'); 
        ladoSensor = "izquierdo"; 
      } else {
        distanciaLateral = leerToF('D'); 
        ladoSensor = "derecho"; 
      }

      if (distanciaLateral == 999) break; 

      SerialBT.print("📏 CORRECCIÓN LATERAL - Distancia ");
      SerialBT.print(ladoSensor);
      SerialBT.print(": ");
      SerialBT.println(distanciaLateral, 1); 

      bool distanciaCorrecta = (distanciaLateral >= controlLateral_PostGiro.UMBRAL_MIN && distanciaLateral <= controlLateral_PostGiro.UMBRAL_MAX);

      if (!distanciaCorrecta) {
        // --- CASO A: LA DISTANCIA ES INCORRECTA ---
        // Se inicia el ciclo de corrección de distancia normal.
        // Calcular diferencia respecto al centro del umbral
        float distanciaObjetivo = (controlLateral_PostGiro.UMBRAL_MIN + controlLateral_PostGiro.UMBRAL_MAX) / 2.0;
        float diferencia = abs(distanciaLateral - distanciaObjetivo);

        // Determinar primero hacia qué lado se moverá la corrección,
        // para poder elegir el rango de tiempo correcto antes de calcular.
        bool moveraADerecha;
        if (sentidoVehiculo == DERECHA) {
          moveraADerecha = (distanciaLateral < controlLateral_PostGiro.UMBRAL_MIN);
        } else { // Sentido IZQUIERDA
          moveraADerecha = !(distanciaLateral < controlLateral_PostGiro.UMBRAL_MIN);
        }

        // Interpolación lineal: diferencia pequeña (6cm) → tiempo mínimo,
        //                       diferencia grande (25cm) → tiempo máximo.
        // Rango de tiempo distinto según hacia qué lado se mueve la corrección.
        const float DIFF_MIN = 6.0;//2.0;
        const float DIFF_MAX = 25.0;//25.0;

        unsigned long TIEMPO_MIN;
        unsigned long TIEMPO_MAX;
        if (moveraADerecha) {
          TIEMPO_MIN = 300;
          TIEMPO_MAX = 800;
        } else {
          TIEMPO_MIN = 300;
          TIEMPO_MAX = 600;//700
        }

        float difClamped = constrain(diferencia, DIFF_MIN, DIFF_MAX);
        float tiempoCalculado = TIEMPO_MIN + ((difClamped - DIFF_MIN) / (DIFF_MAX - DIFF_MIN)) * (TIEMPO_MAX - TIEMPO_MIN);
        controlLateral_PostGiro.tiempoCruceActual = (unsigned long)tiempoCalculado;

        SerialBT.println("📐 Acomodo lateral: Centro objetivo=" + String(distanciaObjetivo, 1) +
                         "cm | Dist actual=" + String(distanciaLateral, 1) +
                         "cm | Diferencia=" + String(diferencia, 1) +
                         "cm → Tiempo=" + String(controlLateral_PostGiro.tiempoCruceActual) + "ms");
         // Si la diferencia es muy grande, activa la bandera para la maniobra de retroceso especial.
       /* if (diferencia > 16.0) {
          SerialBT.println("!!! Diferencia > 15cm. Se activará maniobra de retroceso orientado.");
          controlLateral_PostGiro.esCorreccionLarga = true;
        } else {
          controlLateral_PostGiro.esCorreccionLarga = false;
        }*/

        if (sentidoVehiculo == DERECHA) {
          if (distanciaLateral < controlLateral_PostGiro.UMBRAL_MIN) {
            SerialBT.println("⬇️ Muy cerca. Moviendo a la derecha."); 
            controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::MOVIENDO_DERECHA;
            controlLateral_PostGiro.direccionManiobra = 'D';
            direccion.write(SERVO_RIGHT);
          } else {
            SerialBT.println("⬆️ Muy lejos. Moviendo a la izquierda."); 
            controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::MOVIENDO_IZQUIERDA;
            direccion.write(SERVO_LEFT);
            controlLateral_PostGiro.direccionManiobra = 'I';
          }
        } else { // Sentido IZQUIERDA
          if (distanciaLateral < controlLateral_PostGiro.UMBRAL_MIN) {
            SerialBT.println("⬇️ Muy cerca. Moviendo a la izquierda."); 
            controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::MOVIENDO_IZQUIERDA;
            direccion.write(SERVO_LEFT);
            controlLateral_PostGiro.direccionManiobra = 'I';
          } else {
            SerialBT.println("⬆️ Muy lejos. Moviendo a la derecha."); 
            controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::MOVIENDO_DERECHA;
            direccion.write(SERVO_RIGHT);
            controlLateral_PostGiro.direccionManiobra = 'D';
          }
        }
        controlLateral_PostGiro.tiempoInicioEstado = tiempoActual;
        avanzarMotor();

      } else {
        // --- CASO B: LA DISTANCIA ES CORRECTA ---
        // Ahora realizamos la nueva verificación del ángulo.
        const float UMBRAL_ANGULO_FINAL = 8.0;//5.0
        float anguloObjetivoFinal = calcularAnguloRetroceso();
        float diferenciaAngular = calcularDiferenciaAngular(anguloZ, anguloObjetivoFinal);

        if (abs(diferenciaAngular) > UMBRAL_ANGULO_FINAL) {
          // El ángulo está desviado, se necesita una corrección corta.
          SerialBT.print("! Distancia OK, pero ángulo desviado > ");
          SerialBT.print(UMBRAL_ANGULO_FINAL);
          SerialBT.print("°. | Actual: ");
          SerialBT.print(anguloZ, 1);
          SerialBT.print("°, Objetivo: ");
          SerialBT.print(anguloObjetivoFinal, 1);
          SerialBT.println("°. Iniciando corrección corta.");

        // Usamos una duración de cruce corta y reiniciamos el ciclo de corrección
          controlLateral_PostGiro.tiempoCruceActual = 300; // 300ms de movimiento lateral

          // Si la diferencia es > 0, el robot apunta a la DERECHA del objetivo.
          // Nos movemos lateralmente a la IZQUIERDA para corregir.
          if (diferenciaAngular > 0) {
            controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::MOVIENDO_IZQUIERDA;
            direccion.write(SERVO_LEFT); // Girar ruedas a la izquierda
          }
          // Si no, el robot apunta a la IZQUIERDA y nos movemos a la DERECHA.
          else {
            controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::MOVIENDO_DERECHA;
            direccion.write(SERVO_RIGHT); // Girar ruedas a la derecha
          }
          controlLateral_PostGiro.tiempoInicioEstado = tiempoActual;
          avanzarMotor();// Avanzar para iniciar la corrección

        } else {
          // ÉXITO: La distancia y el ángulo son correctos.
          SerialBT.println("✅ Distancia y ángulo correctos. Corrección finalizada.");
          controlLateral_PostGiro.funcionActiva = false;
          detenerMotor();
          direccion.write(SERVO_CENTRADO);
          cambiarEstado(ESPERANDO_COMANDO_CARRIL);
        }
      }
      break;
    }

    case ControlDistanciaLateral::MOVIENDO_DERECHA: {
      if (tiempoActual - controlLateral_PostGiro.tiempoInicioEstado >= controlLateral_PostGiro.tiempoCruceActual) {
        controlLateral_PostGiro.anguloObjetivo = calcularAnguloRetroceso(); 
        SerialBT.print("⏱️ Tiempo de cruce DERECHA finalizado. Corrigiendo ángulo hacia: "); 
        SerialBT.println(controlLateral_PostGiro.anguloObjetivo); 
        controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::CORRIGIENDO_ANGULO_IZQUIERDA;
        direccion.write(SERVO_LEFT); 
      }
      break;
    }

    case ControlDistanciaLateral::MOVIENDO_IZQUIERDA: {
      if (tiempoActual - controlLateral_PostGiro.tiempoInicioEstado >= controlLateral_PostGiro.tiempoCruceActual) {
        controlLateral_PostGiro.anguloObjetivo = calcularAnguloRetroceso(); 
        SerialBT.print("⏱️ Tiempo de cruce IZQUIERDA finalizado. Corrigiendo ángulo hacia: "); 
        SerialBT.println(controlLateral_PostGiro.anguloObjetivo); 
        controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::CORRIGIENDO_ANGULO_DERECHA;
        direccion.write(SERVO_RIGHT); 
      }
      break;
    }

    case ControlDistanciaLateral::CORRIGIENDO_ANGULO_DERECHA:
    case ControlDistanciaLateral::CORRIGIENDO_ANGULO_IZQUIERDA: {
      //avanzarLento(); // Velocidad reducida durante el enderezado de ángulo

            avanzarMotor(AC_VELOCIDAD_ENDEREZADO);                              // v103: velocidad propia del enderezado
      ac_servoEnderezadoAvance(controlLateral_PostGiro.anguloObjetivo);   // v103: el volante sigue el signo real del error

      // ===== LÓGICA DE INTERRUPCIÓN Y CONTINUACIÓN =====
      float distanciaFrontal = leerToF('F');
      if (distanciaFrontal <= 10 && distanciaFrontal > 0) {
        SerialBT.println("! OBSTÁCULO FRONTAL durante contra-giro !");
        SerialBT.println("  -> Pausando, retrocediendo 1 seg y continuando...");

        // 1. Pausar el avance y enderezar las ruedas temporalmente
        detenerMotor();
        direccion.write(SERVO_CENTRADO);
        delay(100);

        // 2. Retroceder por 1 segundo mientras se actualiza el ángulo
        retrocederMotor();
        unsigned long tiempoInicioRetroceso = millis();
        while (millis() - tiempoInicioRetroceso < 700) {
          actualizarGiro();
          actualizarRampaMotor();
          delay(10);
        }
        detenerMotor();
        delay(100);

        // 3. Reanudar el contra-giro exactamente como estaba
        if (controlLateral_PostGiro.estadoActual == ControlDistanciaLateral::CORRIGIENDO_ANGULO_DERECHA) {
            SerialBT.println("  -> Reanudando contra-giro a la DERECHA.");
            direccion.write(SERVO_RIGHT); // Vuelve a girar las ruedas para la corrección
        } else { // Es CORRIGIENDO_ANGULO_IZQUIERDA
            SerialBT.println("  -> Reanudando contra-giro a la IZQUIERDA.");
            direccion.write(SERVO_LEFT); // Vuelve a girar las ruedas para la corrección
        }
        avanzarLento();// Reanuda el avance
      }
      // ==========
      float anguloActual = anguloZ; 
      float diferenciaAngular = abs(anguloActual - controlLateral_PostGiro.anguloObjetivo);
      if (diferenciaAngular > 180) {
        diferenciaAngular = 360 - diferenciaAngular; 
      }

      if (diferenciaAngular <= controlLateral_PostGiro.TOLERANCIA_ANGULAR) {
        SerialBT.println("🎯 Ángulo corregido. Iniciando retroceso."); 
        controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::RETROCEDIENDO_FINAL;
        detenerMotor();
        delay(100);
         if (controlLateral_PostGiro.esCorreccionLarga) {
          SerialBT.println("🎯 Ángulo corregido. Iniciando RETROCESO ORIENTADO.");
          controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::RETROCEDIENDO_ORIENTADO;
        } else {
          SerialBT.println("🎯 Ángulo corregido. Centrando servo antes de retroceder.");
          controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::PAUSA_CENTRADO_PRE_RETROCESO;
          direccion.write(SERVO_CENTRADO);
        }
        controlLateral_PostGiro.tiempoInicioEstado = millis();
        controlLateral_PostGiro.tiempoInicioEstado = millis();
      }
      break;
    }
    case ControlDistanciaLateral::RETROCEDIENDO_ORIENTADO: {
      // 1. Apuntar en la dirección original de la maniobra
      if (controlLateral_PostGiro.direccionManiobra == 'D') {
        direccion.write(SERVO_RIGHT); // Misma dirección que MOVIENDO_DERECHA
      } else {
        direccion.write(SERVO_LEFT); // Misma dirección que MOVIENDO_IZQUIERDA
      }

      // 2. Retroceder por 500ms
      retrocederMotor();

      // 3. Después de 500ms, pasar a la fase de corrección en retroceso
      if (tiempoActual - controlLateral_PostGiro.tiempoInicioEstado >= 800) {
        SerialBT.println("⏱️ 500ms de retroceso orientado completados. Invirtiendo dirección.");
        controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::CORRIGIENDO_POST_RETROCESO;
        controlLateral_PostGiro.anguloObjetivo = calcularAnguloRetroceso(); // Re-calcular por si acaso
      }
      break;
    }
    case ControlDistanciaLateral::CORRIGIENDO_POST_RETROCESO: {
      // 1. Apuntar en la dirección CONTRARIA (contra-giro)
      if (controlLateral_PostGiro.direccionManiobra == 'D') {
        direccion.write(SERVO_LEFT); // Contra-giro
      } else {
        direccion.write(SERVO_RIGHT); // Contra-giro
      }

      // 2. Continuar retrocediendo y monitorear el ángulo
      retrocederMotor();
      float anguloActual = anguloZ;
      float diferenciaAngular = abs(anguloActual - controlLateral_PostGiro.anguloObjetivo);
      if (diferenciaAngular > 180) {
        diferenciaAngular = 360 - diferenciaAngular;
      }

      // 3. Cuando el ángulo sea correcto, enderezar ruedas y pasar al retroceso final
      if (diferenciaAngular <= controlLateral_PostGiro.TOLERANCIA_ANGULAR) {
        SerialBT.println("🎯 Ángulo enderezado en retroceso. Pasando a retroceso final.");
        direccion.write(SERVO_CENTRADO);
        controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::RETROCEDIENDO_FINAL;
      }
      break;
    }
    case ControlDistanciaLateral::PAUSA_CENTRADO_PRE_RETROCESO: {
      // No hace nada más que esperar: el servo ya recibió la orden de centrarse.
      // Aquí puedes seguir leyendo/actualizando el ángulo con total libertad.
      if (millis() - controlLateral_PostGiro.tiempoInicioEstado >= 300) { // ajustable 200-400ms
        SerialBT.println("🎯 Servo centrado. Iniciando RETROCESO FINAL (recto).");
        controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::RETROCEDIENDO_FINAL;
        retrocederMotor();
        controlLateral_PostGiro.tiempoInicioEstado = millis();
      }
      break;
    }

    case ControlDistanciaLateral::RETROCEDIENDO_FINAL: {
        static int lecturasConsecutivasOK = 0;
        static float ultimaDistanciaTraseraValida = 999.0;
        const int LECTURAS_REQUERIDAS = 3; // Nº de lecturas seguidas por debajo del umbral para confirmar

        ac_servoRumboRetroceso(controlLateral_PostGiro.anguloObjetivo);  // v103: mantiene el rumbo mientras retrocede
        retrocederMotor(AC_VELOCIDAD_RETROCESO_FINAL);

        float distanciaTrasera = leerToF('T');

        // Ignorar lecturas inválidas (timeout/error del sensor) sin resetear el contador de golpe:
        // solo se reinicia si la lectura es válida pero NO cumple el umbral.
        if (distanciaTrasera == 999) {
          break; // Reintentar en el próximo ciclo, sin perder el progreso de confirmación
        }

        if (distanciaTrasera < controlLateral_PostGiro.DISTANCIA_TRASERA_OBJETIVO) {
          lecturasConsecutivasOK++;
          SerialBT.println("🔎 Lectura trasera bajo umbral (" + String(distanciaTrasera, 1) +
                          "cm). Confirmaciones: " + String(lecturasConsecutivasOK) + "/" + String(LECTURAS_REQUERIDAS));
        } else {
          if (lecturasConsecutivasOK > 0) {
            SerialBT.println("↩️ Lectura trasera volvió a subir (" + String(distanciaTrasera, 1) +
                            "cm). Reiniciando contador de confirmación.");
          }
          lecturasConsecutivasOK = 0; // Cualquier lectura válida que no cumpla, reinicia el conteo
        }

        if (lecturasConsecutivasOK >= LECTURAS_REQUERIDAS) {
          SerialBT.print("🏁 Retroceso finalizado (confirmado). Distancia trasera:");
          SerialBT.println(distanciaTrasera);
          SerialBT.print("🏁 Re-evaluando posición lateral y angular.");
          detenerMotor();
          controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::IDLE;
          controlLateral_PostGiro.tiempoInicioEstado = millis();
          controlLateral_PostGiro.esCorreccionLarga = false;
          lecturasConsecutivasOK = 0; // Reset para el próximo acomodo lateral
        }
        break;
      }
  }
}
float calcularControlCarriles() {
  switch (estadoActual) { // Usa la variable global 'estadoActual'
    case ESPERANDO_COMANDO_CARRIL: // Usa el nombre correcto del enum
      return procesarEstadoEsperandoComando();
      
    case EVALUANDO_POSICION:
      return procesarEstadoEvaluandoPosicion();
      
    case GIRANDO_HACIA_CARRIL:
      return procesarEstadoGirandoHaciaCarril();
      
    case ENDEREZANDO_CARRIL:
      return procesarEstadoEnderezandoCarril();

    case MANTENIENDO_CARRIL: {
      float resultado = procesarMantenimientoPared();
      // Si es -1, significa que no se debe cambiar la dirección
      if (resultado == SERVO_SIN_COMANDO) {
        return SERVO_SIN_COMANDO;  // Propagar el valor especial
      }
      return resultado;
    }

      
    default:
      return (float)SERVO_CENTER;
  }
}

// Guarda el comando recibido en la lista de memorización, si aún no está completa.
// No memoriza si contadorLineas < 1, porque eso significa que es el esquive inicial
// (el que ocurre justo tras el retroceso previo, antes de detectar la primera línea)
// y ese no cuenta como parte del ciclo de 4 tramos de la pista.
void memorizarComando(String comando) {
  if (memorizacionCompleta) return; // ya tenemos los 4, no hace falta seguir guardando

  if (contadorLineas < 1) {
    SerialBT.println("🧠 Comando NO memorizado (esquive inicial, contadorLineas=" + String(contadorLineas) + "): " + comando);
    return;
  }

  if (cantidadComandosMemorizados < TOTAL_TRAMOS_PISTA) {
    comandosMemorizados[cantidadComandosMemorizados] = comando;
    cantidadComandosMemorizados++;

    SerialBT.println("🧠 Comando memorizado [" + String(cantidadComandosMemorizados) +
                      "/" + String(TOTAL_TRAMOS_PISTA) + "]: " + comando);

    if (cantidadComandosMemorizados == TOTAL_TRAMOS_PISTA) {
      memorizacionCompleta = true;
      SerialBT.println("✅ Memorización completa. A partir de ahora los esquives se repetirán automáticamente.");
    }
  }
}

// ====================  SELECCIÓN DE UMBRALES DE ACOMODO LATERAL ====================
// Decide si el tramo actual debe usar los umbrales alternativos (más cerrados) o los
// originales, según el comando que corresponde a ese tramo y el sentido de giro vigente.
// Solo aplica a partir de contadorLineas >= 5 (memorización ya completa, ciclo repitiéndose),
// ya que antes de eso no hay un comando de tramo confiable para evaluar.
void aplicarUmbralesSegunComando() {
  bool activarAlterno = false;
  String comandoEvaluado = ""; // solo para el log

  if (memorizacionCompleta && contadorLineas >= 5) {
    int indiceTramo = (contadorLineas - 1) % TOTAL_TRAMOS_PISTA;
    comandoEvaluado = comandosMemorizados[indiceTramo];

    // Los comandos con estacionamiento ("...E") quedan excluidos: nunca activan el umbral alterno.
    if (!comandoEvaluado.endsWith("E")) {
      if (sentidoVehiculo == DERECHA) {
        if (comandoEvaluado == "C07" || comandoEvaluado == "C09" || comandoEvaluado == "C13" ||
            comandoEvaluado == "C37" || comandoEvaluado == "C19" || comandoEvaluado == "C31" ||
            comandoEvaluado == "C01" || comandoEvaluado == "C03" || comandoEvaluado == "C05" ||
            comandoEvaluado == "C25") {
          activarAlterno = true;
        }
      } else if (sentidoVehiculo == IZQUIERDA) {
        if (comandoEvaluado == "C02" || comandoEvaluado == "C04" || comandoEvaluado == "C06" ||
            comandoEvaluado == "C18" || comandoEvaluado == "C24" || comandoEvaluado == "C30" ||
            comandoEvaluado == "C08" || comandoEvaluado == "C10" || comandoEvaluado == "C38" ||
            comandoEvaluado == "C36") {
          activarAlterno = true;
        }
      }
    }
  }

  if (activarAlterno) {
    controlLateral_PostGiro.UMBRAL_MIN = controlLateral_PostGiro.UMBRAL_MIN_ALT;
    controlLateral_PostGiro.UMBRAL_MAX = controlLateral_PostGiro.UMBRAL_MAX_ALT;
    SerialBT.println("🎯 Umbrales ALTERNOS activados (comando=" + comandoEvaluado +
                      ", sentido=" + String(sentidoVehiculo == DERECHA ? "DERECHA" : "IZQUIERDA") +
                      "): MIN=" + String(controlLateral_PostGiro.UMBRAL_MIN, 1) +
                      " MAX=" + String(controlLateral_PostGiro.UMBRAL_MAX, 1));
  } else {
    if (sentidoVehiculo == DERECHA) {
      controlLateral_PostGiro.UMBRAL_MIN = controlLateral_PostGiro.UMBRAL_MIN_DEFAULT_DERECHA;
      controlLateral_PostGiro.UMBRAL_MAX = controlLateral_PostGiro.UMBRAL_MAX_DEFAULT_DERECHA;
      SerialBT.println("🎯 Umbrales ORIGINALES (DERECHA) activados (comando=" + comandoEvaluado +
                        ", contadorLineas=" + String(contadorLineas) +
                        "): MIN=" + String(controlLateral_PostGiro.UMBRAL_MIN, 1) +
                        " MAX=" + String(controlLateral_PostGiro.UMBRAL_MAX, 1));
    } else {
      controlLateral_PostGiro.UMBRAL_MIN = controlLateral_PostGiro.UMBRAL_MIN_DEFAULT_IZQUIERDA;
      controlLateral_PostGiro.UMBRAL_MAX = controlLateral_PostGiro.UMBRAL_MAX_DEFAULT_IZQUIERDA;
      SerialBT.println("🎯 Umbrales ORIGINALES (IZQUIERDA/INDEFINIDO) activados (comando=" + comandoEvaluado +
                        ", contadorLineas=" + String(contadorLineas) +
                        "): MIN=" + String(controlLateral_PostGiro.UMBRAL_MIN, 1) +
                        " MAX=" + String(controlLateral_PostGiro.UMBRAL_MAX, 1));
    }
  }
}

// ==================== CLASIFICACIÓN DE COMANDOS PARA MANIOBRA OPTIMIZADA ====================
// "Grupo A" y "Grupo B" son los dos conjuntos de comandos que definiste. Cuál de los dos es
// "interno" y cuál "externo" depende del sentido de giro vigente:
//   - sentido IZQUIERDA -> Grupo A = interno,  Grupo B = externo
//   - sentido DERECHA   -> Grupo A = externo,  Grupo B = interno
// (Grupo A también es, en todos los casos, el grupo cuyo primer/único carril objetivo es carril2,
//  y Grupo B el que tiene como primer/único carril objetivo carril1 — ver mo_esCarrilObjetivoUno).

bool mo_esGrupoA(String comando) {
  return comando == "C07" || comando == "C09" || comando == "C13" || comando == "C37" ||
         comando == "C19" || comando == "C31" || comando == "C01" || comando == "C03" ||
         comando == "C05" || comando == "C25" ||
         comando == "C21" || comando == "C27" || comando == "C15" || comando == "C33" ||
         comando == "C07E" || comando == "C09E" || comando == "C13E" || comando == "C37E" ||
         comando == "C19E" || comando == "C31E" || comando == "C01E" || comando == "C03E" ||
         comando == "C05E" || comando == "C25E" ||
         comando == "C27E" || comando == "C33E";
}

bool mo_esGrupoB(String comando) {
  return comando == "C02" || comando == "C04" || comando == "C06" || comando == "C18" ||
         comando == "C24" || comando == "C30" || comando == "C08" || comando == "C10" ||
         comando == "C38" || comando == "C36" ||
         comando == "C14" || comando == "C32" || comando == "C20" || comando == "C26" ||
         comando == "C02E" || comando == "C04E" || comando == "C06E" || comando == "C18E" ||
         comando == "C24E" || comando == "C30E" || comando == "C08E" || comando == "C10E" ||
         comando == "C38E" || comando == "C36E" ||
         comando == "C26E" || comando == "C32E";
}

// ¿El comando (evaluado con el sentido dado) terminó/termina en carril INTERNO?
bool mo_esCarrilInterno(String comando, bool esIzquierda) {
  if (esIzquierda) return mo_esGrupoA(comando);
  else             return mo_esGrupoB(comando);
}

// ¿El comando (evaluado con el sentido dado) es de carril EXTERIOR (para el retroceso largo +2.5s)?
bool mo_esComandoDeCarrilExterior(String comando, bool esIzquierda) {
  if (esIzquierda) return mo_esGrupoB(comando);
  else             return mo_esGrupoA(comando);
}

// Comandos DOBLES: arrancan en un carril y terminan en el otro (cambio de carril automático).
// Su PRIMER carril objetivo es el OPUESTO al carril en el que terminan (mo_esGrupoA/B describe
// el carril final, no el inicial) — confirmado directamente en procesarComandoCarril():
//   C21,C27,C15,C33  -> carriles.carrilObjetivo = 1 primero (aunque son Grupo A, que es "carril2 final")
//   C14,C32,C20,C26  -> carriles.carrilObjetivo = 2 primero (aunque son Grupo B, que es "carril1 final")
bool mo_esComandoDobleEntraCarril1(String comando) {
  return comando == "C21" || comando == "C27" || comando == "C15" || comando == "C33" ||
         comando == "C21E" || comando == "C27E" || comando == "C15E" || comando == "C33E";
}
bool mo_esComandoDobleEntraCarril2(String comando) {
  return comando == "C14" || comando == "C32" || comando == "C20" || comando == "C26" ||
         comando == "C14E" || comando == "C32E" || comando == "C20E" || comando == "C26E";
}

// ¿El primer/único carril objetivo del comando SIGUIENTE es carril 1 o carril 2?
// Esto decide qué patrón de movimiento se usa (retroceso+cruce vs avance-hasta-ToF+cruce).
// Para comandos DOBLES usamos su carril de ENTRADA real (arriba). Para comandos SIMPLES,
// coincide con su único carril, que sí es el mismo que indica su grupo (A/B).
bool mo_primerCarrilObjetivoEsUno(String comandoSiguiente) {
  if (mo_esComandoDobleEntraCarril1(comandoSiguiente)) return true;
  if (mo_esComandoDobleEntraCarril2(comandoSiguiente)) return false;
  return mo_esGrupoB(comandoSiguiente); // comando simple: mismo criterio de siempre
}
bool mo_primerCarrilObjetivoEsDos(String comandoSiguiente) {
  if (mo_esComandoDobleEntraCarril2(comandoSiguiente)) return true;
  if (mo_esComandoDobleEntraCarril1(comandoSiguiente)) return false;
  return mo_esGrupoA(comandoSiguiente); // comando simple: mismo criterio de siempre
}
// ==================== v98: ACTIVAR DETECCIÓN DE OBJETO EN LAS PRIMERAS VUELTAS ====================
// Se llama DESPUÉS de procesarComandoCarril() en ESPERANDO_COMANDO_CARRIL (comando recibido o reproducido).
// Misma regla que la maniobra optimizada: solo si el comando es DOBLE y el tramo anterior terminó en EXTERIOR.
void mo_armarObjetoPrimerasVueltas(String comando) {
  comando.trim();
  mo_confirmarObjetoActivo = false;
  mo_ajusteManteniendoDobleActivo = false;
  mo_sumarManteniendoDobleActivo = false;
  mo_reiniciarDeteccionObjeto();

  bool esDoble = mo_esComandoDobleEntraCarril1(comando) || mo_esComandoDobleEntraCarril2(comando);
  if (!esDoble) return;

  bool hayAnterior = false;
  bool anteriorInterno = false;
  if (sentidoVehiculo != INDEFINIDO && contadorLineas >= 2) {
    int idxAnterior = (contadorLineas - 2) % TOTAL_TRAMOS_PISTA;
    if (idxAnterior < cantidadComandosMemorizados) {
      hayAnterior = true;
      anteriorInterno = mo_esCarrilInterno(comandosMemorizados[idxAnterior], sentidoVehiculo == IZQUIERDA);
    }
  }

  bool activar;
  if (hayAnterior) {
    // v100: anterior interior Y línea >= 5 -> por tiempo (el vehículo ya va junto al objeto). Cualquier otro caso -> detecta objeto
    activar = !(anteriorInterno && contadorLineas >= 5); 
   } else if (contadorLineas < 1) {
    activar = MO_OBJ_ACTIVAR_ESQUIVE_INICIAL;
  } else {
    activar = MO_OBJ_ACTIVAR_SIN_ANTERIOR;
  }

  mo_confirmarObjetoActivo = activar;
  SerialBT.println(String("👁️ Primeras vueltas: ") + comando + " | anterior " +
                   (hayAnterior ? (anteriorInterno ? "INTERIOR" : "EXTERIOR") : "desconocido") +
                   " -> detección de objeto " + (activar ? "ACTIVADA" : "desactivada (por tiempo)"));
}
void procesarComandoCarril(String comando) {
  // v97: limpiar TODOS los flags de comando previo. En vueltas optimizadas no se pasa por
  // ESPERANDO_COMANDO_CARRIL (que era el único lugar donde se limpiaban), y quedaban
  // flags "pegados" (ej. C26 activo durante un C07 -> cambio de carril indebido).
  carriles.esComandoC21 = false;
  carriles.esComandoC14 = false;
  carriles.esComandoC15 = false;
  carriles.esComandoC20 = false;
  carriles.esComandoC26 = false;
  carriles.esComandoC33 = false;
  carriles.esComandoC27 = false;
  carriles.esComandoC32 = false;
  carriles.esComandoC36 = false;
  carriles.esComandoC02 = false;
  carriles.esComandoC08 = false;
  carriles.esComandoC07 = false;
  carriles.esComandoC01 = false;
  carriles.esComandoC01ModoDistancia = false;
  carriles.esComandoC07ModoDistancia = false;
  carriles.esComandoC02ModoDistancia = false;
  carriles.esComandoC08ModoDistancia = false; 
  omitioGiro = false; // v98: se recalcula en EVALUANDO_POSICION; evita que quede "pegado" en vueltas optimizadas
   SerialBT.println("DEBUG comando recibido: [" + comando + "] longitud=" + String(comando.length()));

  int carrilAnterior = carriles.carrilObjetivo;
  //Control Cruce sin pared tiempos 
  // VERDE se esquiva por la izquierda. y posicionado a la izquierda **apartir del 13 son dobles verdes
  //if ( comando == "C07" || comando == "C09" || comando == "C13" || comando == "C37" ||  comando == "C19"|| comando == "C31") {
  
  // COMANDOS POR ESTACIONAMIENTO DEPENDIENDO DEL SENTIDO DE GIRO
  comando.trim(); // quita espacios, \r, \n al inicio y final
  if (comando.endsWith("E") ){
    SerialBT.println("COMANDO CON ESTACIONAMIENTO RECIBIDO");
    if (sentidoVehiculo == DERECHA) {

      if (comando == "C02E" || comando == "C04E" ||comando == "C06E" ||comando == "C18E"||comando == "C24E"||comando == "C30E" || comando == "C08E" || comando == "C10E"||comando == "C38E"||comando == "C36E") {
        carriles.carrilObjetivo = 1;
        carriles.esComandoC02 = true;
        carriles.distanciaCarril1 = DIST_CARRIL1_CERCA_PARED_DER;//🔴
        //carriles.distanciaCarril1 = 17.0; 
        carriles.esComandoC21 = false;  // Resetear flag 
        carriles.esComandoC14 = false;  // Resetear flag C14 (faltaba, causaba arrastre entre vueltas)
          // Modo distancia solo para C07 con sentido IZQUIERDA (giro a carril 2)
          if (comando == "C02E" || comando == "C04E" ||comando == "C06E" ||comando == "C18E"||comando == "C24E"||comando == "C30E" || comando == "C08E" || comando == "C10E"||comando == "C38E"||comando == "C36E" && sentidoVehiculo == DERECHA) {
            SerialBT.println("COMANDO C02 EJECUTADO");
            carriles.esComandoC02ModoDistancia = true;
            //carriles.distanciaCarril2 = 38.0; 
            // Medir distancia derecha y calcular tiempo de giro proporcional
            float distTof = leerToF('I');
            // Clampear entre 30cm (máx tiempo) y 65cm (mín tiempo)
            float distClamped = constrain(distTof, 30.0, 65.0);
            // Interpolación lineal: 30cm → 1800ms, 65cm → 8F00ms
            //float tiempoC01 = 1800.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1800.0 - 800.0);
            //            float tiempoC01 = 1600.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1600.0 - 500.0);

            float tiempoC01 = 1800.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1800.0 - 700.0);
            carriles.tiempoGiroCarril1 = (unsigned long)tiempoC01; // v99: C02 va a carril 1, el giro lee tiempoGiroCarril1  
            carriles.tiempoGiroCarril2 = (unsigned long)tiempoC01;
            SerialBT.println("🎯 C02 Modo Distancia: ToF I=" + String(distTof, 1) +
                            "cm → Tiempo giro=" + String((unsigned long)tiempoC01) + "ms");
          } else {
            carriles.esComandoC02ModoDistancia = false;
          }
        
        //  Casaos Rojos a la izquierda. mayor distancia con la pared. 
      }else if (comando == "C01E" || comando == "C03E"|| comando == "C05E"|| comando == "C25E" || comando == "C07E" || comando == "C09E" || comando == "C13E" || comando == "C37E" ||  comando == "C19E"|| comando == "C31E" ) {
        //  Casaos Verde a la derecha. **apartir del 13 son dobles verdes **apartir del 25 son dobles verdes 
        SerialBT.println("COMANDO C01 EJECUTADO VERDE  A LA DERECHA");
        carriles.esComandoC01 = true;
        carriles.carrilObjetivo = 2;
        //carriles.distanciaCarril2 = 40;//35.0;//20
        carriles.distanciaCarril2 = DIST_CARRIL2_LEJOS_PARED_IZQ;//🐸
        carriles.esComandoC21 = false;
        carriles.esComandoC14 = false;
        // Modo distancia solo para C01 con sentido IZQUIERDA (giro a carril 2)
        if (comando == "C01E" || comando == "C03E"|| comando == "C05E"|| comando == "C25E" || comando == "C07E" || comando == "C09E" || comando == "C13E" || comando == "C37E" ||  comando == "C19E"|| comando == "C31E" && sentidoVehiculo == IZQUIERDA) {
          carriles.esComandoC01ModoDistancia = true;
          // Medir distancia derecha y calcular tiempo de giro proporcional
          float distTof = leerToF('D');
          // Clampear entre 30cm (máx tiempo) y 65cm (mín tiempo)
          float distClamped = constrain(distTof, 30.0, 65.0);
          // Interpolación lineal: 30cm → 1800ms, 65cm → 800ms
          float tiempoC01 = 300.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (300.0 - 200.0);
          carriles.tiempoGiroCarril2 = (unsigned long)tiempoC01;
          SerialBT.println("🎯 C01 Modo Distancia: ToF D=" + String(distTof, 1) +
                          "cm → Tiempo giro=" + String((unsigned long)tiempoC01) + "ms");
        } else {
          carriles.esComandoC01ModoDistancia = false;
        }
      }else if (comando == "C27E" || comando == "C33E") { 
        SerialBT.println("COMANDO C27 EJECUTADO");
        carriles.carrilObjetivo = 1;      // 1. El primer objetivo es el carril 1.
        //carriles.distanciaCarril1 = 17.0;//25 EN PRUEBAS
        carriles.distanciaCarril1 = DIST_CARRIL1_ALEJANDOSE_PARED_DER;//📛
        carriles.distanciaCarril2 = DIST_CARRIL2_LEJOS_PARED_IZQ;//🐸
        //carriles.distanciaCarril2 = 40.0;
        carriles.esComandoC27 = true;      // 2. Activamos el flag especial para C27.
        // 3. Nos aseguramos de que los otros flags estén desactivados.
        carriles.esComandoC21 = false;
        
        carriles.esComandoC14 = false;
        carriles.esComandoC15 = false;
        carriles.esComandoC20 = false;
        // Tiempo dinámico por distancia ──
        if (sentidoVehiculo == DERECHA) {
          float distTof = leerToF('I');
          float distClamped = constrain(distTof, 30.0, 65.0);
          float tiempoCalculado = 1600.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1600.0 - 500.0);
          //float tiempoCalculado = 1800.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1800.0 - 800.0);
          //float tiempoC01 = 1800.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1800.0 - 800.0);
          carriles.tiempoGiroCarril1 = (unsigned long)tiempoCalculado;
          SerialBT.println("🎯 C27 Modo Distancia: ToF I=" + String(distTof, 1) +
                          "cm → Tiempo giro=" + String((unsigned long)tiempoCalculado) + "ms");
        }
        SerialBT.println("🔄 Comando C27: A Carril 1, luego a Carril 2.");
      // Comando especial C15 - primero va a carril 2, luego a carril 1
      }else if (comando == "C26E" || comando == "C32E") {  //Verde, Adelante Derecha y Rojo, Atrás Derecha)
        SerialBT.println("COMANDO C26 EJECUTADO");
        carriles.carrilObjetivo = 2;      // 1. El primer objetivo es el carril 2.
        carriles.distanciaCarril2 = DIST_CARRIL2_LEJOS_PARED_IZQ;//🐸
        //carriles.distanciaCarril2 = 40.0;
        carriles.distanciaCarril1 = DIST_CARRIL1_CERCA_PARED_DER;//🔴
        //carriles.distanciaCarril1 = 17.0;
        carriles.esComandoC26 = true;      // 2. Activamos el flag especial para C26.
        // 3. Nos aseguramos de que los otros flags estén desactivados.
        carriles.esComandoC21 = false;
        if (carriles.esComandoC14){
            SerialBT.println("reseteado EscomandoC14 en c26*/*****  ********* *******");
          }
        carriles.esComandoC14 = false;
        carriles.esComandoC15 = false;
        carriles.esComandoC20 = false;
        // ── NUEVO: Tiempo dinámico por distancia ──
        if (sentidoVehiculo == IZQUIERDA) {
          float distTof = leerToF('D');
          float distClamped = constrain(distTof, 30.0, 65.0);
          float tiempoCalculado = 1100.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1100.0 - 300.0);
          carriles.tiempoGiroCarril1 = (unsigned long)tiempoCalculado;
          SerialBT.println("🎯 C26 Modo Distancia: ToF D=" + String(distTof, 1) +
                          "cm → Tiempo giro=" + String((unsigned long)tiempoCalculado) + "ms");
        }
        SerialBT.println("🔄 Comando C26: A Carril 2 (140°), luego a Carril 1 (20°).");
      }
      
    }else if (sentidoVehiculo == IZQUIERDA) {
      
      if ( comando == "C08E" || comando == "C10E"||comando == "C38E"||comando == "C36E" || comando == "C02E" || comando == "C04E"  ||comando == "C06E" ||comando == "C18E"||comando == "C24E"||comando == "C30E") {
        SerialBT.println("COMANDO C08 EJECUTADO ROJO IZQUIERDA");
        carriles.esComandoC08 = true;
        carriles.carrilObjetivo = 1;
        //carriles.distanciaCarril1 = 35.0; // Cambia la distancia solo para este comando
        carriles.distanciaCarril1 = DIST_CARRIL1_LEJOS_PARED_DER;//🥊
        carriles.esComandoC21 = false;
        carriles.esComandoC14 = false;
          // Modo distancia solo para C01 con sentido IZQUIERDA (giro a carril 2)
        if (comando == "C08E" || comando == "C10E"||comando == "C38E"||comando == "C36E" || comando == "C02E" || comando == "C04E"  ||comando == "C06E" ||comando == "C18E"||comando == "C24E"||comando == "C30E" && sentidoVehiculo == DERECHA) {
          carriles.esComandoC08ModoDistancia = true;
          // Medir distancia derecha y calcular tiempo de giro proporcional
          float distTof = leerToF('I');
          // Clampear entre 30cm (máx tiempo) y 65cm (mín tiempo)
          float distClamped = constrain(distTof, 30.0, 65.0);
          // Interpolación lineal: 30cm → 1800ms, 65cm → 800ms
          //float tiempoC01 = 1100.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1100.0 - 300.0);
          float tiempoC01 = 800.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (800.0 - 300.0);
          carriles.tiempoGiroCarril2 = (unsigned long)tiempoC01;
          SerialBT.println("🎯 C01 Modo Distancia: ToF D=" + String(distTof, 1) +
                          "cm → Tiempo giro=" + String((unsigned long)tiempoC01) + "ms");
        } else {
          carriles.esComandoC08ModoDistancia = false;
        }

      }else if (comando == "C07E" || comando == "C09E" || comando == "C13E" || comando == "C37E" ||  comando == "C19E"|| comando == "C31E" || comando == "C01E" || comando == "C03E"|| comando == "C05E"|| comando == "C25E") {  
        SerialBT.println("COMANDO C07 EJECUTADO VERDE IZQUIERDA");
        carriles.esComandoC07 = true;
        carriles.carrilObjetivo = 2;
        //carriles.distanciaCarril2 = 22.0; // Restablece la distancia al valor por defecto
        carriles.distanciaCarril2 = DIST_CARRIL2_CERCA_PARED_IZQ;//🟢
        carriles.esComandoC21 = false;
        carriles.esComandoC14 = false;
        // Modo distancia solo para C07 con sentido IZQUIERDA (giro a carril 2)
          if (comando == "C07E" || comando == "C09E" || comando == "C13E" || comando == "C37E" ||  comando == "C19E"|| comando == "C31E" || comando == "C01E" || comando == "C03E"|| comando == "C05E"|| comando == "C25E"&& sentidoVehiculo == IZQUIERDA) {
            carriles.esComandoC07ModoDistancia = true;
            // Medir distancia derecha y calcular tiempo de giro proporcional
            float distTof = leerToF('D');
            // Clampear entre 30cm (máx tiempo) y 65cm (mín tiempo)
            float distClamped = constrain(distTof, 30.0, 65.0);
            // Interpolación lineal: 30cm → 1800ms, 65cm → 800ms
            //float tiempoCalculado = 1300.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1300.0 - 500.0);

            float tiempoC01 = 1650.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1650.0 - 650.0);
            carriles.tiempoGiroCarril2 = (unsigned long)tiempoC01;
            SerialBT.println("🎯 C01 Modo Distancia: ToF D=" + String(distTof, 1) +
                            "cm → Tiempo giro=" + String((unsigned long)tiempoC01) + "ms");
          } else {
            carriles.esComandoC07ModoDistancia = false;
          }
      
      }else if (comando == "C32E") {
        SerialBT.println("COMANDO C32 EJECUTADO");
        carriles.carrilObjetivo = 2;      // 1. The first target is lane 1.
        //carriles.distanciaCarril2 = 26.0;
        carriles.distanciaCarril2 = DIST_CARRIL2_ALEJANDOSE_PARED_IZQ;//❎ PARA QUE VAYA ALEJANDOSE DE LA PARED
        carriles.distanciaCarril1 = DIST_CARRIL1_LEJOS_PARED_DER;//🥊
        
        carriles.esComandoC32 = true;       // 2. Activate the special flag for C33.
        // 3. Make sure other special flags are disabled.
        carriles.esComandoC21 = false;
        
        carriles.esComandoC14 = false;
        carriles.esComandoC15 = false;
        carriles.esComandoC20 = false;
        carriles.esComandoC26 = false;
        if (sentidoVehiculo == IZQUIERDA) {
          float distTof = leerToF('D');
          float distClamped = constrain(distTof, 30.0, 60.0);
          //float tiempoCalculado = 1100.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1100.0 - 300.0);
          float tiempoCalculado = 1400.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1400.0 - 400.0);
          carriles.tiempoGiroCarril2 = (unsigned long)tiempoCalculado;
          SerialBT.println("🎯 C32 Modo Distancia: ToF D=" + String(distTof, 1) +
                          "cm → Tiempo giro=" + String((unsigned long)tiempoCalculado) + "ms");
        }
        SerialBT.println("🔄 Comando C32: A Carril 2 (160°), luego a Carril 1 (20°).");
        // Comandos para carril 1  DERECHA COMANDOS PARA ROJO. rojos a la derecha.
      }else if (comando == "C33E"||comando == "C27E") { //Rojo, Adelante Izquierda y Verde, Atrás Izquierda)
        SerialBT.println("COMANDO C33 EJECUTADO");
        carriles.carrilObjetivo = 1;      
        //carriles.distanciaCarril1 = 35.0; // Cambia la distancia solo para este comando
        //carriles.distanciaCarril2 = 22.0; 
        carriles.distanciaCarril1 = DIST_CARRIL1_LEJOS_PARED_DER;//🥊
        //carriles.distanciaCarril2 = DIST_CARRIL2_CERCA_PARED_IZQ;//🟢 PARA QUE VAYA ALEJANDOSE DE LA PARED
        carriles.distanciaCarril2 = DIST_CARRIL2_CERCA_PARED_IZQ;
        carriles.esComandoC33 = true;       
        carriles.esComandoC21 = false;
        carriles.esComandoC14 = false;
        carriles.esComandoC15 = false;
        carriles.esComandoC20 = false;
        carriles.esComandoC26 = false;
        SerialBT.println("🔄 Comando C33: To Lane 1 (20°), then to Lane 2 (140°).");
        // ── NUEVO: Tiempo dinámico por distancia ──
        if (sentidoVehiculo == DERECHA) {
          float distTof = leerToF('I');
          float distClamped = constrain(distTof, 30.0, 65.0);
          float tiempoCalculado = 800.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (800.0 - 300.0);
          carriles.tiempoGiroCarril1 = (unsigned long)tiempoCalculado;
          SerialBT.println("🎯 C33 Modo Distancia: ToF I=" + String(distTof, 1) +
                          "cm → Tiempo giro=" + String((unsigned long)tiempoCalculado) + "ms");
        }  
      }
    }    
  }else if (comando == "C07" || comando == "C09" || comando == "C13" || comando == "C37" ||  comando == "C19"|| comando == "C31" || comando == "C01" || comando == "C03"|| comando == "C05"|| comando == "C25") {  
    carriles.esComandoC07 = true;
    carriles.carrilObjetivo = 2;
    //carriles.distanciaCarril2 = 22.0; //25 modificacion en casa 15/8/2026
    carriles.distanciaCarril2 = DIST_CARRIL2_CERCA_PARED_IZQ;//🟢
    carriles.esComandoC21 = false;
    // Modo distancia solo para C07 con sentido IZQUIERDA (giro a carril 2)
      if ((comando == "C07" || comando == "C09" || comando == "C13" || comando == "C37" ||  comando == "C19"|| comando == "C31" || comando == "C01" || comando == "C03"|| comando == "C05"|| comando == "C25") && sentidoVehiculo == IZQUIERDA) {
        carriles.esComandoC07ModoDistancia = true;
        // Medir distancia derecha y calcular tiempo de giro proporcional
        float distTof = leerToF('D');
        // Clampear entre 30cm (máx tiempo) y 65cm (mín tiempo)
        float distClamped = constrain(distTof, 30.0, 65.0);
        // Sentido IZQUIERDA: CERCA de la pared (30cm) → MÁS tiempo de giro.
        // LEJOS de la pared (65cm) → MENOS tiempo de giro.
        // Interpolación lineal: 30cm → 1100ms, 65cm → 300ms
        //float tiempoC01 = 1200.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1200.0 - 400.0);
        float tiempoC01 = 1650.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1650.0 - 650.0);
        carriles.tiempoGiroCarril2 = (unsigned long)tiempoC01;
        SerialBT.println("🎯 C07 Modo Distancia: ToF D=" + String(distTof, 1) +
                        "cm → Tiempo giro=" + String((unsigned long)tiempoC01) + "ms");
      } else if ((comando == "C07" || comando == "C09" || comando == "C13" || comando == "C37" ||  comando == "C19"|| comando == "C31" || comando == "C01" || comando == "C03"|| comando == "C05"|| comando == "C25") && sentidoVehiculo == DERECHA) {
        carriles.esComandoC07ModoDistancia = true;
        // Medir distancia izquierda y calcular tiempo de giro proporcional
        float distTof = leerToF('I');
        // Clampear entre 30cm (mín tiempo) y 65cm (máx tiempo)
        float distClamped = constrain(distTof, 30.0, 65.0);
        // Sentido DERECHA: CERCA de la pared (30cm) → MENOS tiempo de giro.
        // LEJOS de la pared (65cm) → MÁS tiempo de giro.
        // Interpolación lineal: 30cm → 750ms, 65cm → 1900ms
        // (antes 400→1400ms: a 38-44cm daba 630-800ms y no alcanzaba a cruzar; el avance arranca con rampa)
        float tiempoC07Der = 750.0 + ((distClamped - 30.0) / (65.0 - 30.0)) * (1900.0 - 750.0);

        carriles.tiempoGiroCarril2 = (unsigned long)tiempoC07Der;
        SerialBT.println("🎯 C07 Modo Distancia (DER): ToF I=" + String(distTof, 1) +
                        "cm → Tiempo giro=" + String((unsigned long)tiempoC07Der) + "ms");
      } else {
        carriles.esComandoC07ModoDistancia = false;
      }
  
  }else if (comando == "C01" || comando == "C03"|| comando == "C05"|| comando == "C25") {
    //  Casaos Verde a la derecha. **apartir del 13 son dobles verdes **apartir del 25 son dobles verdes 
    carriles.esComandoC01 = true;
    carriles.carrilObjetivo = 2;
    carriles.distanciaCarril2 = 20.0;
    carriles.esComandoC21 = false;

    // Modo distancia solo para C01 con sentido IZQUIERDA (giro a carril 2)
    if (comando == "C01" || comando == "C03"|| comando == "C05"|| comando == "C25" && sentidoVehiculo == IZQUIERDA) {
      carriles.esComandoC01ModoDistancia = true;
      // Medir distancia derecha y calcular tiempo de giro proporcional
      float distTof = leerToF('D');
      // Clampear entre 30cm (máx tiempo) y 65cm (mín tiempo)
      float distClamped = constrain(distTof, 30.0, 65.0);
      // Interpolación lineal: 30cm → 1800ms, 65cm → 800ms
      float tiempoC01 = 1100.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1100.0 - 300.0);
      carriles.tiempoGiroCarril2 = (unsigned long)tiempoC01;
      SerialBT.println("🎯 C01 Modo Distancia: ToF D=" + String(distTof, 1) +
                       "cm → Tiempo giro=" + String((unsigned long)tiempoC01) + "ms");
    } else {
      carriles.esComandoC01ModoDistancia = false;
    }

  // Comando especial C21 - primero va a carril 1, luego a carril 2
  //} else if (comando == "C21") {
  } else if (comando == "C21" || comando =="C27" || comando =="C15" || comando =="C33") {
    carriles.carrilObjetivo = 1;    // Primero ir a carril 1
    carriles.esComandoC21 = true;   // Marcar como comando especial
    //carriles.distanciaCarril1 = 25.0; //12 en c02
    carriles.distanciaCarril1 = DIST_CARRIL1_CERCA_PARED_DER;//🔴
    //carriles.distanciaCarril2 = 25.0;
    carriles.distanciaCarril2 = DIST_CARRIL2_CERCA_PARED_IZQ;//🟢 PARA QUE VAYA ALEJANDOSE DE LA PARED

    // ── NUEVO: Tiempo dinámico por distancia ──
    if (sentidoVehiculo == DERECHA) {
      float distTof = leerToF('I');
      //float distClamped = constrain(distTof, 30.0, 65.0);
      float distClamped = constrain(distTof, 30.0, 65.0);
      float tiempoCalculado = 1500.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1500.0 - 400.0);
      carriles.tiempoGiroCarril1 = (unsigned long)tiempoCalculado;
      SerialBT.println("🎯 C21 Modo Distancia: ToF D=" + String(distTof, 1) +
                       "cm → Tiempo giro=" + String((unsigned long)tiempoCalculado) + "ms");
    } else if ((comando == "C21" || comando =="C27" || comando =="C15" || comando =="C33") && sentidoVehiculo == IZQUIERDA) {
        // Medir distancia derecha y calcular tiempo de giro proporcional
        float distTof = leerToF('D');
        // Clampear entre 30cm (mín tiempo) y 65cm (máx tiempo)
        float distClamped = constrain(distTof, 30.0, 65.0);
        // Sentido IZQUIERDA: CERCA de la pared (30cm) → MENOS tiempo de giro.
        // LEJOS de la pared (65cm) → MÁS tiempo de giro.
        // Interpolación lineal: 30cm → 300ms, 65cm → 1100ms
        float tiempoC21Izq = 300.0 + ((distClamped - 30.0) / (65.0 - 30.0)) * (1100.0 - 300.0);
        carriles.tiempoGiroCarril2 = (unsigned long)tiempoC21Izq;
        SerialBT.println("🎯 C21 Modo Distancia (IZQ): ToF D=" + String(distTof, 1) +
                        "cm → Tiempo giro=" + String((unsigned long)tiempoC21Izq) + "ms");
      }
    SerialBT.println("🔄 Comando C21: A Carril 1, luego a Carril 2.");  // C27 Rojo, Adelante Derecha y Verde, Atrás Derecha
  }else if (comando == "C27") { 
    carriles.carrilObjetivo = 1;      // 1. El primer objetivo es el carril 1.
    carriles.esComandoC27 = true;      // 2. Activamos el flag especial para C27.
    // 3. Nos aseguramos de que los otros flags estén desactivados.
    carriles.esComandoC21 = false;
    if (carriles.esComandoC14){
        SerialBT.println("reseteado EscomandoC14 c27*/*****  ********* *******");
      }
    carriles.esComandoC14 = false;
    carriles.esComandoC15 = false;
    carriles.esComandoC20 = false;
    // Tiempo dinámico por distancia ──
    if (sentidoVehiculo == DERECHA) {
      float distTof = leerToF('I');
      float distClamped = constrain(distTof, 30.0, 65.0);
      float tiempoCalculado = 1500.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1500.0 - 600.0);
      carriles.tiempoGiroCarril1 = (unsigned long)tiempoCalculado;
      SerialBT.println("🎯 C27 Modo Distancia: ToF I=" + String(distTof, 1) +
                       "cm → Tiempo giro=" + String((unsigned long)tiempoCalculado) + "ms");
    }
    SerialBT.println("🔄 Comando C27: A Carril 1, luego a Carril 2.");
  // Comando especial C15 - primero va a carril 2, luego a carril 1
  } else if (comando == "C15") { 
    carriles.carrilObjetivo = 1;      // 1. El primer objetivo es el carril 1.
    carriles.esComandoC15 = true;     // 2. Activamos el flag especial para C15.
    carriles.esComandoC21 = false;    // 3. Nos aseguramos de que los otros flags estén desactivados.
   
    carriles.esComandoC14 = false;
    SerialBT.println("🔄 Comando C15: A Carril 1 (40°), luego a Carril 2 (140°).");
    // ── NUEVO: Tiempo dinámico por distancia ──
    if (sentidoVehiculo == DERECHA) {
      float distTof = leerToF('I');
      float distClamped = constrain(distTof, 30.0, 65.0);
      float tiempoCalculado = 800.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (800.0 - 300.0);
      carriles.tiempoGiroCarril1 = (unsigned long)tiempoCalculado;
      SerialBT.println("🎯 C15 Modo Distancia: ToF I=" + String(distTof, 1) +
                       "cm → Tiempo giro=" + String((unsigned long)tiempoCalculado) + "ms");
    }
  //}  else if (comando == "C14") {
  }  else if (comando == "C14" || comando =="C32" || comando =="C20" || comando =="C26") {  
    carriles.carrilObjetivo = 2;    // Primero ir a carril 2
    carriles.esComandoC14 = true;   // Marcar como comando especial
     //carriles.distanciaCarril2 = 25.0;
    carriles.distanciaCarril2 = DIST_CARRIL2_CERCA_PARED_IZQ;//🟢
     //carriles.distanciaCarril1 = 17.0;
     carriles.distanciaCarril1 = DIST_CARRIL1_CERCA_PARED_DER;//🔴

    carriles.esComandoC21 = false;  // Resetear flag C21
    // ── NUEVO: Tiempo dinámico por distancia ──
    if (sentidoVehiculo == IZQUIERDA) {
      float distTof = leerToF('D');
      float distClamped = constrain(distTof, 30.0, 65.0);
      float tiempoCalculado = 1650.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1650.0 - 650.0);//1400 400
      carriles.tiempoGiroCarril2 = (unsigned long)tiempoCalculado;
      SerialBT.println("🎯 C14 Modo Distancia: ToF D=" + String(distTof, 1) +
                       "cm → Tiempo giro=" + String((unsigned long)tiempoCalculado) + "ms");
    } else if ((comando == "C14" || comando =="C32" || comando =="C20" || comando =="C26") && sentidoVehiculo == DERECHA) {
         carriles.carrilObjetivo = 2;    // Primero ir a carril 2
        carriles.esComandoC14 = true;   // Marcar como comando especial
        //carriles.distanciaCarril2 = 30.0;
        carriles.esComandoC21 = false;  // Resetear flag C21
        // Medir distancia izquierda y calcular tiempo de giro proporcional
        float distTof = leerToF('I');
        // Clampear entre 30cm (mín tiempo) y 65cm (máx tiempo)
        float distClamped = constrain(distTof, 30.0, 65.0);
        // Sentido DERECHA: CERCA de la pared (30cm) → MENOS tiempo de giro.
        // LEJOS de la pared (65cm) → MÁS tiempo de giro.
        // Interpolación lineal: 30cm → 300ms, 65cm → 1100ms
        float tiempoC14Der = 750.0 + ((distClamped - 30.0) / (65.0 - 30.0)) * (1900.0 - 750.0);
        carriles.tiempoGiroCarril2 = (unsigned long)tiempoC14Der;
        SerialBT.println("🎯 C14 Modo Distancia (DER): ToF I=" + String(distTof, 1) +
                        "cm → Tiempo giro=" + String((unsigned long)tiempoC14Der) + "ms");
      }
    SerialBT.println("🔄 Comando C14: Primero carril 2, luego carril 1 automáticamente");
  
  // Comandos para carril 1  DERECHA COMANDOS PARA ROJO. rojos a la derecha.
  }else if (comando == "C32") {
    carriles.carrilObjetivo = 2;      // 1. The first target is lane 1.
    carriles.esComandoC32 = true;       // 2. Activate the special flag for C33.
    // 3. Make sure other special flags are disabled.
    carriles.esComandoC21 = false;
    if (carriles.esComandoC14){
        SerialBT.println("reseteado EscomandoC14 en c32*/*****  ********* *******");
      }
    carriles.esComandoC14 = false;
    carriles.esComandoC15 = false;
    carriles.esComandoC20 = false;
    carriles.esComandoC26 = false;
    if (sentidoVehiculo == IZQUIERDA) {
      float distTof = leerToF('D');
      float distClamped = constrain(distTof, 30.0, 50.0);
      //float tiempoCalculado = 1100.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1100.0 - 300.0);
      float tiempoCalculado = 1400.0 - ((distClamped - 30.0) / (50.0 - 30.0)) * (1400.0 - 600.0);
      carriles.tiempoGiroCarril2 = (unsigned long)tiempoCalculado;
      SerialBT.println("🎯 C32 Modo Distancia: ToF D=" + String(distTof, 1) +
                       "cm → Tiempo giro=" + String((unsigned long)tiempoCalculado) + "ms");
    }
    SerialBT.println("🔄 Comando C32: A Carril 2 (160°), luego a Carril 1 (20°).");
  // Comandos para carril 1  DERECHA COMANDOS PARA ROJO. rojos a la derecha.
  }  else if (comando == "C20") { 
    carriles.carrilObjetivo = 2;      // 1. El primer objetivo es el carril 2.
    carriles.distanciaCarril2 = 35.0;
    carriles.esComandoC20 = true;     // 2. Activamos el flag especial para C20.
    carriles.esComandoC21 = false;    // 3. Nos aseguramos de que los otros flags estén desactivados.
    if (carriles.esComandoC14){
        SerialBT.println("reseteado EscomandoC14 en c20*/*****  ********* *******");
      }
    carriles.esComandoC14 = false;
    carriles.esComandoC15 = false;
    
    SerialBT.println("🔄 Comando C20: A Carril 2 (140°), luego a Carril 1 (40°).");
    // ── NUEVO: Tiempo dinámico por distancia ──
    if (sentidoVehiculo == IZQUIERDA) {
      float distTof = leerToF('D');  
      float distClamped = constrain(distTof, 30.0, 65.0);
      float tiempoCalculado = 800.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (800.0 - 300.0);
      carriles.tiempoGiroCarril1 = (unsigned long)tiempoCalculado;
      SerialBT.println("🎯 C20 Modo Distancia: ToF D=" + String(distTof, 1) +
                       "cm → Tiempo giro=" + String((unsigned long)tiempoCalculado) + "ms");
    }
  } else if (comando == "C26") {  //Verde, Adelante Derecha y Rojo, Atrás Derecha)
    carriles.carrilObjetivo = 2;      // 1. El primer objetivo es el carril 2.
    carriles.esComandoC26 = true;      // 2. Activamos el flag especial para C26.
    // 3. Nos aseguramos de que los otros flags estén desactivados.
    carriles.esComandoC21 = false;
    if (carriles.esComandoC14){
        SerialBT.println("reseteado EscomandoC14 en c26*/*****  ********* *******");
      }
    carriles.esComandoC14 = false;
    carriles.esComandoC15 = false;
    carriles.esComandoC20 = false;
    // ── NUEVO: Tiempo dinámico por distancia ──
    if (sentidoVehiculo == IZQUIERDA) {
      float distTof = leerToF('D');
      float distClamped = constrain(distTof, 30.0, 65.0);
      float tiempoCalculado = 1100.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1100.0 - 300.0);
      carriles.tiempoGiroCarril1 = (unsigned long)tiempoCalculado;
      SerialBT.println("🎯 C26 Modo Distancia: ToF D=" + String(distTof, 1) +
                       "cm → Tiempo giro=" + String((unsigned long)tiempoCalculado) + "ms");
    }
    SerialBT.println("🔄 Comando C26: A Carril 2 (140°), luego a Carril 1 (20°).");

  } else if (comando == "C33") { //Rojo, Adelante Izquierda y Verde, Atrás Izquierda)
    carriles.carrilObjetivo = 1;      
    carriles.esComandoC33 = true;       
    carriles.esComandoC21 = false;
    if (carriles.esComandoC14){
        SerialBT.println("reseteado EscomandoC14 en c33*/*****  ********* *******");
      }
    carriles.esComandoC14 = false;
    carriles.esComandoC15 = false;
    carriles.esComandoC20 = false;
    carriles.esComandoC26 = false;
    SerialBT.println("🔄 Comando C33: To Lane 1 (20°), then to Lane 2 (140°).");
    // ── NUEVO: Tiempo dinámico por distancia ──
    if (sentidoVehiculo == DERECHA) {
      float distTof = leerToF('I');
      float distClamped = constrain(distTof, 30.0, 65.0);
      float tiempoCalculado = 800.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (800.0 - 300.0);
      carriles.tiempoGiroCarril1 = (unsigned long)tiempoCalculado;
      SerialBT.println("🎯 C33 Modo Distancia: ToF I=" + String(distTof, 1) +
                       "cm → Tiempo giro=" + String((unsigned long)tiempoCalculado) + "ms");
    }

  //} else if (comando == "C02" || comando == "C04"  ||comando == "C06" ||comando == "C18"||comando == "C24"||comando == "C30") {
  
  } else if (comando == "C02" || comando == "C04"  ||comando == "C06" ||comando == "C18"||comando == "C24"||comando == "C30" || comando == "C08" || comando == "C10"||comando == "C38"||comando == "C36") {
    carriles.carrilObjetivo = 1;
    carriles.esComandoC02 = true;
    //carriles.distanciaCarril1 = 17;//12.0; 
     carriles.distanciaCarril1 = DIST_CARRIL1_CERCA_PARED_DER;//🔴
    carriles.esComandoC21 = false;  // Resetear flag 
      // Modo distancia solo para C07 con sentido IZQUIERDA (giro a carril 2)
       if ((comando == "C02" || comando == "C04"  ||comando == "C06" ||comando == "C18"||comando == "C24"||comando == "C30" || comando == "C08" || comando == "C10"||comando == "C38"||comando == "C36") && sentidoVehiculo == DERECHA) {
        carriles.esComandoC02ModoDistancia = true;
        carriles.distanciaCarril2 = 38.0; 
        // Medir distancia izquierda y calcular tiempo de giro proporcional
        float distTof = leerToF('I');
        // Clampear entre 30cm (máx tiempo) y 65cm (mín tiempo)
        //float distClamped = constrain(distTof, 30.0, 65.0);
        float distClamped = constrain(distTof, 30.0, 65.0);
        // Sentido DERECHA: CERCA de la pared (30cm) → MÁS tiempo de giro.
        // LEJOS de la pared (65cm) → MENOS tiempo de giro.
        // Interpolación lineal: 30cm → 1800ms, 65cm → 800ms
        
        float tiempoC01 = 1700.0 - ((distClamped - 30.0) / (60.0 - 25.0)) * (1700.0 - 600.0);//1500 400
        carriles.tiempoGiroCarril1 = (unsigned long)tiempoC01; // v99: C02 va a carril 1, el giro lee tiempoGiroCarril1

        carriles.tiempoGiroCarril2 = (unsigned long)tiempoC01;
        SerialBT.println("🎯 C02 Modo Distancia: ToF I=" + String(distTof, 1) +
                        "cm → Tiempo giro=" + String((unsigned long)tiempoC01) + "ms");
      } else if ((comando == "C02" || comando == "C04"  ||comando == "C06" ||comando == "C18"||comando == "C24"||comando == "C30" || comando == "C08" || comando == "C10"||comando == "C38"||comando == "C36") && sentidoVehiculo == IZQUIERDA) {
        carriles.esComandoC02ModoDistancia = true;
        carriles.distanciaCarril2 = 38.0;
        // Medir distancia derecha y calcular tiempo de giro proporcional
        float distTof = leerToF('D');
        // Clampear entre 30cm (mín tiempo) y 65cm (máx tiempo)
        float distClamped = constrain(distTof, 30.0, 65.0);
        // Sentido IZQUIERDA: CERCA de la pared (30cm) → MENOS tiempo de giro.
        // LEJOS de la pared (65cm) → MÁS tiempo de giro.
        // Interpolación lineal: 30cm → 300ms, 65cm → 1100ms
        // 30cm → 950ms, 65cm → 2050ms (a ~40cm ≈ 1280ms, igual al valor por defecto que ya funcionaba)
        float tiempoC02Izq = 950.0 + ((distClamped - 30.0) / (65.0 - 30.0)) * (2050.0 - 950.0);
        carriles.tiempoGiroCarril1 = (unsigned long)tiempoC02Izq; // C02 va a carril 1: el giro lee tiempoGiroCarril1
        SerialBT.println("🎯 C02 Modo Distancia (IZQ): ToF D=" + String(distTof, 1) +
                        "cm → Tiempo giro=" + String((unsigned long)tiempoC02Izq) + "ms");
      } else {
        carriles.esComandoC02ModoDistancia = false;
      }
  //  Casaos Rojos a la izquierda. mayor distancia con la pared. 
  } else if (comando == "C08" || comando == "C10"||comando == "C38"||comando == "C36") {
    carriles.esComandoC08 = true;
    carriles.carrilObjetivo = 1;
    carriles.distanciaCarril1 = 20.0; // Cambia la distancia solo para este comando
    carriles.esComandoC21 = false;
      // Modo distancia solo para C01 con sentido IZQUIERDA (giro a carril 2)
    if (comando == "C08" || comando == "C10"||comando == "C38"||comando == "C36" && sentidoVehiculo == DERECHA) {
      carriles.esComandoC08ModoDistancia = true;
      // Medir distancia derecha y calcular tiempo de giro proporcional
      float distTof = leerToF('I');
      // Clampear entre 30cm (máx tiempo) y 65cm (mín tiempo)
      float distClamped = constrain(distTof, 30.0, 65.0);
      // Interpolación lineal: 30cm → 1800ms, 65cm → 800ms
      //float tiempoC01 = 1100.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (1100.0 - 300.0);
      float tiempoC01 = 800.0 - ((distClamped - 30.0) / (65.0 - 30.0)) * (800.0 - 300.0);
      carriles.tiempoGiroCarril2 = (unsigned long)tiempoC01;
      SerialBT.println("🎯 C01 Modo Distancia: ToF D=" + String(distTof, 1) +
                       "cm → Tiempo giro=" + String((unsigned long)tiempoC01) + "ms");
    } else {
      carriles.esComandoC08ModoDistancia = false;
    }

  // Comando especial C21 - primero va a carril 1, luego a carril 2
  }  else {
    SerialBT.println(" Comando no reconocido: " + comando);
    return;
  }
   if (carriles.carrilObjetivo == 2) {
      carriles.direccionGiro = -1.0; // Valor negativo para giro a la IZQUIERDA
      SerialBT.println("   -> Dirección de giro establecida a: IZQUIERDA");
     //  Destello Verde por 400ms (No bloqueante)
      encenderLedTemporal(0, 255, 0, 400);
  } else if (carriles.carrilObjetivo == 1) {
      carriles.direccionGiro = 1.0; // Valor positivo para giro a la DERECHA
      SerialBT.println("   -> Dirección de giro establecida a: DERECHA");
     // Destello Rojo por 400ms (No bloqueante)
      encenderLedTemporal(255, 0, 0, 400);
  }
  // CAMBIO PRINCIPAL: Inicia el cooldown de detección de líneas INMEDIATAMENTE.
  // Esto evita que se detecte una línea mientras se evalúa la posición, antes de empezar el giro.
  tiempoUltimaDeteccion = millis();
  SerialBT.println("COOLDOWN DE DETECCIÓN DE LÍNEAS ACTIVADO INMEDIATAMENTE.");

  SerialBT.println(" Comando recibido: " + comando + " → Carril " + String(carriles.carrilObjetivo));
  
  // Si es el primer giro (y es uno de los comandos de inicio), salta directo a la acción
  if (!primerGiroRealizado && (comando == "C01" || comando == "C02" || comando == "C07" || comando == "C08")) {
    SerialBT.println(">>> PRIMER GIRO: Saltando evaluación, yendo directo a GIRANDO_HACIA_CARRIL.");
    cambiarEstado(GIRANDO_HACIA_CARRIL); 
  } 
  // Para cualquier otro caso, sigue la lógica normal
  else if (carrilAnterior != carriles.carrilObjetivo || estadoActual == ESPERANDO_COMANDO_CARRIL) { 
    cambiarEstado(EVALUANDO_POSICION); 
  }
}
void resetSistema(bool conservarAngulo = true) {
  // Reset de variables de control lateral
  controlLateral.integralDistancia = 0;
  controlLateral.errorDistanciaAnterior = 0;
  controlLateral.filtroIzquierda.inicializar();
  controlLateral.filtroDerecha.inicializar();

  // Reset de flags de comandos especiales
  carriles.esComandoC21 = false;
  if (carriles.esComandoC14){
        SerialBT.println("reseteado EscomandoC14 en reset sistem*/*****  ********* *******");
      }
  carriles.esComandoC14 = false;

  // Reset de estado del sistema de carriles
  carriles.carrilObjetivo = 1;
  carriles.ladoControl = 'D';
  estadoActual = ESTADO_NORMAL; // Asigna a la variable global 'estadoActual'
  carriles.tiempoInicioEstado = millis();
  carriles.necesitaCambio = false;

  // Recentrar servo
 // direccion.write(90);

  SerialBT.println("🔄 Sistema reseteado" + String(conservarAngulo ? " (ángulo conservado)" : " (ángulo reiniciado)"));
}
// Función auxiliar para generar pulsos cortos (volumen bajo)
void buzzerVolumenBajo(int duracionTotal) {
  int tiempoTranscurrido = 0;
  while (tiempoTranscurrido < duracionTotal) {
    tone(BUZZER_PIN, 1500); // Enciende el tono
    delay(100); 
    noTone(BUZZER_PIN);     // Apaga el tono
    delay(20); 
    tiempoTranscurrido += 120;
  }
}

void esperarInterruptor() {
  ////ln(F("Esperando activacion del interruptor (LOW)..."));
  
  // LED parpadea lentamente mientras espera
  bool ledState = false;
  unsigned long ultimoParpadeo = 0;

  
  while (digitalRead(INTERRUPTOR_PIN) == HIGH) {  // Espera a que pase a LOW
    unsigned long ahora = millis();
    
    // Parpadeo lento del LED (cada 500ms)
    if (ahora - ultimoParpadeo > 500) {
      ledState = !ledState;
      colorVehiculo(255, 100, 0);
      ultimoParpadeo = ahora;
    }
    
    delay(10);
  }
  
  // Cuando el interruptor pasa a LOW, activar sistema
  apagarVehiculo(); // Apagar LED de espera
  delay(50); // Pequeña pausa para debounce
  iniciarSistema();
} 
  // Función mejorada para verificar el interruptor y controlar el sistema
void verificarInterruptor() {
  bool estadoActual = digitalRead(INTERRUPTOR_PIN);
  unsigned long tiempoActual = millis();
  
  // Debounce del interruptor
  if (estadoActual != interruptorAnterior) {
    tiempoDebounce = tiempoActual;
  }
  
  if ((tiempoActual - tiempoDebounce) > DEBOUNCE_DELAY) {
    if (estadoActual != interruptorAnterior) {
      // Cambio de estado confirmado
      if (estadoActual == LOW) {
        // Interruptor en LOW - INICIAR SISTEMA
        if (!sistemaActivo) {
          iniciarSistema();
        }
      } else {
        // Interruptor en HIGH - DETENER SISTEMA
        if (sistemaActivo) {
          detenerSistema();
        }
      }
      interruptorAnterior = estadoActual;
    }
  }
}
// LED indicador de sistema activo - 3 parpadeos rápidos con sonido
void indicadorSistemaActivo() {
  for (int i = 0; i < 3; i++) {
    colorVehiculo(255, 0, 200);
 //   buzzerVolumenBajo(100);  // 100ms con pulsos cortos
    apagarVehiculo();
    delay(100);
  }
}


// Función para iniciar el sistema completamente
void iniciarSistema() {
  // //ln(F("=== INICIANDO SISTEMA ==="));
  
  // Activar el sistema
  sistemaActivo = true;
  sistemaIniciado = true;
  resetYaw(); // Fija el 0° exactamente cuando arranca la carrera/ruta
  
  // Reinicializar todas las variables del sistema
  estadoActual = ESTADO_NORMAL;
  prioridadActual = PRIORIDAD_NORMAL;
  emergenciaActivada = false;
  retrocesoPorEmergencia = false;
  esquiveDiagonalActivo = false;
  giroEnProceso = false;
  enGiro = false;
  cruceEnProceso = false;
  retrocesoOrientadoActivo = false;
  
  // Reinicializar variables de detección
  colorDetectado = 'N';
  colorObjetivo = 'N';
  ultimoColorDetectado = 'N';
  sentidoVehiculo = INDEFINIDO;
  primeraDeteccion = true;
  lineaNaranjaDetectada = false;
  
  // Reinicializar contadores y timers
  contadorLineas = 0;
  tiempoEsquive = 0;
  tiempoUltimaDeteccion = millis();
  ultimoTiempoCentrado = millis();
  lastTime = millis();
  
  // Reinicializar PIDs
  pidGiro.error_anterior = 0;
  pidGiro.integral = 0;
  pidGiro.tiempo_anterior = 0;
  
  pidCentrado.error_anterior = 0;
  pidCentrado.integral = 0;
  pidCentrado.tiempo_anterior = 0;
  pidCentrado.inicializado = false;
  
 // Centrar servo
  direccion.write(SERVO_CENTRADO);
  
  // Inicializar variables para el arranque retrasado del motor
  tiempoInicioSistema = millis();
  motorArrancado = false;
  
  // Motor permanece apagado hasta que pase el delay
  detenerMotor();  // Motor apagado
  
  indicadorSistemaActivo();
  // //ln(F("Sistema iniciado - Motor arrancará en 1 segundo"));
}

void resetearRetrocesoEmergencia() {
  ////ln("RESET COMPLETO DEL SISTEMA DE RETROCESO");
  
  // Apagar motores de forma segura
  detenerMotor();           // Apagar motor principal temporalmente
  delay(100);
  apagarVehiculo();              // Apagar LED
  
  // Resetear TODAS las variables de estado
  retrocesoPorEmergencia = false;
  motorRetrocesoConfigurado = false;  // IMPORTANTE: Resetear flag
  prioridadActual = PRIORIDAD_NORMAL;

  estadoActual = ESTADO_NORMAL;

  tiempoInicioRetroceso = 0;
  tiempoUltimaVerificacionRetroceso = 0;
  
  // Centrar servo
  direccion.write(SERVO_CENTRADO);
  delay(100);
  
  // Reactivar motor principal
  avanzarMotor();
  
  ////ln("Reset completo finalizado");
}
// Función mejorada para detener el sistema completamente
void detenerSistema() {
  ////ln(F("=== DETENIENDO SISTEMA ==="));
  
  // Desactivar el sistema
  sistemaActivo = false;
  sistemaIniciado = false;
  
  // Detener todos los motores inmediatamente
  detenerMotor();  // HIGH para detener según tu configuración  
  // Centrar servo
  direccion.write(SERVO_CENTRADO);
  
  // Resetear sistema de retroceso y emergencias
  resetearRetrocesoEmergencia();
  retrocesoOrientadoActivo = false;
  emergenciaActivada = false;
  retrocesoPorEmergencia = false;
  esquiveDiagonalActivo = false;
  giroEnProceso = false;
  enGiro = false;
  cruceEnProceso = false;
  
  // Resetear estado a normal
  estadoActual = ESTADO_NORMAL;
  prioridadActual = PRIORIDAD_NORMAL;
  
  // Resetear variables de detección
  colorDetectado = 'N';
  colorObjetivo = 'N';
  ultimoColorDetectado = 'N';
  
  // LED parpadea rápidamente indicando sistema detenido, luego se apaga
  for (int i = 0; i < 6; i++) {
    colorVehiculo(0, 255, 0);
    delay(150);
    apagarVehiculo();
    delay(150);
  }
  apagarVehiculo(); // Asegurar que LED esté apagado
  
  SerialBT.println(F("Sistema detenido completamente"));
}
int DELAYESTA = 500;
// ==================== FUNCIÓN DE SALIDA DE ESTACIONAMIENTO ====================
/*void ejecutarSalidaEstacionamiento(bool salirHaciaIzquierda) {
  // Imprime en el monitor qué maniobra se va a ejecutar
  if (salirHaciaIzquierda) {
    SerialBT.println("INICIANDO MANIOBRA: Salida de Estacionamiento (Hacia la Izquierda)");
  } else {
    SerialBT.println("INICIANDO MANIOBRA: Salida de Estacionamiento (Hacia la Derecha - Invertida)");
  }
  
  // Pausa inicial para asegurar que todo esté estable
  delay(500);

  // Se definen las direcciones del servo usando un operador ternario.
  // Esto asigna el valor correcto a cada variable dependiendo de la dirección de salida elegida.
  int primerGiroAvance    = salirHaciaIzquierda ? SERVO_LEFT : SERVO_RIGHT;
  int primerGiroRetroceso = salirHaciaIzquierda ? SERVO_RIGHT : SERVO_LEFT;
  int segundoGiroLargo    = salirHaciaIzquierda ? SERVO_LEFT : SERVO_RIGHT;
  int giroEnderezado      = salirHaciaIzquierda ? SERVO_RIGHT : SERVO_LEFT;
  
  // --- PASOS 1 Y 2: Maniobra corta de vaivén (se repite dos veces) ---
  for (int i = 0; i < 2; i++) {
    SerialBT.println("Paso 1." + String(i + 1) + ": Avance corto en giro");
    direccion.write(primerGiroAvance);
    tiempoInicioPausa = millis();
    while (millis() - tiempoInicioPausa < DELAYESTA) {
      actualizarGiro(); // ¡Ahora el ángulo se actualiza durante la pausa!
      actualizarRampaMotor();
      delay(5); // Pequeña pausa para no sobrecargar el procesador
    }
    avanzarLento(); // Avanza
    unsigned long tiempoInicioPaso1 = millis();
    while (millis() - tiempoInicioPaso1 < 275) {
      actualizarGiro(); // Sigue actualizando el ángulo del MPU6050
      actualizarRampaMotor();
      delay(10);
        SerialBT.println("esta");

    }
    detenerMotor(); // Detiene avance
    tiempoInicioPausa = millis();
    while (millis() - tiempoInicioPausa < DELAYESTA) {
      actualizarGiro(); // ¡Ahora el ángulo se actualiza durante la pausa!
      actualizarRampaMotor();
      delay(5); // Pequeña pausa para no sobrecargar el procesador
    }

    SerialBT.println("Paso 2." + String(i + 1) + ": Retroceso corto en giro");
    direccion.write(primerGiroRetroceso);
    tiempoInicioPausa = millis();
    while (millis() - tiempoInicioPausa < DELAYESTA) {
      actualizarGiro(); // ¡Ahora el ángulo se actualiza durante la pausa!
      actualizarRampaMotor();
      delay(5); // Pequeña pausa para no sobrecargar el procesador
    }
    retrocederLento();// Retrocede
    unsigned long tiempoInicioPaso2 = millis();
    while (millis() - tiempoInicioPaso2 < 350) {
      actualizarGiro();
      actualizarRampaMotor();
      delay(10);
       SerialBT.println("esta");
    }
    detenerMotor(); // Detiene retroceso
    tiempoInicioPausa = millis();
    while (millis() - tiempoInicioPausa < DELAYESTA) {
      actualizarGiro(); // ¡Ahora el ángulo se actualiza durante la pausa!
      actualizarRampaMotor();
      delay(5); // Pequeña pausa para no sobrecargar el procesador
    }
  }

  // --- PASO 3: Avanzar en giro por un tiempo más largo para salir del cajón ---
  SerialBT.println("Paso 3: Avance largo en giro (700ms)");
  direccion.write(segundoGiroLargo);
  tiempoInicioPausa = millis();
    while (millis() - tiempoInicioPausa < DELAYESTA) {
      actualizarGiro(); // ¡Ahora el ángulo se actualiza durante la pausa!
      actualizarRampaMotor();
      delay(5); // Pequeña pausa para no sobrecargar el procesador
    }

  avanzarLento();
  unsigned long tiempoInicioPaso3 = millis();
  while (millis() - tiempoInicioPaso3 < 900) {
    actualizarGiro();
    actualizarRampaMotor();
    delay(10);
     SerialBT.println("esta");
  }
  detenerMotor();
  
  // --- PASO 4: Avanzar en el giro opuesto hasta que el vehículo esté recto (ángulo ~ 0) ---
  SerialBT.println("Paso 4: Enderezando hasta anguloZ = 0");
  direccion.write(giroEnderezado);
  tiempoInicioPausa = millis();
    while (millis() - tiempoInicioPausa < DELAYESTA) {
      actualizarGiro(); // ¡Ahora el ángulo se actualiza durante la pausa!
      actualizarRampaMotor();
      delay(5); // Pequeña pausa para no sobrecargar el procesador
    }
  avanzarMotor();
  unsigned long tiempoInicioEnderezado = millis();
  // El bucle se detiene si el ángulo está cerca de 0 o si tarda demasiado (timeout de 4s)
  while (abs(anguloZ) > 1.0 && (millis() - tiempoInicioEnderezado < 4000)) {
    actualizarGiro();
    actualizarRampaMotor();
    SerialBT.print("Enderezando... Angulo actual: ");
    SerialBT.println(anguloZ);
    delay(10);
     SerialBT.println("esta");
  }
  //delay(300);
  detenerMotor();

  SerialBT.println("Maniobra de salida de estacionamiento completada.");
  direccion.write(SERVO_CENTER); // Enderezar ruedas
  // --- PASO 5 (AÑADIDO): Retroceso final en línea recta ---
  SerialBT.println("Paso 5: Iniciando retroceso final de 1.5 segundos.");
  delay(200); // Pequeña pausa para asegurar que las ruedas estén centradas
  
  // Se define una duración variable para el retroceso final.
  unsigned long duracionRetrocesoFinal;
  if (salirHaciaIzquierda) {
    duracionRetrocesoFinal = 3500;// Duración original para la salida a la izquierda. 
    SerialBT.println("   -> Duracion de retroceso: " + String(duracionRetrocesoFinal) + "ms (Salida Izquierda)");
  } else {
    duracionRetrocesoFinal = 2000; // Duración más corta para la salida a la derecha.
    SerialBT.println("   -> Duracion de retroceso: " + String(duracionRetrocesoFinal) + "ms (Salida Derecha)");
  }
  retrocederMotor(); // Inicia el retroceso
  unsigned long tiempoInicioRetrocesoFinal = millis();
  while (millis() - tiempoInicioRetrocesoFinal < duracionRetrocesoFinal) {
      actualizarGiro(); // Es buena práctica seguir actualizando el ángulo 
      actualizarRampaMotor();
      delay(10);
       SerialBT.println("esta");
  }
  SerialBT.println("Retroceso final completado. Motores detenidos.");

  // Detener todos los motores por seguridad
  detenerMotor();


  // --- PASO 6: Esperar comando de inicio específico ---
  if (salirHaciaIzquierda) {
    SerialBT.println("Esperando comando para salida IZQUIERDA (C01 o C07)...");
  } else {
    SerialBT.println("Esperando comando para salida DERECHA (C02 o C08)...");
  }
  
  const unsigned long TIEMPO_ESPERA_COMANDO_MS = 5000; // 5 segundos de espera
  unsigned long tiempoInicioEspera = millis();
  bool comandoValidoRecibido = false;

  bool ledState = false;
  unsigned long ultimoParpadeo = 0;
  
  // El bucle se detiene si recibe un comando válido O si se acaba el tiempo
  while (millis() - tiempoInicioEspera < TIEMPO_ESPERA_COMANDO_MS) {
    // Parpadeo del LED para indicar que está esperando
    unsigned long ahora = millis();
    if (ahora - ultimoParpadeo > 500) {
      ledState = !ledState;
      colorVehiculo(255, 100, 0);
      ultimoParpadeo = ahora;
    }

    String comando = "";
    if (Serial.available() > 0) {
      comando = Serial.readStringUntil('\n');
    } else if (Serial.available() > 0) {
      comando = Serial.readStringUntil('\n');
    }

    if (comando != "") {
      comando.trim();
      comando.toUpperCase();

      bool comandoEsValido = false;
      // Lógica para validar el comando según la dirección de salida
      if (salirHaciaIzquierda) {
        // Si la salida fue a la izquierda, solo aceptar C01 o C07
        if (comando == "C01" || comando == "C07") {
          comandoEsValido = true;
        }
      } else {
        // Si la salida fue a la derecha, solo aceptar C02 o C08
        if (comando == "C02" || comando == "C08") {
          comandoEsValido = true;
        }
      }

      if (comandoEsValido) {
        SerialBT.println("Comando de inicio válido recibido: " + comando);
        procesarComandoCarril(comando);
        comandoValidoRecibido = true;
        break; // Sale del bucle de espera
      } else {
        // Informa que el comando no es válido para esta dirección de salida
        SerialBT.print("Comando '" + comando + "' inválido. ");
        if (salirHaciaIzquierda) {
          SerialBT.println("Solo se acepta C01 o C07.");
        } else {
          SerialBT.println("Solo se acepta C02 o C08.");
        }
      }
    }
    delay(50);
  }
 
  // Si al salir del bucle no se recibió un comando válido, el tiempo se agotó.
  if (!comandoValidoRecibido) {
    SerialBT.println("Tiempo de espera agotado. El sistema continuará en modo normal.");
    // No es necesario hacer nada extra, la función loop() se encargará de arrancar el motor
    // y continuar en ESTADO_NORMAL.
  }

  apagarVehiculo(); // Apaga el LED de espera al finalizar
}*/
void ejecutarSalidaEstacionamiento(bool salirHaciaIzquierda) {
  // Imprime en el monitor qué maniobra se va a ejecutar
  if (salirHaciaIzquierda) {
    SerialBT.println("INICIANDO MANIOBRA: Salida de Estacionamiento (Hacia la Izquierda)");
  } else {
    SerialBT.println("INICIANDO MANIOBRA: Salida de Estacionamiento (Hacia la Derecha - Invertida)");
  }
  
  // Pausa inicial para asegurar que todo esté estable
  delay(500);

  // Se definen las direcciones del servo usando un operador ternario.
  // Esto asigna el valor correcto a cada variable dependiendo de la dirección de salida elegida.
  int primerGiroAvance    = salirHaciaIzquierda ? SERVO_LEFT : SERVO_RIGHT;
  int primerGiroRetroceso = salirHaciaIzquierda ? SERVO_RIGHT : SERVO_LEFT;
  int segundoGiroLargo    = salirHaciaIzquierda ? SERVO_LEFT : SERVO_RIGHT;
  int giroEnderezado      = salirHaciaIzquierda ? SERVO_RIGHT : SERVO_LEFT;
  
      // --- PASOS 1 y 2: Retroceso corto → Avance corto → Retroceso corto ---
  // (el avance final de la secuencia es el Paso 3: avance largo, que ya saca del cajón)
  SerialBT.println("Paso 1: Retroceso corto en giro");
  direccion.write(primerGiroRetroceso);
  tiempoInicioPausa = millis();
  while (millis() - tiempoInicioPausa < DELAYESTA) {
    actualizarGiro();
    delay(5);
  }
  retrocederLento();
  unsigned long tiempoInicioPaso1 = millis();
  while (millis() - tiempoInicioPaso1 < 350) {
    actualizarGiro();
    delay(10);
     SerialBT.println("esta");
  }
  detenerMotor();
  tiempoInicioPausa = millis();
  while (millis() - tiempoInicioPausa < DELAYESTA) {
    actualizarGiro();
    delay(5);
  }

  SerialBT.println("Paso 2: Avance corto en giro");
  direccion.write(primerGiroAvance);
  tiempoInicioPausa = millis();
  while (millis() - tiempoInicioPausa < DELAYESTA) {
    actualizarGiro();
    delay(5);
  }
  avanzarLento();
  unsigned long tiempoInicioPaso2 = millis();
  while (millis() - tiempoInicioPaso2 < 275) {
    actualizarGiro();
    delay(10);
      SerialBT.println("esta");
  }
  detenerMotor();
  tiempoInicioPausa = millis();
  while (millis() - tiempoInicioPausa < DELAYESTA) {
    actualizarGiro();
    delay(5);
  }

  SerialBT.println("Paso 2.5: Retroceso corto en giro");
  direccion.write(primerGiroRetroceso);
  tiempoInicioPausa = millis();
  while (millis() - tiempoInicioPausa < DELAYESTA) {
    actualizarGiro();
    delay(5);
  }
  retrocederLento();
  unsigned long tiempoInicioPaso2b = millis();
  while (millis() - tiempoInicioPaso2b < 350) {
    actualizarGiro();
    delay(10);
     SerialBT.println("esta");
  }
  detenerMotor();
  tiempoInicioPausa = millis();
  while (millis() - tiempoInicioPausa < DELAYESTA) {
    actualizarGiro();
    delay(5);
  }

  // --- PASO 3: Avanzar en giro por un tiempo más largo para salir del cajón ---
  SerialBT.println("Paso 3: Avance largo en giro (700ms)");
  direccion.write(segundoGiroLargo);
  tiempoInicioPausa = millis();
    while (millis() - tiempoInicioPausa < DELAYESTA) {
      actualizarGiro(); // ¡Ahora el ángulo se actualiza durante la pausa!
      actualizarRampaMotor();
      delay(5); // Pequeña pausa para no sobrecargar el procesador
    }

  avanzarLento();
  unsigned long tiempoInicioPaso3 = millis();
  while (millis() - tiempoInicioPaso3 < 900) {
    actualizarGiro();
    actualizarRampaMotor();
    delay(10);
     SerialBT.println("esta");
  }
  detenerMotor();
  
  // --- PASO 4: Avanzar en el giro opuesto hasta que el vehículo esté recto (ángulo ~ 0) ---
  SerialBT.println("Paso 4: Enderezando hasta anguloZ = 0");
  direccion.write(giroEnderezado);
  tiempoInicioPausa = millis();
    while (millis() - tiempoInicioPausa < DELAYESTA) {
      actualizarGiro(); // ¡Ahora el ángulo se actualiza durante la pausa!
      actualizarRampaMotor();
      delay(5); // Pequeña pausa para no sobrecargar el procesador
    }
  avanzarMotor();
  unsigned long tiempoInicioEnderezado = millis();
  // El bucle se detiene si el ángulo está cerca de 0 o si tarda demasiado (timeout de 4s)
  while (abs(anguloZ) > 1.0 && (millis() - tiempoInicioEnderezado < 4000)) {
    actualizarGiro();
    actualizarRampaMotor();
    SerialBT.print("Enderezando... Angulo actual: ");
    SerialBT.println(anguloZ);
    delay(10);
     SerialBT.println("esta");
  }
  //delay(300);
  detenerMotor();

  SerialBT.println("Maniobra de salida de estacionamiento completada.");
  direccion.write(SERVO_CENTER); // Enderezar ruedas
  // --- PASO 5 (AÑADIDO): Retroceso final en línea recta ---
  SerialBT.println("Paso 5: Iniciando retroceso final de 1.5 segundos.");
  delay(200); // Pequeña pausa para asegurar que las ruedas estén centradas
  
  // Se define una duración variable para el retroceso final.
  unsigned long duracionRetrocesoFinal;
  if (salirHaciaIzquierda) {
    duracionRetrocesoFinal = 3500;// Duración original para la salida a la izquierda. 
    SerialBT.println("   -> Duracion de retroceso: " + String(duracionRetrocesoFinal) + "ms (Salida Izquierda)");
  } else {
    duracionRetrocesoFinal = 2000; // Duración más corta para la salida a la derecha.
    SerialBT.println("   -> Duracion de retroceso: " + String(duracionRetrocesoFinal) + "ms (Salida Derecha)");
  }
  retrocederMotor(); // Inicia el retroceso
  unsigned long tiempoInicioRetrocesoFinal = millis();
  while (millis() - tiempoInicioRetrocesoFinal < duracionRetrocesoFinal) {
      actualizarGiro(); // Es buena práctica seguir actualizando el ángulo 
      actualizarRampaMotor();
      delay(10);
       SerialBT.println("esta");
  }
  SerialBT.println("Retroceso final completado. Motores detenidos.");

  // Detener todos los motores por seguridad
  detenerMotor();


  // --- PASO 6: Esperar comando de inicio (unificado con modo pista abierta) ---
  SerialBT.println("🟣 Esperando comando Cxx por 5 segundos...");

  const unsigned long TIEMPO_ESPERA_COMANDO_MS = 5000; // 5 segundos de espera
  unsigned long tiempoInicioEspera = millis();
  bool comandoValidoRecibido = false;

  bool ledState = false;
  unsigned long ultimoParpadeo = 0;
  colorVehiculo(128, 0, 255); // Morado = esperando comando

  // El bucle se detiene si recibe un comando válido O si se acaba el tiempo
  while (millis() - tiempoInicioEspera < TIEMPO_ESPERA_COMANDO_MS) {
    // Parpadeo morado, igual que en modo pista abierta
    unsigned long ahora = millis();
    if (ahora - ultimoParpadeo > 400) {
      ledState = !ledState;
      if (ledState) colorVehiculo(128, 0, 255);
      else          apagarVehiculo();
      ultimoParpadeo = ahora;
    }

    String comando = "";
    if (Serial.available() > 0) {
      comando = Serial.readStringUntil('\n');
    } else if (Serial.available() > 0) {
      comando = Serial.readStringUntil('\n');
    }

    if (comando != "") {
      comando.trim();
      comando.toUpperCase();

      if (comando.startsWith("C") && comando.length() >= 3) {
        comando = comando.substring(0, 3);
        SerialBT.println("✅ Comando recibido tras salida de estacionamiento: " + comando);

        // Retroceso corto para ganar espacio antes del esquive (igual que modo pista abierta)
        SerialBT.println("⏪ Retroceso previo al esquive...");
        direccion.write(SERVO_CENTRADO);
        retrocederMotor();
        unsigned long tiempoInicioRetrocesoPrevio = millis();
        while (millis() - tiempoInicioRetrocesoPrevio < 700) {
          actualizarGiro();
          actualizarRampaMotor();
          delay(10);
        }
        detenerMotor();
        delay(150);

        procesarComandoCarril(comando);
        comandoValidoRecibido = true;
        break; // Sale del bucle de espera
      } else {
        SerialBT.println("⚠️ Comando '" + comando + "' inválido. Esperando Cxx...");
      }
    }
    delay(50);
  }
 
  // Si al salir del bucle no se recibió un comando válido, el tiempo se agotó.
  if (!comandoValidoRecibido) {
    SerialBT.println("⏱️ Tiempo agotado. Continuando en modo normal.");
    colorVehiculo(0, 255, 0);
    // No es necesario hacer nada extra, la función loop() se encargará de arrancar el motor
    // y continuar en ESTADO_NORMAL.
  }

  apagarVehiculo(); // Apaga el LED de espera al finalizar
}

float calcularPID(PIDController* pid, float setpoint, float medicion) {
  unsigned long tiempo_actual = millis();
  float dt = (tiempo_actual - pid->tiempo_anterior) / 1000.0;
  
  // Evitar divisiones por cero o deltas muy grandes
  if (dt <= 0 || dt > 0.1) {
    dt = 0.02; // 20ms por defecto
  }
  
  // Calcular error
  float error = setpoint - medicion;
  
  // Normalizar error angular (manejar cruce de 0°/360°)
  if (error > 180) error -= 360;
  if (error < -180) error += 360;
  
  // Término proporcional
  float proporcional = pid->kp * error;
  
  // Término integral con anti-windup
  pid->integral += error * dt;
  pid->integral = constrain(pid->integral, -pid->integral_max, pid->integral_max);
  float integral = pid->ki * pid->integral;
  
  // Término derivativo
  float derivativo = 0;
  if (pid->tiempo_anterior > 0) {
    derivativo = pid->kd * (error - pid->error_anterior) / dt;
  }
  
  // Salida PID total
  float salida = proporcional + integral + derivativo;
  
  // Guardar valores para próxima iteración
  pid->error_anterior = error;
  pid->tiempo_anterior = tiempo_actual;
  
  return salida;
}

void resetearPID(PIDController* pid) {
  pid->error_anterior = 0;
  pid->integral = 0;
  pid->tiempo_anterior = millis();
}

float calcularPIDCentrado(PIDCentrado* pid, float error) {
  unsigned long tiempo_actual = millis();
  
  // Inicializar en primera ejecución
  if (!pid->inicializado) {
    pid->tiempo_anterior = tiempo_actual;
    pid->error_anterior = error;
    pid->integral = 0;
    pid->inicializado = true;
    return 0; // No aplicar corrección en primera ejecución
  }
  
  float dt = (tiempo_actual - pid->tiempo_anterior) / 1000.0;
  
  // Evitar deltas muy grandes o muy pequeños
  if (dt <= 0 || dt > 0.5) {
    dt = 0.1; // 100ms por defecto
  }
  
  // Término proporcional
  float proporcional = pid->kp * error;
  
  // Término integral con anti-windup
  pid->integral += error * dt;
  pid->integral = constrain(pid->integral, -pid->integral_max, pid->integral_max);
  float integral = pid->ki * pid->integral;
  
  // Término derivativo
  float derivativo = pid->kd * (error - pid->error_anterior) / dt;
  
  // Salida PID total
  float salida = proporcional + integral + derivativo;
  
  // Guardar valores para próxima iteración
  pid->error_anterior = error;
  pid->tiempo_anterior = tiempo_actual;
  
  return salida;
}

void resetearPIDCentrado(PIDCentrado* pid) {
  pid->error_anterior = 0;
  pid->integral = 0;
  pid->tiempo_anterior = millis();
  pid->inicializado = false;
}

void ajustarParametrosPIDCentrado(float kp, float ki, float kd) {
  pidCentrado.kp = kp;
  pidCentrado.ki = ki;
  pidCentrado.kd = kd;
  resetearPIDCentrado(&pidCentrado);

}


// ==================== FUNCIONES ORIGINALES ====================

// Función para confirmar color - MEJORADA
bool confirmarColor(int hueMin, int hueMax, float satMin, float valueMin, float valueMax, int lecturas, int confirmacionesRequeridas) {
  int detecciones = 0;
  
  for (int i = 0; i < lecturas; i++) {
    uint16_t r, g, b, c;
    double hue, saturation, value;
    
    tcs.getRawData(&r, &g, &b, &c);
    
    if (c > 0) {
      float r_norm = (float)r / c;
      float g_norm = (float)g / c;
      float b_norm = (float)b / c;
      
      uint8_t r_byte = (uint8_t)(r_norm * 255.0);
      uint8_t g_byte = (uint8_t)(g_norm * 255.0);
      uint8_t b_byte = (uint8_t)(b_norm * 255.0);
      
      ColorConverter::RgbToHsv(r_byte, g_byte, b_byte, hue, saturation, value);
      hue *= 360;
      
      //if (hueMin >= 180) { // Es azul
      if (hueMax > 100) { // Es azul (naranja usa hueMin=340 que rompía esta condición) 
        float proporcionAzul = b_norm / (r_norm + g_norm + 0.001);
        if (hue >= hueMin && hue <= hueMax && 
            saturation > satMin && 
            value > valueMin && value < valueMax &&
            proporcionAzul > 0.9) {
          detecciones++;
        }
      } 
      else { // Es naranja
          bool enRangoNormal = (hue >= 340 && hue <= 360) || (hue >= 0 && hue <= hueMax);
        //if ((enRangoNormal || enRangoAlto) && 
        if ((enRangoNormal) && 
            saturation > satMin && 
            value > valueMin && value < valueMax) {
          detecciones++;
        }
      }
    }
    delay(5);
  }
  
  return (detecciones >= confirmacionesRequeridas);
}

// Función auxiliar para normalizar la diferencia angular
float normalizarDiferenciaAngular(float diferencia) {
  while (diferencia > 180.0) diferencia -= 360.0;
  while (diferencia < -180.0) diferencia += 360.0;
  return diferencia;
}

void calcularAnguloObjetivoCenter() {
  if (sentidoVehiculo == IZQUIERDA) {
    // Giro a la izquierda: 1->0°, 2->90°, 3->180°, 4->270°, 5->0°, 6->90°...
    int ciclo = (contadorLineas - 1) % 4;  // Ajustar para que comience en 1
    switch (ciclo) {
      case 0: anguloObjetivo = 0.0; break;   // contadorLineas = 1, 5, 9...
      case 1: anguloObjetivo = 90.0; break;  // contadorLineas = 2, 6, 10...
      case 2: anguloObjetivo = 180.0; break; // contadorLineas = 3, 7, 11...
      case 3: anguloObjetivo = 270.0; break; // contadorLineas = 4, 8, 12...
    }
  } else {
    // Giro a la derecha: 1->0°, 2->270°, 3->180°, 4->90°, 5->0°, 6->270°...
    int ciclo = (contadorLineas - 1) % 4;  // Ajustar para que comience en 1
    switch (ciclo) {
      case 0: anguloObjetivo = 0.0; break;   // contadorLineas = 1, 5, 9...
      case 1: anguloObjetivo = 270.0; break; // contadorLineas = 2, 6, 10...
      case 2: anguloObjetivo = 180.0; break; // contadorLineas = 3, 7, 11...
      case 3: anguloObjetivo = 90.0; break;  // contadorLineas = 4, 8, 12...
    }
  }
}
void calcularAnguloObjetivoMantener() {
  if (sentidoVehiculo == IZQUIERDA) {
    // Giro a la izquierda: 1->90°, 2->180°, 3->270°, 4->0°, etc.
    int ciclo = (contadorLineas -1) % 4; // Ajuste para que la primera línea sea ciclo 0
    switch (ciclo) {
      case 0: anguloObjetivo = 90.0; break;  // Línea 1, 5, 9...
      case 1: anguloObjetivo = 180.0; break; // Línea 2, 6, 10...
      case 2: anguloObjetivo = 270.0; break; // Línea 3, 7, 11...
      case 3: anguloObjetivo = 0.0; break;   // Línea 4, 8, 12...
    }
  } else { // sentidoVehiculo == DERECHA
    // Giro a la derecha: 1->270°, 2->180°, 3->90°, 4->0°, etc.
    int ciclo = (contadorLineas - 1) % 4; // Ajuste para que la primera línea sea ciclo 0
    switch (ciclo) {
      case 0: anguloObjetivo = 270.0; break; // Línea 1, 5, 9...
      case 1: anguloObjetivo = 180.0; break; // Línea 2, 6, 10...
      case 2: anguloObjetivo = 90.0; break;  // Línea 3, 7, 11...
      case 3: anguloObjetivo = 0.0; break;   // Línea 4, 8, 12...
    }
  }
}
// Función para centrar el vehículo con PRIORIDAD ANGULAR
void centrarVehiculo() {
    if (retrocesoOrientadoActivo) {
    return;
  }
  // VERIFICACIÓN: No centrar si hay emergencias activas
  if (prioridadActual != PRIORIDAD_NORMAL || 
      esquiveDiagonalActivo || 
      retrocesoPorEmergencia) {
    if (pidCentrado.inicializado) {
      resetearPIDCentrado(&pidCentrado);
    }
    return;
  }
  
  // No centrar si está en giro, recién terminó giro, pausado por detección, o en esquive
  if (enGiro || 
      millis() - tiempoFinGiro < tiempoEsperaPosgiro ||
      (pausarCentrado && millis() - tiempoInicioDeteccion < tiempoPausaCentrado)) {
    
    if (pidCentrado.inicializado) {
      resetearPIDCentrado(&pidCentrado);
    }
    return;
  }
  
  if (millis() - ultimoTiempoCentrado < intervaloCentrado) {
    return;
  }
  
  // Calcular el ángulo objetivo SIEMPRE antes de proceder
  calcularAnguloObjetivoCenter();
  
  // Leer sensores con pequeña pausa para estabilidad
  distanciaDerecha = leerToF('D');
  delay(5);
  distanciaIzquierda = leerToF('I');
  
  // Validar lecturas de sensores
  if (distanciaDerecha > 80 || distanciaIzquierda > 80 || 
      distanciaDerecha < 1 || distanciaIzquierda < 1) {
    resetearPIDCentrado(&pidCentrado);
    return;
  }
  
  // VERIFICACIÓN FINAL antes de mover servo
  if (prioridadActual != PRIORIDAD_NORMAL || 
      esquiveDiagonalActivo || 
      retrocesoPorEmergencia) {
    resetearPIDCentrado(&pidCentrado);
    return;
  }
  
  // ===== CÁLCULO CON PRIORIDAD ANGULAR =====
  
  // 1. Calcular diferencia angular
  float diferenciaAngular = normalizarDiferenciaAngular(anguloZ - anguloObjetivo);
  
  // 2. VERIFICAR UMBRAL MÁXIMO - Si excede ±20°, SOLO corregir ángulo
  if (abs(diferenciaAngular) > UMBRAL_ANGULO_MAX) {
    // EMERGENCIA ANGULAR: Solo corregir ángulo, ignorar distancia
    float errorFinal = diferenciaAngular * FACTOR_CORRECCION_ANGULAR;
    
    // Debug para emergencia angular
    /*
    SerialBT.print(F("EMERGENCIA_ANG - AngZ:"));
    SerialBT.print(anguloZ, 1);
    SerialBT.print(F(" AngObj:"));
    SerialBT.print(anguloObjetivo, 1);
    SerialBT.print(F(" Dif:"));
    SerialBT.print(diferenciaAngular, 1);
    SerialBT.print(F(" ErrFinal:"));
    SerialBT.println(errorFinal, 1);
    */
    
    // Aplicar corrección PID solo angular
    float salidaPID = calcularPIDCentrado(&pidCentrado, errorFinal);
    salidaPID = constrain(salidaPID, -ERROR_MAXIMO_CENTRADO, ERROR_MAXIMO_CENTRADO);
    
    // Convertir a ángulo de servo
    float factorEscala = ((salidaPID > 0) ? SERVO_CORR_CENTRADO_DER : SERVO_CORR_CENTRADO_IZQ) / ERROR_MAXIMO_CENTRADO;
    int correccionAngulo = salidaPID * factorEscala;
    
    static int anguloAnterior = SERVO_CENTRADO;
    int anguloDeseado = SERVO_CENTRADO + correccionAngulo;
    anguloDeseado = constrain(anguloDeseado, SERVO_CENTRADO - SERVO_CORR_CENTRADO_IZQ, 
                              SERVO_CENTRADO + SERVO_CORR_CENTRADO_DER);
    
    int anguloFinal = anguloAnterior * (1 - FACTOR_SUAVIZADO) + anguloDeseado * FACTOR_SUAVIZADO;
    anguloAnterior = anguloFinal;
    
    // Aplicar al servo
    if (prioridadActual == PRIORIDAD_NORMAL && 
        !esquiveDiagonalActivo && 
        !retrocesoPorEmergencia) {
      direccion.write(anguloFinal);
    }
    
    ultimoTiempoCentrado = millis();
    return; // Salir sin considerar distancia
  }
  
  // 3. Si está dentro del umbral, proceder con lógica de prioridad
  float errorDistancia = distanciaDerecha - distanciaIzquierda;
  float errorFinal = 0;
  
  // PRIORIDAD 1: Corregir ángulo si está fuera de tolerancia fina
  if (abs(diferenciaAngular) > TOLERANCIA_ANGULO_FINO) {
    // El ángulo es la prioridad principal
    errorFinal = diferenciaAngular * FACTOR_CORRECCION_ANGULAR;
    
    // Solo agregar corrección de distancia si también está muy descentrado
    if (abs(errorDistancia) > TOLERANCIA_CENTRADO_PID * 2) {
      errorFinal += errorDistancia * FACTOR_CORRECCION_DISTANCIA;
    }
    
    /*
    SerialBT.print(F("MODO_ANGULAR - Dif:"));
    SerialBT.print(diferenciaAngular, 1);
    SerialBT.print(F(" ErrDist:"));
    SerialBT.print(errorDistancia, 1);
    SerialBT.print(F(" ErrFinal:"));
    SerialBT.println(errorFinal, 1);
    */
    
  } 
  // PRIORIDAD 2: Si el ángulo está bien, entonces corregir distancia
  else if (abs(errorDistancia) > TOLERANCIA_CENTRADO_PID) {
    // Solo corrección por distancia
    errorFinal = errorDistancia;
    
    /*
    SerialBT.print(F("MODO_DISTANCIA - ErrDist:"));
    SerialBT.print(errorDistancia, 1);
    SerialBT.print(F(" ErrFinal:"));
    SerialBT.println(errorFinal, 1);
    */
  }
  // PRIORIDAD 3: Todo está bien
  else {
    // Ya está centrado tanto en ángulo como en distancia
    direccion.write(SERVO_CENTRADO);
    pidCentrado.integral = 0;
    pausarCentrado = false;
    
    /*
    SerialBT.println(F("CENTRADO_TOTAL"));
    */
    
    ultimoTiempoCentrado = millis();
    return;
  }
  
  // ===== APLICAR CONTROL PID =====
  float salidaPID = calcularPIDCentrado(&pidCentrado, errorFinal);
  salidaPID = constrain(salidaPID, -ERROR_MAXIMO_CENTRADO, ERROR_MAXIMO_CENTRADO);
  
  // Convertir salida PID a ángulo de servo
  float factorEscala = ((salidaPID > 0) ? SERVO_CORR_CENTRADO_DER : SERVO_CORR_CENTRADO_IZQ) / ERROR_MAXIMO_CENTRADO;
  int correccionAngulo = salidaPID * factorEscala;
  
  // Aplicar suavizado para evitar cambios bruscos
  static int anguloAnterior = SERVO_CENTRADO;
  int anguloDeseado = SERVO_CENTRADO + correccionAngulo;
  anguloDeseado = constrain(anguloDeseado, SERVO_CENTRADO - SERVO_CORR_CENTRADO_IZQ, 
                            SERVO_CENTRADO + SERVO_CORR_CENTRADO_DER);
  
  // Aplicar suavizado
  int anguloFinal = anguloAnterior * (1 - FACTOR_SUAVIZADO) + anguloDeseado * FACTOR_SUAVIZADO;
  anguloAnterior = anguloFinal;
  
  // VERIFICACIÓN FINAL antes de mover servo
  if (prioridadActual == PRIORIDAD_NORMAL && 
      !esquiveDiagonalActivo && 
      !retrocesoPorEmergencia) {
    direccion.write(anguloFinal);
  }
  
 
  ultimoTiempoCentrado = millis();
}

void detenerRobot() {
  // Detener ambos motores para asegurar una parada completa
             
  detenerMotor(); 

  // Centrar el servo de la dirección
  direccion.write(SERVO_CENTRADO); 

  // Resetear cualquier flag de estado de movimiento
  enGiro = false; 
  
  // YA NO HAY BUCLE INFINITO. La función termina aquí.
}
// ==================== FUNCIÓN realizarGiro MEJORADA CON PID ====================


//calcular el ángulo dinámico
float calcularAnguloGiroDinamico(bool giroIzquierda) {
  // Medir SIEMPRE la distancia lateral primero
  medirDistanciasLateralesPrecisas(distanciaIzquierda, distanciaDerecha);

  // NUEVA REGLA: si el carril objetivo es 2 con sentido IZQUIERDA, o el
  // carril objetivo es 1 con sentido DERECHA, se GARANTIZA apertura MÁXIMA
  // sin importar la distancia medida ni qué rama se siga más abajo.
  bool forzarAperturaMaxima =
      (ultimoCarrilActivo == 2 && sentidoVehiculo == IZQUIERDA) ||
      (ultimoCarrilActivo == 1 && sentidoVehiculo == DERECHA);

  // Condición especial por comando puntual: fuerza el ÁNGULO a GIRO_MINIMO.
  if ((sentidoVehiculo == IZQUIERDA && ultimoCarrilActivo == 2 && (carriles.esComandoC07||carriles.esComandoC01||carriles.esComandoC21||carriles.esComandoC33 ||carriles.esComandoC15 ||carriles.esComandoC27 )) ||
      (sentidoVehiculo == DERECHA && ultimoCarrilActivo == 1 && (carriles.esComandoC02||carriles.esComandoC14||carriles.esComandoC26|| carriles.esComandoC32 || carriles.esComandoC20))) {

    float distanciaSensorPrioridad = (sentidoVehiculo == DERECHA) ? distanciaIzquierda : distanciaDerecha;

    esGiroApertura = true;
    duracionAperturaActual = forzarAperturaMaxima
        ? DURACION_APERTURA_MAXIMA
        : calcularDuracionAperturaPorDistancia(distanciaSensorPrioridad);

    SerialBT.println(">>> Condición prioritaria cumplida: Forzando GIRO_MINIMO. Duración apertura: "
                      + String(duracionAperturaActual) + " ms (dist: " + String(distanciaSensorPrioridad, 1) + " cm)"
                      + (forzarAperturaMaxima ? " [APERTURA MÁXIMA por carril/sentido]" : ""));
    return (sentidoVehiculo == DERECHA) ? GIRO_MINIMO_DERECHA : GIRO_MINIMO;
  }

  float distanciaSensor;

  if (sentidoVehiculo == DERECHA) {
    distanciaSensor = distanciaIzquierda;
  } else if (sentidoVehiculo == IZQUIERDA) {
    distanciaSensor = distanciaDerecha;
  } else {
    float anguloFallback = giroIzquierda ? giroObjetivoAzul : giroObjetivoNaranja;
    SerialBT.println("VAINA RARA EN calcularAnguloGiroDinamico ME VUELVE EL ES GIRO APERTURA A FALSE");
    return giroParaEstacionamiento ? anguloFallback + 5.0 : anguloFallback;
  }

    

  float anguloCalculado;

  // Seleccionar el par de límites según el sentido de giro:
  // IZQUIERDA (azul) usa GIRO_MAXIMO/GIRO_MINIMO originales.
  // DERECHA (naranja) usa el rango propio GIRO_MAXIMO_DERECHA/GIRO_MINIMO_DERECHA.
  float giroMaximoActivo = (sentidoVehiculo == DERECHA) ? GIRO_MAXIMO_DERECHA : GIRO_MAXIMO;
  float giroMinimoActivo = (sentidoVehiculo == DERECHA) ? GIRO_MINIMO_DERECHA : GIRO_MINIMO;

  if (distanciaSensor < 25.0) {
    SerialBT.println("Directo a Giro Maximo menos de 25cm ");
    // Muy cerca de la pared: NO hacer apertura, ir directo al giro máximo
    esGiroApertura = false;
    duracionAperturaActual = 0;
    anguloCalculado = giroMaximoActivo;
  } else {
    // Interpolación lineal: 25cm → giroMaximoActivo, 65cm → giroMinimoActivo
    esGiroApertura = true;
    float distClamped = constrain(distanciaSensor, 25.0, 65.0);
    anguloCalculado = giroMaximoActivo - ((distClamped - 25.0) / (65.0 - 25.0)) * (giroMaximoActivo - giroMinimoActivo);
    SerialBT.println("📐 Interpolación: dist=" + String(distanciaSensor, 1) + "cm → ángulo=" + String(anguloCalculado, 1) + "°");
  }

  duracionAperturaActual = forzarAperturaMaxima
      ? DURACION_APERTURA_MAXIMA
      : calcularDuracionAperturaPorDistancia(distanciaSensor);

  if (giroParaEstacionamiento) {
    anguloCalculado += 5.0;
  }

  SerialBT.print("Distancia: ");
  SerialBT.print(distanciaSensor);
  SerialBT.print(" cm, Ángulo: ");
  SerialBT.print(anguloCalculado);
  SerialBT.print("°, Duración apertura: ");
  SerialBT.print(duracionAperturaActual);
  SerialBT.print(" ms");
  if (forzarAperturaMaxima) SerialBT.print(" [APERTURA MÁXIMA por carril/sentido]");
  SerialBT.println();

  return anguloCalculado;
}
void realizarGiroApertura(bool giroIzquierda) {
  SerialBT.println(" INICIANDO GIRO DE APERTURA...");
  SerialBT.println("   Duración objetivo: " + String(duracionAperturaActual) + " ms");
  
  // Activar flags de apertura
  giroAperturaActivo = true;
  giroAperturaCompletado = false;
  tiempoInicioApertura = millis();

  //Guardar la dirección del giro para usarla después
  direccionGiroAperturaIzquierda = giroIzquierda;
  
  // Determinar dirección del servo para apertura (sentido CONTRARIO al giro actual)
  int servoApertura;
  if (giroIzquierda) {
    // Si el giro principal es a la IZQUIERDA, la apertura es a la DERECHA.
    servoApertura = SERVO_RIGHT;
  } else {
    //apertura antes de giro
    // Si el giro principal es a la DERECHA, la apertura es a la IZQUIERDA.
    servoApertura = SERVO_LEFT;	
  }
  
  // Imprimir un mensaje de debug para confirmar el ángulo que se va a usar
  SerialBT.println("   Giro Apertura: Orientando servo a " + String(servoApertura) + "°");

  // Activar motor y orientar servo para apertura
  avanzarMotor();
  direccion.write(servoApertura);
  
  // Debug opcional

  SerialBT.print("Iniciando giro de apertura - Servo: ");
  SerialBT.print(servoApertura);
  SerialBT.print(", Sentido vehículo: ");
  SerialBT.println(sentidoVehiculo);

}
bool procesarGiroApertura() {
  // Si no está activo el giro de apertura, no hacer nada
  if (!giroAperturaActivo) {
    return false; // 
  }
  actualizarGiro();
  actualizarRampaMotor();
  // Verificar si ha pasado el tiempo de apertura
  if (millis() - tiempoInicioApertura >= duracionAperturaActual) { // 
    unsigned long tiempoRealTranscurrido = millis() - tiempoInicioApertura;
    SerialBT.println(" Giro de Apertura completado en " + String(tiempoRealTranscurrido) + " ms. Iniciando giro principal PID.");
   
    // *** PASO 1: Marcar la apertura como finalizada ***
    giroAperturaActivo = false;
    giroAperturaCompletado = true; // Marcar como completado por si se necesita
    
    // Centrar servo momentáneamente para una transición limpia
    //direccion.write(SERVO_CENTER); // 
    delay(50); // Pausa breve

    // *** PASO 2: INICIAR EL GIRO MÁXIMO INMEDIATAMENTE ***
    // Usamos la variable guardada 'direccionGiroAperturaIzquierda' para saber la dirección
    ejecutarGiroPID(anguloObjetivoGiroGlobal, direccionGiroAperturaIzquierda);

    // *** PASO 3: Resetear flags de apertura para el próximo ciclo ***
    giroAperturaCompletado = false;
    esGiroApertura = false;
    
    return false; // Se completó la transición de apertura a giro máximo.
  }
  
  return true; // La apertura aún está en proceso. 
}


// Revisa si hay un giro pendiente esperando la pausa pre-giro.
// Debe llamarse en cada iteración del loop() (núcleo 1).
// Retorna true si consumió el ciclo (pausa activa o giro recién disparado).
bool procesarPausaPreGiro() {
  if (!giroPendiente) {
    return false;
  }

  // Mantener el vehículo detenido y el servo neutro mientras se asienta
  detenerMotor();

  if (millis() - tiempoInicioPausaPreGiro < DURACION_PAUSA_PRE_GIRO) {
    return true; // seguimos en pausa, no bloqueante
  }

  // Pausa cumplida: consumir la bandera y ejecutar el giro UNA sola vez,
  // desde el núcleo 1, para no pelear con detectarColor() del núcleo 0.
  bool esIzquierda = giroPendienteEsIzquierda;
  giroPendiente = false;

  SerialBT.println("⏱️ Pausa pre-giro completada. Ejecutando giro (" +
                    String(esIzquierda ? "IZQUIERDA" : "DERECHA") + ")...");

  realizarGiro(esIzquierda);
  return true;
}
// ==================== PAUSA PREVIA A LA MANIOBRA OPTIMIZADA ====================
bool procesarPausaPreManiobraOptimizada() {
  if (!maniobraOptimizadaPendiente) {
    return false;
  }

  detenerMotor();

  if (millis() - tiempoInicioPausaPreManiobraOptimizada < MO_DURACION_PAUSA_PRE_MANIOBRA) {
    return true; // seguimos en pausa, no bloqueante
  }

  // Pausa cumplida: consumimos la bandera y preparamos todos los datos de la maniobra
  // UNA sola vez, antes de entrar al estado MANIOBRA_OPTIMIZADA.
  bool esIzquierda = maniobraOptimizadaEsIzquierda;
  maniobraOptimizadaPendiente = false;

  mo_giroIzquierda = esIzquierda;

  // 1) Ángulo PREVIO (= ángulo vigente al detectar esta línea): usamos la misma función
  //    determinística que ya existe, basada en contadorLineas ACTUAL (ya incrementado por
  //    detectarColor() al contar esta línea) y sentidoVehiculo. Es el ángulo con el que
  //    veníamos circulando en MANTENIENDO_CARRIL justo antes de esta detección, y es el que
  //    usamos para asegurarnos de ir derechos (MO_ENDEREZANDO) y para mantenernos rectos
  //    mientras avanzamos hacia la pared (MO_AVANZANDO_HASTA_TOF).
  calcularAnguloObjetivo();
  mo_anguloPrevio = anguloObjetivo;

  // 2) Ángulo NUEVO (hacia el que cruzamos al girar en MO_CRUZANDO): es el ángulo del
  //    tramo SIGUIENTE, es decir el que correspondería a (contadorLineas + 1). Lo calculamos
  //    sin tocar el global anguloObjetivo, replicando la misma tabla de calcularAnguloObjetivo()
  //    pero un paso adelante.
  {
    int contadorSiguienteTemp = contadorLineas + 1;
    int ciclo = (contadorSiguienteTemp - 1) % 4;
    if (ciclo < 0) ciclo += 4; // seguridad por si acaso
    if (sentidoVehiculo == IZQUIERDA) {
      switch (ciclo) {
        case 0: mo_anguloNuevo = 0.0; break;
        case 1: mo_anguloNuevo = 90.0; break;
        case 2: mo_anguloNuevo = 180.0; break;
        case 3: mo_anguloNuevo = 270.0; break;
      }
    } else {
      switch (ciclo) {
        case 0: mo_anguloNuevo = 0.0; break;
        case 1: mo_anguloNuevo = 270.0; break;
        case 2: mo_anguloNuevo = 180.0; break;
        case 3: mo_anguloNuevo = 90.0; break;
      }
    }
  }

  // 3) Comando ANTERIOR (el tramo recién completado) y comando SIGUIENTE (el próximo tramo),
  //    ambos sacados de comandosMemorizados[] ya que memorizacionCompleta es true en este punto.
  int indiceTramoAnterior = (contadorLineas - 2 + TOTAL_TRAMOS_PISTA) % TOTAL_TRAMOS_PISTA;
  int indiceTramoSiguiente = (contadorLineas - 1) % TOTAL_TRAMOS_PISTA;
  String comandoAnterior = comandosMemorizados[indiceTramoAnterior];
  mo_comandoSiguiente = comandosMemorizados[indiceTramoSiguiente];

  // 4) ¿El tramo anterior terminó en carril interno o externo? (según la lista y el sentido)
  mo_carrilAnteriorInterno = mo_esCarrilInterno(comandoAnterior, esIzquierda);

  // 5) ¿El comando SIGUIENTE es de carril exterior? (para aplicar el retroceso largo de 2s
  //    y el +2.5s en MANTENIENDO_CARRIL / espera de detección)
  mo_esComandoExterior = mo_esComandoDeCarrilExterior(comandoAnterior, esIzquierda);

  // Espacios estrechos: exterior->interior o interior->interior. En estos casos la
  // maniobra de emergencia (retroceso durante el cruce) se fuerza una vez, arbitrariamente,
  // 1seg después de iniciado el cruce, sin esperar a que el Sharp detecte nada -- el
  // espacio físico ya lo amerita. Después de esa vez, el Sharp queda vigilando normalmente.
  //
  // IMPORTANTE: para saber si el comando SIGUIENTE "empieza" en interno, no podemos usar
  // mo_esCarrilInterno() (que clasifica por el carril FINAL, vía el grupo A/B del comando).
  // En comandos dobles (C21, C14, etc.) el carril de entrada puede ser el opuesto al carril
  // final -- igual que corregimos para mo_primerCarrilObjetivoEsUno/Dos. Usamos esas mismas
  // funciones (carril de ENTRADA real) y la relación carril->interno/externo según sentido.
  {
    bool entraPorCarril1 = mo_primerCarrilObjetivoEsUno(mo_comandoSiguiente);
    // Relación carril->interno/externo según sentido (misma que usa mo_esCarrilInterno/mo_esComandoDeCarrilExterior):
    //   DERECHA: carril1 = interno, carril2 = externo
    //   IZQUIERDA: carril1 = externo, carril2 = interno
    bool comandoSiguienteInternoParaForzado = esIzquierda ? !entraPorCarril1 : entraPorCarril1;

    // mo_carrilAnteriorInterno ya fue calculado un poco más arriba en esta misma función.
    mo_emergenciaForzadaPendienteInteriorInterior = mo_carrilAnteriorInterno && comandoSiguienteInternoParaForzado;
    mo_emergenciaForzadaPendienteExteriorInterior = (!mo_carrilAnteriorInterno) && comandoSiguienteInternoParaForzado;
    mo_emergenciaForzadaYaEjecutada = false;
    mo_emergenciaActivaEsForzadaInteriorInterior = false;
    mo_esTramoInteriorAInterior = mo_emergenciaForzadaPendienteInteriorInterior; // misma clasificación, válida durante toda la maniobra (no solo el forzado puntual)
    // v96: exterior->exterior = el tramo anterior terminó en exterior y el siguiente NO empieza en interno
    mo_esTramoExteriorAExterior = (!mo_carrilAnteriorInterno) && !comandoSiguienteInternoParaForzado;
        // v97: interior->exterior = el tramo anterior terminó en interior y el siguiente empieza en exterior
    mo_esTramoInteriorAExterior = mo_carrilAnteriorInterno && !comandoSiguienteInternoParaForzado;
    mo_emergenciaCruzadaIntExtYaUsada = false; // cada maniobra nueva vuelve a tener su "primera vez"

        mo_contadorEmergenciasIntInt = 0;      // v104: cada maniobra nueva vuelve a empezar con retroceso largo
    mo_duracionEmergIntIntActual = 0;

    // v98: exterior->exterior Y el comando siguiente termina en "E"
    mo_esTramoExtExtConE = mo_esTramoExteriorAExterior && mo_comandoSiguiente.endsWith("E"); 
        mo_esTramoExteriorAInterior = (!mo_carrilAnteriorInterno) && comandoSiguienteInternoParaForzado; // v103
    mo_extIntConfirmaciones = 0;
    mo_emergenciaServoContrarioActiva = false;
    Serial.println("🔎 Forzado emergencia: comandoSiguiente=" + mo_comandoSiguiente +
                      " entraPorCarril1=" + String(entraPorCarril1) +
                      " sigInterno=" + String(comandoSiguienteInternoParaForzado) +
                      " ->IntInt=" + String(mo_emergenciaForzadaPendienteInteriorInterior) +
                      " ->ExtInt=" + String(mo_emergenciaForzadaPendienteExteriorInterior));
  }
  // 6) ¿Qué patrón de movimiento usamos? "retroceso corto/largo + cruce" cuando el comando
  //    siguiente tiene como primer/único carril objetivo el que corresponde según la tabla
  //    que definiste; en caso contrario, "avanzar hasta 40cm (ToF) + cruce".
  //    (Ver Parte 2: mo_primerCarrilObjetivoEsUno / mo_primerCarrilObjetivoEsDos)
  bool primerObjetivoEsUno = mo_primerCarrilObjetivoEsUno(mo_comandoSiguiente);
  bool primerObjetivoEsDos = mo_primerCarrilObjetivoEsDos(mo_comandoSiguiente);

    // Retroceso reducido (1100ms): anterior con "E" terminando en EXTERIOR y siguiente entrando por carril INTERIOR
  {
    // IZQUIERDA: carril2 = interno | DERECHA: carril1 = interno
    bool siguienteEntraPorInterior = esIzquierda ? primerObjetivoEsDos : primerObjetivoEsUno;
    mo_retrocesoEExtAIntActivo = comandoAnterior.endsWith("E") && mo_esComandoExterior && siguienteEntraPorInterior;

    if (mo_retrocesoEExtAIntActivo) {
      SerialBT.println("⏪ Anterior con E exterior -> siguiente interior: retroceso reducido a " +
                       String(MO_TIEMPO_RETROCESO_E_EXT_A_INT) + "ms");
    }
  }
    // Distancia ToF para empezar a girar: 60cm si el comando siguiente termina en "E" y ENTRA por carril exterior
  // (sin importar si el tramo anterior terminó en interior o exterior). Si no, la distancia normal.
  {
    bool siguienteEsConE = mo_comandoSiguiente.endsWith("E");
    // IZQUIERDA: carril1 = externo | DERECHA: carril2 = externo (misma relación que usa el resto del código)
    bool siguienteEntraPorExterior = esIzquierda ? primerObjetivoEsUno : primerObjetivoEsDos;

    if (siguienteEsConE && siguienteEntraPorExterior) {
      mo_distanciaFrontalObjetivoActual = MO_DISTANCIA_FRONTAL_OBJETIVO_E_EXTERIOR_CM;
      SerialBT.println("📏 Comando con E entrando por EXTERIOR -> ToF de giro = " +
                       String(MO_DISTANCIA_FRONTAL_OBJETIVO_E_EXTERIOR_CM, 0) + "cm");
    } else {
      mo_distanciaFrontalObjetivoActual = MO_DISTANCIA_FRONTAL_OBJETIVO_CM;
    }
  }
  
  if (esIzquierda) {
    // Sentido IZQUIERDA: "retroceder y cruzar" si primer objetivo == carril2; "avanzar-ToF y cruzar" si == carril1
    mo_usarPatronRetrocesoAvance = primerObjetivoEsDos;
  } else {
    // Sentido DERECHA: "retroceder y cruzar" si primer objetivo == carril1; "avanzar-ToF y cruzar" si == carril2
    mo_usarPatronRetrocesoAvance = primerObjetivoEsUno;
  }
  // Anterior EXTERIOR + siguiente con "E" que ENTRA por carril EXTERIOR:
  // retroceso de 500ms y cruce directo (sin avanzar hasta distancia ToF).
  {
    bool siguienteEsConE_ExtExt = mo_comandoSiguiente.endsWith("E");
    // IZQUIERDA: carril1 = externo | DERECHA: carril2 = externo
    bool siguienteEntraPorExterior_ExtExt = esIzquierda ? primerObjetivoEsUno : primerObjetivoEsDos;

    mo_retrocesoEExtAExtActivo = mo_esComandoExterior && siguienteEsConE_ExtExt && siguienteEntraPorExterior_ExtExt;

    if (mo_retrocesoEExtAExtActivo) {
      mo_usarPatronRetrocesoAvance = true; // fuerza RETROCESO+CRUCE en lugar de AVANCE_TOF+CRUCE
      SerialBT.println("⏪ Anterior exterior -> siguiente con E exterior: retroceso " +
                       String(MO_TIEMPO_RETROCESO_E_EXT_A_EXT) + "ms y cruce directo (sin ToF)");
    }
  }
    // ── v96: ¿el comando siguiente COMIENZA en carril exterior? ──
  // IZQUIERDA: carril1 = externo | DERECHA: carril2 = externo
  mo_siguienteEntraPorExterior = esIzquierda ? primerObjetivoEsUno : primerObjetivoEsDos;

    // ── v96: ¿el comando siguiente COMIENZA en carril exterior? ──
  // IZQUIERDA: carril1 = externo | DERECHA: carril2 = externo
  mo_siguienteEntraPorExterior = esIzquierda ? primerObjetivoEsUno : primerObjetivoEsDos;

  // int->ext + comando DOBLE: tras el cruce (ya recto) retrocede 300ms antes del cambio de carril.
  // Si el comando no es doble, no se activa y pasa directo a MANTENIENDO_CARRIL.
  {
    bool siguienteEsDobleV96 = mo_esComandoDobleEntraCarril1(mo_comandoSiguiente) ||
                               mo_esComandoDobleEntraCarril2(mo_comandoSiguiente);
    mo_retrocesoPostCrucePendiente = mo_carrilAnteriorInterno &&
                                     mo_siguienteEntraPorExterior &&
                                     siguienteEsDobleV96;
    if (mo_retrocesoPostCrucePendiente) {
      SerialBT.println("⏪ int->ext con comando doble: retroceso de " + String(MO_TIEMPO_RETROCESO_POST_CRUCE) + "ms tras terminar el cruce");
    }
  }
  SerialBT.println("🟢 MANIOBRA OPTIMIZADA — sentido:" + String(esIzquierda ? "IZQ" : "DER") +
                    " | comandoAnterior:" + comandoAnterior +
                    " | comandoSiguiente:" + mo_comandoSiguiente +
                    " | carrilAnteriorInterno:" + String(mo_carrilAnteriorInterno) +
                    " | comandoSiguienteExterior:" + String(mo_esComandoExterior) +
                    " | patron:" + String(mo_usarPatronRetrocesoAvance ? "RETROCESO+CRUCE" : "AVANCE_TOF+CRUCE") +
                    " | anguloPrevio:" + String(mo_anguloPrevio) +
                    " | anguloNuevo:" + String(mo_anguloNuevo));

  // Reseteo de variables de compensación de retroceso, igual que hace realizarGiro()/
  // procesarRetrocesoPostGiro() al iniciar un giro nuevo, para no arrastrar estado de un giro viejo.
  correccionCompletada = false;
  estadoActualCorreccion = RECTO;
  tiempoTotalRetroceso = 0;
  huboRetrocesoEnCorreccion = false;
  mo_faseEmergenciaCruce = MO_EMERG_INACTIVA;
  mo_huboEmergenciaEnCruce = false;
    mo_emergIntExtActiva = false;      // v102
  mo_emergIntExtContador = 0;        // v102
  mo_emergIntExtUltima = 0;          // v102

  mo_ajusteManteniendoDobleActivo = false;

  faseManiobraOptimizada = MO_ENDEREZANDO;
  mo_tiempoInicioFase = millis();
  
  mo_confirmarObjetoActivo = false; // v96: se vuelve a calcular en MO_FINALIZADA
  mo_reiniciarDeteccionObjeto();

  mo_tiempoInicioEnderezado = mo_tiempoInicioFase; // para descontar luego del retroceso
    mo_endPaso = 0; // v103

  detenerMotor();
  cambiarEstado(MANIOBRA_OPTIMIZADA);

  return true;
}

// ==================== CORRECCIÓN SUAVE DE ÁNGULO PARA LA MANIOBRA OPTIMIZADA ====================
// A diferencia de controlarDireccion() (que gira el servo A FONDO hasta encajar en rango,
// pensada para el giro brusco de 90°), esta función aplica una corrección PROPORCIONAL y
// suave: mientras más grande el error de ángulo, más se mueve el servo hacia ese lado, pero
// sin irse a los extremos salvo que el error sea grande. Pensada para tramos donde el auto
// debe avanzar recto (o retroceder recto) corrigiendo solo pequeñas desviaciones.
const float MO_ANGULO_CORRECCION_MAX = 35.0; // grados de error a partir de los cuales satura

// Servo proporcional alrededor del centro, con límite distinto por lado
int mo_calcularServoSuave(float diferencia) {
  float dif  = constrain(diferencia, -MO_ANGULO_CORRECCION_MAX, MO_ANGULO_CORRECCION_MAX);
  float frac = dif / MO_ANGULO_CORRECCION_MAX;   // -1.0 .. +1.0
  // frac > 0 -> corregir DERECHA (servo baja) | frac < 0 -> IZQUIERDA (servo sube)
  int correccion = (frac > 0) ? (int)(frac * MO_SERVO_CORR_MAX_DER)
                              : (int)(frac * MO_SERVO_CORR_MAX_IZQ);
  int servoObjetivo = SERVO_CENTER + correccion;   // frac>0 = derecha (positivo), frac<0 = izquierda (negativo)
  return constrain(servoObjetivo, SERVO_CENTER - MO_SERVO_CORR_MAX_IZQ,
                                  SERVO_CENTER + MO_SERVO_CORR_MAX_DER);
}

void mo_corregirRumboSuave(float anguloObjetivoParam) {
  float diferencia = calcularDiferenciaAngular(anguloZ, anguloObjetivoParam);
  if (abs(diferencia) <= TOLERANCIA_ANGULO) {
    direccion.write(SERVO_CENTER);
    return;
  }
  direccion.write(mo_calcularServoSuave(diferencia));
}

// v103: corrección suave de rumbo PARA RETROCESO. Al ir en reversa el volante actúa al revés:
// ángulo > objetivo (apunta a la izquierda) -> servo a la IZQUIERDA (igual que iniciarCorreccion()).
void mo_corregirRumboRetroceso(float anguloObjetivoParam) {
  float dif = calcularDiferenciaAngular(anguloZ, anguloObjetivoParam);
  if (abs(dif) <= MO_RETRO_ZONA_MUERTA_GRADOS) {
    direccion.write(SERVO_CENTER);
    return;
  }
  float frac = constrain(dif, -MO_RETRO_SATURA_GRADOS, MO_RETRO_SATURA_GRADOS) / MO_RETRO_SATURA_GRADOS;
  int servo = (int)(frac * MO_RETRO_SERVO_MAX);
  if (MO_RETRO_INVERTIR_CORRECCION) servo = -servo;
  direccion.write(constrain(servo, SERVO_LEFT, SERVO_RIGHT));
}

// v96: igual que mo_corregirRumboSuave() pero DEVUELVE el valor del servo (no lo escribe).
int mo_servoRumboSuaveValor(float anguloObjetivoParam) {
  float diferencia = calcularDiferenciaAngular(anguloZ, anguloObjetivoParam);
  if (abs(diferencia) <= TOLERANCIA_ANGULO) return SERVO_CENTER;
  return mo_calcularServoSuave(diferencia);
}

// ==================== v102: ESQUIVE LATERAL SUAVE EN EL AVANCE ====================
// Offset (grados) que se suma al ángulo objetivo. Positivo = izquierda (anguloZ sube).
// Objeto cerca por la IZQUIERDA -> offset negativo (se aleja a la derecha), y viceversa.
// 0 si no hay objeto cercano o la función está desactivada.
float mo_calcularOffsetEsquiveLateral() {
  if (!MO_HABILITAR_ESQUIVE_LATERAL_AVANCE) return 0.0f;

  float dI = leerToF('I');
  float dD = leerToF('D');
  bool iValida = (dI > 0 && dI < MO_ESQUIVE_DIST_ACTIVA_CM);
  bool dValida = (dD > 0 && dD < MO_ESQUIVE_DIST_ACTIVA_CM);
  if (!iValida && !dValida) return 0.0f;

  float exceso, signo;
  if (iValida && (!dValida || dI <= dD)) {
    exceso = MO_ESQUIVE_DIST_ACTIVA_CM - dI;
    signo = -1.0f;   // objeto a la izquierda -> alejarse a la derecha
  } else {
    exceso = MO_ESQUIVE_DIST_ACTIVA_CM - dD;
    signo = 1.0f;    // objeto a la derecha -> alejarse a la izquierda
  }
  return constrain(signo * MO_ESQUIVE_KP_GRADOS_POR_CM * exceso,
                   -MO_ESQUIVE_MAX_OFFSET_GRADOS, MO_ESQUIVE_MAX_OFFSET_GRADOS);
}

// Mantiene el rumbo (anguloBase + offset). Sin offset = comportamiento original (recto).
// Con offset usa tolerancia estricta, porque la tolerancia normal (8°) se comería el desvío.
void mo_corregirRumboConEsquive(float anguloBase) {
  float offset = mo_calcularOffsetEsquiveLateral();
  if (offset == 0.0f) {
    mo_corregirRumboSuave(anguloBase);
    return;
  }
  float diferencia = calcularDiferenciaAngular(anguloZ, anguloBase + offset);
  if (abs(diferencia) <= MO_ESQUIVE_TOLERANCIA_GRADOS) {
    direccion.write(SERVO_CENTER);
  } else {
    direccion.write(mo_calcularServoSuave(diferencia));
  }
}

// Resta el tiempo que tardó MO_ENDEREZANDO corrigiendo, del retroceso que sigue, para no
// "retroceder de más" en total. Nunca devuelve menos de un piso mínimo de seguridad.
const unsigned long MO_PISO_MINIMO_RETROCESO = 50; // ms, para que siempre haya al menos un toque de retroceso
unsigned long mo_calcularDuracionRetrocesoAjustada() {
   unsigned long duracionBase = mo_retrocesoEExtAExtActivo ? MO_TIEMPO_RETROCESO_E_EXT_A_EXT
                             : (mo_retrocesoEExtAIntActivo ? MO_TIEMPO_RETROCESO_E_EXT_A_INT
                             : (mo_esComandoExterior ? MO_TIEMPO_RETROCESO_LARGO : MO_TIEMPO_RETROCESO_CORTO));
  unsigned long tiempoEnderezando = (millis() - mo_tiempoInicioEnderezado)/2;

  if (tiempoEnderezando >= duracionBase) {
    return MO_PISO_MINIMO_RETROCESO;
  }
  unsigned long ajustada = duracionBase - tiempoEnderezando;
  if (ajustada < MO_PISO_MINIMO_RETROCESO) {
    return MO_PISO_MINIMO_RETROCESO;
  }
  return ajustada;
}


// Arranca saltando la "zona muerta" del motor (PWM bajos que no lo mueven), así no hay escalones
// lentos mientras la rampa sube de a un paso por vuelta. Después la rampa normal termina el resto.
void mo_avanzarConArranque(int velocidad) {
  avanzarMotor(velocidad);
  int piso = min(MO_PWM_ARRANQUE_MIN, velocidad);
  if (pwmActualMotor < piso) {
    pwmActualMotor = piso;
    ledcWrite(MOTOR_ENA, pwmActualMotor);
  }
}

// v103: mantiene el ángulo mientras RETROCEDE. En reversa el volante actúa al revés:
// error>0 (apunta a la izquierda del objetivo) -> volante a la IZQUIERDA (igual que iniciarCorreccion()).
void mo_servoRumboRetrocesoExtInt(float objetivo) {
  float err = calcularDiferenciaAngular(anguloZ, objetivo);
  if (abs(err) <= MO_EXTINT_ZONA_MUERTA_GRADOS) { direccion.write(SERVO_CENTER); return; }
  float frac = constrain(err, -MO_EXTINT_SATURA_GRADOS, MO_EXTINT_SATURA_GRADOS) / MO_EXTINT_SATURA_GRADOS;
  int servo = (int)(frac * MO_EXTINT_SERVO_MAX);
  if (MO_EXTINT_INVERTIR) servo = -servo;
  direccion.write(constrain(servo, SERVO_LEFT, SERVO_RIGHT));
}

// ==================== MÁQUINA DE ESTADOS DE LA MANIOBRA OPTIMIZADA ====================
// No bloqueante: se llama una vez por cada vuelta del loop() mientras estadoActual == MANIOBRA_OPTIMIZADA.
// El ángulo (actualizarGiro) y la rampa del motor (actualizarRampaMotor) se actualizan en
// TODAS las fases, en cada llamada.
void procesarManiobraOptimizada() {
  actualizarGiro();
  actualizarRampaMotor();
  giroEnProceso = true;

  switch (faseManiobraOptimizada) {

    // ---------------------------------------------------------------
    case MO_ENDEREZANDO: {
      // Si el vehículo está desviado respecto al ángulo que traía (mo_anguloPrevio),
      // lo enderezamos antes de arrancar la maniobra. Reutiliza la misma lógica de
      // rango/corrección que usa realizarGiro() antes de girar.
      //bool enRango = estaEnRangoCorrectoCorrecionDeLineaAntesDelGiro(anguloZ, mo_anguloPrevio);
            bool enRango = mo_estaEnRangoEnderezado(anguloZ, mo_anguloPrevio);
      if (enRango) {
                // v103: enderezado del servo en 2 pasos (+-10° y luego centro)
        if (mo_endPaso != 3) {
          detenerMotor();
          if (mo_endPaso == 0) { mo_endPaso = 1; mo_endPasoInicio = millis(); }
          if (mo_endPaso == 1) {
            direccion.write(mo_giroIzquierda ? -MO_SERVO_GRADOS_PASO_INTERMEDIO : MO_SERVO_GRADOS_PASO_INTERMEDIO);
            if (millis() - mo_endPasoInicio >= MO_PASO_INTERMEDIO_MS) { mo_endPaso = 2; mo_endPasoInicio = millis(); }
            break;
          }
          if (mo_endPaso == 2) {
            direccion.write(SERVO_CENTER);
            if (millis() - mo_endPasoInicio >= MO_PASO_INTERMEDIO_MS) { mo_endPaso = 3; } else { break; }
          }
        }
        mo_tiempoInicioEnderezado += 2UL * MO_PASO_INTERMEDIO_MS; // estas esperas no deben restarse del retroceso
        mo_endPaso = 0;

        SerialBT.println("✅ ENDEREZADO MO: rango estricto logrado | yaw=" + String(anguloZ, 1) +
                  " obj=" + String(mo_anguloPrevio, 1) +
                  " | tardó " + String(millis() - mo_tiempoInicioEnderezado) + "ms");
        detenerMotor();
        direccion.write(SERVO_CENTER);
        estadoActualCorreccion = RECTO;

        // Bifurcación por patrón (tabla del punto 1 vs punto 2 que diste):
        if (mo_usarPatronRetrocesoAvance) {
          faseManiobraOptimizada = mo_esComandoExterior ? MO_RETROCESO_LARGO : MO_RETROCESO_CORTO;
          mo_duracionRetrocesoObjetivo = mo_calcularDuracionRetrocesoAjustada();
          SerialBT.println("↔️ MANIOBRA OPTIMIZADA: enderezado OK, iniciando retroceso (" +
                            String(mo_esComandoExterior ? "LARGO" : "CORTO") + ", ajustado a " +
                            String(mo_duracionRetrocesoObjetivo) + "ms)");
        } else {
          faseManiobraOptimizada = MO_AVANZANDO_HASTA_TOF;
          SerialBT.println("↔️ MANIOBRA OPTIMIZADA: enderezado OK, avanzando hasta 40cm (ToF frontal)");
        }
        mo_tiempoInicioFase = millis();
      } else {
        //mo_controlarDireccionEnderezado(mo_anguloPrevio);
                mo_endPaso = 0; // v103: si se salió del rango, se cancelan los 2 pasos del servo y se vuelve a corregir
        mo_controlarDireccionEnderezado(mo_anguloPrevio);
        // v103: mientras corrige (retrocede con el volante lleno), hacerlo más despacio para no pasarse del ángulo
        if (estadoActualCorreccion == CORRIGIENDO_IZQUIERDA || estadoActualCorreccion == CORRIGIENDO_DERECHA) {
          retrocederMotor(MO_VELOCIDAD_ENDEREZADO);
        }

        if (millis() - mo_tiempoInicioFase > TIEMPO_MAX_CORRECCION) {
          SerialBT.println("⏱️ MANIOBRA OPTIMIZADA: timeout en enderezado, se continúa igual. | yaw=" +
                           String(anguloZ, 1) + " obj=" + String(mo_anguloPrevio, 1) +
                           " dif=" + String(calcularDiferenciaAngular(anguloZ, mo_anguloPrevio), 1) +
                           " estadoCorreccion=" + String((int)estadoActualCorreccion));          detenerMotor();
          direccion.write(SERVO_CENTER);
          estadoActualCorreccion = RECTO;
          faseManiobraOptimizada = mo_usarPatronRetrocesoAvance
                                      ? (mo_esComandoExterior ? MO_RETROCESO_LARGO : MO_RETROCESO_CORTO)
                                      : MO_AVANZANDO_HASTA_TOF;
          if (mo_usarPatronRetrocesoAvance) {
            mo_duracionRetrocesoObjetivo = mo_calcularDuracionRetrocesoAjustada();
          }
          mo_tiempoInicioFase = millis();
        }
      }
      break;
    }

    // ---------------------------------------------------------------
    case MO_RETROCESO_CORTO: {
      // Carril anterior INTERNO -> retroceso breve (duración ya ajustada restando el
      // tiempo que tardó el enderezado previo), corrigiendo suavemente si se desvía.
      //mo_corregirRumboSuave(mo_anguloPrevio);

            mo_corregirRumboRetroceso(mo_anguloPrevio);

      retrocederMotor();

      if (millis() - mo_tiempoInicioFase >= mo_duracionRetrocesoObjetivo) {
        detenerMotor();
        faseManiobraOptimizada = MO_ESPERANDO_PAUSA_RETROCESO;
        mo_tiempoInicioFase = millis();
      }
      break;
    }

    // ---------------------------------------------------------------
    case MO_RETROCESO_LARGO: {

            // v103: exterior->interior = retroceso lento, mantiene el ángulo y para cuando el ToF frontal marca > 68cm
      if (MO_EXTINT_ACTIVO && mo_esTramoExteriorAInterior) {
        mo_servoRumboRetrocesoExtInt(mo_anguloPrevio);
        retrocederMotor(MO_EXTINT_VELOCIDAD_RETROCESO);

        float dFrontalRetro = leerToF('F');
        bool lejos = (dFrontalRetro >= 999.0) ? MO_EXTINT_999_CUENTA_LEJOS
                                              : (dFrontalRetro > MO_EXTINT_DIST_FRONTAL_PARAR_CM);
        if (lejos) mo_extIntConfirmaciones++; else mo_extIntConfirmaciones = 0;

        bool llegoDistancia = (mo_extIntConfirmaciones >= MO_EXTINT_CONFIRMACIONES);
        bool timeoutRetro = (millis() - mo_tiempoInicioFase >= MO_EXTINT_TIMEOUT_RETROCESO_MS);
        if (llegoDistancia || timeoutRetro) {
          SerialBT.println(llegoDistancia
              ? "📏 EXT->INT: ToF frontal " + String(dFrontalRetro, 1) + "cm > " + String(MO_EXTINT_DIST_FRONTAL_PARAR_CM, 0) + "cm. Deteniendo retroceso y cruzando."
              : "⏱️ EXT->INT: timeout del retroceso (" + String(MO_EXTINT_TIMEOUT_RETROCESO_MS) + "ms) sin confirmar distancia. Cruzando igual.");
          detenerMotor();
          mo_extIntConfirmaciones = 0;
          faseManiobraOptimizada = MO_ESPERANDO_PAUSA_RETROCESO;
          mo_tiempoInicioFase = millis();
        }
        break;
      }

      // Carril anterior EXTERIOR -> retroceso de hasta 2 segundos (duración ya ajustada
      // restando el tiempo que tardó el enderezado previo), corrigiendo suavemente.
      //mo_corregirRumboSuave(mo_anguloPrevio);
            mo_corregirRumboRetroceso(mo_anguloPrevio);

      retrocederMotor();

      if (millis() - mo_tiempoInicioFase >= mo_duracionRetrocesoObjetivo) {
        detenerMotor();
        faseManiobraOptimizada = MO_ESPERANDO_PAUSA_RETROCESO;
        mo_tiempoInicioFase = millis();
      }
      break;
    }

    // ---------------------------------------------------------------
    case MO_ESPERANDO_PAUSA_RETROCESO: {
      // Pausa no bloqueante para asentar antes de cruzar.
      detenerMotor();
      direccion.write(SERVO_CENTER);

      if (millis() - mo_tiempoInicioFase >= MO_PAUSA_TRAS_RETROCESO) {
        faseManiobraOptimizada = MO_CRUZANDO;
        mo_tiempoInicioFase = millis();
        SerialBT.println("↪️ MANIOBRA OPTIMIZADA: pausa post-retroceso cumplida, cruzando hacia " +
                          String(mo_anguloNuevo) + "°");
      }
      break;
    }

    // ---------------------------------------------------------------
    case MO_AVANZANDO_HASTA_TOF: {
      // Avanza manteniendo el ángulo PREVIO (el que traía antes de la línea) hasta que
      // el sensor frontal detecte ~40cm. Usa corrección SUAVE (no controlarDireccion, que
      // gira a fondo) porque aquí queremos ir recto, no ejecutar un giro de 90°.
      // ── v102: EMERGENCIA interior->exterior mientras avanza hacia el ToF ──
      // Giro izquierda -> Sharp izquierdo | giro derecha -> Sharp derecho.
      // Si ve < 15cm: se detiene y retrocede 200ms con la dirección hacia el lado del giro,
      // y luego sigue avanzando cuidando el ángulo.
      if (MO_HABILITAR_EMERGENCIA_INT_EXT && mo_esTramoInteriorAExterior) {

        if (mo_emergIntExtActiva) {
          direccion.write(mo_giroIzquierda ? SERVO_LEFT : SERVO_RIGHT);
          retrocederMotor();
          if (millis() - mo_emergIntExtInicio >= MO_EMERG_INT_EXT_RETROCESO_MS) {
            detenerMotor();
            mo_emergIntExtActiva = false;
            mo_emergIntExtUltima = millis();
            mo_tiempoInicioFase += MO_EMERG_INT_EXT_RETROCESO_MS; // el retroceso no cuenta para el timeout del avance
            Serial.println("▶️ EMERGENCIA INT->EXT: retroceso terminado, se reanuda el avance.");
          }
          break;
        }

        if (mo_emergIntExtContador < MO_EMERG_INT_EXT_MAX_REPETICIONES &&
            millis() - mo_emergIntExtUltima > MO_EMERG_INT_EXT_COOLDOWN_MS) {
          int pinMismoLado = mo_giroIzquierda ? SHARP_DIAGONAL_IZQ_PIN : SHARP_DIAGONAL_DER_PIN;
          float dLado = leerSensorSharp(pinMismoLado);
          if (dLado > 0 && dLado < MO_EMERG_INT_EXT_DIST_CM) {
            Serial.println("⚠️ EMERGENCIA INT->EXT: Sharp " + String(mo_giroIzquierda ? "IZQ" : "DER") +
                           " = " + String(dLado, 1) + "cm. Deteniendo y retrocediendo " +
                           String(MO_EMERG_INT_EXT_RETROCESO_MS) + "ms.");
            detenerMotor();
            mo_emergIntExtActiva = true;
            mo_emergIntExtInicio = millis();
            mo_emergIntExtContador++;
            break;
          }
        }
      }

      mo_corregirRumboConEsquive(mo_anguloPrevio);   // v102: mantiene rumbo + se aleja suave del objeto lateral
      mo_avanzarConArranque(MO_VELOCIDAD_AVANCE_TOF);
      
      avanzarMotor();
      float distanciaFrontal = leerToF('F');
      bool distanciaAlcanzada = (distanciaFrontal > 0 && distanciaFrontal <= mo_distanciaFrontalObjetivoActual);

      // Timeout de seguridad por si el sensor falla o nunca alcanza la distancia.
      bool timeoutAvance = (millis() - mo_tiempoInicioFase > TIEMPO_MAX_CORRECCION * 2);

      if (distanciaAlcanzada || timeoutAvance) {
        if (timeoutAvance && !distanciaAlcanzada) {
          SerialBT.println("⏱️ MANIOBRA OPTIMIZADA: timeout esperando " + String(mo_distanciaFrontalObjetivoActual, 0) + "cm frontales, se continúa igual.");
                  } else {
          SerialBT.println("📏 MANIOBRA OPTIMIZADA: " + String(mo_distanciaFrontalObjetivoActual, 0) + "cm frontales alcanzados (" + String(distanciaFrontal, 1) + "cm).");
                  }
        detenerMotor();
        //direccion.write(SERVO_CENTER);
        direccion.write(mo_giroIzquierda ? -MO_SERVO_GRADOS_PASO_INTERMEDIO : MO_SERVO_GRADOS_PASO_INTERMEDIO); // v103: paso 1
        
        faseManiobraOptimizada = MO_ESPERANDO_PAUSA_TOF;
        mo_tiempoInicioFase = millis();
      }
      break;
    }

    // ---------------------------------------------------------------
    case MO_ESPERANDO_PAUSA_TOF: {
      // Pausa no bloqueante tras alcanzar los 40cm, antes de cruzar.
      detenerMotor();
      mo_servoEnderezadoDosPasos(); // v103: +-10° y luego centro

      if (millis() - mo_tiempoInicioFase >= MO_PASO_INTERMEDIO_MS + MO_PAUSA_TRAS_TOF) {
        faseManiobraOptimizada = MO_CRUZANDO;
        mo_tiempoInicioFase = millis();
        SerialBT.println("↪️ MANIOBRA OPTIMIZADA: pausa post-ToF cumplida, cruzando hacia " +
                          String(mo_anguloNuevo) + "°");
      }
      break;
    }

    // ---------------------------------------------------------------
    case MO_CRUZANDO: {
      // ── FORZADO ARBITRARIO: INTERIOR->INTERIOR (servo CONTRARIO al cruce durante el retroceso) ──
      /*if (mo_faseEmergenciaCruce == MO_EMERG_INACTIVA &&
          mo_emergenciaForzadaPendienteInteriorInterior && !mo_emergenciaForzadaYaEjecutada &&
          millis() - mo_tiempoInicioFase >= MO_TIEMPO_FORZAR_EMERGENCIA_CRUCE_INT_INT) {*/

        if (mo_faseEmergenciaCruce == MO_EMERG_INACTIVA &&
          MO_HABILITAR_EMERGENCIAS_FORZADAS && mo_emergenciaForzadaPendienteInteriorInterior && !mo_emergenciaForzadaYaEjecutada &&
          millis() - mo_tiempoInicioFase >= MO_TIEMPO_FORZAR_EMERGENCIA_CRUCE_INT_INT) {
  

        Serial.println("⚠️ MANIOBRA OPTIMIZADA: emergencia FORZADA interior->interior (servo contrario, 1seg cumplido). Deteniendo y retrocediendo...");

        detenerMotor();
        direccion.write(mo_giroIzquierda ? SERVO_RIGHT : SERVO_LEFT); // servo CONTRARIO al lado del cruce
        mo_faseEmergenciaCruce = MO_EMERG_RETROCEDIENDO;
        mo_tiempoInicioEmergenciaCruce = millis();
        mo_emergenciaForzadaYaEjecutada = true;
        mo_emergenciaActivaEsForzadaInteriorInterior = true; // para que el retroceso sepa mantener el servo contrario
        break;
      }

            // ── FORZADO ARBITRARIO: EXTERIOR->INTERIOR (v96: servo CONTRARIO al cruce durante el retroceso) ──
     /* if (mo_faseEmergenciaCruce == MO_EMERG_INACTIVA &&
          mo_emergenciaForzadaPendienteExteriorInterior && !mo_emergenciaForzadaYaEjecutada &&
          millis() - mo_tiempoInicioFase >= MO_TIEMPO_FORZAR_EMERGENCIA_CRUCE) {
        */
        if (mo_faseEmergenciaCruce == MO_EMERG_INACTIVA &&
         MO_HABILITAR_EMERGENCIAS_FORZADAS && mo_emergenciaForzadaPendienteExteriorInterior && !mo_emergenciaForzadaYaEjecutada &&
          millis() - mo_tiempoInicioFase >= MO_TIEMPO_FORZAR_EMERGENCIA_CRUCE) {
        Serial.println("⚠️ MANIOBRA OPTIMIZADA: emergencia FORZADA exterior->interior (1seg cumplido). Deteniendo y retrocediendo con servo contrario...");

        detenerMotor();
        direccion.write(mo_giroIzquierda ? SERVO_RIGHT : SERVO_LEFT); // v96: servo CONTRARIO al lado del cruce
        mo_faseEmergenciaCruce = MO_EMERG_RETROCEDIENDO;
        mo_tiempoInicioEmergenciaCruce = millis();
        mo_emergenciaForzadaYaEjecutada = true;
        mo_emergenciaActivaEsForzadaInteriorInterior = false; // la duración sigue siendo la normal (600ms)
        mo_emergenciaServoContrarioActiva = true;             // v96: solo cambia el servo
        break;
      }
      // ── v98: FORZADO ARBITRARIO EXTERIOR->EXTERIOR con comando "E" (servo CONTRARIO al cruce, UNA sola vez) ──
      /*if (mo_faseEmergenciaCruce == MO_EMERG_INACTIVA &&
          mo_esTramoExtExtConE && !mo_emergenciaForzadaYaEjecutada && !mo_huboEmergenciaEnCruce &&
          millis() - mo_tiempoInicioFase >= MO_TIEMPO_FORZAR_EMERGENCIA_CRUCE_EXT_EXT) {*/
      if (mo_faseEmergenciaCruce == MO_EMERG_INACTIVA &&
          MO_HABILITAR_EMERGENCIAS_FORZADAS && mo_esTramoExtExtConE && !mo_emergenciaForzadaYaEjecutada && !mo_huboEmergenciaEnCruce &&
          millis() - mo_tiempoInicioFase >= MO_TIEMPO_FORZAR_EMERGENCIA_CRUCE_EXT_EXT) {

        Serial.println("⚠️ MANIOBRA OPTIMIZADA: emergencia FORZADA exterior->exterior (E). Retrocediendo con servo contrario...");

        detenerMotor();
        direccion.write(mo_giroIzquierda ? SERVO_RIGHT : SERVO_LEFT); // servo CONTRARIO al lado del cruce
        mo_faseEmergenciaCruce = MO_EMERG_RETROCEDIENDO;
        mo_tiempoInicioEmergenciaCruce = millis();
        mo_emergenciaForzadaYaEjecutada = true;               // solo esta vez va cruzada
        mo_emergenciaActivaEsForzadaInteriorInterior = false; // duración normal
        mo_emergenciaServoContrarioActiva = true;             // el retroceso mantiene el servo cruzado
        break;
      }
      // ── VERIFICACIÓN DE EMERGENCIA POR SHARP (solo si no hay una emergencia ya en curso) ──
      if (mo_faseEmergenciaCruce == MO_EMERG_INACTIVA) {
        int pinSharpVigilar = mo_giroIzquierda ? SHARP_DIAGONAL_DER_PIN : SHARP_DIAGONAL_IZQ_PIN;
        float distSharpCruce = leerSensorSharp(pinSharpVigilar);

        if (distSharpCruce > 0 && distSharpCruce <= DISTANCIA_EMERGENCIA_SHARP_PARED &&
            millis() - mo_ultimoRetrocesoEmergenciaCruce > MO_COOLDOWN_EMERGENCIA_CRUCE) {

          Serial.println("⚠️ MANIOBRA OPTIMIZADA: SHARP emergencia en cruce (" + String(distSharpCruce, 1) +
                            "cm). Deteniendo y retrocediendo...");

          detenerMotor();
          // v96: exterior->exterior: servo CONTRARIO siempre.
          // v97: interior->exterior: servo CONTRARIO SOLO la primera emergencia; después centrado.
          bool usarServoContrario = (mo_esTramoExteriorAExterior && !mo_esTramoExtExtConE) ||
                                              (mo_esTramoInteriorAExterior && !mo_emergenciaCruzadaIntExtYaUsada);
          if (MO_EMERG_CRUCE_SERVO_RECTO) usarServoContrario = false; // v103: retroceso de emergencia con servo recto
          if (usarServoContrario && mo_esTramoInteriorAExterior && !mo_emergenciaCruzadaIntExtYaUsada) {
            mo_emergenciaCruzadaIntExtYaUsada = true; // las siguientes emergencias van centradas
            Serial.println("↩️ v97: interior->exterior, 1ra emergencia con servo CRUZADO.");
          }
          //direccion.write(usarServoContrario ? (mo_giroIzquierda ? SERVO_RIGHT : SERVO_LEFT) : SERVO_CENTER);
          direccion.write(usarServoContrario && !mo_huboEmergenciaEnCruce ? (mo_giroIzquierda ? SERVO_RIGHT : SERVO_LEFT) : SERVO_CENTER);          
          mo_faseEmergenciaCruce = MO_EMERG_RETROCEDIENDO;
          mo_tiempoInicioEmergenciaCruce = millis();
          mo_emergenciaActivaEsForzadaInteriorInterior = false;
          //mo_emergenciaServoContrarioActiva = usarServoContrario;
          mo_emergenciaServoContrarioActiva = usarServoContrario && !mo_huboEmergenciaEnCruce;
                    mo_registrarEmergenciaIntInt(); // v104

             break; // no seguimos evaluando el resto de este ciclo, ya entramos en emergencia
          }
      }

      // ── VERIFICACIÓN DE EMERGENCIA POR TOF FRONTAL (solo interior->interior, mismo disparador que el Sharp) ──
      if (mo_faseEmergenciaCruce == MO_EMERG_INACTIVA && mo_esTramoInteriorAInterior) {
        float distFrontalCruce = leerToF('F');

        if (distFrontalCruce < 999.0 && distFrontalCruce <= MO_DISTANCIA_EMERGENCIA_TOF_FRONTAL &&
            millis() - mo_ultimoRetrocesoEmergenciaCruce > MO_COOLDOWN_EMERGENCIA_CRUCE) {

          Serial.println("⚠️ MANIOBRA OPTIMIZADA: ToF FRONTAL emergencia en cruce interior->interior (" +
                            String(distFrontalCruce, 1) + "cm). Deteniendo y retrocediendo...");

          detenerMotor();
          direccion.write(SERVO_CENTER); // misma máquina de emergencia que el Sharp: servo centrado
                    mo_registrarEmergenciaIntInt(); // v104

          mo_faseEmergenciaCruce = MO_EMERG_RETROCEDIENDO;
          mo_tiempoInicioEmergenciaCruce = millis();
          mo_emergenciaActivaEsForzadaInteriorInterior = false;
          break;
        }
      }

      // ── MÁQUINA DE LA EMERGENCIA (si está activa, tiene prioridad sobre el cruce normal) ──
      if (mo_faseEmergenciaCruce == MO_EMERG_RETROCEDIENDO) {
         mo_huboEmergenciaEnCruce = true; 
                 // v98: en ext->ext con "E" el retroceso SÍ respeta el ángulo objetivo: si llega, se termina
        // la emergencia y se entrega a MO_FINALIZADA (igual que cuando el cruce normal lo alcanza).
        if ((mo_esTramoExtExtConE || mo_esTramoInteriorAExterior) && estaEnRangoCorrecto(anguloZ, mo_anguloNuevo)) {
          Serial.println("✅ MANIOBRA OPTIMIZADA: ángulo nuevo alcanzado DURANTE el retroceso de emergencia (" +
                         String(anguloZ, 1) + "°). Finalizando.");
          detenerMotor();
          direccion.write(SERVO_CENTER);
          mo_ultimoRetrocesoEmergenciaCruce = millis();
          anguloObjetivo = mo_anguloNuevo;
          faseManiobraOptimizada = MO_FINALIZADA;
          mo_faseEmergenciaCruce = MO_EMERG_INACTIVA;
          mo_tiempoInicioFase = millis();
          break;
        }
        // Servo: CONTRARIO al cruce si es el forzado interior->interior; centrado en cualquier
        // otro caso (forzado exterior->interior o emergencia normal por Sharp).
        // Duración: también independiente para el forzado interior->interior.
             int servoRetroceso = (mo_emergenciaActivaEsForzadaInteriorInterior || mo_emergenciaServoContrarioActiva)
                                ? (mo_giroIzquierda ? SERVO_RIGHT : SERVO_LEFT)
                                : SERVO_CENTER;
            unsigned long duracionRetrocesoEmergencia = mo_emergenciaActivaEsForzadaInteriorInterior
                            ? MO_DURACION_RETROCESO_EMERGENCIA_INT_INT
                            : ((mo_duracionEmergIntIntActual > 0)
                                ? mo_duracionEmergIntIntActual   // v104: int->int, 1ra larga / siguientes cortas
                                : ((mo_emergenciaServoContrarioActiva && mo_emergenciaForzadaPendienteExteriorInterior && mo_giroIzquierda)
                                    ? MO_DURACION_RETROCESO_EMERGENCIA_EXT_INT_IZQ
                                    : MO_DURACION_RETROCESO_EMERGENCIA));
        direccion.write(servoRetroceso);

        if (millis() - mo_tiempoInicioEmergenciaCruce < MO_PAUSA_ANTES_DE_RETROCEDER_EMERGENCIA) {
          detenerMotor();
          break;
        }
        retrocederMotor();
        if (millis() - mo_tiempoInicioEmergenciaCruce >=
              MO_PAUSA_ANTES_DE_RETROCEDER_EMERGENCIA + duracionRetrocesoEmergencia) {
          detenerMotor();
          mo_faseEmergenciaCruce = MO_EMERG_PAUSA_ANTES_REANUDAR;
          mo_tiempoInicioEmergenciaCruce = millis();
        }
        break;
      }

      if (mo_faseEmergenciaCruce == MO_EMERG_PAUSA_ANTES_REANUDAR) {
        // Volvemos a girar el servo hacia el lado del cruce y esperamos a que llegue
        // antes de reanudar el avance (igual que hace el código original con su delay(300)).
        detenerMotor();
        direccion.write(mo_giroIzquierda ? SERVO_LEFT : SERVO_RIGHT);
        if (millis() - mo_tiempoInicioEmergenciaCruce >= MO_PAUSA_ANTES_DE_REANUDAR_EMERGENCIA) {
          Serial.println("▶️ MANIOBRA OPTIMIZADA: reanudando cruce tras emergencia SHARP.");
          mo_ultimoRetrocesoEmergenciaCruce = millis();
          mo_faseEmergenciaCruce = MO_EMERG_INACTIVA; // vuelve a vigilar por si hace falta repetir
          // v96: si el siguiente comando empieza en carril EXTERIOR (ext->ext e int->ext),
          // reiniciamos el reloj del timeout del cruce para que las emergencias no lo agoten
          // (antes se cortaba tras ~3 emergencias por los 4s de TIEMPO_MAX_CORRECCION).
          if (mo_siguienteEntraPorExterior) {
            mo_tiempoInicioFase = millis();
          }
        }
        break;
      }

      // ── CRUCE NORMAL (sin emergencia activa) ──
      if (mo_esTramoInteriorAInterior) {
        avanzarLento();
      } else {
        avanzarMotor();
      }
      direccion.write(mo_giroIzquierda ? SERVO_LEFT : SERVO_RIGHT);

      // Umbral de fin de cruce: en exterior->interior se termina ANTES de llegar al ángulo exacto
      float umbralFinCruce = TOLERANCIA_ANGULO;
      if (mo_emergenciaForzadaPendienteExteriorInterior) {
        umbralFinCruce = mo_giroIzquierda ? MO_UMBRAL_FIN_CRUCE_EXT_INT_IZQ
                                          : MO_UMBRAL_FIN_CRUCE_EXT_INT_DER;
      }
      bool enRango = abs(calcularDiferenciaAngular(anguloZ, mo_anguloNuevo)) <= umbralFinCruce;
      bool timeoutCruce = (millis() - mo_tiempoInicioFase > TIEMPO_MAX_CORRECCION);

      if (enRango || timeoutCruce) {
        if (timeoutCruce && !enRango) {
          Serial.println("⏱️ MANIOBRA OPTIMIZADA: timeout en cruce, se continúa igual.");
        } else {
          Serial.println("✅ MANIOBRA OPTIMIZADA: ángulo nuevo alcanzado (" + String(anguloZ, 1) + "°).");
        }
        direccion.write(SERVO_CENTER);

        if (!enRango) mo_retrocesoPostCrucePendiente = false; // v96: terminó por timeout, no retroceder

        anguloObjetivo = mo_anguloNuevo; // consolidamos el ángulo objetivo global
        faseManiobraOptimizada = MO_FINALIZADA;
        mo_faseEmergenciaCruce = MO_EMERG_INACTIVA; // reset por si acaso, para la próxima maniobra
        mo_tiempoInicioFase = millis();
      }
      break;
    }

    // ---------------------------------------------------------------
    case MO_FINALIZADA: {
            // ── v96: int->ext con comando doble: retroceso corto siguiendo el ángulo objetivo ──
      // El tiempo se mide desde mo_tiempoInicioFase, que se reinicia al terminar el cruce.
      if (mo_retrocesoPostCrucePendiente) {
        mo_corregirRumboSuave(mo_anguloNuevo);
        retrocederMotor();
        if (millis() - mo_tiempoInicioFase >= MO_TIEMPO_RETROCESO_POST_CRUCE) {
          detenerMotor();
          mo_retrocesoPostCrucePendiente = false;
          SerialBT.println("✅ int->ext: retroceso post-cruce completado, iniciando cambio de carril.");
        }
        break; // mientras retrocede no se ejecuta el resto del cierre
      }
      // Cierre: entregamos el control al pipeline normal de carriles, tal como lo pediste.
      // 1) Reseteo de variables de compensación (igual que al inicio de un giro normal).
      detenerMotor();
      direccion.write(SERVO_CENTER);
      correccionCompletada = false;
      estadoActualCorreccion = RECTO;
      tiempoTotalRetroceso = 0;
      huboRetrocesoEnCorreccion = false;
      giroEnProceso = false;

      // 2) Aplicar (o no) el tiempo extra de +2.5s para tramos de carril exterior.
      //    Afecta tanto TIEMPO_MINIMO_MANTENIENDO_PARA_DETECTAR como tiempoEsperaDeteccion,
      //    y se mantiene activo mientras dure este tramo en MANTENIENDO_CARRIL.
      if (mo_esComandoExterior) {
        TIEMPO_MINIMO_MANTENIENDO_PARA_DETECTAR = TIEMPO_MINIMO_MANTENIENDO_PARA_DETECTAR_BASE + MO_EXTRA_TIEMPO_CARRIL_EXTERIOR;
        tiempoEsperaDeteccion = TIEMPO_ESPERA_DETECCION_BASE + MO_EXTRA_TIEMPO_CARRIL_EXTERIOR_DETECCION;
        mo_extenderEsperaDeteccionActivo = true;
        SerialBT.println("⏳ MANIOBRA OPTIMIZADA: comando siguiente de carril EXTERIOR -> +2.5s en tiempos de detección/mantenimiento.");
      } else {
        // Carril anterior INTERNO: verificamos si el comando siguiente también es interno
        // (mismo criterio mo_esCarrilInterno, basado en el grupo A/B del comando).
        bool comandoSiguienteInterno = mo_esCarrilInterno(mo_comandoSiguiente, mo_giroIzquierda);

        if (mo_carrilAnteriorInterno && comandoSiguienteInterno) {
          // INTERNO -> INTERNO: se llega más rápido a la línea, así que bajamos AMBOS
          // tiempos (son un OR en el guard de detección: si uno se queda alto, el otro
          // no basta por sí solo).
          TIEMPO_MINIMO_MANTENIENDO_PARA_DETECTAR = MO_TIEMPO_MINIMO_MANTENIENDO_INTERNO_A_INTERNO;
          tiempoEsperaDeteccion = MO_TIEMPO_ESPERA_DETECCION_INTERNO_A_INTERNO;
          SerialBT.println("⏩ MANIOBRA OPTIMIZADA: tramo INTERNO->INTERNO -> tiempos de detección reducidos (" +
                            String(MO_TIEMPO_MINIMO_MANTENIENDO_INTERNO_A_INTERNO) + "ms / " +
                            String(MO_TIEMPO_ESPERA_DETECCION_INTERNO_A_INTERNO) + "ms).");
        } else {
          // Cualquier otro caso (interno->externo, etc.) vuelve a los valores base normales.
          TIEMPO_MINIMO_MANTENIENDO_PARA_DETECTAR = TIEMPO_MINIMO_MANTENIENDO_PARA_DETECTAR_BASE;
          tiempoEsperaDeteccion = TIEMPO_ESPERA_DETECCION_BASE;
        }
        mo_extenderEsperaDeteccionActivo = false;
      }

      // 3) Entregamos el control al pipeline normal: procesamos el comando siguiente
      //    (configura carriles.carrilObjetivo, flags esComandoXX, etc. y puede dejar el
      //    estado en GIRANDO_HACIA_CARRIL o, si ya estaba en el carril, en MANTENIENDO_CARRIL),
      //    y luego forzamos MANTENIENDO_CARRIL vía cambiarEstado() para heredar sus efectos
      //    colaterales (ultimoCarrilActivo, reseteo de controlLateral, tiempoInicioEstado, etc.),
      //    saltándonos el giro brusco de esquina que procesarComandoCarril() habría intentado.
      mo_aproximacionLimitadaActiva = true; // limita la corrección por distancia hasta estar cerca de la pared
      procesarComandoCarril(mo_comandoSiguiente);
      cambiarEstado(MANTENIENDO_CARRIL);

            // v98: si esta maniobra optimizada corresponde a la línea final (limiteLineas), al terminar el cruce
      // el vehículo debe avanzar en MANTENIENDO_CARRIL el tiempo final y detenerse para siempre.
      // (En vueltas optimizadas no se pasa por ESPERANDO_COMANDO_CARRIL, que era donde se activaba esta bandera.)
      if (contadorLineas >= limiteLineas) {
        esUltimoTramoFinal = true;
        tiempoInicioUltimoAvance = 0; // el bloque del tramo final arranca el conteo en su primer ciclo
        mo_confirmarObjetoActivo = false; // el tramo final no espera objeto, solo avanza y se detiene
        SerialBT.println("🏁 Línea final (" + String(contadorLineas) + "/" + String(limiteLineas) +
                         ") tras maniobra optimizada: MANTENIENDO_CARRIL avanzará y se detendrá para siempre.");
      }
            // v104: entrada al carril interior guiada por la pared contraria también tras maniobra optimizada
      // (igual que tras GIRANDO_HACIA_CARRIL en la vuelta de reconocimiento)
      if (!esUltimoTramoFinal && sentidoVehiculo != INDEFINIDO) {
        bool carrilInteriorMO = (sentidoVehiculo == IZQUIERDA && carriles.carrilObjetivo == 2) ||
                                (sentidoVehiculo == DERECHA   && carriles.carrilObjetivo == 1);
        if (carrilInteriorMO) {
          ent_activa = true;
          ent_contEntro = 0;
          ent_tInicio = millis();
          SerialBT.println("🎯 ENTRADA CARRIL INTERIOR (maniobra optimizada): fase armada");
        }
      }

            // Ajuste de tiempo de manteniendo para comandos DOBLES cuando el tramo anterior terminó en carril INTERNO
      // (cubre interno->interno e interno->externo, según el carril de ENTRADA del comando doble)
      mo_ajusteManteniendoDobleActivo = false;
      bool siguienteEsDoble = mo_esComandoDobleEntraCarril1(mo_comandoSiguiente) ||
                              mo_esComandoDobleEntraCarril2(mo_comandoSiguiente);
      
            // v96: tramo anterior terminó en EXTERIOR + comando siguiente DOBLE (empiece en exterior o interior)
      //      -> el cambio de carril del primer carril espera a detectar el objeto, no al tiempo.
      mo_confirmarObjetoActivo = siguienteEsDoble && !esUltimoTramoFinal && !(mo_carrilAnteriorInterno && contadorLineas >= 5); // v100: interior + línea>=5 -> por tiempo
      mo_reiniciarDeteccionObjeto();
      if (mo_confirmarObjetoActivo) {
        SerialBT.println("👁️ Confirmación por OBJETO activada para el cambio de carril del comando doble.");
      }

      if (mo_carrilAnteriorInterno && siguienteEsDoble) {
        mo_ajusteManteniendoDobleActivo  = true;
        mo_manteniendoDobleEsEmergencia  = mo_huboEmergenciaEnCruce;
        mo_valorManteniendoDobleMs = mo_huboEmergenciaEnCruce
            ? (mo_giroIzquierda ? MO_MANT_DOBLE_EMERG_IZQ_MS : MO_MANT_DOBLE_EMERG_DER_MS)
            : MO_MANT_DOBLE_NORMAL_MS;
        SerialBT.println("⏱️ Ajuste manteniendo doble: " + String(mo_valorManteniendoDobleMs) +
                         "ms (emergencia=" + String(mo_huboEmergenciaEnCruce) + ")");
      }
            // NUEVO: EXTERIOR -> comando DOBLE que ENTRA por carril exterior + emergencia durante el cruce => +200ms
      mo_sumarManteniendoDobleActivo = false; // se reinicia en cada maniobra
      if (!mo_carrilAnteriorInterno && siguienteEsDoble && mo_huboEmergenciaEnCruce) {
        bool entraPorCarril1 = mo_primerCarrilObjetivoEsUno(mo_comandoSiguiente);
        // IZQUIERDA: carril1 = externo | DERECHA: carril2 = externo
        bool siguienteEntraPorExterior = mo_giroIzquierda ? entraPorCarril1 : !entraPorCarril1;
        if (siguienteEntraPorExterior) {
          mo_sumarManteniendoDobleActivo = true;
          SerialBT.println("⏱️ Ajuste manteniendo doble EXT->EXT con emergencia: +" +
                           String(MO_MANT_DOBLE_EXT_EXT_EMERG_EXTRA_MS) + "ms");
        }
      }
      SerialBT.println("🏁 MANIOBRA OPTIMIZADA finalizada. Entregado a MANTENIENDO_CARRIL (carril " +
                        String(carriles.carrilObjetivo) + ").");
      break;
    }
  }
}
// ==================== FUNCIÓN ejecutarGiroPID  ====================
void ejecutarGiroPID(float anguloObjetivo, bool giroIzquierda) {
  enGiro = true;
  giroEnProceso = true;
  cruceEnProceso = true;
  interrupcionCruce = false; 
  startTime = millis();

  // Se mantiene el cálculo del ángulo objetivo final
  actualizarGiro();
  actualizarRampaMotor();
  float anguloInicial = anguloZ;
  anguloObjetivoActual = anguloInicial + (giroIzquierda ? anguloObjetivo : -anguloObjetivo);

  // Se mantiene la normalización del ángulo objetivo (0-360 grados)
  while (anguloObjetivoActual >= 360) anguloObjetivoActual -= 360;
  while (anguloObjetivoActual < 0) anguloObjetivoActual += 360;

  // Activar motor para el giro
  avanzarMotor();
  //avanzarRapido(); 
  // Se elimina el cálculo PID. En su lugar, se aplica un giro constante y
  // brusco en la dirección deseada. Este comando se ejecuta una sola vez.
  if (giroIzquierda) {
    direccion.write(SERVO_LEFT); // Giro máximo a la izquierda
  } else {
    direccion.write(SERVO_RIGHT); // Giro máximo a la derecha
  }
  
  
  bool giroCompletado = false;
  int contadorEstable = 0;
  const int CONFIRMACIONES_ESTABLE = 1;

  // El bucle ahora solo verifica si se ha alcanzado el ángulo, sin calcular PID
  while (millis() - startTime < timeoutGiro && !giroCompletado && !interrupcionCruce) {
    actualizarGiro(); // Se sigue actualizando el ángulo actual
    actualizarRampaMotor();
  
    // VERIFICACIÓN DE EMERGENCIA FRONTAL DURANTE EL GIRO
    float distanciaFrontal = leerToF('F');
    if (distanciaFrontal < 14 && distanciaFrontal > 0) {//14
        SerialBT.println("!!! OBSTÁCULO FRONTAL DURANTE GIRO !!! Abortando.");
        break; // Sale del bucle de giro inmediatamente.
    }

    // Se calcula la diferencia con el objetivo para saber cuándo parar
    float errorActual = anguloObjetivoActual - anguloZ;
    if (errorActual > 180) errorActual -= 360;
    if (errorActual < -180) errorActual += 360;
    SerialBT.print("ERROR ACTUAL DE ANGULO GIRO DE: ");
    SerialBT.println(errorActual);
    // Si el error es menor que la tolerancia, consideramos que el giro terminó
    if (abs(errorActual) <= TOLERANCIA_PID) {
      contadorEstable++;
      if (contadorEstable >= CONFIRMACIONES_ESTABLE) {
        giroCompletado = true;
        SerialBT.println("Giro Completado***************");
        break;
      }
    } else {
      contadorEstable = 0;
    }
    
    delay(20);
  }

  // Se mantiene toda la lógica de finalización del giro:
  // centrar dirección, resetear flags y activar el retroceso post-giro.
  direccion.write(SERVO_CENTRADO);
  avanzarMotor();

  enGiro = false;
  giroEnProceso = false;
  cruceEnProceso = false;
  tiempoFinGiro = millis();

  direccionRetroceso = giroIzquierda ? SERVO_OrientadoObjeto_RIGH : SERVO_OrientadoObjeto_LEFT;
  ultimoGiroFueApertura = esGiroApertura; // Congelamos el estado ANTES de que se resetee más adelante
  retrocesoOrientadoActivo = true;
  tiempoInicioRetrocesoOrientado = millis();
}
void controlarDireccion(float anguloObjetivoParam) {
  // Determinar si el ángulo está en rango correcto respecto al ángulo objetivo
  bool enRangoCorrecto = estaEnRangoCorrecto(anguloZ, anguloObjetivoParam);
  
  // Debug: mostrar estado del rango cada 500ms
  static unsigned long lastRangeDebug = 0;
  if (millis() - lastRangeDebug > 500) {
    SerialBT.print("Ángulo actual: ");
    SerialBT.print(anguloZ);
    SerialBT.print("° - Objetivo: ");
    SerialBT.print(anguloObjetivoParam);
    SerialBT.print("° - En rango: ");
    SerialBT.print(enRangoCorrecto ? "SÍ" : "NO");
    SerialBT.print(" - Estado: ");
    
    switch (estadoActualCorreccion) {
      case RECTO: SerialBT.println("RECTO"); break;
      case CORRIGIENDO_IZQUIERDA: SerialBT.println("CORRIGIENDO IZQUIERDA"); break;
      case CORRIGIENDO_DERECHA: SerialBT.println("CORRIGIENDO DERECHA"); break;
      case ESTABILIZANDO: SerialBT.println("ESTABILIZANDO"); break;
    }
    lastRangeDebug = millis();
  }
  
  switch (estadoActualCorreccion) {
    case RECTO:
      if (!enRangoCorrecto) {
        iniciarCorreccion(anguloObjetivoParam);
      }
      break;
      
    case CORRIGIENDO_IZQUIERDA:
      direccion.write(SERVO_LEFT); 
      if (enRangoCorrecto) {
          finalizarCorreccion();
      } else if (millis() - tiempoInicioCorreccion > TIEMPO_MAX_CORRECCION) {
          // Timeout de corrección - detener y reintentar
          SerialBT.println("Timeout de corrección - reiniciando");
          finalizarCorreccion();
      } 
      break;

    case CORRIGIENDO_DERECHA:
      direccion.write(SERVO_RIGHT); 
      if (enRangoCorrecto) {
          finalizarCorreccion();
      } else if (millis() - tiempoInicioCorreccion > TIEMPO_MAX_CORRECCION) {
          // Timeout de corrección - detener y reintentar
          SerialBT.println("Timeout de corrección - reiniciando");
          finalizarCorreccion();
      }
      break;
      
    case ESTABILIZANDO:
      if (millis() - tiempoInicioCorreccion > TIEMPO_ESTABILIZACION) {
        estadoActualCorreccion = RECTO;
        correccionCompletada = true;
        SerialBT.println("Estabilización completa - modo recto");
      }
      break;
  }
}

void iniciarCorreccion(float anguloObjetivoParam) {
  SerialBT.print("Iniciando corrección - Ángulo actual: ");
  SerialBT.print(anguloZ);
  SerialBT.print("° - Objetivo: ");
  SerialBT.print(anguloObjetivoParam);
  SerialBT.println("°");
  
  // LÓGICA PULL-UP: HIGH = detenido, LOW = activado
  // Detener motor principal y activar retroceso
  detenerMotor();  // DETENER motor principal (pull-up)
  retrocederMotor(); // ACTIVAR retroceso si es necesario
  
  // Determinar dirección de corrección basada en la diferencia angular
  float diferenciaAngular = calcularDiferenciaAngular(anguloZ, anguloObjetivoParam);
  
  if (diferenciaAngular > 0) {
    // El ángulo actual está a la derecha del objetivo -> corregir hacia la izquierda
    direccion.write(SERVO_LEFT);
    estadoActualCorreccion = CORRIGIENDO_IZQUIERDA;
    SerialBT.print("Corrigiendo hacia la IZQUIERDA (diferencia: +");
    SerialBT.print(diferenciaAngular);
    SerialBT.println("°)");
    
  } else {
    // El ángulo actual está a la izquierda del objetivo -> corregir hacia la derecha
    direccion.write(SERVO_RIGHT);
    estadoActualCorreccion = CORRIGIENDO_DERECHA;
    SerialBT.print("Corrigiendo hacia la DERECHA (diferencia: ");
    SerialBT.print(diferenciaAngular);
    SerialBT.println("°)");
  }
  
  tiempoInicioCorreccion = millis();
}

void finalizarCorreccion() {
 /* SerialBT.print("Corrección completada - Ángulo final: ");
  SerialBT.print(anguloZ);
  SerialBT.println("°");
  */
  // Detener retroceso
  detenerMotor(); // DETENER retroceso (pull-up)
  
  // Enderezar dirección
  direccion.write(SERVO_CENTER);
  
  // Período de estabilización
  estadoActualCorreccion = ESTABILIZANDO;
  tiempoInicioCorreccion = millis();
  
  // Pausa para estabilización
  delay(100);
  
  // Solo reactivar motor si la distancia objetivo ya está alcanzada
  float distanciaTrasera = leerToF('T');
  bool distanciaAlcanzada = (distanciaTrasera <= DISTANCIA_MINIMA_RETROCESO && distanciaTrasera > 0);
  
  if (distanciaAlcanzada) {
    // Si ya estamos en la distancia objetivo, podemos reactivar el motor de avance
    avanzarMotor();// ACTIVAR motor principal (pull-up)
    SerialBT.println("Motor de avance reactivado - distancia objetivo ya alcanzada");
  } else {
    // Si no hemos alcanzado la distancia, mantener el motor de avance detenido
    // El retroceso orientado se encargará de activarlo cuando sea necesario
    detenerMotor(); // MANTENER motor principal detenido (pull-up)
    SerialBT.println("Motor de avance mantenido detenido - falta alcanzar distancia objetivo");
  }
}
// ===== FUNCIONES DE NAVEGACIÓN Y LÍNEAS =====
void calcularAnguloObjetivo() {
  if (sentidoVehiculo == IZQUIERDA) {
    // Giro a la izquierda: 1->0°, 2->90°, 3->180°, 4->270°, 5->0°, 6->90°...
    int ciclo = (contadorLineas - 1) % 4;  // Ajustar para que comience en 1
    switch (ciclo) {
      case 0: anguloObjetivo = 0.0; break;   // contadorLineas = 1, 5, 9...
      case 1: anguloObjetivo = 90.0; break;  // contadorLineas = 2, 6, 10...
      case 2: anguloObjetivo = 180.0; break; // contadorLineas = 3, 7, 11...
      case 3: anguloObjetivo = 270.0; break; // contadorLineas = 4, 8, 12...
    }
  } else {
    // Giro a la derecha: 1->0°, 2->270°, 3->180°, 4->90°, 5->0°, 6->270°...
    int ciclo = (contadorLineas - 1) % 4;  // Ajustar para que comience en 1
    switch (ciclo) {
      case 0: anguloObjetivo = 0.0; break;   // contadorLineas = 1, 5, 9...
      case 1: anguloObjetivo = 270.0; break; // contadorLineas = 2, 6, 10...
      case 2: anguloObjetivo = 180.0; break; // contadorLineas = 3, 7, 11...
      case 3: anguloObjetivo = 90.0; break;  // contadorLineas = 4, 8, 12...
    }
  }
}

void mostrarEstadoActual() {
  SerialBT.println("=== ESTADO ACTUAL ===");
  SerialBT.print("Contador de líneas: ");
  SerialBT.println(contadorLineas);
  SerialBT.print("Sentido de giro: ");
  SerialBT.println(sentidoVehiculo == IZQUIERDA ? "IZQUIERDA" : "DERECHA");
  SerialBT.print("Ángulo objetivo: ");
  SerialBT.print(anguloObjetivo);
  SerialBT.println("°");
  SerialBT.print("Ángulo actual: ");
  SerialBT.print(anguloZ);
  SerialBT.println("°");
  SerialBT.print("Tolerancia: ±");
  SerialBT.print(TOLERANCIA_ANGULO);
  SerialBT.println("°");
  SerialBT.println("====================");
}

bool estaEnRangoCorrecto(float anguloActual, float objetivo) {
  float diferencia = abs(calcularDiferenciaAngular(anguloActual, objetivo));
  return diferencia <= TOLERANCIA_ANGULO;
}
bool estaEnRangoCorrectoCorrecionDeLineaAntesDelGiro(float anguloActual, float objetivo) {
  float diferencia = abs(calcularDiferenciaAngular(anguloActual, objetivo));
  return diferencia <= TOLERANCIA_ANGULO_CORRECCION_LINEA_ANTES_DEL_GIRO;
}
bool mo_estaEnRangoEnderezado(float anguloActual, float objetivo) {
  float diferencia = abs(calcularDiferenciaAngular(anguloActual, objetivo));
  return diferencia <= MO_TOLERANCIA_ANGULO_ENDEREZADO;
}
// Igual que controlarDireccion() pero usando el rango ESTRICTO del enderezado (MO_TOLERANCIA_ANGULO_ENDEREZADO)
// y con mensajes de depuración. Solo se usa en MO_ENDEREZANDO.
void mo_controlarDireccionEnderezado(float anguloObjetivoParam) {
  bool enRangoEstricto = mo_estaEnRangoEnderezado(anguloZ, anguloObjetivoParam);

  static unsigned long ultimoDebugEnderezado = 0;
  if (millis() - ultimoDebugEnderezado > 250) {
    float dif = calcularDiferenciaAngular(anguloZ, anguloObjetivoParam);
    String nombreEstado;
    switch (estadoActualCorreccion) {
      case RECTO: nombreEstado = "RECTO"; break;
      case CORRIGIENDO_IZQUIERDA: nombreEstado = "CORRIGIENDO_IZQ"; break;
      case CORRIGIENDO_DERECHA: nombreEstado = "CORRIGIENDO_DER"; break;
      case ESTABILIZANDO: nombreEstado = "ESTABILIZANDO"; break;
    }
    SerialBT.println("🧭 ENDEREZADO MO | yaw=" + String(anguloZ, 1) +
                     " obj=" + String(anguloObjetivoParam, 1) +
                     " dif=" + String(dif, 1) +
                     " tol=" + String(MO_TOLERANCIA_ANGULO_ENDEREZADO, 1) +
                     " enRango=" + String(enRangoEstricto) +
                     " estado=" + nombreEstado);
    ultimoDebugEnderezado = millis();
  }

  switch (estadoActualCorreccion) {
    case RECTO:
      if (!enRangoEstricto) {
        iniciarCorreccion(anguloObjetivoParam);
      }
      break;

       case CORRIGIENDO_IZQUIERDA: {
      direccion.write(SERVO_LEFT);
      float difI = calcularDiferenciaAngular(anguloZ, anguloObjetivoParam);
      if (enRangoEstricto) {
        finalizarCorreccion();
      } else if (difI < 0) {
        // v104: empezó con yaw > objetivo; ya se pasó al otro lado -> parar y que corrija hacia el lado contrario
        SerialBT.println("↩️ ENDEREZADO MO: se pasó del objetivo (dif=" + String(difI, 1) + "), invirtiendo corrección");
        finalizarCorreccion();
      } else if (millis() - tiempoInicioCorreccion > TIEMPO_MAX_CORRECCION) {
        SerialBT.println("⏱️ ENDEREZADO MO: timeout interno corrigiendo IZQ, reiniciando");
        finalizarCorreccion();
      }
      break;
    }

    case CORRIGIENDO_DERECHA: {
      direccion.write(SERVO_RIGHT);
      float difD = calcularDiferenciaAngular(anguloZ, anguloObjetivoParam);
      if (enRangoEstricto) {
        finalizarCorreccion();
      } else if (difD > 0) {
        // v104: empezó con yaw < objetivo; ya se pasó al otro lado -> parar y que corrija hacia el lado contrario
        SerialBT.println("↩️ ENDEREZADO MO: se pasó del objetivo (dif=" + String(difD, 1) + "), invirtiendo corrección");
        finalizarCorreccion();
      } else if (millis() - tiempoInicioCorreccion > TIEMPO_MAX_CORRECCION) {
        SerialBT.println("⏱️ ENDEREZADO MO: timeout interno corrigiendo DER, reiniciando");
        finalizarCorreccion();
      }
      break;
    }

    case ESTABILIZANDO:
      if (millis() - tiempoInicioCorreccion > TIEMPO_ESTABILIZACION) {
        estadoActualCorreccion = RECTO;
        correccionCompletada = true;
        SerialBT.println("🧭 ENDEREZADO MO: estabilización completa, reevaluando rango");
      }
      break;
  }
}
float calcularDiferenciaAngular(float actual, float objetivo) {
  // Calcula la diferencia angular considerando el cruce de 0°/360°
  float diferencia = actual - objetivo;
  
  // Normalizar la diferencia al rango [-180, 180]
  while (diferencia > 180.0) diferencia -= 360.0;
  while (diferencia < -180.0) diferencia += 360.0;
  
  return diferencia;
}
//funcion de detectar color reducida solo para reconfirmar que detecto una linea sin activar nada.
bool chequearLinea() {
  // No chequear si estamos en maniobras de alta prioridad como el retroceso final.
  if (retrocesoOrientadoActivo) {
    return false;
  }

  uint16_t r, g, b, c;
  double hue, saturation, value;

  tcs.getRawData(&r, &g, &b, &c);
  if (c > 0) {
    // Se realiza la misma conversión de color que en la función principal
    float r_norm = (float)r / c; 
    float g_norm = (float)g / c; 
    float b_norm = (float)b / c; 

    uint8_t r_byte = (uint8_t)(r_norm * 255.0); 
    uint8_t g_byte = (uint8_t)(g_norm * 255.0); 
    uint8_t b_byte = (uint8_t)(b_norm * 255.0); 

    ColorConverter::RgbToHsv(r_byte, g_byte, b_byte, hue, saturation, value); 

    bool esBlanco = (value > 0.85 && saturation < 0.15); 
    float proporcionAzul = b_norm / (r_norm + g_norm + 0.001); 
    // Detección azul (usando las mismas condiciones que en detectarColor)
    if (hue >= HUE_AZUL_MIN && hue <= HUE_AZUL_MAX &&
      saturation > SATURACION_AZUL_MIN &&
      value > VALOR_AZUL_MIN && value < VALOR_AZUL_MAX &&
      proporcionAzul > PROPORCION_AZUL_MIN &&
      !esBlanco) {
      if (confirmarColor(HUE_AZUL_MIN, HUE_AZUL_MAX, SATURACION_AZUL_MIN, VALOR_AZUL_MIN, VALOR_AZUL_MAX, CONFIRMAR_AZUL_TOLERANCIA, CONFIRMAR_AZUL_DELAY)) {
   
        return true; // ¡Línea azul encontrada!
      }
    }
             // Detección naranja/rojo/amarillo
       else {
        bool enRangoNaranja = (hue >= 340 && hue <= 360) || (hue >= 0 && hue <= 48);
        
        if (enRangoNaranja &&
            saturation > SATURACION_NARANJA_MIN &&
            value > VALOR_NARANJA_MIN && value < VALOR_NARANJA_MAX &&
            !esBlanco) {
        if (confirmarColor(HUE_NARANJA_MIN, HUE_NARANJA_MAX, SATURACION_NARANJA_MIN, VALOR_NARANJA_MIN, VALOR_NARANJA_MAX, CONFIRMAR_NARANJA_TOLERANCIA, CONFIRMAR_NARANJA_DELAY)) {
        return true; // ¡Línea naranja encontrada!
      }
      }
    }
  }
  // Si llegamos hasta aquí, no se detectó nada.
  return false;
}

// ===== FUNCIÓN PRINCIPAL DE INTEGRACIÓN =====
void realizarGiro(bool giroIzquierda) {
  // RESETEAR VARIABLES DE ESTADO ANTES DE INICIAR
  correccionCompletada = false;
  estadoActualCorreccion = RECTO;
  
  // RESETEAR variables de compensación
  tiempoTotalRetroceso = 0;
  huboRetrocesoEnCorreccion = false;

  // ===================== =====================
  // PASO 1: Calcular el ángulo de giro dinámico y el tiempo de retroceso necesario AL PRINCIPIO.
  // Esto nos permite saber de antemano si se requerirá un retroceso por giro cerrado.
  float anguloObjetivoGiro = calcularAnguloGiroDinamico(giroIzquierda);
  anguloObjetivoGiroGlobal = anguloObjetivoGiro;
  unsigned long tiempoRetrocesoAdicional = 0;
  bool retrocesoGestionado = false; // Flag para controlar que solo se ejecute una lógica de retroceso.

  if ((giroIzquierda && anguloObjetivoGiro == GIRO_MAXIMO) ||
    (!giroIzquierda && anguloObjetivoGiro == GIRO_MAXIMO_DERECHA)) {
    tiempoRetrocesoAdicional = TIEMPO_RETROCESO_MAXIMO;
  }
  /*if (anguloObjetivoGiro == GIRO_MAXIMO) {
    tiempoRetrocesoAdicional = TIEMPO_RETROCESO_MAXIMO;
  }*/ /*else if (anguloObjetivoGiro == GIRO_ALTO) {
    tiempoRetrocesoAdicional = TIEMPO_RETROCESO_ALTO;
  } else if (anguloObjetivoGiro == GIRO_MEDIO_ALTO) {
    tiempoRetrocesoAdicional = TIEMPO_RETROCESO_MEDIO_ALTO;
  }*/
  // ==========================================

  // PASO 2: Actualizar ángulo objetivo de navegación basado en contadorLineas actual
  calcularAnguloObjetivo();

  // PASO 3: Verificar si estamos en el rango correcto
  bool enRangoCorrecto = estaEnRangoCorrectoCorrecionDeLineaAntesDelGiro(anguloZ, anguloObjetivo);

  if (!enRangoCorrecto) {
    SerialBT.println("Ángulo fuera de rango - iniciando corrección antes del giro");
    SerialBT.print(" > Actual: "); SerialBT.print(anguloZ);
    SerialBT.print(" | Objetivo: "); SerialBT.println(anguloObjetivo);
    
    tiempoInicioCorreccion = millis();
    unsigned long tiempoInicioRetrocesoLocal = 0;
    bool retrocediendoAhora = false;
    
      // Bucle de corrección hasta que el ángulo esté en rango
      while (!estaEnRangoCorrectoCorrecionDeLineaAntesDelGiro(anguloZ, anguloObjetivo)) {
        actualizarGiro();
        actualizarRampaMotor();
        
        // NUENA FORMA: Comprueba si IN1 está en HIGH (tu estado de retroceso)
        bool motorRetrocesoActivo = (digitalRead(MOTOR_IN1) == HIGH); 
        
        if (motorRetrocesoActivo && !retrocediendoAhora) {
          tiempoInicioRetrocesoLocal = millis();
          retrocediendoAhora = true;
          huboRetrocesoEnCorreccion = true;
        } else if (!motorRetrocesoActivo && retrocediendoAhora) {
          tiempoTotalRetroceso += (millis() - tiempoInicioRetrocesoLocal);
          retrocediendoAhora = false;
        }
      
      controlarDireccion(anguloObjetivo);
      delay(10);
      
      if (millis() - tiempoInicioCorreccion > TIEMPO_MAX_CORRECCION) {
        SerialBT.println("Timeout de corrección - procediendo con giro");
        if (retrocediendoAhora) {
          tiempoTotalRetroceso += (millis() - tiempoInicioRetrocesoLocal);
        }
        break;
      }
    }
    
    if (retrocediendoAhora) {
      tiempoTotalRetroceso += (millis() - tiempoInicioRetrocesoLocal);
    }
    
    finalizarCorreccion();
    delay(100);


     if (huboRetrocesoEnCorreccion && tiempoTotalRetroceso > 0) {
      // CASO B: NO se necesita retroceso pre-giro, PERO sí hubo retroceso por corrección.
      // Solo en este caso, hacemos el avance compensatorio.
      unsigned long tiempoAvanceCompensatorio = tiempoTotalRetroceso/2;
      SerialBT.print("Iniciando avance compensatorio por hasta ");
      SerialBT.print(tiempoAvanceCompensatorio);
      SerialBT.println(" ms, buscando línea activamente...");
      
      int servoDireccion = giroIzquierda ? SERVO_OrientadoObjeto_RIGH : SERVO_OrientadoObjeto_LEFT;
      
      detenerMotor();
      avanzarMotor();
      direccion.write(servoDireccion);
      
      unsigned long tiempoInicioAvance = millis();
      bool lineaReDetectada = false;
      while ((millis() - tiempoInicioAvance) < tiempoAvanceCompensatorio) {
        actualizarGiro();
        actualizarRampaMotor();
        if (chequearLinea()) {
          lineaReDetectada = true;
          break;
        }
        delay(5);
      }
      
      detenerMotor();
      direccion.write(SERVO_CENTER); 

      if (lineaReDetectada) {
        SerialBT.println("Avance compensatorio interrumpido por re-detección de línea.");
      } else {
        SerialBT.println("Avance compensatorio completado por tiempo.");
      }
      delay(50);
    }
    // ===================== FIN DE LA LÓGICA DE DECISIÓN =====================

  } else {
    SerialBT.println("PRE-GIRO: Ángulo ya en rango. Omitiendo corrección.");
    SerialBT.print(" > Actual: "); SerialBT.print(anguloZ);
    SerialBT.print(" | Objetivo: "); SerialBT.println(anguloObjetivo);
  }
 

  // PASO 4: Ejecutar la lógica de giro.

  // Si es un giro de apertura (suave), la lógica no cambia.
  // EXCEPCIÓN: si la duración es 0 (muy pegado a la pared), saltar directo al giro PID.
  if (esGiroApertura && duracionAperturaActual > 0) {
    // La duración ya se calculó en calcularAnguloGiroDinamico() según
    // la distancia real medida a la pared. Aquí solo se ejecuta.
    if (!giroAperturaCompletado && !giroAperturaActivo) {
       realizarGiroApertura(giroIzquierda);
    }
    return;
  }

   // ===================== LÓGICA DE RETROCESO MODIFICADA =====================
  // Este bloque ahora solo se ejecuta si el ángulo ESTABA BIEN desde el principio
  // y, por lo tanto, la lógica de unificación no se activó.
  if (HABILITAR_RETROCESO_GIRO_CERRADO && tiempoRetrocesoAdicional > 0 && !retrocesoGestionado) {
    SerialBT.print("GIRO CERRADO (sin corrección previa): Retrocediendo recto por ");
    SerialBT.print(tiempoRetrocesoAdicional);
    SerialBT.println(" ms.");

    detenerMotor();
    direccion.write(SERVO_CENTER);
    retrocederMotor();

    // IMPORTANTE: actualizarRampaMotor() es quien realmente escribe el PWM hacia el
    // motor (retrocederMotor() solo fija el objetivo) y debe llamarse en cada vuelta
    // del loop. Un delay() simple aquí bloqueaba el loop entero y el motor nunca
    // llegaba a moverse. Replicamos el mismo patrón que ya usa retrocesoCompartidoPorObstaculo().
    unsigned long t0RetrocesoGiroCerrado = millis();
    while (millis() - t0RetrocesoGiroCerrado < tiempoRetrocesoAdicional) {
      actualizarGiro();
      actualizarRampaMotor();
      delay(10);
    }

    detenerMotor();
    delay(100);
  }
  // ===================== FIN =====================

  // Finalmente, ejecutar el giro PID normal.
  ejecutarGiroPID(anguloObjetivoGiro, giroIzquierda);
}
float calcularAnguloRetroceso() {
  float anguloRetroceso;
  
  if (sentidoVehiculo == IZQUIERDA) {
    // Giro a la izquierda: 1->90°, 2->180°, 3->270°, 4->0°, 5->90°...
    int ciclo = (contadorLineas - 1) % 4;
    switch (ciclo) {
      case 0: anguloRetroceso = 90.0; break;   // contadorLineas = 1, 5, 9...
      case 1: anguloRetroceso = 180.0; break;  // contadorLineas = 2, 6, 10...
      case 2: anguloRetroceso = 270.0; break;  // contadorLineas = 3, 7, 11...
      case 3: anguloRetroceso = 0.0; break;    // contadorLineas = 4, 8, 12...
    }
  } else {
    // Giro a la derecha: 1->270°, 2->180°, 3->90°, 4->0°, 5->270°...
    int ciclo = (contadorLineas - 1) % 4;
    switch (ciclo) {
      case 0: anguloRetroceso = 270.0; break;  // contadorLineas = 1, 5, 9...
      case 1: anguloRetroceso = 180.0; break;  // contadorLineas = 2, 6, 10...
      case 2: anguloRetroceso = 90.0; break;   // contadorLineas = 3, 7, 11...
      case 3: anguloRetroceso = 0.0; break;    // contadorLineas = 4, 8, 12...
    }
  }
  
  return anguloRetroceso;
}


// ============== FUNCIÓN DE RETROCESO POST-GIRO ORIENTADO  ==============
void realizarAvanceDeReintento() {
  SerialBT.println(">> Ejecutando avance de reintento...");
  
  // 1. Detener el retroceso y centrar la dirección.
  detenerMotor();
  direccion.write(SERVO_CENTER);
  delay(200); // Pausa para estabilizar.

  // 2. Avanzar por un tiempo definido, actualizando el giro constantemente.
  avanzarLento();
  unsigned long tiempoInicioAvance = millis();
  while (millis() - tiempoInicioAvance < TIEMPO_AVANCE_REINTENTO) {
    actualizarGiro(); // Actualiza el ángulo mientras avanza.
    actualizarRampaMotor();
    direccion.write(SERVO_CENTER);
    delay(10);        // Peqfueña pausa para no saturar el procesador.
  }

  // 3. Detener el avance.
  detenerMotor();
  delay(200); // Pausa antes de volver a intentar.
}
bool procesarRetrocesoPostGiro() {
  // Flags estáticos para controlar el flujo dentro de la función.
  static bool anguloCorregido = false;
  static unsigned long ultimoTiempoInicio = 0;
  static bool casoEspecialEvaluado = false;
  static bool esCasoEspecialAperturaIzquierda = false;
  static bool avanceIntermedioHecho = false;

  // Si el tiempo de inicio ha cambiado, es un nuevo giro. Reseteamos los flags.
  if (tiempoInicioRetrocesoOrientado != ultimoTiempoInicio) {
    anguloCorregido = false;
    casoEspecialEvaluado = false;
    esCasoEspecialAperturaIzquierda = false;
    avanceIntermedioHecho = false;
    ultimoTiempoInicio = tiempoInicioRetrocesoOrientado;
  }

  // Si el retroceso no está activo, no hacer nada.
  if (!retrocesoOrientadoActivo) {
    return false;
  }

  // Determinar una sola vez, al inicio del ciclo, si estamos en el caso especial:
  // sentido IZQUIERDA, carril objetivo 2, y el giro que originó este retroceso fue de apertura.
  /*if (!casoEspecialEvaluado) {
    esCasoEspecialAperturaIzquierda = (sentidoVehiculo == IZQUIERDA &&
                                        carriles.carrilObjetivo == 2 &&
                                        ultimoGiroFueApertura);*/
    if (!casoEspecialEvaluado) {
      esCasoEspecialAperturaIzquierda = (sentidoVehiculo == IZQUIERDA &&
                                        ultimoGiroFueApertura);
      casoEspecialEvaluado = true;
    if (esCasoEspecialAperturaIzquierda) {
      SerialBT.println("🔶 CASO ESPECIAL: Retroceso post-giro de apertura (IZQUIERDA, carril 2). "
                        "Se insertará avance de 600ms a la izquierda tras 1s de retroceso.");
    }
  }

  actualizarGiro();
  actualizarRampaMotor();
  unsigned long tiempoTranscurrido = millis() - tiempoInicioRetrocesoOrientado;

  // ==================== FASE 1: CORRECCIÓN DE ÁNGULO (SE EJECUTA UNA SOLA VEZ) ====================
  if (!anguloCorregido) {
    float anguloObjetivoRetroceso = calcularAnguloRetroceso();
    bool enRangoCorrecto = estaEnRangoCorrecto(anguloZ, anguloObjetivoRetroceso); 

    if (!enRangoCorrecto) {
      SerialBT.println("POST-GIRO: Ángulo fuera de rango. Iniciando corrección...");
      unsigned long tiempoInicioCorreccionLocal = millis();

      // Bucle para corregir el ángulo hasta que esté en rango o haya un timeout.
      // CASO ESPECIAL: si es apertura+izquierda+carril2, el bucle se limita a 1 segundo
      // de retroceso normal, tras lo cual se inserta el avance intermedio.
      while (!estaEnRangoCorrecto(anguloZ, anguloObjetivoRetroceso)) {
        actualizarGiro();
        actualizarRampaMotor();

        // --- Inserción del avance intermedio para el caso especial ---
        if (HABILITAR_AVANCE_ESPECIAL_IZQ && esCasoEspecialAperturaIzquierda && !avanceIntermedioHecho &&
          (millis() - tiempoInicioCorreccionLocal) >= 1000) {
          SerialBT.println("↩️ CASO ESPECIAL: 1s de retroceso cumplido. Avance de 600ms girando a la izquierda...");
          detenerMotor();
          direccion.write(SERVO_LEFT); // Girar hacia la izquierda
          delay(50);
          avanzarLento();
          unsigned long tiempoInicioAvanceEsp = millis();
          while (millis() - tiempoInicioAvanceEsp < 600) {
            actualizarGiro();
            actualizarRampaMotor();
            delay(20);
          }
          detenerMotor();
          delay(100);
          avanceIntermedioHecho = true;
          // Reiniciamos el temporizador de corrección para continuar el retroceso
          // normal hacia el ángulo objetivo desde aquí.
          tiempoInicioCorreccionLocal = millis();
          continue;
        }

        float diferencia = calcularDiferenciaAngular(anguloZ, anguloObjetivoRetroceso);
        // Gira en la dirección opuesta al error para corregir.
        if (diferencia > 0) {
          direccion.write(SERVO_LEFT);
        } else {
          direccion.write(SERVO_RIGHT);
        }
        
        retrocederMotor(); 
        //detenerMotor();
        actualizarRampaMotor();
        delay(20);

        // ===== VERIFICACIÓN DE PROXIMIDAD TRASERA DURANTE LA CORRECCIÓN =====
        float distanciaTrasera = leerToF('T');
        if (distanciaTrasera < 6.0 && distanciaTrasera > 0) {
            SerialBT.println("! Muy cerca por detrás (" + String(distanciaTrasera, 1) + "cm).");
            realizarAvanceDeReintento(); // Ejecuta la maniobra de avance.
            tiempoInicioCorreccionLocal = millis(); // Reinicia el temporizador de timeout.
            SerialBT.println("Reintentando corrección de ángulo...");
            continue; // Vuelve al inicio del bucle para re-evaluar el ángulo.
        }
        // =========================================================================

        // Timeout de seguridad para la corrección.
        if (millis() - tiempoInicioCorreccionLocal > TIEMPO_MAX_CORRECCION) {
          SerialBT.println("Timeout de corrección.");
          // ===== MODIFICADO: Llamamos a la nueva función de avance =====
          realizarAvanceDeReintento(); 
          // ============================================================
          
          // Reiniciar el temporizador del timeout para el nuevo intento de corrección.
          tiempoInicioCorreccionLocal = millis(); 
          SerialBT.println("Reintentando corrección de ángulo...");
        }
      }
    }
    
    SerialBT.println("POST-GIRO: Corrección de ángulo finalizada.");
    detenerMotor();
    direccion.write(SERVO_CENTER); 
    delay(250); // Pausa para estabilizar.
    anguloCorregido = true; // Marcamos que la corrección ya se hizo.
  }
  // ==================== FASE 2: CONTROL DE DISTANCIA (RETROCESO O AVANCE) ====================
  
  // Timeout de seguridad general para toda la maniobra.
  if (tiempoTranscurrido >= TIMEOUT_RETROCESO_MAXIMO) {
    SerialBT.println("Timeout de seguridad alcanzado en retroceso post-giro.");
    finalizarRetrocesoPostGiro();
    return false;
  }

  // Medimos la distancia trasera.
  float distanciaTrasera = leerToF('T');
  // Si la lectura del sensor falla, esperamos un poco y continuamos.
  if (distanciaTrasera > 200) {
      delay(50);
      return true; // Maniobra sigue activa.
  }

  float errorDistancia = distanciaTrasera - DISTANCIA_MINIMA_RETROCESO;
  const float toleranciaDistancia = 1.5;

  // CASO 1: La distancia es correcta (dentro de la tolerancia).
  if (abs(errorDistancia) <= toleranciaDistancia) {
    SerialBT.println("Distancia objetivo (" + String(DISTANCIA_MINIMA_RETROCESO) + " cm) alcanzada. Finalizando.");
    finalizarRetrocesoPostGiro();
    return false; // Maniobra terminada.
  }
  // CASO 2: Estamos muy lejos de la pared, hay que seguir retrocediendo.
  else if (errorDistancia > toleranciaDistancia) {
    direccion.write(SERVO_CENTER); // Servo recto.
    retrocederMotor(); // Activar retroceso.
    actualizarRampaMotor();
    //detenerMotor();
    return true; // Maniobra sigue activa.
  }
  // CASO 3: Nos pasamos (estamos muy cerca), hay que avanzar para corregir.
  else { // (errorDistancia < -toleranciaDistancia)
    SerialBT.println("Nos pasamos de largo. Avanzando para corregir...");
    detenerMotor();
    direccion.write(SERVO_CENTER);
    delay(100);

    avanzarLento();// Activar avance.
    unsigned long tiempoInicioAvance = millis();
    while (leerToF('T') < DISTANCIA_MINIMA_RETROCESO) {
      if (millis() - tiempoInicioAvance > 300) {
        actualizarGiro(); 
        actualizarRampaMotor();
        SerialBT.println("Timeout en avance de corrección.");
        break;
      }
      delay(20);
    }
    
    SerialBT.println("Corrección por avance finalizada.");
    finalizarRetrocesoPostGiro();
    return false; // Maniobra terminada.
  }
}
// ============== FUNCIÓN PARA FINALIZAR RETROCESO ==============
void finalizarRetrocesoPostGiro() {
  // Paso 1: Detener motores inmediatamente
  
  detenerMotor();
  
  // Paso 2: Centrar dirección
  direccion.write(SERVO_CENTER);
  //delay(5000);
  
  // Paso 3: Pausa para estabilizar sistema
  delay(100);
  
  // Paso 4: Resetear flags de estado completamente
  retrocesoOrientadoActivo = false;
  enGiro = false;
  giroEnProceso = false;
  cruceEnProceso = false;
  
  // Paso 5: Resetear variables de tiempo
  tiempoInicioRetrocesoOrientado = 0;

  // Paso 6: Ya NO se salta a ESTADO_DETENCION_FINAL aquí.
  // Tanto el giro normal como el giro de la línea 12 (estacionamiento)
  // pasan siempre por la CORRECCIÓN LATERAL antes de continuar.
  if (giroParaEstacionamiento) {
    SerialBT.println("🅿️ Giro de línea 12 (estacionamiento) completado. Yendo a CORRECCIÓN LATERAL antes de ESPERANDO_COMANDO_CARRIL.");
    giroParaEstacionamiento = false;
  }

  estadoActual = ESTADO_CORRECCION_LATERAL;
  aplicarUmbralesSegunComando(); // Decide UMBRAL_MIN/MAX (originales o alternos) según el tramo/comando
  controlLateral_PostGiro.funcionActiva = true; // Activamos la nueva lógica
  controlLateral_PostGiro.estadoActual = ControlDistanciaLateral::IDLE;

  SerialBT.println("🚀 Transición a ESTADO_CORRECCION_LATERAL post-giro.");
  controlLateral_PostGiro.tiempoInicioEstado = millis(); // Reset timer para la pausa


  // Resetear el PID de centrado
  resetearPIDCentrado(&pidCentrado);

  // El motor se activará cuando comience un nuevo estado de carril
  detenerMotor(); // Apagado por defecto en espera
  
  // Resetear variables de apertura
  giroAperturaActivo = false;
  giroAperturaCompletado = false;
  esGiroApertura = false;
}

void manejarInterrupcionCruce() {
  // Detener el giro inmediatamente
  direccion.write(SERVO_CENTRADO);
  avanzarMotor();// Mantener motor activo
  
  // Resetear flags de giro
  enGiro = false;
  giroEnProceso = false;
  cruceEnProceso = false;
  tiempoFinGiro = millis();
  
  // Procesar el comando que causó la interrupción
  colorDetectado = comandoInterrupcion;

  // Resetear flags de interrupción
  interrupcionCruce = false;
  comandoInterrupcion = 'N';
}

// ========== ESTRATEGIA 1: CORRECCIÓN PERIÓDICA SIMPLE ==========
void actualizarGiro() {
  // ===== WATCHDOG v97: mide el tiempo entre una actualización del giro y la siguiente =====
  static unsigned long tPrevGiro = 0;
  static unsigned long maxGapGiro = 0;
  static unsigned int  nGapsLargos = 0;
  static unsigned long tResumenGiro = 0;
  unsigned long tIni = millis();
  if (tPrevGiro != 0) {
    unsigned long gap = tIni - tPrevGiro;
    if (gap > maxGapGiro) maxGapGiro = gap;
    if (gap > 100) nGapsLargos++;
    if (gap > 250) {   // bloqueo fuerte: dice dónde estaba el robot
      SerialBT.println("⚠️ GIRO SIN ACTUALIZAR " + String(gap) + "ms | estado=" + String((int)estadoActual) +
                       " | faseMO=" + String((int)faseManiobraOptimizada) +
                       " | objetoActivo=" + String(mo_confirmarObjetoActivo) +
                       " | yaw=" + String(anguloZ, 1));
    }
  }
  tPrevGiro = tIni;

  readIMU(); // Lee el sensor BNO055 y actualiza la variable anguloZ

  unsigned long durIMU = millis() - tIni;
  if (durIMU > 15) {
    SerialBT.println("⚠️ readIMU lento: " + String(durIMU) + "ms (bus I2C ocupado?)");
  }

  // Resumen cada 2 s: peor intervalo y cuántos pasaron de 100ms
  if (millis() - tResumenGiro > 2000) {
    tResumenGiro = millis();
    SerialBT.println("📊 periodo giro: max=" + String(maxGapGiro) + "ms | >100ms: " + String(nGapsLargos) +
                     " veces | estado=" + String((int)estadoActual));
    maxGapGiro = 0;
    nGapsLargos = 0;
  }
}
void applyCalibrationOffsets() {
  adafruit_bno055_offsets_t calOffsets;
  calOffsets.accel_offset_x = 12;
  calOffsets.accel_offset_y = -46;
  calOffsets.accel_offset_z = -13;
  calOffsets.accel_radius   = 1000;
  calOffsets.gyro_offset_x  = 0;
  calOffsets.gyro_offset_y  = 1;
  calOffsets.gyro_offset_z  = 0;
  calOffsets.mag_offset_x   = 156;
  calOffsets.mag_offset_y   = 563;
  calOffsets.mag_offset_z   = -287;
  calOffsets.mag_radius     = 651;
  bno.setSensorOffsets(calOffsets);
  // SerialBT.println(F("Offsets de calibracion de IMU aplicados."));
}
void readIMU() {
  imu::Quaternion quat = bno.getQuat();
  double w = quat.w(), x = quat.x(), y = quat.y(), z = quat.z();
  double norm = sqrt(w*w + x*x + y*y + z*z);
  if (norm < 1e-6) return;
  w /= norm; x /= norm; y /= norm; z /= norm;

  double yaw = atan2(2.0*(w*z + x*y), 1.0 - 2.0*(y*y + z*z));
  float yaw_deg_temp = (float)(yaw * 180.0 / M_PI);
  
  yaw_deg_temp -= yaw_offset;
  while (yaw_deg_temp < 0.0f) yaw_deg_temp += 360.0f;
  while (yaw_deg_temp >= 360.0f) yaw_deg_temp -= 360.0f;

  anguloZ = yaw_deg_temp;
}

void resetYaw() {
  imu::Quaternion quat = bno.getQuat();
  double w = quat.w(), x = quat.x(), y = quat.y(), z = quat.z();
  double norm = sqrt(w*w + x*x + y*y + z*z);
  if (norm < 1e-6) return;
  w /= norm; x /= norm; y /= norm; z /= norm;

  float rawYaw = (float)(atan2(2.0*(w*z + x*y), 1.0 - 2.0*(y*y + z*z)) * 180.0 / M_PI);
  yaw_offset = rawYaw; 
  anguloZ = 0.0f;
}


void mostrarInformacion() {
    if (false && millis() - lastYawPrint >= YAW_INTERVAL) {
    SerialBT.print("Yaw: ");
    SerialBT.print(anguloZ, 1);
    SerialBT.print(" | Objetivo: ");
    SerialBT.print(anguloObjetivo);
    SerialBT.print("° | Estado: ");
    
    switch (estadoActualCorreccion) {
      case RECTO: SerialBT.println("RECTO"); break;
      case CORRIGIENDO_IZQUIERDA: SerialBT.println("CORRIGIENDO IZQUIERDA"); break;
      case CORRIGIENDO_DERECHA: SerialBT.println("CORRIGIENDO DERECHA"); break;
      case ESTABILIZANDO: SerialBT.println("ESTABILIZANDO"); break;
    }
    
    lastYawPrint = millis();
  }
}
#ifdef TEST_LINEA_SIMULADA
volatile char simLineaPendiente = 0;          // 'A', 'N' o 0 (nada)
volatile unsigned long simLineaTiempo = 0;    // cuándo llegó el comando (para caducarlo)

// Se llama desde loop() (núcleo 1) SOLO en MANTENIENDO_CARRIL.
// Lee carácter por carácter: A o N (mayúscula o minúscula); ignora todo lo demás.
void leerComandoSimuladoLinea() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    c = toupper(c);
    if (c == 'A' || c == 'N') {
      simLineaTiempo = millis();   // primero el tiempo...
      simLineaPendiente = c;       // ...luego la bandera (evita carreras entre núcleos)
      SerialBT.println(String("🧪 [TEST] Línea simulada recibida: ") + c);
    }
  }
}
#endif

void detectarColor() {
  if (giroEnProceso) return;
  if (esUltimoTramoFinal || detencionPermanenteActiva) return; // v98: tramo final, ya no se cuentan líneas
  uint16_t r, g, b, c;
  double hue, saturation, value;
  
  // Ignorar TODA detección durante retroceso post-giro
  if (retrocesoOrientadoActivo && estadoActual != MANTENIENDO_CARRIL) {
    return; // Solo bloquear por retroceso si no estamos ya en MANTENIENDO_CARRIL
}
   // ── NUEVO GUARD ──────────────────────────────────────────────────────────
  // No contar líneas ni cambiar ángulo objetivo durante maniobras de carril.
  // Solo se permite detectar si estamos en ESTADO_NORMAL, MANTENIENDO_CARRIL,
  // o ESTADO_DETENCION_FINAL. En cualquier otro estado de maniobra se ignora.
  /*bool estadoPermiteDeteccion = (estadoActual == ESTADO_NORMAL     ||
                              estadoActual == MANTENIENDO_CARRIL ||
                              estadoActual == ESTADO_DETENCION_FINAL ||
                              (estadoActual == ENDEREZANDO_CARRIL  && ultimoCarrilActivo != 0));*/

    bool estadoPermiteDeteccion = (estadoActual == ESTADO_NORMAL     ||
                                estadoActual == MANTENIENDO_CARRIL ||
                                estadoActual == ESTADO_DETENCION_FINAL ||
                                (estadoActual == ENDEREZANDO_CARRIL && contadorLineas == 0));

   if (!estadoPermiteDeteccion) {
    static unsigned long ultimoPrintBloqueo = 0;
    if (millis() - ultimoPrintBloqueo > 500) {
      SerialBT.println("⛔ Bloqueado por estado: " + String(estadoActual));
      ultimoPrintBloqueo = millis();
    }
    return;
  }
  // ─────────────────────────────────────────────────────────────────────────

    // ===== TEST: línea simulada =====
    bool simAzul = false, simNaranja = false;
  #ifdef TEST_LINEA_SIMULADA
    if (simLineaPendiente != 0) {
      char simLinea = simLineaPendiente;
      simLineaPendiente = 0;                              // se consume una sola vez
      if (millis() - simLineaTiempo <= 500) {             // si caducó, se descarta
        simAzul    = (simLinea == 'A');
        simNaranja = (simLinea == 'N');
      }
    }
  #endif
    // ===== FIN TEST =====
  tcs.getRawData(&r, &g, &b, &c);

  if (c > 0) {
    float r_norm = (float)r / c;
    float g_norm = (float)g / c;
    float b_norm = (float)b / c;

    uint8_t r_byte = (uint8_t)(r_norm * 255.0);
    uint8_t g_byte = (uint8_t)(g_norm * 255.0);
    uint8_t b_byte = (uint8_t)(b_norm * 255.0);

    ColorConverter::RgbToHsv(r_byte, g_byte, b_byte, hue, saturation, value);
    hue *= 360;

        // ===== datos de color para el monitor (se actualiza SIEMPRE que el sensor lee) =====
    {
      bool mBlanco = (value > 0.85 && saturation < 0.15);
      float mPropAzul = b_norm / (r_norm + g_norm + 0.001);
      bool mAzul = (hue >= HUE_AZUL_MIN && hue <= HUE_AZUL_MAX &&
                    saturation > SATURACION_AZUL_MIN &&
                    value > VALOR_AZUL_MIN && value < VALOR_AZUL_MAX &&
                    mPropAzul > PROPORCION_AZUL_MIN && !mBlanco);
      bool mNaranja = (((hue >= 340 && hue <= 360) || (hue >= 0 && hue <= 48)) &&
                       saturation > SATURACION_NARANJA_MIN &&
                       value > VALOR_NARANJA_MIN && value < VALOR_NARANJA_MAX && !mBlanco);
      dbgColor = mBlanco ? 'B' : (mAzul ? 'A' : (mNaranja ? 'N' : 'P'));
      dbgHue = hue; dbgSat = saturation; dbgVal = value;
      dbgTLectura = millis();
    }

    // MODIFICACIÓN: Solo aplicar espera si ya se detectó al menos una línea
//    bool puedeDetectar = (contadorLineas == 0) || (millis() - tiempoUltimaDeteccion > tiempoEsperaDeteccion)|| estadoActual == MANTENIENDO_CARRIL;
    //bool puedeDetectar = (contadorLineas == 0) || (millis() - tiempoUltimaDeteccion > tiempoEsperaDeteccion);
   /*bool puedeDetectar = (contadorLineas == 0) 
                          || (millis() - tiempoUltimaDeteccion > tiempoEsperaDeteccion)
                          || (estadoActual == MANTENIENDO_CARRIL && (millis() - carriles.tiempoInicioEstado > TIEMPO_MINIMO_MANTENIENDO_PARA_DETECTAR));
*/
        bool puedeDetectar = (contadorLineas == 0) 
                          || (millis() - tiempoUltimaDeteccion > tiempoEsperaDeteccion)
                          || (estadoActual == MANTENIENDO_CARRIL && (millis() - carriles.tiempoInicioEstado > TIEMPO_MINIMO_MANTENIENDO_PARA_DETECTAR))
                          || simAzul || simNaranja;   // TEST
        dbgPuedeDetectar = puedeDetectar;
    if (puedeDetectar) {
      bool esBlanco = (value > 0.85 && saturation < 0.15);
      float proporcionAzul = b_norm / (r_norm + g_norm + 0.001);
       // === DIAGNÓSTICO TEMPORAL — BORRAR DESPUÉS DE CALIBRAR ===
      /*SerialBT.println("H:" + String(hue,1) + " S:" + String(saturation,2) + 
                        " V:" + String(value,2) + " esBlanco:" + String(esBlanco));
      */
      // ===========================================================
      bool azulDetectado = false;
      bool naranjaDetectado = false;
        
      // Detección azul
     // Detección azul
    /*if (hue >= HUE_AZUL_MIN && hue <= HUE_AZUL_MAX &&
        saturation > SATURACION_AZUL_MIN &&
        value > VALOR_AZUL_MIN && value < VALOR_AZUL_MAX &&
        proporcionAzul > PROPORCION_AZUL_MIN &&
        !esBlanco) {
        if (confirmarColor(HUE_AZUL_MIN, HUE_AZUL_MAX, SATURACION_AZUL_MIN, VALOR_AZUL_MIN, VALOR_AZUL_MAX, CONFIRMAR_AZUL_TOLERANCIA, CONFIRMAR_AZUL_DELAY)) {
            */
      if (!simNaranja && (simAzul || (hue >= HUE_AZUL_MIN && hue <= HUE_AZUL_MAX &&
        saturation > SATURACION_AZUL_MIN &&
        value > VALOR_AZUL_MIN && value < VALOR_AZUL_MAX &&
        proporcionAzul > PROPORCION_AZUL_MIN &&
        !esBlanco))) {
        if (simAzul || confirmarColor(HUE_AZUL_MIN, HUE_AZUL_MAX, SATURACION_AZUL_MIN, VALOR_AZUL_MIN, VALOR_AZUL_MAX, CONFIRMAR_AZUL_TOLERANCIA, CONFIRMAR_AZUL_DELAY)) { 
          azulDetectado = true;
          
          pausarCentrado = true;
          tiempoInicioDeteccion = millis();
          
          if (primeraDeteccion) {
            sentidoVehiculo = IZQUIERDA;
            primeraDeteccion = false;
          }
        }
      }
        // Detección naranja/rojo/amarillo
       else {
        bool enRangoNaranja = (hue >= 340 && hue <= 360) || (hue >= 0 && hue <= 48);
        
       /* if (enRangoNaranja &&
            saturation > SATURACION_NARANJA_MIN &&
            value > VALOR_NARANJA_MIN && value < VALOR_NARANJA_MAX &&
            !esBlanco) {
              SerialBT.println("Color detectado pasado a confirmar############### naranja");


            if (confirmarColor(HUE_NARANJA_MIN, HUE_NARANJA_MAX, SATURACION_NARANJA_MIN, VALOR_NARANJA_MIN, VALOR_NARANJA_MAX, CONFIRMAR_NARANJA_TOLERANCIA, CONFIRMAR_NARANJA_DELAY)) {
              */
                      if (simNaranja || (enRangoNaranja &&
            saturation > SATURACION_NARANJA_MIN &&
            value > VALOR_NARANJA_MIN && value < VALOR_NARANJA_MAX &&
            !esBlanco)) {
              SerialBT.println("Color detectado pasado a confirmar############### naranja");

            if (simNaranja || confirmarColor(HUE_NARANJA_MIN, HUE_NARANJA_MAX, SATURACION_NARANJA_MIN, VALOR_NARANJA_MIN, VALOR_NARANJA_MAX, CONFIRMAR_NARANJA_TOLERANCIA, CONFIRMAR_NARANJA_DELAY)) {
              naranjaDetectado = true;
              pausarCentrado = true;
              tiempoInicioDeteccion = millis();
              
              if (primeraDeteccion) {
                sentidoVehiculo = DERECHA;
                primeraDeteccion = false;
            }
          }
        }
        }

      // Al detectar línea, verificar si puede interrumpir esquive
      if (azulDetectado || naranjaDetectado) {
        
        // INTERRUMPIR ESQUIVE DIAGONAL si está activo
        if (esquiveDiagonalActivo) {
          resetearEsquiveDiagonal();
          resetearPIDCentrado(&pidCentrado);
          estadoActual = ESTADO_NORMAL;
        }
        
        // Ejecutar giro normal según el sentido del vehículo
        if (azulDetectado && sentidoVehiculo == IZQUIERDA) {
          //  LÓGICA DE INTERRUPCIÓN: Si estamos en una maniobra de carril, la reseteamos
          if (estadoActual == GIRANDO_HACIA_CARRIL || estadoActual == ENDEREZANDO_CARRIL || estadoActual == MANTENIENDO_CARRIL) {
            resetSistema(true); // Resetea la lógica de carriles conservando el ángulo
          }
          // Destello azul por 300ms (No bloqueante)
          encenderLedTemporal(0, 0, 255, 300);
          contadorLineas++;
          SerialBT.println(">>> LÍNEA DETECTADA (AZUL) — Contador: " + String(contadorLineas) + "/" + String(limiteLineas));

          // Al contar la primera línea, se desactiva el modo de inicio especial
          if (contadorLineas == 1 && !primerGiroRealizado) {
              primerGiroRealizado = true;
              SerialBT.println(">>> Primera línea detectada. El sistema operará de forma normal a partir de ahora.");
          }

          if (contadorLineas >= limiteLineas) {
            giroParaEstacionamiento = true;
          }
          resetearPIDCentrado(&pidCentrado);

          // En vez de girar aquí mismo (núcleo 0), armamos la bandera
          // y detenemos el motor. El loop() del núcleo 1 hará el giro real
          // tras la pausa, evitando choque de comandos entre núcleos.
          detenerMotor();

          // A partir de que ya memorizamos la vuelta completa y este es el 5to tramo
          // (o posterior), usamos la maniobra optimizada en vez del giro normal de esquina.
          if (cantidadComandosMemorizados == TOTAL_TRAMOS_PISTA && contadorLineas > TOTAL_TRAMOS_PISTA) {
            maniobraOptimizadaPendiente = true;
            maniobraOptimizadaEsIzquierda = true; // azul = izquierda
            tiempoInicioPausaPreManiobraOptimizada = millis();
          } else {
            giroPendiente = true;
            giroPendienteEsIzquierda = true; // azul = izquierda
            tiempoInicioPausaPreGiro = millis();
          }

          apagarVehiculo();
          tiempoUltimaDeteccion = millis(); // Actualizar tiempo después de procesar la detección
        }
        else if (naranjaDetectado && sentidoVehiculo == DERECHA) {
          //  LÓGICA DE INTERRUPCIÓN: Si estamos en una maniobra de carril, la reseteamos
          if (estadoActual == GIRANDO_HACIA_CARRIL || estadoActual == ENDEREZANDO_CARRIL || estadoActual == MANTENIENDO_CARRIL) {
            resetSistema(true); // Resetea la lógica de carriles conservando el ángulo
          }
          // Destello naranja por 300ms (No bloqueante)
          encenderLedTemporal(255, 128, 0, 300);
          contadorLineas++;
          SerialBT.println(">>> LÍNEA DETECTADA (NARANJA) — Contador: " + String(contadorLineas) + "/" + String(limiteLineas));

           // Al contar la primera línea, se desactiva el modo de inicio especial
          if (contadorLineas == 1 && !primerGiroRealizado) {
              primerGiroRealizado = true;
              SerialBT.println(">>> Primera línea detectada. El sistema operará de forma normal a partir de ahora.");
          }

           if (contadorLineas >= limiteLineas) {
              giroParaEstacionamiento = true;
            }
            resetearPIDCentrado(&pidCentrado);

            detenerMotor();

            // A partir de que ya memorizamos la vuelta completa y este es el 5to tramo
            // (o posterior), usamos la maniobra optimizada en vez del giro normal de esquina.
            if (cantidadComandosMemorizados == TOTAL_TRAMOS_PISTA && contadorLineas > TOTAL_TRAMOS_PISTA) {
              maniobraOptimizadaPendiente = true;
              maniobraOptimizadaEsIzquierda = false; // naranja = derecha
              tiempoInicioPausaPreManiobraOptimizada = millis();
            } else {
              giroPendiente = true;
              giroPendienteEsIzquierda = false; // naranja = derecha
              tiempoInicioPausaPreGiro = millis();
            }

            apagarVehiculo();
            tiempoUltimaDeteccion = millis();// Actualizar tiempo después de procesar la detección
        }
        else if (naranjaDetectado && sentidoVehiculo == IZQUIERDA) {
          // Línea naranja detectada pero el vehículo va hacia la izquierda
          // Parpadeo de advertencia
          for (int i = 0; i < 2; i++) {
            colorVehiculo(255, 128, 0); // Naranja
            delay(100);
            apagarVehiculo();
            delay(100);
          }
        }
      }
    }
  }
}

// ==================== FUNCIÓN LEER SENSOR SHARP ====================
/*float leerSensorSharp(int pin) {
  // Leer valor analógico del sensor Sharp
  int valorAnalogico = analogRead(pin);
  
  // Convertir a voltaje (asumiendo 3.3V de referencia del ESP32)
  float voltaje = valorAnalogico * (3.3 / 4095.0);
  
  // Conversión aproximada para sensor Sharp GP2Y0A21YK0F (10–80 cm)
  // La fórmula puede requerir ajuste fino con pruebas reales
  if (voltaje < 0.3) return 999; // Fuera de rango útil
  
  float distancia = 27.728 / (voltaje - 0.1696);
  
  // Limitar rango válido
  if (distancia < 10 || distancia > 80) return 999;
  
  return distancia;
}*/
float leerSensorSharp(int pin) {
  // Ahora lee los VL53L0X diagonales. Se conserva el nombre y las constantes SHARP_DIAGONAL_*_PIN
  // para no tocar los demás usos en el código.
  return leerToF_L0X(pin == SHARP_DIAGONAL_IZQ_PIN ? 'X' : 'Y');
}
// ==================== FUNCIÓN VERIFICAR OBSTÁCULO DIAGONAL ====================

bool verificarObstaculoDiagonal() {
  // No verificar si ya está en esquive diagonal o retroceso, o muy reciente
  if (esquiveDiagonalActivo || 
      prioridadActual == PRIORIDAD_RETROCESO ||
      millis() - tiempoUltimaDeteccionSharp < TIEMPO_ESPERA_SHARP) {
    return false;
  }
  
  // Leer ambos sensores Sharp
  float distanciaIzq = leerSensorSharp(SHARP_DIAGONAL_IZQ_PIN);
  float distanciaDer = leerSensorSharp(SHARP_DIAGONAL_DER_PIN);
  
  // Verificar obstáculo en diagonal izquierda
  if (distanciaIzq <= DISTANCIA_SHARP_EMERGENCIA && distanciaIzq > 0) {
    esquiveDiagonalActivo = true;
    direccionEsquiveDiagonal = 'D'; // Esquivar hacia la derecha
    tiempoInicioEsquiveDiagonal = millis();
    tiempoUltimaDeteccionSharp = millis();
    etapaEsquiveDiagonal = 0;
    return true;
  }
  
  // Verificar obstáculo en diagonal derecha
  if (distanciaDer <= DISTANCIA_SHARP_EMERGENCIA && distanciaDer > 0) {
    esquiveDiagonalActivo = true;
    direccionEsquiveDiagonal = 'I'; // Esquivar hacia la izquierda
    tiempoInicioEsquiveDiagonal = millis();
    tiempoUltimaDeteccionSharp = millis();
    etapaEsquiveDiagonal = 0;
    return true;
  }
  
  return false;
}

// ==================== FUNCIÓN PROCESAR ESQUIVE DIAGONAL ====================

void procesarEsquiveDiagonal() {
  // Verificar si ha sido interrumpido por una línea
  if (interrupcionCruce) {
    resetearEsquiveDiagonal();
    return;
  }
  if (!esquiveDiagonalActivo) {
    return;
  }
  
  unsigned long tiempoTranscurrido = millis() - tiempoInicioEsquiveDiagonal;
  
  switch(etapaEsquiveDiagonal) {
    case 0: // Orientar servo para cruce diagonal
      if (direccionEsquiveDiagonal == 'D') {
        direccion.write(SERVO_RIGHT); // Cruzar hacia la derecha
      } else if (direccionEsquiveDiagonal == 'I') {
        direccion.write(SERVO_LEFT); // Cruzar hacia la izquierda
      }
      
      // Asegurar que el motor esté activo
      avanzarMotor();
      
      etapaEsquiveDiagonal = 1;
      tiempoInicioEsquiveDiagonal = millis();
      break;
      
    case 1: // Ejecutar cruce diagonal
      if (tiempoTranscurrido > TIEMPO_CRUCE_DIAGONAL) {
        etapaEsquiveDiagonal = 2;
        tiempoInicioEsquiveDiagonal = millis();
      }
      break;
      
    case 2: // Centrar servo y finalizar
      direccion.write(SERVO_CENTRADO);
      
      // Finalizar esquive diagonal
      esquiveDiagonalActivo = false;
      direccionEsquiveDiagonal = 'N';
      etapaEsquiveDiagonal = 0;
      
      // Resetear PID de centrado para retomar navegación normal
      // (Si tienes sistema PID de centrado)
      // resetearPIDCentrado(&pidCentrado);
      break;
  }
}
void resetearEsquiveDiagonal() {
  //direccion.write(SERVO_CENTRADO);
  esquiveDiagonalActivo = false;
  direccionEsquiveDiagonal = 'N';
  etapaEsquiveDiagonal = 0;
  prioridadActual = PRIORIDAD_NORMAL;
  //estadoActual = ESTADO_NORMAL;
  //Reactivar la navegación normal ---
  enGiro = false;
  giroEnProceso = false;
  cruceEnProceso = false;
}

bool verificarRetrocesoEmergencia() {
  // NO verificar si ya está en retroceso o si no ha pasado suficiente tiempo
  if (retrocesoPorEmergencia || 
      millis() - tiempoUltimaVerificacionRetroceso < INTERVALO_VERIFICACION_RETROCESO) {
    return false;
  }
  
  tiempoUltimaVerificacionRetroceso = millis();
  
  // Medir distancia frontal
  float distanciaFrontal = leerToF('F');
  
  // Verificar si hay obstáculo muy cerca y no está ya en retroceso
  if (distanciaFrontal <= DISTANCIA_RETROCESO_EMERGENCIA && 
    distanciaFrontal > 0 && distanciaFrontal < 200) {
    
    // Activar retroceso de emergencia
    retrocesoPorEmergencia = true;
    motorRetrocesoConfigurado = false;  // RESETEAR FLAG AQUÍ
    tiempoInicioRetroceso = millis();
    prioridadActual = PRIORIDAD_RETROCESO;  // MÁXIMA PRIORIDAD
    
    SerialBT.println("ACTIVANDO RETROCESO DE EMERGENCIA");
    
    return true;
  }
  
  return false;
}
void procesarRetrocesoEmergencia() {
  if (!retrocesoPorEmergencia) {
    return;
  }
  
  unsigned long tiempoTranscurrido = millis() - tiempoInicioRetroceso;
  
  // Verificar timeout de seguridad
  if (tiempoTranscurrido > TIMEOUT_RETROCESO) {
    SerialBT.println("TIMEOUT DE RETROCESO - FINALIZANDO FORZADAMENTE");
    
    // Timeout alcanzado - forzar finalización
    detenerMotor(); // Apagar motor de retroceso
    delay(100);
    avanzarMotor();           // Reactivar motor principal
    apagarVehiculo();              // Apagar LED
    
    // RESETEAR TODAS LAS VARIABLES
    retrocesoPorEmergencia = false;
    motorRetrocesoConfigurado = false;
    prioridadActual = PRIORIDAD_NORMAL;
    estadoActual = ESTADO_NORMAL;
    
    delay(TIEMPO_ESPERA_POST_RETROCESO);
    return;
  }
  
  if (tiempoTranscurrido < TIEMPO_RETROCESO) {
    // FASE DE RETROCESO ACTIVO
    
    // Configurar motores solo una vez por ciclo
    if (!motorRetrocesoConfigurado) {
     // //ln("Configurando motores para retroceso...");
      
      detenerMotor();           // APAGAR motor principal COMPLETAMENTE
      delay(150);                              // Pausa para asegurar parada
      retrocederMotor();  // ACTIVAR motor de retroceso
      direccion.write(SERVO_CENTRADO);         // Centrar servo para retroceso recto
      
      motorRetrocesoConfigurado = true;
     // //ln("Motores configurados - Retrocediendo...");
    }
    
    // Parpadeo rápido del LED para indicar retroceso
    if ((tiempoTranscurrido / 150) % 2 == 0) {
      colorVehiculo(255, 0, 0); // Rojo de emergencia
    } else {
      apagarVehiculo();
    }
    
  } else {
    // FASE DE FINALIZACIÓN
    ////ln("Finalizando retroceso...");
    
    detenerMotor(); // Apagar motor de retroceso PRIMERO
    delay(200);                              // Pausa para que se detenga completamente
    
    // Verificar distancia SOLO al final
    float distanciaFinal = leerToF('F');
    ////("Distancia final tras retroceso: ");
    ////ln(distanciaFinal);
    
    if (distanciaFinal > DISTANCIA_RETROCESO_EMERGENCIA + 5 || distanciaFinal > 100) {
      // Hay suficiente espacio O sensor fuera de rango (espacio libre)
      avanzarMotor();           // Reactivar motor principal
      apagarVehiculo();              // Apagar LED
      // Se resetea el búfer para ignorar comandos recibidos durante la emergencia.
      bufferIndex = 0;
      buffer[0] = '\0';
      // RESETEAR TODAS LAS VARIABLES CORRECTAMENTE
      retrocesoPorEmergencia = false;
      motorRetrocesoConfigurado = false;  // IMPORTANTE: Resetear para próxima vez
      prioridadActual = PRIORIDAD_NORMAL;
      estadoActual = ESTADO_NORMAL;
      
      ////ln("Retroceso completado exitosamente - Reanudando marcha normal");
      delay(TIEMPO_ESPERA_POST_RETROCESO);
      
    } else {
      // AÚN hay obstáculo - reiniciar ciclo de retroceso
     SerialBT.println("Obstáculo aún presente - Reiniciando ciclo de retroceso");
      
      // RESETEAR PARA NUEVO CICLO
      motorRetrocesoConfigurado = false;  // PERMITIR NUEVA CONFIGURACIÓN
      tiempoInicioRetroceso = millis();   //  TIEMPO DE INICIO
      
      // Mantener retrocesoPorEmergencia = true para continuar
    }
  }
}


// Función adicional para verificar si el sistema puede ejecutar operaciones
bool sistemaListo() {
  return sistemaActivo && sistemaIniciado;
}

// Modificación para tu función setup() - agregar al final
/*void configurarInterruptor() {
  pinMode(INTERRUPTOR_PIN, INPUT_PULLUP);  // Configurar con pull-up interno
  sistemaActivo = false;  // Sistema inicia desactivado
  sistemaIniciado = false;
  
  // Si el interruptor ya está presionado al encender, iniciar automáticamente
  if (digitalRead(INTERRUPTOR_PIN) == LOW) {
    delay(100);  // Pequeña pausa para estabilizar la lectura
    if (digitalRead(INTERRUPTOR_PIN) == LOW) {
      iniciarSistema();
      //delay(1500);
    }
  } else {
    esperarInterruptor();  // Si no está presionado, esperar
  }
}*/

void configurarInterruptorInteligente() {
  pinMode(INTERRUPTOR_PIN, INPUT_PULLUP);
  sistemaActivo  = false;
  sistemaIniciado = false;

  // ── Esperar el interruptor ──────────────────────────────────────────────────
  bool ledState = false;
  unsigned long ultimoParpadeo = 0;
  while (digitalRead(INTERRUPTOR_PIN) == HIGH) {
    unsigned long ahora = millis();
    if (ahora - ultimoParpadeo > 500) {
      ledState = !ledState;
      if (ledState) colorVehiculo(255, 100, 0);
      else          apagarVehiculo();
      ultimoParpadeo = ahora;
    }
    delay(10);
  }
  apagarVehiculo();
  delay(100); // debounce

  // ── Leer sensores frontal y trasero ────────────────────────────────────────
  colorVehiculo(0, 100, 255); // Azul = midiendo
  SerialBT.println("🔍 Interruptor activado. Midiendo ToF frontal y trasero...");

  float distF = leerToF('F');
  delay(30);
  float distT = leerToF('T');

  SerialBT.println("   ToF Frontal: " + String(distF, 1) + " cm");
  SerialBT.println("   ToF Trasero: " + String(distT, 1) + " cm");

  // Distancia fuera de cajón: > 15 cm O lectura inválida (999)
  const float UMBRAL_CAJON = 15.0;
  bool frontalLibre = (distF > UMBRAL_CAJON || distF >= 999);
  bool traseroLibre = (distT > UMBRAL_CAJON || distT >= 999);

  if (frontalLibre && traseroLibre) {
    // ── MODO PISTA ABIERTA: saltar directo a espera de comando ──────────────
    SerialBT.println("✅ Sensores libres → Modo pista abierta. Iniciando sistema...");
    iniciarSistema();

   // Queda en ESTADO_NORMAL: el loop() centrará y avanzará igual que siempre
    estadoActual = ESTADO_NORMAL;
    detenerMotor();
    motorArrancado = false; // verificarArranqueMotor lo arrancará tras el DELAY_ARRANQUE_MOTOR

    colorVehiculo(128, 0, 255); // Morado = esperando comando
    SerialBT.println("🟣 Esperando comando Cxx por 5 segundos...");

    const unsigned long TIEMPO_ESPERA_PISTA = 5000;
    unsigned long tiempoInicioPista = millis();
    bool comandoPistaRecibido = false;

    while (millis() - tiempoInicioPista < TIEMPO_ESPERA_PISTA) {
      // Parpadeo morado
      unsigned long ahora = millis();
      if (ahora - ultimoParpadeo > 400) {
        ledState = !ledState;
        if (ledState) colorVehiculo(128, 0, 255);
        else          apagarVehiculo();
        ultimoParpadeo = ahora;
      }

      String cmd = "";
      if (Serial.available() > 0) {
        cmd = Serial.readStringUntil('\n');
      } else if (Serial.available() > 0) {
        cmd = Serial.readStringUntil('\n');
      }

      if (cmd != "") {
        cmd.trim();
        cmd.toUpperCase();
        if (cmd.startsWith("C") && cmd.length() >= 3) {
          cmd = cmd.substring(0, 3);
          SerialBT.println("✅ Comando recibido en pista: " + cmd);

          // Retroceso corto para ganar espacio antes del esquive
          //SerialBT.println("⏪ Retroceso previo al esquive...");
          direccion.write(SERVO_CENTRADO);
          /*retrocederMotor();
          unsigned long tiempoInicioRetrocesoPrevio = millis();
          while (millis() - tiempoInicioRetrocesoPrevio < 700) { // 500ms ajustable
            actualizarGiro();
            actualizarRampaMotor();
            delay(10);
          }*/
          detenerMotor();
          delay(150); // pausa breve para estabilizar

          procesarComandoCarril(cmd);
          comandoPistaRecibido = true;
          break;

        } else {
          SerialBT.println("⚠️ Comando '" + cmd + "' inválido. Esperando Cxx...");
        }
      }
      delay(50);
    }

    if (!comandoPistaRecibido) {
      // Tiempo agotado → dejar que el loop arranque normalmente en ESTADO_NORMAL
      SerialBT.println("⏱️ Tiempo agotado. Continuando en modo normal.");
      colorVehiculo(0, 255, 0);
      // No llamar avanzarMotor() aquí: verificarArranqueMotor() lo hará en el loop
    }

  } else {
    // ── MODO CAJÓN: ejecutar salida de estacionamiento completa ─────────────
    SerialBT.println("🅿️ Sensor cercano detectado → Modo cajón. Ejecutando salida...");

    // Decidir dirección igual que iniciarConSalidaEstacionamiento()
    float distDer = leerToF('D');
    delay(30);
    float distIzq = leerToF('I');
    SerialBT.println("   ToF Der: " + String(distDer, 1) + " cm | ToF Izq: " + String(distIzq, 1) + " cm");

    bool salirHaciaIzquierda = !(distIzq < distDer);
    if (salirHaciaIzquierda) {
      SerialBT.println(">> Salida hacia IZQUIERDA (original).");
    } else {
      SerialBT.println(">> Salida hacia DERECHA (invertida).");
    }

    iniciarSistema();
    ejecutarSalidaEstacionamiento(salirHaciaIzquierda);
  }
}
//----------------- FIN Funciones de evasion y recepcion de serial---------------

// Función que debes llamar en tu loop principal
void verificarArranqueMotor() {
  // Solo verificar si el sistema está activo y el motor no ha arrancado aún
  if (sistemaActivo && !motorArrancado) {
    // Verificar si ha pasado el tiempo de delay
    if (millis() - tiempoInicioSistema >= DELAY_ARRANQUE_MOTOR) {
      // Arrancar el motor
      avanzarMotor(); // Activar motor (LOW para avanzar según tu configuración)
      motorArrancado = true;
      SerialBT.println(F("Tiempo de espera serial agotado. Arrancando motor..."));
      // //ln(F("Motor arrancado"));
    }
  }
}
void iniciarConSalidaEstacionamiento() {
  // --- Paso 1: Esperar la activación del interruptor ---
  SerialBT.println("Esperando activación del interruptor (LOW) para iniciar...");
  bool ledState = false;
  unsigned long ultimoParpadeo = 0;
  while (digitalRead(INTERRUPTOR_PIN) == HIGH) {
    unsigned long ahora = millis();
    if (ahora - ultimoParpadeo > 500) {
      ledState = !ledState;
      colorVehiculo(255, 100, 0);
      ultimoParpadeo = ahora;
    }
    delay(10);
  }

  // --- Paso 2: Medir distancias laterales para decidir la dirección de salida ---
  colorVehiculo(255, 0, 255); // LED fijo para indicar que está procesando
  SerialBT.println("Interruptor activado. Midiendo distancias laterales para decidir la maniobra...");
  
  // Medir distancia de ambos sensores
  float distDer = leerToF('D');
  delay(50); // Pequeña pausa entre lecturas para evitar interferencias
  float distIzq = leerToF('I');

  SerialBT.println("-> Distancia Derecha: " + String(distDer) + " cm, Izquierda: " + String(distIzq) + " cm");

  // --- Paso 3: Decidir la dirección de la maniobra ---
  bool salirHaciaIzquierda; // true para la maniobra original, false para la invertida

  // Lógica de decisión: salir por el lado con MÁS espacio.
  // Si el sensor izquierdo mide una distancia MENOR, significa que la pared está a la izquierda,
  // por lo que debemos salir por la derecha (maniobra invertida).
  if (distIzq < distDer) {
      salirHaciaIzquierda = false; // Hay menos espacio a la izquierda, salimos por la derecha.
      SerialBT.println(">> Decisión: Menor distancia a la izquierda. Se ejecutará la salida hacia la DERECHA (invertida).");
  } else {
      salirHaciaIzquierda = true; // Menor distancia a la derecha (o son iguales), salimos por la izquierda (original).
      SerialBT.println(">> Decisión: Menor distancia a la derecha. Se ejecutará la salida hacia la IZQUIERDA (original).");
  }
  
  // --- Paso 4: Iniciar el sistema y ejecutar la maniobra seleccionada ---
  iniciarSistema(); 
  ejecutarSalidaEstacionamiento(salirHaciaIzquierda);
}
void esperarInicioConComando() {
  // --- Parte 1: Esperar la activación del interruptor ---
  SerialBT.println("Esperando activación del interruptor (LOW)...");
  SerialBT.println("Esperando activación del interruptor (LOW)...");
  bool ledState = false;
  unsigned long ultimoParpadeo = 0;
  
   // (Descomenta este bloque si quieres que espere el botón físico primero)
  while (digitalRead(INTERRUPTOR_PIN) == HIGH) {

    unsigned long ahora = millis();
    if (ahora - ultimoParpadeo > 500) {
      ledState = !ledState;
      if(ledState) colorVehiculo(255, 100, 0);
      else apagarVehiculo();
      ultimoParpadeo = ahora;
    }
    delay(10);
  }
  

  // --- Parte 2: Esperar comando 'I' indefinidamente ---
  colorVehiculo(255, 0, 200);
  SerialBT.println("Interruptor activado. Esperando comando de inicio 'I' ...");
  SerialBT.println("Interruptor activado. Esperando comando de inicio ..");

  bool comandoValidoRecibido = false; 
  unsigned long ultimomsg = 0;

  // Bucle infinito: NO saldrá de aquí hasta que comandoValidoRecibido sea true
  while (!comandoValidoRecibido) {
    
    unsigned long ahora = millis();
    // Parpadeo lento del LED para indicar que el robot está vivo y esperando
    if (ahora - ultimomsg > 500) {
      ledState = !ledState;
      if(ledState) colorVehiculo(255, 0, 200);
      else apagarVehiculo();
      ultimomsg = ahora;
    }
    
    String comando = "";
    
    // Lee el comando desde o el cable USB (corregido)
    if (Serial.available() > 0) {
      comando = Serial.readStringUntil('\n');
    }

    if (comando != "") {
      comando.trim();
      // No hacer toUpperCase() para distinguir "com" de otras cosas

      if (comando == "I") {
        //SerialBT.println("✅ Comando de inicio 'I' recibido. ¡Arrancando!");
        SerialBT.println("✅ Comando de inicio 'I' recibido. ¡Arrancando!");
        
        iniciarSistema();              // Inicia todas las variables y el estado normal
        comandoValidoRecibido = true;  // Esto rompe el bucle while y permite continuar al loop()
        
      }else if (comando == "com") {
        //SerialBT.println("✅ Comando 'com' recibido. Iniciando en modo ESPERANDO_COMANDO_CARRIL...");
        SerialBT.println("✅ Comando 'com' recibido. Iniciando en modo ESPERANDO_COMANDO_CARRIL...");
        
        iniciarSistema();

        // Sobreescribir estado: modo prueba de carriles
        estadoActual = ESPERANDO_COMANDO_CARRIL;
        carriles.tiempoInicioEstado = millis();
        sentidoVehiculo = IZQUIERDA;
        primeraDeteccion = false;


        // Forzar ángulo objetivo a 0
        anguloObjetivo = 0.0;
        primerGiroRealizado = false;

        // Motor detenido, evitar que arranque solo
        detenerMotor();
        motorArrancado = true;

        colorVehiculo(128, 0, 255); // LED morado = esperando comando
        SerialBT.println("🟣 Listo. Envía un comando Cxx para empezar.");

        comandoValidoRecibido = true;
        
      } else {
        SerialBT.println("❌ Comando '" + comando + "' inválido. Esperando 'com' o i para arrancar.");
        SerialBT.println("❌ Comando '" + comando + "' inválido. Esperando 'com' para arrancar.");
      }
    }
    delay(50);
  }
}
void setup() {

  //analogWrite(BUZZER_PIN, 160);
  //delay(100);
  //analogWrite(BUZZER_PIN, 0);
  direccion.write(SERVO_CENTRADO);
  Serial.begin(115200);
  Serial.setTimeout(10);




    /// Iniciar ambos buses I2C
  Wire.begin(); // Pines por defecto para BNO055 y ToFs
  I2CSegundo.begin(I2C2_SDA, I2C2_SCL);
  Wire.setClock(400000);       // Acelera el bus 1 a 400kHz
  I2CSegundo.setClock(400000); // Acelera el bus 2 a 400kHz

  // --- Iniciar NeoPixels ---
  tiraLED.begin();
  tiraLED.setBrightness(50); // Ajusta el brillo de 0 a 255 (50 es seguro para no consumir demasiada corriente)
  apagarVehiculo();          // Asegurar que inicien apagados
    SerialBT.println("Vehiculo_Autonomo");

    SerialBT.println(F("\n--- INICIANDO DIAGNÓSTICO DE SENSORES ---"));

  // 1. Configuración y Apagado inicial de ToFs
  pinMode(XSHUT_FRONTAL, OUTPUT); pinMode(XSHUT_TRASERO, OUTPUT);
  pinMode(XSHUT_IZQ, OUTPUT); pinMode(XSHUT_DER, OUTPUT);

    pinMode(XSHUT_DIAG_IZQ, OUTPUT); pinMode(XSHUT_DIAG_DER, OUTPUT);

  digitalWrite(XSHUT_FRONTAL, LOW); digitalWrite(XSHUT_TRASERO, LOW);
  digitalWrite(XSHUT_IZQ, LOW); digitalWrite(XSHUT_DER, LOW);

    digitalWrite(XSHUT_DIAG_IZQ, LOW); digitalWrite(XSHUT_DIAG_DER, LOW);

  delay(100);

  // 2. Encendido secuencial y verificación de ToFs VL53L1X (modo largo, 50 ms)
  SerialBT.print(F("Iniciando ToF Frontal (0x30)... "));
  digitalWrite(XSHUT_FRONTAL, HIGH); delay(50);
  if (!iniciarToF(tofF, ADDR_FRONTAL)) {
    SerialBT.println(F("FALLO. Verifica cableado o pin 18. Reintentando..."));
    reintentarSensor([]() -> bool { return iniciarToF(tofF, ADDR_FRONTAL); }, 255, 0, 0, "ToF Frontal");
  }
  SerialBT.println(F("OK"));

  SerialBT.print(F("Iniciando ToF Trasero (0x31)... "));
  digitalWrite(XSHUT_TRASERO, HIGH); delay(50);
  if (!iniciarToF(tofT, ADDR_TRASERO)) {
    SerialBT.println(F("FALLO. Verifica cableado o pin 17. Reintentando..."));
    reintentarSensor([]() -> bool { return iniciarToF(tofT, ADDR_TRASERO); }, 255, 128, 0, "ToF Trasero");
  }
  SerialBT.println(F("OK"));

  SerialBT.print(F("Iniciando ToF Izquierdo (0x32)... "));
  digitalWrite(XSHUT_IZQ, HIGH); delay(50);
  if (!iniciarToF(tofI, ADDR_IZQ)) {
    SerialBT.println(F("FALLO. Verifica cableado o pin 5. Reintentando..."));
    reintentarSensor([]() -> bool { return iniciarToF(tofI, ADDR_IZQ); }, 255, 255, 0, "ToF Izquierdo");
  }
  SerialBT.println(F("OK"));

  SerialBT.print(F("Iniciando ToF Derecho (0x33)...   "));
  digitalWrite(XSHUT_DER, HIGH); delay(50);
  if (!iniciarToF(tofD, ADDR_DER)) {
    SerialBT.println(F("FALLO. Verifica cableado o pin 16. Reintentando..."));
    reintentarSensor([]() -> bool { return iniciarToF(tofD, ADDR_DER); }, 0, 255, 255, "ToF Derecho");
  }
  SerialBT.println(F("OK"));

  SerialBT.print(F("Iniciando ToF Diagonal Izq VL53L0X (0x34)... "));
  digitalWrite(XSHUT_DIAG_IZQ, HIGH); delay(50);
  if (!iniciarToF_L0X(tofDI, ADDR_DIAG_IZQ)) {
    SerialBT.println(F("FALLO. Verifica cableado o pin 19. Reintentando..."));
    reintentarSensor([]() -> bool { return iniciarToF_L0X(tofDI, ADDR_DIAG_IZQ); }, 255, 255, 255, "ToF Diagonal Izq");
  }
  SerialBT.println(F("OK"));

  SerialBT.print(F("Iniciando ToF Diagonal Der VL53L0X (0x35)... "));
  digitalWrite(XSHUT_DIAG_DER, HIGH); delay(50);
  if (!iniciarToF_L0X(tofDD, ADDR_DIAG_DER)) {
    SerialBT.println(F("FALLO. Verifica cableado o pin 15. Reintentando..."));
    reintentarSensor([]() -> bool { return iniciarToF_L0X(tofDD, ADDR_DIAG_DER); }, 0, 255, 0, "ToF Diagonal Der");
  }
  SerialBT.println(F("OK"));

  // Configurar pin del interruptor con pull-up interno
  pinMode(INTERRUPTOR_PIN, INPUT_PULLUP);
  
// 3. Iniciar Sensor de Color en Bus 2
   SerialBT.print(F("Iniciando Color TCS34725 (Bus 2)... "));
  if (tcs.begin(TCS34725_ADDRESS, &I2CSegundo)) {
    SerialBT.println(F("OK"));
  } else {
    SerialBT.println(F("FALLO sensor de color. Verifica los pines 32 y 33. Reintentando..."));
    reintentarSensor([]() -> bool { return tcs.begin(TCS34725_ADDRESS, &I2CSegundo); }, 255, 0, 255, "Sensor de Color");
  }
  /*************/
 // 4. Iniciar BNO055
  SerialBT.print(F("Iniciando IMU BNO055 (Bus 1)...     "));
  if (bno.begin()) {
    SerialBT.println(F("OK"));
    bno.setExtCrystalUse(true);
    bno.setMode(OPERATION_MODE_IMUPLUS);
    applyCalibrationOffsets();
    delay(100); // Dar tiempo al sensor para estabilizarse en el nuevo modo
    
    // Lecturas de calentamiento
    for (int i = 0; i < 5; i++) { 
      bno.getQuat(); 
      delay(10); 
    } 
    //resetYaw(); // Fija el frente del vehículo como 0°
  } else {
    SerialBT.println(F("FALLO bno055. Verifica cableado o el pin ADR a GND. Reintentando..."));
    reintentarSensor([]() -> bool { return bno.begin(); }, 0, 0, 255, "BNO055");
    SerialBT.println(F("OK"));
    bno.setExtCrystalUse(true);
    bno.setMode(OPERATION_MODE_IMUPLUS);
    applyCalibrationOffsets();
    delay(100);
    for (int i = 0; i < 5; i++) {
      bno.getQuat();
      delay(10);
    }
  }

  SerialBT.println(F("--- TODOS LOS SENSORES LISTOS ---"));

  direccion.setPeriodHertz(50); // Le decimos explícitamente que trabaje a 50Hz
  direccion.attach(SERVO_PIN, 500, 2400); // Mismos anchos de pulso que usaste en la prueba
 
  
  //*******************************

  //direccion.attach(SERVO_PIN);
  // Configurar pines como salidas digitales
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_ENA, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  // 1. Configurar PWM del Motor (Nueva sintaxis ESP32 v3)
  ledcAttach(MOTOR_ENA, freqMotor, resolucionPWM);

  // 2. Configurar PWM del Buzzer (Nueva sintaxis ESP32 v3)
  ledcAttach(BUZZER_PIN, 2000, resolucionPWM); 
  noTone(BUZZER_PIN); // Asegurar silencio inicial

  // Inicializar el motor detenido de forma segura
  detenerMotor();

 // Inicializar variables de motor
  tiempoInicioSistema = 0;
  motorArrancado = false;
  
  // Inicializar variables de interruptor
  interruptorAnterior = digitalRead(INTERRUPTOR_PIN);
  tiempoDebounce = 0;
  sistemaActivo = false;
  sistemaIniciado = false;
  
  // Iniciar período de evaluación para retroceso post-giro
  tiempoInicioEvaluacion = millis();
  
   // Inicializar variables de memoria de color
  ultimoColorDetectado = 'N';
  tiempoUltimoColor = 0;
  bufferIndex = 0;
  lastOrientation = 'C';
  // Calibración inicial
  ////ln(F("Calibrando..."));
  // //(F("Offset: "));
  ////ln(offsetGz, 4);
  resetearPIDCentrado(&pidCentrado);

  //iniciarConSalidaEstacionamiento(); 
  
  //ejecutarSalidaEstacionamiento();

  //configurarInterruptor(); // Inicia por interruptor físico (pin 23)
  
  configurarInterruptorInteligente(); // Detecta si está en cajón o pista abierta
  
  ////ln(F("Sistema listo"));
  lastTime = millis();
  tiempoUltimaDeteccion = millis();
  ultimoTiempoCentrado = millis();

  // Crear la tarea para ejecutar la detección de color en el Núcleo 0
  xTaskCreatePinnedToCore(
    TareaDeteccionColor,    // Función que contiene la lógica de la tarea
    "TareaColor",           // Nombre descriptivo de la tarea
    4096,                   // Tamaño de la pila (Stack de 4KB es seguro para I2C)
    NULL,                   // Parámetros de entrada de la tarea
    1,                      // Prioridad de la tarea (1 es prioridad normal)
    &TareaColorHandle,      // Referencia del Handle declarado globalmente
    0                       // Código del núcleo: Núcleo 0
  );

#ifdef MODO_PRUEBA_MANIOBRA_OPTIMIZADA
  // ---- INYECCIÓN DE PRUEBA: simula que ya se completó la primera vuelta ----
  // Con esto, la PRIMERA línea que detectes en pista al encender el auto ya se
  // tratará como la línea 5, y disparará la maniobra optimizada directamente.
  sentidoVehiculo = IZQUIERDA; // C02 es un comando de sentido DERECHA según tu tabla

  comandosMemorizados[0] = "C07";
  comandosMemorizados[1] = "C07";
  comandosMemorizados[2] = "C21";
  comandosMemorizados[3] = "C02E";
  cantidadComandosMemorizados = TOTAL_TRAMOS_PISTA;
  memorizacionCompleta = true;

  contadorLineas = TOTAL_TRAMOS_PISTA; // = 4, así la próxima detección real será tratada como línea 5
  primerGiroRealizado = true;          // ya no estamos en el modo de arranque especial

  // Ángulo objetivo coherente con haber completado 4 tramos en sentido DERECHA
  // (mismo cálculo que hace calcularAnguloObjetivo() para contadorLineas=4, sentido DERECHA)
  anguloObjetivo = 0.0;

  SerialBT.println("🧪 MODO PRUEBA ACTIVO: se simula vuelta ya memorizada (4x C02, sentido DERECHA).");
  SerialBT.println("🧪 La próxima línea detectada disparará la MANIOBRA OPTIMIZADA.");
#endif
}

// ============================================================
//  MONITOR LEGIBLE DE MANTENIENDO_CARRIL
// ============================================================
void imprimirMonitorCarril(int servo) {
  unsigned long ahora = millis();
  char lado = obtenerLadoControlPorCarril();
  const float TOL = controlLateral.toleranciaDistancia;

  // ---------- DISTANCIA ----------
  bool datosFrescos = (ahora - dbgTDist) < 600;
  bool crudaValida  = (dbgDistCruda > 0 && dbgDistCruda <= 150);

  // Detector de lectura congelada (ToF clavado en el mismo valor)
  static float ultimaCruda = -1;
  static unsigned long tCrudaCambio = 0;
  if (fabs(dbgDistCruda - ultimaCruda) > 0.05) { ultimaCruda = dbgDistCruda; tCrudaCambio = ahora; }
  bool congelada = crudaValida && (ahora - tCrudaCambio > 2000);

  // Detector de "no converge" (error grande y que no cambia)
  static float errRef = 0;
  static unsigned long tErrRef = 0;
  if (fabs(dbgErrDist - errRef) > 1.0) { errRef = dbgErrDist; tErrRef = ahora; }
  bool sinConverger = (fabs(dbgErrDist) > TOL) && (ahora - tErrRef > 2000);

  String distTxt;
  if (!datosFrescos) {
    distTxt = "SIN DATOS (el control lateral no esta corriendo: emergencia/frontal)";
  } else {
    distTxt = String(dbgDistFiltrada, 1) + "cm";
    distTxt += crudaValida ? (" (crudo " + String(dbgDistCruda, 1) + ")")
                           : (" (crudo INVALIDO " + String(dbgDistCruda, 0) + ")");
  }

  // ---------- VEREDICTO ----------
  String veredicto;
  if (!datosFrescos)                     veredicto = "❌ NO CONFIABLE: sin datos recientes";
  else if (dbgSinRef)                    veredicto = "❌ NO CONFIABLE: sin referencia lateral, solo sigue el angulo (invalidas seguidas: " + String(dbgInvalidas) + ")";
  else if (dbgInvalidas > 0)             veredicto = "⚠️ DUDOSO: filtro devolviendo valor viejo (" + String(dbgInvalidas) + " seguidas)";
  else if (congelada)                    veredicto = "❌ SENSOR CONGELADO: mismo valor hace " + String((ahora - tCrudaCambio) / 1000.0, 1) + "s (falsa lectura?)";
  else if (crudaValida && fabs(dbgDistCruda - dbgDistFiltrada) > 5.0)
                                         veredicto = "⚠️ DUDOSO: crudo y filtrado difieren mucho (salto)";
  else if (sinConverger)                 veredicto = "⚠️ NO CONVERGE: error " + String(dbgErrDist, 1) + "cm sin cambiar hace " + String((ahora - tErrRef) / 1000.0, 1) + "s (pegado?)";
  else if (fabs(dbgErrDist) <= TOL)      veredicto = "✅ MANTENIENDO: en distancia objetivo";
  else                                   veredicto = "🔄 CORRIGIENDO: " + String(dbgErrDist < 0 ? "muy CERCA de la pared" : "muy LEJOS de la pared");

  // ---------- COLOR ----------
  String colorTxt;
  unsigned long edadColor = ahora - dbgTLectura;
  if (dbgTLectura == 0 || edadColor > 500) {
    colorTxt = "❌ SIN LECTURAS hace " + String(edadColor) + "ms (detectarColor esta retornando antes: giro/retroceso/estado)";
  } else {
    String nombre;
    switch (dbgColor) {
      case 'B': nombre = "BLANCO";   break;
      case 'A': nombre = "AZUL";     break;
      case 'N': nombre = "NARANJA";  break;
      default:  nombre = "piso/otro"; break;
    }
    colorTxt = nombre + " (H:" + String(dbgHue, 0) + " S:" + String(dbgSat, 2) + " V:" + String(dbgVal, 2) + ")";
    colorTxt += dbgPuedeDetectar ? " | ✅ deteccion ACTIVA" : " | ⏸ EN ESPERA (lee pero no cuenta lineas)";
  }

  // ---------- ANGULO + SHARP ----------
  float errorAngulo = controlLateral.anguloObjetivo - anguloZ;
  if (errorAngulo > 180.0) errorAngulo -= 360.0;
  else if (errorAngulo < -180.0) errorAngulo += 360.0;

  float sI = leerSensorSharp(SHARP_DIAGONAL_IZQ_PIN);
  float sD = leerSensorSharp(SHARP_DIAGONAL_DER_PIN);
  String sharpTxt = "Izq " + String((sI > 0 && sI < 150) ? String(sI, 1) + "cm" : String("N/A")) +
                    " | Der " + String((sD > 0 && sD < 150) ? String(sD, 1) + "cm" : String("N/A"));

  // ---------- ARMAR Y MANDAR TODO DE UNA SOLA VEZ ----------
  String out = "\n━━━━━━━━━ MANTENIENDO CARRIL ━━━━━━━━━\n";
  out += "🛣 Carril objetivo: " + String(carriles.carrilObjetivo) +
         " (pared " + String(lado == 'I' ? "IZQ" : "DER") + ") | Servo: " + String(servo) + "\n";
  out += "📏 Distancia: " + distTxt + " | Objetivo: " + String(dbgDistObjetivo, 1) +
         "cm | Error: " + String(dbgErrDist, 1) + "cm\n";
  out += "🔎 " + veredicto + "\n";
  out += "🎨 Color: " + colorTxt + "\n";
  out += "📐 Err Ang: " + String(errorAngulo, 1) + "° | Sharp: " + sharpTxt + "\n";
  out += "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
  SerialBT.println(out);
}

void loop() {

  // PRIMERA PRIORIDAD: Verificar interruptor SIEMPRE
  //verificarInterruptor();
  
  verificarArranqueMotor();

   // EJECUTAR SIEMPRE (independiente de comandos seriales)
  actualizarGiro();
  actualizarRampaMotor();  //funcion de arranque progresivo. 
  actualizarLedTemporal(); //  Vigila los LEDs sin bloquear
    // Detección de color se ejecuta en estados de navegación, no durante maniobras de carril

  // Si el sistema no está activo, no ejecutar nada más
  if (!sistemaActivo) {
    delay(50);  // Pequeña pausa para no saturar el procesador
    return;
  }
  //////////////////////////////////
  #ifdef TEST_LINEA_SIMULADA
  if (estadoActual == MANTENIENDO_CARRIL) {
    leerComandoSimuladoLinea();
  }
  #endif
  // ────────────────────────────────────────────────────────────────────────
  // Prioridad alta: si hay un giro esperando su pausa pre-giro, gestionarlo
  // aquí y saltar el resto del ciclo para no pisar el servo/motor.
  if (procesarPausaPreGiro()) {
    return;
  }

  if (procesarPausaPreManiobraOptimizada()) {
    return;
  }

  if (estadoActual == MANIOBRA_OPTIMIZADA) {
    procesarManiobraOptimizada();
    return;
  }  
    // Procesar giro de apertura si está activo
  if (giroAperturaActivo) {
    procesarGiroApertura();
    return; // No ejecutar nada más mientras se procesa la apertura
  }

    // ====== DETENCIÓN PERMANENTE: ignorar TODO sensor a partir de aquí ======
  if (detencionPermanenteActiva) {
    detenerMotor();
    direccion.write(SERVO_CENTRADO);
    delay(20);
    return; // No se lee color, ToF, Sharp, ni ningún otro sensor.
  }
  // ==========================================================================
   
   // MÁQUINA DE ESTADOS - Solo ejecutar si no hay procesos de alta prioridad
  switch(estadoActual) {
    case ESTADO_NORMAL:
      //centrarVehiculo();
           if (!enGiro && 
          prioridadActual == PRIORIDAD_NORMAL && 
          !esquiveDiagonalActivo && 
          !retrocesoPorEmergencia &&
          !retrocesoOrientadoActivo) { 
        centrarVehiculo();
      }
      break;
    case ESPERANDO_COMANDO_CARRIL:
      detenerMotor();
      // ============ REPRODUCCIÓN AUTOMÁTICA DE COMANDOS MEMORIZADOS ============
      // Usa el mismo criterio que el resto del código: (contadorLineas - 1) % 4
      // Línea 1 -> índice 0, línea 2 -> índice 1, línea 3 -> índice 2, línea 4 -> índice 3,
      // línea 5 -> índice 0 de nuevo (se repite la vuelta), etc.

      if (memorizacionCompleta && contadorLineas >= 1) {
        int indiceTramo = (contadorLineas - 1) % TOTAL_TRAMOS_PISTA;
        String comandoAReproducir = comandosMemorizados[indiceTramo];

        // Si venimos de completar la línea 12 (limiteLineas), este es el
        // último tramo: el próximo MANTENIENDO_CARRIL debe avanzar 2s y
        // detenerse para siempre.
        if (contadorLineas >= limiteLineas) {
          esUltimoTramoFinal = true;
          SerialBT.println("🏁 Último tramo activado: el próximo MANTENIENDO_CARRIL avanzará 2s y se detendrá para siempre.");
        }

        SerialBT.println("🔁 Reproduciendo comando memorizado (línea " + String(contadorLineas) +
                          ", índice " + String(indiceTramo) + "): " + comandoAReproducir);

        procesarComandoCarril(comandoAReproducir);
        mo_armarObjetoPrimerasVueltas(comandoAReproducir); // v98
        break; // salimos del case: procesarComandoCarril ya cambió el estado
      }

      // ==========================================================================

      
      while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
          if (bufferIndex > 0) {
            buffer[bufferIndex] = '\0';
            comandoRecibido = String(buffer);
            comandoListo = true;
            bufferIndex = 0;
          }
        } else if (bufferIndex < BUFFER_SIZE - 1) {
          buffer[bufferIndex++] = c;
        }
      }



      if (comandoListo) {
        comandoListo = false;
        comandoRecibido.trim();

        // Comando de parada directo desde aquí también
        if (comandoRecibido == "s") {
          detenerMotor();
          direccion.write(SERVO_CENTRADO);
          anguloObjetivo = 0.0;
          primerGiroRealizado = false;
          cambiarEstado(ESPERANDO_COMANDO_CARRIL);
          SerialBT.println("🟣 Detenido. anguloObjetivo = 0°.");
          break;
        }

        comandoRecibido.toUpperCase();

        if (comandoRecibido.startsWith("C") && comandoRecibido.length() >= 3) {
          // Si el 4to caracter es 'E' (estacionamiento), nos quedamos con 4; si no, con 3.
          int largoValido = (comandoRecibido.length() >= 4 && comandoRecibido.charAt(3) == 'E') ? 4 : 3;
          comandoRecibido = comandoRecibido.substring(0, largoValido);

          //SerialBT.println("Comando aceptado: " + comandoRecibido);
          SerialBT.println("Comando aceptado: " + comandoRecibido);
          procesarComandoCarril(comandoRecibido);
          memorizarComando(comandoRecibido);
          mo_armarObjetoPrimerasVueltas(comandoRecibido); // v98
        } else {
          SerialBT.println("Comando ignorado: " + comandoRecibido);
        }
      }

      // Reporte de sensores cada 1000ms (sin bloquear)
      static unsigned long ultimoReporteEspera = 0;
      if (millis() - ultimoReporteEspera > 1000) {
        float distF = leerToF('F');
        float distI = leerToF('I');
        float distD = leerToF('D');
        float distSharpIzq = leerSensorSharp(SHARP_DIAGONAL_IZQ_PIN);
        float distSharpDer = leerSensorSharp(SHARP_DIAGONAL_DER_PIN);
        SerialBT.print("⏸️ ESTADO ESPERA | ToF F: ");
        SerialBT.print(distF > 0 && distF < 999 ? String(distF, 1) + "cm" : "N/A");
        SerialBT.print(" | ToF I: ");
        SerialBT.print(distI > 0 && distI < 999 ? String(distI, 1) + "cm" : "N/A");
        SerialBT.print(" | ToF D: ");
        SerialBT.print(distD > 0 && distD < 999 ? String(distD, 1) + "cm" : "N/A");
        SerialBT.print(" | Sharp Izq: ");
        SerialBT.print(distSharpIzq > 0 && distSharpIzq < 150 ? String(distSharpIzq, 1) + "cm" : "N/A");
        SerialBT.print(" | Sharp Der: ");
        SerialBT.println(distSharpDer > 0 && distSharpDer < 150 ? String(distSharpDer, 1) + "cm" : "N/A");
        ultimoReporteEspera = millis();
      }

      break;

     case EVALUANDO_POSICION:
    case GIRANDO_HACIA_CARRIL:
    //case ENDEREZANDO_CARRIL:
    case MANTENIENDO_CARRIL:
      { 
        EstadoSistema estadoPrevio = estadoActual; // Guardar el estado actual

        // ====== TRAMO FINAL: avanzar 2s y detenerse para siempre ======
        if (estadoActual == MANTENIENDO_CARRIL && esUltimoTramoFinal) {
          if (!detencionPermanenteActiva) {
            if (tiempoInicioUltimoAvance == 0) {
              tiempoInicioUltimoAvance = millis();
              SerialBT.println("🏁 Iniciando avance final de 1.3s antes de detención permanente.");
            }
            if (millis() - tiempoInicioUltimoAvance < 1000) {
              // v98: sigue el ángulo objetivo y la distancia de pared del comando (mismo control que MANTENIENDO_CARRIL)
              if (millis() - ultimaActualizacionf >= INTERVALO_CONTROLf) {
                float anguloServoFinal = calcularControlCarriles();
                if (anguloServoFinal != SERVO_SIN_COMANDO) {
                  direccion.write((int)anguloServoFinal);
                }
                ultimaActualizacionf = millis();
              }
              avanzarMotor();
            } else {
              detencionPermanenteActiva = true;
              esUltimoTramoFinal = false;
              detenerMotor();
              direccion.write(SERVO_CENTRADO);
              SerialBT.println("🛑 Detención permanente activada. Ignorando color y sensores en adelante.");
            }
          } else {
            detenerMotor();
          }
          break; // No ejecutar la lógica normal de carriles en este tramo
        }
        // ====== FIN TRAMO FINAL ======

        // Durante todos los estados de carril, permitir interrupción por línea
        //detectarColor(); // 

        // Si detectarColor cambió el estado, significa que encontró una línea
        // y comenzó un giro. Debemos salir del switch INMEDIATAMENTE.
       /* if (estadoActual != estadoPrevio) {
            break; 
        }*/

        // Si no hubo cambio de estado, continuar con la lógica de carriles
        if (millis() - ultimaActualizacionf >= INTERVALO_CONTROLf) {
            float anguloServo = calcularControlCarriles(); // 
            if (anguloServo != SERVO_SIN_COMANDO) {
                direccion.write((int)anguloServo); // 
                
                
               if (millis() - ultimoReportef > INTERVALO_REPORTEf) {
                                  if (millis() - ultimoReportef > INTERVALO_REPORTEf) {
                    imprimirMonitorCarril((int)anguloServo);
                    ultimoReportef = millis();
                }
                }
            }
            if (mo_objRetrocediendo) {
                mo_objAplicarMotorRetroceso(); // v96: pausa (detenido) + retroceso, no bloqueante
            } else if (mo_objVentanaDeteccionActiva()) {
                mo_objAplicarMotorDeteccion(); // v96: pausa inicial + avance lento mientras busca el objeto
            } else if (esperandoCambioCarril && estadoActual == EVALUANDO_POSICION) {
                avanzarLento(); // v104: espera corta antes del cambio de carril: avanza lento, no se pasa de largo
            } else if (ent_activa && estadoActual == MANTENIENDO_CARRIL) {
                avanzarMotor(velocidadMedia); // v101: velocidad media antes de entrar al carril interior
            } else if (enFaseContragiro) {
                avanzarLento(); // Lento solo durante el contragiro de enderezado || el c08 y c01 son los de E cortos. probar como se comporta.
            } else if (estadoActual == GIRANDO_HACIA_CARRIL) {
                avanzarMotor(velocidadGiroCarril); // Giro inicial más lento: el ToF frontal alcanza a confirmar
            } else if ((carriles.esComandoC02 || carriles.esComandoC07 || carriles.esComandoC08 || carriles.esComandoC01)
                       && (millis() - carriles.tiempoInicioEstado < 2500)) {
                avanzarRapido(); // Velocidad máxima: solo durante los primeros 3s de este MANTENIENDO_CARRIL
            } else {
                avanzarMotor(); // Velocidad normal: pasados los 3s (o en cualquier otro caso), para tomar bien la línea del cruce
            }
            ultimaActualizacionf = millis();
        }

      } 
      break;
      case ENDEREZANDO_CARRIL:
      {
        EstadoSistema estadoPrevio = estadoActual;

        // Aquí SÍ llamamos detectarColor(), pero solo tendrá efecto
        // si contadorLineas == 0 gracias al guard que ya modificamos
        detectarColor();

        // Si detectarColor cambió el estado (detectó línea e inició giro),
        // salir inmediatamente para no pisar ese cambio
        if (estadoActual != estadoPrevio) {
            break; 
        }

        if (millis() - ultimaActualizacionf >= INTERVALO_CONTROLf) {
            float anguloServo = calcularControlCarriles();
            if (anguloServo != SERVO_SIN_COMANDO) {
                direccion.write((int)anguloServo);
            }
            if (enFaseContragiro) {
                avanzarLento();
            } else {
                avanzarMotor();
            }
            ultimaActualizacionf = millis();
        }
      }
      break;
   /* float anguloServo = calcularControlCarriles();
    if (anguloServo != SERVO_SIN_COMANDO) {
        direccion.write((int)anguloServo);

        // ====== INICIO: NUEVO MONITOR DE DEPURACIÓN ======
        // Este bloque imprimirá solo cuando esté en el estado de mantenimiento de carril
        if (millis() - ultimoReportef > INTERVALO_REPORTEf) {
            SerialBT.print("Servo: ");
            SerialBT.print((int)anguloServo);
            SerialBT.print(" | Carril: ");
            SerialBT.print(carriles.carrilObjetivo);
            SerialBT.print(" | Lado: ");
            SerialBT.print(controlLateral.ladoControl);
            SerialBT.print(" | Err Dist: ");
            SerialBT.print(controlLateral.errorDistanciaAnterior, 2); // Muestra el último error de distancia calculado
            SerialBT.print(" | Err Ang: ");
            float errorAngulo = controlLateral.anguloObjetivo - anguloZ;
            if (errorAngulo > 180.0) errorAngulo -= 360.0;
            else if (errorAngulo < -180.0) errorAngulo += 360.0;
            SerialBT.print(errorAngulo, 1);
            SerialBT.println();
            ultimoReportef = millis();
        }
        // ====== FIN: NUEVO MONITOR DE DEPURACIÓN ======
    }
    avanzarMotor();
    ultimaActualizacionf = millis();
    }
      break;
*/
    case ESTADO_CORRECCION_LATERAL:
      ejecutarCorreccionDistanciaLateral();
      break;
    case ESTADO_DETENCION_FINAL:
     if (contadorEntradaEstadoFinal >= 1){
      /*  analogWrite(BUZZER_PIN, 255);
        delay(50);
        analogWrite(BUZZER_PIN, 0);
        delay(50);
        analogWrite(BUZZER_PIN, 255);
        delay(50);
        analogWrite(BUZZER_PIN, 0);*/
        contadorEntradaEstadoFinal += 1; 
      }
      // Avanzar en línea recta durante 2.5 segundos
      if (millis() - tiempoInicioDetencionFinal < 1500) {
        if (!enGiro && 
          prioridadActual == PRIORIDAD_NORMAL) {
        centrarVehiculo();

        //SerialBT.println("normal");
          }
        
        // Debug cada segundo
       /* static unsigned long ultimoDebug = 0;
        if (millis() - ultimoDebug > 1000) {
          //("Avance final - Tiempo restante: ");
          //(2500 - (millis() - tiempoInicioAvanceFinal));
          //ln(" ms");
          ultimoDebug = millis();
        }*/
        
      } else {
        //ln("Avance final completado - Motor detenido");
        detenerRobot();
       // estadoActual = ESTADO_NORMAL; // Opcional: volver a estado normal
        
      }
      break;
      
  }  

    
  // MÁXIMA PRIORIDAD: Retroceso de emergencia (SOLO si NO está girando)
  if (prioridadActual == PRIORIDAD_RETROCESO && !giroEnProceso && !retrocesoOrientadoActivo && estadoActual !=ESTADO_DETENCION_FINAL){
    if (retrocesoPorEmergencia) {
      //procesarRetrocesoEmergencia();
    }
    return;
  }
  
  // ALTA PRIORIDAD: Retroceso post-giro
  if (procesarRetrocesoPostGiro()) {
    // El retroceso está activo, no ejecutar nada más
    return;
  }
    // Verificar obstáculos diagonales SOLO si NO está girando
    //if (!esquiveDiagonalActivo && !giroEnProceso && verificarObstaculoDiagonal()) {
      // Se activó esquive diagonal
    //}

    // Procesar esquive diagonal si está activo Y NO está girando
    if (esquiveDiagonalActivo && !giroEnProceso) {
      procesarEsquiveDiagonal();
    }
   
  // Mostrar el valor cada 300 ms
  if (false && millis() - lastYawPrint >= YAW_INTERVAL) {
        SerialBT.print("Yaw: ");
        //SerialBT.print(yawFiltrado);
        SerialBT.println(anguloZ);
        SerialBT.print("°  ");
        lastYawPrint = millis();
        SerialBT.println("Estado actual: " + obtenerNombreEstado(estadoActual));
        
    }

  delay(20);
}


