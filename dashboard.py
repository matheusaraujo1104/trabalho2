import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget, QLabel
from PyQt5.QtCore import QTimer
import pyqtgraph as pg
import serial  # Biblioteca para ler a porta serial

# =====================
# CONFIGURAÇÃO SERIAL
# =====================
# Substitua 'COM3' pela porta correta da sua placa no Windows
# Para ver a porta, abra o Gerenciador de Dispositivos → Portas (COM & LPT)
SERIAL_PORT = "COM3"

# Substitua 115200 pelo baud rate do seu programa na placa
BAUD_RATE = 115200

# Inicializa a conexão serial
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
except Exception as e:
    print(f"Erro ao abrir a porta serial: {e}")
    ser = None  # Se der erro, o programa ainda vai rodar mas com valores 0


class SensorDashboard(QMainWindow):
    def _init_(self):
        super()._init_()

        self.setWindowTitle("Sensor Dashboard")
        self.setGeometry(100, 100, 600, 400)

        # Widget principal
        self.main_widget = QWidget()
        self.setCentralWidget(self.main_widget)

        # Layout
        self.layout = QVBoxLayout()
        self.main_widget.setLayout(self.layout)

        # Label para valor atual
        self.label = QLabel("Corrente: -- A")
        self.label.setStyleSheet("font-size: 24px;")
        self.layout.addWidget(self.label)

        # Gráfico
        self.plot_widget = pg.PlotWidget(title="Corrente ao longo do tempo")
        self.plot_widget.setYRange(0, 100)  # ajuste conforme seu sensor
        self.layout.addWidget(self.plot_widget)

        self.data = []  # lista para guardar os últimos valores
        self.x = list(range(50))  # eixo x fixo para últimos 50 pontos
        self.curve = self.plot_widget.plot(self.x, [0]*50, pen='g')

        # Timer para atualizar o gráfico
        self.timer = QTimer()
        self.timer.setInterval(1000)  # 1 segundo
        self.timer.timeout.connect(self.update_plot)
        self.timer.start()

    def read_sensor(self):
        """Lê o valor da corrente da porta serial."""
        if ser is None:
            return 0  # se a serial não abriu, retorna 0
        try:
            line = ser.readline().decode().strip()  # lê uma linha da serial
            if line == "":
                return 0  # se nada chegou, retorna 0
            return float(line)  # converte para número
        except Exception as e:
            print(f"Erro ao ler a serial: {e}")
            return 0

    def update_plot(self):
        value = self.read_sensor()
        self.label.setText(f"Corrente: {value} A")
        self.data.append(value)
        if len(self.data) > 50:
            self.data.pop(0)
        self.curve.setData(self.x, self.data + [0]*(50-len(self.data)))


if _name_ == "_main_":
    app = QApplication(sys.argv)
    window = SensorDashboard()
    window.show()
    sys.exit(app.exec_())