# Tarea 1 Sistemas Operativos

- Guillermo Oliva Orellana
- Matías Olivas Henríquez
- Fabián González Uribe

## Compilación

Para la compilación del código se requiere usar el comando "make" en la línea de comandos.

## Ejecución

Para ejecutarlo, se inicia el archivo main en la terminal con "./main" y estará lista para su uso.

## Tests

En el caso de querer hacer tests, primero se debe ejecutar el comando "test" en la shell personalizada, el cual genera texto completamente aleatorio de distintos largos.
Luego, se debe ejecutar el comando "batchrun cmds.txt" que prueba con la instrucción "sort" todos los textos generados y los guarda en dos archivos, last\_profile.csv y profile.txt;

- last\_profile.csv: Contiene los datos crudos de tiempo de usuario, sistema, real y memoria máxima residente para poder graficar los resultados
- profile.txt: Contiene la salida de cada ejecución de "miprof" en los casos de prueba
