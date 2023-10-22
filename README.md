# demo_expo

## Hardware utilizado: 
- ESP32-WROVER-E.
## S0:
- Freertos
## Recursos utilizados:
- 2 Salidas digitales para accionamiento de leds.
- 1 Salida digital para accionamiento de Relé Vege.
- 1 entrada ADC para lectura de potenciómetro.
- 1 salida de PWM para control de driver de potencia de luces led.

## Funcionamiento:
- Led Power ON: para indicar que el equipo se encuentra encendido.
- Led Rele VEGE ON:
   - En ON indica Relé Vege ACTIVADO.
   - En OFF indica Relé  Vege DESACTIVADO.

- El ciclo empieza con la salida de PWM en 10% de ciclo de trabajo e incrementa un 20% cada un tiempo T en segundos configurado mediante el potenciómetro. El tiempo T se encuentra en el Rango 5 - 15 segundos según el ajuste del potenciómetro.
- Se queda el tiempo T segundos al 100%.
- Se activa el relé vege T segundos.
- Empieza a descender la salida de PWM en decrementos de 20% cada T segundos.
- Al 0% se apaga el relé Vege y empieza el ciclo nuevamente.
- El tiempo entre pasos se ajusta con el potenciómetro. Cuando el
potenciómetro se encuentra a la mitad, T= 10 seg. Cuando se encuentra al máximo, T= 15 seg y cuando se encuentra
al mínimo, T= 15 seg.
