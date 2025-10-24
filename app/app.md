Este código es un proyecto para un microcontrolador STM32 que prueba tres métodos diferentes para leer una señal analógica usando el ADC (Conversor Analógico a Digital). El archivo está diseñado para compilar y ejecutar solo **una** de las tres pruebas a la vez, seleccionada por la macro `TEST_NUMBER`.

El objetivo general es tomar 100 "muestras" de la señal analógica y registrar el tiempo total que tomó el proceso. La gran diferencia entre las pruebas es **cómo** se obtienen esas muestras y **cuánto trabajo se le da al CPU**.

Aquí está el análisis detallado de cada caso:

---

### Cómo funciona la Conversión A/D

El ADC es un periférico que mide un voltaje (una señal analógica, por ejemplo, de un sensor de temperatura o un potenciómetro) y lo convierte en un número digital (generalmente de 0 a 4095 en estos micros).

Este código utiliza dos técnicas fundamentalmente diferentes para gestionar esta conversión:

1.  **Medición por Sondeo (Polling) - (TEST 1 y TEST 2):**
    * El CPU le dice al ADC: "Inicia una conversión".
    * El CPU se queda "esperando activamente" (se bloquea), preguntando en un bucle: "¿Terminaste? ¿Terminaste? ¿Terminaste?".
    * Cuando el ADC termina, el CPU toma el valor y sigue.
    * **Desventaja:** El CPU desperdicia todo su tiempo esperando. Si la conversión tarda 20 microsegundos, el CPU se bloquea por 20 microsegundos.

2.  **Medición por Interrupción - (TEST 3):**
    * El CPU le dice al ADC: "Inicia una conversión y avísame cuando termines".
    * El CPU **no espera** y queda libre para hacer otras tareas.
    * Cuando el ADC termina, "interrumpe" al CPU.
    * El CPU pausa lo que estaba haciendo, salta a una función especial (la *Callback*), guarda el valor rápidamente y vuelve a sus tareas.
    * **Ventaja:** Es la forma más eficiente. El CPU solo trabaja al inicio y al final de la conversión.

---

### Análisis de Cada Caso

#### `TEST_NUMBER == TEST_1`: Medición Simple por Sondeo

En este modo, el código ejecuta `test1_tick()`.

* **Funcionamiento:**
    1.  `app_update()` llama a `test1_tick()`.
    2.  `test1_tick()` llama a `ADC_Poll_Read()`.
    3.  `ADC_Poll_Read()` **bloquea el CPU**:
        * `HAL_ADC_Start()`: Inicia una conversión.
        * `HAL_ADC_PollForConversion()`: El CPU espera aquí sin hacer nada más hasta que la conversión termine.
        * `HAL_ADC_GetValue()`: Lee el resultado digital (ej. `2048`).
    4.  `test1_tick()` recibe el valor.
    5.  `LOGGER_LOG("%u\n", value)`: Imprime el valor inmediatamente.
    6.  Esto se repite 100 veces.
* **Procesamiento:** Ninguno. Simplemente lee el valor crudo del ADC y lo imprime.
* **Resultado:** Verás 100 números impresos en la consola, uno tras otro. Esta prueba será **lenta** porque el CPU se bloquea 100 veces (una por cada muestra). El tiempo total medido (en *ticks*) será significativo.

---

#### `TEST_NUMBER == TEST_2`: Medición por Sondeo con Promediado

En este modo, el código ejecuta `test2_tick()`.

* **Funcionamiento:**
    1.  `app_update()` llama a `test2_tick()`.
    2.  Para obtener **una sola muestra** (de las 100), entra en un bucle `for` que se repite 16 veces (`AVERAGER_SIZE`).
    3.  Dentro de ese bucle, llama a `ADC_Poll_Read()` 16 veces. Cada una de estas llamadas **bloquea el CPU** (inicia, espera, lee).
    4.  Suma los 16 valores en la variable `averaged`.
    5.  Después del bucle `for`, divide la suma total por 16 para obtener el promedio.
    6.  `LOGGER_LOG("%lu\n", averaged)`: Imprime ese valor promediado.
    7.  Esto se repite 100 veces.
* **Procesamiento:** **Sobremuestreo y Promediado (Oversampling & Averaging)**.
    * **¿Para qué sirve?** Para reducir el ruido. Si la señal analógica tiene pequeñas vibraciones (ruido), tomar 16 muestras muy rápido y promediarlas da un valor mucho más estable y preciso.
* **Resultado:** Verás 100 números impresos, pero esta vez estarán "suavizados" por el promedio. Esta prueba será **extremadamente lenta**, mucho más que el TEST 1, porque el CPU se bloquea $16 \times 100 = 1600$ veces. El tiempo total medido será el más alto de todos.

---

#### `TEST_NUMBER == TEST_3`: Medición por Interrupción (Modo Eficiente)

Este es el modo más avanzado y el que está seleccionado por defecto (`TEST_NUMBER TEST_3`).

* **Funcionamiento:** Es un "baile" entre el bucle principal (`test3_tick`) y la interrupción (`HAL_ADC_ConvCpltCallback`).
    1.  **Inicio:** `app_init()` configura y activa la interrupción del ADC. `test3_tick()` se llama por primera vez, pone el *flag* `b_trig_new_conversion` en `true`.
    2.  **Bucle Principal (`test3_tick`):**
        * Ve el *flag* `b_trig_new_conversion`.
        * Limpia el *flag* (`false`).
        * Llama a `HAL_ADC_Start_IT()`. Esta función **regresa al instante**. Le dice al ADC "empieza" y el CPU sigue su camino.
    3.  **CPU Libre:** El CPU sale de `app_update` y queda libre.
    4.  **Interrupción (Hardware):** Unos microsegundos después, el ADC termina la conversión.
    5.  **Callback (`HAL_ADC_ConvCpltCallback`):** El ADC "interrumpe" al CPU. El código salta automáticamente a esta función.
        * Lee el valor con `HAL_ADC_GetValue()`.
        * Guarda el valor en el *array* `sample_array[sample_idx]`.
        * Incrementa `sample_idx`.
        * Si `sample_idx` es menor que 100, pone el *flag* `b_trig_new_conversion` en `true` para avisarle al bucle principal que inicie la *siguiente* conversión.
    6.  **Se repite:** El bucle principal (Paso 2) ve el *flag* y arranca la siguiente conversión. Este ciclo se repite 100 veces.
* **Finalización:**
    * Cuando la interrupción guarda la muestra 99, `sample_idx` se vuelve 100. Ya no activa el *flag*.
    * La próxima vez que corre `test3_tick()`, ve que `sample_idx >= SAMPLES_COUNTER`.
    * En este punto, entra en un bucle `for` e imprime **todos los 100 valores** del `sample_array` de una sola vez.
    * Retorna `true` para finalizar la prueba.
* **Procesamiento:** **Almacenamiento en Búfer (Buffering)**. Las muestras se recolectan de forma asíncrona (sin bloquear al CPU) y se guardan en un búfer (el `sample_array`) para ser procesadas (impresas) todas juntas al final.
* **Resultado:** Esta será la prueba **más rápida**. El CPU casi no trabaja, solo da la orden de inicio y recoge el resultado. El tiempo total medido será el más bajo. Verás los 100 valores impresos de golpe al final de la prueba.