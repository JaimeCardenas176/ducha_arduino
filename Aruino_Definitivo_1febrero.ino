#include <Time.h>
#include <TimeLib.h>
//timepo de subida 0.04us tiempo de bajada 0.18us


volatile time_t momento;

const int sensorPin = 2;
//const int measureInterval = 2500;
const int measureInterval = 1000;
const float factorK = 7.5;
float gasto_total = 0;
float gasto_total_luismi = 0;
boolean grifoAbierto = true;

volatile int pulseConter;

const float TOPE_AGUA = 10;
int cantidad_leds_encendidos = 0;
void ISRCountPulse()
{
  pulseConter++;
}

float GetFrequency()
{
  pulseConter = 0;

  interrupts();
  delay(measureInterval);
  noInterrupts();

  return (float)pulseConter * 1000 / measureInterval;
}
String getFecha(time_t fecha) {
  // devuelve la fecha en formato normal
  //return day(fecha)+"/"+month(fecha)+"/"+year(fecha)+"/  "+hour(fecha)+":"+minute(fecha)":"+second(fecha);
  return String(day(fecha)) + "/" + String(month(fecha)) + "/" + String(year(fecha)) + "  " + String(hour(fecha)) + ":" + String(minute(fecha)) + ":" + String(second(fecha));
  //return  String(0,day(fecha));
}
void setup()
{
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(sensorPin), ISRCountPulse, RISING);
}

void leds(float gasto_actual) {

  // TOPE_AGUA*cantidad_leds_encendidos/16 < gasto_actual < TOPE_AGUA*(cantidad_leds_encendidos+1)/16
  int cantidad_leds_a_encender = floor((gasto_actual/TOPE_AGUA)*100/6.25);
  Serial.print("Cantidad de leds encendidos: ");
  Serial.println(cantidad_leds_a_encender);

  
}

void loop()
{
  // obtener frecuencia en Hz
  float frequency = GetFrequency();
  time_t contador_tiempo;
  // calcular caudal L/min
  float flow_Lmin = frequency / factorK;
  if (flow_Lmin > 0) {
    //momento-fecha en la que el grifo se abre
    contador_tiempo = now(); //establecido en tiempo de unix (segundos desde 1/1/1970)
    String fecha_apertura = getFecha(contador_tiempo);
    //Serial.println(fecha_apertura);

    //calcular caudal L/s
    //Serial.println("Cálculos de Luismi");
    Serial.print("Litros por minuto: ");
    Serial.print(flow_Lmin);
    Serial.println("");
    Serial.print("Litros por segundo: ");
    Serial.print(flow_Lmin/60);
    Serial.println("");
    gasto_total_luismi += flow_Lmin/60;
    leds(gasto_total_luismi);
    Serial.print("\t\t\t\tGasto acumulado: ");
    Serial.print(gasto_total_luismi);
    Serial.println(" (L/s)");
    Serial.println("");    
    Serial.println("******************");
    
    float litros_segundo = (flow_Lmin * measureInterval) / (60 * measureInterval);

    Serial.println(fecha_apertura);
    Serial.print("Frecuencia: ");
    Serial.print(frequency, 0);
    Serial.print(" (Hz)\t\t\tCaudal: ");
    Serial.print(litros_segundo, 3);
    Serial.println(" (L/s)");

    gasto_total += litros_segundo;
    grifoAbierto = true;

    //Serial.print("Litros acumulados: ");
    //Serial.print(gasto_total);
    //Serial.println("");

    
  }

  if (flow_Lmin == 0 && grifoAbierto) {
    momento = now();
    byte tiempo_transcurrido = float(momento - contador_tiempo);
    String fecha_cierre = getFecha(momento);
    //Serial.println(fecha_cierre);
    Serial.println("El grifo ha estado abierto: " + (String) tiempo_transcurrido + " segundos");
    //Serial.println(gasto_total);
    momento = 0;
    contador_tiempo = 0;
    gasto_total = 0;
    tiempo_transcurrido = 0;
    grifoAbierto = false;
  }
}
