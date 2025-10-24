"""
Código que lee los archivos "TestX.txt" y calcula la media y el desvío 
estándar de los valores, ignorando su primera y última línea. 
Los resultados se guardan en un nuevo archivo "resultados.txt"
"""
import numpy as np
import glob
import logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
def procesar_archivos():
    archivos = glob.glob("Test*.txt")
    resultados: list[tuple[str, float, float]] = []

    for archivo in archivos:
        try:
            with open(archivo, 'r') as f:
                lineas = f.readlines()
                if len(lineas) <= 2:
                    logging.warning(f"El archivo {archivo} no tiene suficientes líneas para procesar.")
                    continue
                
                # Ignorar la primera y última línea
                datos = [float(linea.strip()) for linea in lineas[1:-1]]

                media: float = float(np.mean(datos))
                desvio: float = float(np.std(datos, ddof=1))  # Desvío estándar muestral

                resultados.append((archivo, media, desvio))
                logging.info(f"Procesado {archivo}: Media={media}, Desvío={desvio}")
        except Exception as e:
            logging.error(f"Error al procesar el archivo {archivo}: {e}")

    with open("resultados.txt", 'w') as f:
        for archivo, media, desvio in resultados:
            f.write(f"{archivo}: Media={media}, Desvío={desvio}\n")
    logging.info("Resultados escritos en resultados.txt")

if __name__ == "__main__":
    procesar_archivos()



