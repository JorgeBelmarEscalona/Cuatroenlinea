
# Cuatro en linea 


## Descripcion
Este código implementa un juego de Cuatro en linea, completo con funcionalidad para servidor. El servidor gestiona las conexiones de los jugadores, el tablero del juego y la lógica del juego. Ademas de notificar a los clientes sobre los cambios en el tablero y el estado del juego. El juego termina cuando un jugador conecta cuatro fichas.

***Instrucciones para ejecutar el juego:***

**Compilación:**

- Abra una terminal en la carpeta del proyecto.
- Escriba el comando make y presione Enter.
- Esto compilará el servidor y el cliente del juego.

## Ejecución del servidor:

- Abra una nueva terminal en la carpeta del proyecto.
- Escriba el comando `./servidor 8888` y presione Enter.
- Esto iniciará el servidor del juego en el puerto 8888.


**Notas:**

Reemplace 8888 con el puerto que desee utilizar para el servidor si lo desea.
Asegúrese de tener instalado el compilador y las bibliotecas necesarias para compilar el juego.
Al momento de tener un ganador el servidor finalizara las conexiones con los clientes en 5 segundos, mas no terminara su ejecucion.


¡Diviértete jugando!
