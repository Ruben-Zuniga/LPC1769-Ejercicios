import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Configura el puerto serial (ajusta el nombre del puerto y la velocidad de baudios según sea necesario)
port = 'COM3'  # Cambia esto al puerto correspondiente
baudrate = 9600

# Inicializa la conexión serial
ser = serial.Serial(port, baudrate)

# Configura la figura para graficar
plt.style.use('fivethirtyeight')
fig, ax = plt.subplots()
x_data, y_data = [], []

def update(frame):
    # Lee un valor del puerto serial
    line = ser.readline().decode('utf-8').strip()
    try:
        # Intenta convertir el valor en un número (float o int)
        y = float(line)
        x_data.append(len(x_data))  # Tiempo o índice en el eje x
        y_data.append(y)            # Valor en el eje y
        ax.clear()
        ax.plot(x_data, y_data)
        ax.set_title("Gráfico en tiempo real desde USB")
        ax.set_xlabel("Tiempo")
        ax.set_ylabel("Valor")
    except ValueError:
        pass  # Ignora líneas que no sean valores numéricos

# Configura la animación para actualizar la gráfica en tiempo real
ani = FuncAnimation(fig, update, interval=100)  # Actualiza cada 100 ms

plt.show()

# Cierra el puerto serial al terminar
ser.close()
