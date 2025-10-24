# tdse-tp4-adc

1. Colocar el punto medio del potenciómetro en PA0 y los otros dos en 3V3 y GND.
2. Ajustar el potenciónmetro aproximadamente en la mitad de la posición.
3. Forkear el repositorio y abrir el proyecto en STM32CubeIDE.
4. Conectar la placa NUCLEO-F103RB.
5. Analizar la configuración del ADC1 configurado, sin modificarla.


## Test 1

6. Volver al código y configurar el primer test, verificando la directiva `#define TEST_NUMBER` (línea 18 de `app.c`), es decir que diga `#define TEST_NUMBER TEST_1`.
7. Compilar y debuggear. En la consola deben listarse 100 muestras.
8. Obtener la media y el desvío estándar. Se puede usar cualquier herramienta, por ejemplo Excel, GCalc, [We Do Data Science](https://www.wedodatascience.com), ...


## Test 2
9. Volver al código y configurar el segundo test, mediante la directiva `#define TEST_NUMBER TEST_2`.
10. Compilar y debuggear. En la consola deben listarse 100 muestras.
11. Obtener la media y el desvío estándar y comparar resultados.


## Test 3
12. Volver al código y configurar el tercer test, mediante la directiva `#define TEST_NUMBER TEST_3`.
13. Compilar y debuggear. En la consola deben listarse 100 muestras.
14. Comparar esta implementación con la del primer test.


*Nota:* Los comentarios deberán registrarse en los archivo adc.txt, test_1.txt, test_2.txt y test_3.txt, en el root de la carpeta app de su repositorio. Asimismo, en esa misma ubicación se debe incorporar todo material de soporte o argumentación, tal como capturas de pantalla de computadora, capturas de pantalla de osciloscopio, fotografías, etcétera.
