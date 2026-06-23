/*
 * calculoColorimetria.h
 * --------------------------------------------------------------------------
 * Biblioteca de colorimetria CIE para un espectro de 380-780nm en pasos de
 * 5nm (81 puntos), pensada para correr en un ESP32/ESP32-C6 dentro de un
 * proyecto ESPHome (componente AS7341 + reconstruccion espectral propia).
 *
 * Calcula, a partir de un espectro de potencia/radiancia relativa:
 *   - Tristimulo CIE XYZ
 *   - Coordenadas de cromaticidad CIE 1931 (x,y)
 *   - Coordenadas CIE 1960 UCS (u,v)
 *   - CCT (temperatura de color correlacionada) por el metodo oficial
 *     de Robertson (1968), el mismo que usan los colorimetros comerciales
 *   - Duv (distancia al lugar planckiano: que tan "fuera de blanco" esta
 *     la fuente; valores |Duv| > 0.05 indican que el concepto de CCT deja
 *     de ser fisicamente significativo)
 *   - CRI general Ra y los 8 indices individuales R1-R8, segun el metodo
 *     oficial CIE 13.3-1995 (8 muestras de color de referencia, iluminante
 *     de referencia por cuerpo negro o serie D segun corresponda, adaptacion
 *     cromática y espacio CIE 1964 U*V*W*)
 *
 * Todas las tablas numericas (observador CIE 1931 2 grados, muestras de
 * color TCS01-TCS08, vectores S0/S1/S2 del iluminante D) provienen de los
 * datasets estandar CIE 15:2004 / CIE 13.3-1995, verificados contra la
 * biblioteca de referencia "colour-science".
 *
 * NOTA IMPORTANTE SOBRE EL CRI: el metodo Ra esta diseñado para evaluarse
 * con un espectro medido con resolucion fina (este es justamente tu caso,
 * 5nm de 380 a 780nm). Aun asi, recuerda que tu espectro de entrada es una
 * RECONSTRUCCION a partir de los 8-10 canales del AS7341, no una medicion
 * directa de un espectrometro de rejilla. La calidad final del CCT/Ra
 * depende totalmente de cuan buena sea esa reconstruccion.
 * --------------------------------------------------------------------------
 */

#pragma once

#define PSPECTRO_N 81  // 380 a 780 nm, paso de 5 nm

struct ColorimetryResult {
  // Tristimulo (escala relativa, no normalizada a Y=100; sirve para x,y,u,v)
  float X, Y, Z;

  // Cromaticidad
  float x, y;    // CIE 1931 (x,y)
  float u, v;    // CIE 1960 UCS (u,v)

  // Temperatura de color
  float CCT;        // Kelvin
  float Duv;         // distancia al lugar planckiano (+ por encima/verde, - por debajo/magenta)
  bool  cct_reliable; // false si |Duv| > 0.05 (fuente muy alejada del blanco)

  // Indice de rendimiento de color
  float Ra;      // CRI general (promedio de R1..R8)
  float Ri[8];   // R1..R8 individuales (TCS01..TCS08)
};

// Funcion principal: le pasas tu espectro reconstruido (380-780nm, 5nm,
// en cualquier unidad relativa de potencia/radiancia) y te devuelve todo.
ColorimetryResult calcular_colorimetria(const float spectrum[PSPECTRO_N]);

// ---- Funciones auxiliares (expuestas por si las quieres usar sueltas) ----

// Integra el espectro contra las funciones de igualacion de color CIE 1931.
// Nota: el resultado NO esta normalizado a Y=100; sirve para razones (x,y,u,v).
void spectrum_to_XYZ(const float spectrum[PSPECTRO_N], float &X, float &Y, float &Z);

void XYZ_to_xy(float X, float Y, float Z, float &x, float &y);
void XYZ_to_uv1960(float X, float Y, float Z, float &u, float &v);

// CCT por el metodo de Robertson (1968). Valido aproximadamente para
// CCT entre 1667K y temperaturas muy altas (sin limite superior real).
float uv_to_CCT_Robertson(float u, float v, float &Duv);
