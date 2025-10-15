import re
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
from datetime import datetime
import threading
import queue
import time

try:
    import serial
    SERIAL_AVAILABLE = True
except ImportError:
    print("pyserial não instalado. Use: pip install pyserial")
    SERIAL_AVAILABLE = False

class BatSignalMonitor:
    def __init__(self, port='COM3', baudrate=115200, max_points=50):
        self.max_points = max_points
        self.data_queue = queue.Queue()
        self.serial_running = True
        self.start_time = None
        self.monitoring_started = False
        self.current_cycle_data = {}  # Armazena todos os dados do ciclo atual
        self.awaiting_cycle_completion = False
        
        # Arrays para dados (apenas inteiros)
        self.timestamps = deque(maxlen=max_points)
        self.seconds = deque(maxlen=max_points)
        self.temperature = deque(maxlen=max_points)
        self.humidity = deque(maxlen=max_points)
        self.accel_x = deque(maxlen=max_points)
        self.accel_y = deque(maxlen=max_points)
        self.accel_z = deque(maxlen=max_points)
        
        # Configurar tema dark moderno
        self.setup_dark_theme()
        self.setup_plots()
        self.ser = self.connect_serial(port, baudrate)
        
    def setup_dark_theme(self):
        """Configura o tema dark moderno"""
        plt.style.use('dark_background')
        self.colors = {
            'background': '#0d1117',
            'card': '#161b22',
            'accent': '#58a6ff',
            'success': '#3fb950',
            'warning': '#d29922',
            'danger': '#f85149',
            'text_primary': '#f0f6fc',
            'text_secondary': '#8b949e',
            'grid': '#30363d',
            'temp': '#ff7b72',
            'humidity': '#79c0ff',
            'accel_x': '#7ee787',
            'accel_y': '#ffa657',
            'accel_z': '#d2a8ff',
        }
        
    def connect_serial(self, port, baudrate):
        """Conecta à porta serial da STM32"""
        if not SERIAL_AVAILABLE:
            print("pyserial não disponível.")
            return None
            
        try:
            ser = serial.Serial(
                port=port,
                baudrate=baudrate,
                timeout=1,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE
            )
            print(f"Conectado à {port} - {baudrate} bps")
            print("Aguardando dados da STM32...")
            return ser
        except Exception as e:
            print(f"Erro ao conectar à {port}: {e}")
            print("Soluções:")
            print("1. Feche o PuTTY e outras apps usando COM3")
            print("2. Verifique se a porta está correta")
            print("3. Execute como Administrador")
            return None
    
    def parse_stm32_line(self, line):
        """Extrai dados da linha da STM32"""
        try:
            # Verifica se é o primeiro READ SENSORS
            if 'Current State -> 1 - READ SENSORS' in line and not self.monitoring_started:
                self.start_time = datetime.now()
                self.monitoring_started = True
                self.awaiting_cycle_completion = True
                self.current_cycle_data = {}  # Inicia novo ciclo
                return {'state': 'start_monitoring'}
            
            # Se é um novo ciclo de READ SENSORS
            if 'Current State -> 1 - READ SENSORS' in line:
                self.awaiting_cycle_completion = True
                self.current_cycle_data = {}  # Inicia novo ciclo
                return None
            
            # Temperatura
            temp_match = re.search(r'Temperature ---->\s*(-?\d+)\s*C', line)
            if temp_match:
                self.current_cycle_data['temperature'] = int(temp_match.group(1))
                return None
            
            # Humidade
            hum_match = re.search(r'Humidity ------->\s*(-?\d+)\s*%', line)
            if hum_match:
                self.current_cycle_data['humidity'] = int(hum_match.group(1))
                return None
            
            # Acelerações
            accel_x_match = re.search(r'X Acceleration ->\s*(-?\d+)\s*mg', line)
            if accel_x_match:
                self.current_cycle_data['accel_x'] = int(accel_x_match.group(1))
                return None
            
            accel_y_match = re.search(r'Y Acceleration ->\s*(-?\d+)\s*mg', line)
            if accel_y_match:
                self.current_cycle_data['accel_y'] = int(accel_y_match.group(1))
                return None
            
            accel_z_match = re.search(r'Z Acceleration ->\s*(-?\d+)\s*mg', line)
            if accel_z_match:
                self.current_cycle_data['accel_z'] = int(accel_z_match.group(1))
                return None
            
            # Se chegou ao COMMS, o ciclo está completo
            if 'Current State -> 2 - COMMS' in line and self.awaiting_cycle_completion:
                if self.current_cycle_data:  # Se temos dados do ciclo
                    self.current_cycle_data['timestamp'] = datetime.now()
                    self.awaiting_cycle_completion = False
                    return self.current_cycle_data.copy()  # Envia ponto completo
            
            return None
            
        except Exception as e:
            print(f"Erro no parse: {e} - Linha: {line}")
            return None
    
    def read_serial_data(self):
        """Lê dados da STM32 LINHA A LINHA"""
        if not self.ser:
            print("Porta serial não disponível.")
            return
            
        buffer = ""
        while self.serial_running:
            try:
                if self.ser and self.ser.in_waiting:
                    # Lê dados disponíveis
                    data = self.ser.read(self.ser.in_waiting).decode('utf-8', errors='ignore')
                    buffer += data
                    
                    # Processa CADA LINHA individualmente
                    while '\n' in buffer:
                        line, buffer = buffer.split('\n', 1)
                        line = line.strip()
                        
                        if line:
                            print(f"Linha recebida: {line}")
                            parsed_data = self.parse_stm32_line(line)
                            if parsed_data:
                                # Envia APENAS UM PONTO COMPLETO por ciclo
                                self.data_queue.put(parsed_data)
                                print(f"✅ Ponto completo preparado: {parsed_data}")
                                
            except Exception as e:
                print(f"Erro serial: {e}")
                time.sleep(1)
    
    def setup_plots(self):
        """Configura os 3 gráficos com design moderno e dark"""
        self.fig = plt.figure(figsize=(16, 10), facecolor=self.colors['background'])
        
        # Título principal com design moderno
        self.fig.suptitle(
            'BAT-SIGNAL • REAL-TIME MONITORING PLATFORM\n',
            fontsize=18,
            fontweight='bold',
            color=self.colors['text_primary'],
            y=0.985,
            fontfamily='monospace'
        )
        
        # Ajusta o layout para dar mais espaço ao título
        plt.subplots_adjust(top=0.88, bottom=0.08, hspace=0.4, wspace=0.3)
        
        # Gráfico 1: Temperatura - Design moderno
        self.ax1 = plt.subplot(2, 2, 1, facecolor=self.colors['card'])
        self.ax1.set_title('TEMPERATURE', fontweight='bold', 
                          color=self.colors['temp'], fontsize=14, pad=20)
        self.ax1.set_ylabel('Temperature (°C)', color=self.colors['text_primary'], fontsize=11)
        self.ax1.set_xlabel('Time (seconds)', color=self.colors['text_secondary'], fontsize=10)
        self.ax1.grid(True, color=self.colors['grid'], alpha=0.3, linestyle='--')
        self.line_temp, = self.ax1.plot([], [], color=self.colors['temp'], 
                                       linewidth=3, marker='o', markersize=6,
                                       markerfacecolor=self.colors['temp'],
                                       markeredgecolor='white', markeredgewidth=1)
        self.ax1.tick_params(colors=self.colors['text_secondary'])
        self.ax1.yaxis.set_major_locator(plt.MaxNLocator(integer=True))
        
        # Valor atual da temperatura
        self.temp_text = self.ax1.text(0.02, 0.925, 'Current: --°C', 
                                      transform=self.ax1.transAxes, color=self.colors['temp'],
                                      fontsize=12, fontweight='bold',
                                      bbox=dict(boxstyle="round,pad=0.3", 
                                              facecolor=self.colors['background'], 
                                              alpha=0.8))
        
        # Gráfico 2: Humidade - Design moderno
        self.ax2 = plt.subplot(2, 2, 2, facecolor=self.colors['card'])
        self.ax2.set_title('HUMIDITY', fontweight='bold', 
                          color=self.colors['humidity'], fontsize=14, pad=20)
        self.ax2.set_ylabel('Humidity (%)', color=self.colors['text_primary'], fontsize=11)
        self.ax2.set_xlabel('Time (seconds)', color=self.colors['text_secondary'], fontsize=10)
        self.ax2.grid(True, color=self.colors['grid'], alpha=0.3, linestyle='--')
        self.line_hum, = self.ax2.plot([], [], color=self.colors['humidity'], 
                                      linewidth=3, marker='s', markersize=6,
                                      markerfacecolor=self.colors['humidity'],
                                      markeredgecolor='white', markeredgewidth=1)
        self.ax2.tick_params(colors=self.colors['text_secondary'])
        self.ax2.yaxis.set_major_locator(plt.MaxNLocator(integer=True))
        
        # Valor atual da humidade
        self.hum_text = self.ax2.text(0.02, 0.925, 'Current: --%', 
                                     transform=self.ax2.transAxes, color=self.colors['humidity'],
                                     fontsize=12, fontweight='bold',
                                     bbox=dict(boxstyle="round,pad=0.3", 
                                             facecolor=self.colors['background'], 
                                             alpha=0.8))
        
        # Gráfico 3: Aceleração XYZ - Design moderno
        self.ax3 = plt.subplot(2, 1, 2, facecolor=self.colors['card'])
        self.ax3.set_title('ACCELERATION • XYZ AXES', fontweight='bold', 
                          color=self.colors['accent'], fontsize=14, pad=20)
        self.ax3.set_ylabel('Acceleration (mg)', color=self.colors['text_primary'], fontsize=11)
        self.ax3.set_xlabel('Time (seconds)', color=self.colors['text_secondary'], fontsize=10)
        self.ax3.grid(True, color=self.colors['grid'], alpha=0.3, linestyle='--')
        
        # Linhas com cores modernas
        self.line_x, = self.ax3.plot([], [], color=self.colors['accel_x'], 
                                    linewidth=2.5, label='X-Axis', alpha=0.9)
        self.line_y, = self.ax3.plot([], [], color=self.colors['accel_y'], 
                                    linewidth=2.5, label='Y-Axis', alpha=0.9)
        self.line_z, = self.ax3.plot([], [], color=self.colors['accel_z'], 
                                    linewidth=2.5, label='Z-Axis', alpha=0.9)
        
        # Legenda moderna
        self.ax3.legend(loc='upper right', facecolor=self.colors['background'],
                       edgecolor=self.colors['grid'], fontsize=10)
        
        self.ax3.tick_params(colors=self.colors['text_secondary'])
        self.ax3.yaxis.set_major_locator(plt.MaxNLocator(integer=True))
        
        # Linha zero com estilo moderno
        self.ax3.axhline(y=0, color=self.colors['text_secondary'], 
                        linestyle='-', alpha=0.5, linewidth=1)
        
        # Valores atuais das acelerações
        self.accel_text = self.ax3.text(0.015, 0.925, 'X: --  Y: --  Z: --', 
                                       transform=self.ax3.transAxes, 
                                       color=self.colors['text_primary'],
                                       fontsize=11, fontweight='bold',
                                       bbox=dict(boxstyle="round,pad=0.3", 
                                               facecolor=self.colors['background'], 
                                               alpha=0.8))
    
    def get_elapsed_seconds(self, timestamp):
        """Calcula segundos desde o início do monitoring"""
        if self.start_time:
            return (timestamp - self.start_time).total_seconds()
        return 0
    
    def update_plots(self, frame):
        """Atualiza gráficos com dados da STM32 - UM PONTO POR CICLO"""
        # Processa APENAS UM ponto completo por vez
        if not self.data_queue.empty():
            data = self.data_queue.get()
            
            # Verifica se é o sinal para iniciar monitoring
            if 'state' in data and data['state'] == 'start_monitoring':
                print("⏱️  Contador de tempo iniciado em 0!")
                # Limpa dados anteriores se houver
                self.timestamps.clear()
                self.seconds.clear()
                self.temperature.clear()
                self.humidity.clear()
                self.accel_x.clear()
                self.accel_y.clear()
                self.accel_z.clear()
            else:
                # Adiciona APENAS UM PONTO com todos os dados do ciclo
                self.update_arrays(data)
                print(f"📊 Ponto adicionado | Total: {len(self.temperature)} | Tempo: {self.seconds[-1]:.1f}s")
        
        # Atualiza gráficos se tiver dados E monitoring iniciado
        if self.monitoring_started and self.temperature:
            # Usa SEGUNDOS no eixo X (começando em 0)
            x_data = list(self.seconds)
            
            # Gráfico Temperatura
            self.line_temp.set_data(x_data, list(self.temperature))
            self.ax1.relim()
            self.ax1.autoscale_view()
            self.temp_text.set_text(f'Current: {self.temperature[-1]}°C')
            
            # Gráfico Humidade
            self.line_hum.set_data(x_data, list(self.humidity))
            self.ax2.relim()
            self.ax2.autoscale_view()
            self.hum_text.set_text(f'Current: {self.humidity[-1]}%')
            
            # Gráfico Aceleração
            self.line_x.set_data(x_data, list(self.accel_x))
            self.line_y.set_data(x_data, list(self.accel_y))
            self.line_z.set_data(x_data, list(self.accel_z))
            self.ax3.relim()
            self.ax3.autoscale_view()
            self.accel_text.set_text(f'X: {self.accel_x[-1]}  Y: {self.accel_y[-1]}  Z: {self.accel_z[-1]}')
            
            # Atualiza título principal
            self.fig.suptitle(
                'BAT-SIGNAL • REAL-TIME MONITORING PLATFORM',
                fontsize=18,
                fontweight='bold',
                color=self.colors['text_primary'],
                y=0.985,
                fontfamily='monospace'
            )
        
        # Atualiza título se ainda não começou
        elif not self.monitoring_started:
            self.fig.suptitle(
                'BAT-SIGNAL • REAL-TIME MONITORING PLATFORM\n',
                fontsize=18,
                fontweight='bold',
                color=self.colors['text_primary'],
                y=0.985,
                fontfamily='monospace'
            )
    
    def update_arrays(self, data):
        """Atualiza arrays com UM PONTO COMPLETO do ciclo"""
        timestamp = data.get('timestamp', datetime.now())
        elapsed_seconds = self.get_elapsed_seconds(timestamp)
        
        # Adiciona APENAS UM PONTO com todos os dados
        self.timestamps.append(timestamp)
        self.seconds.append(elapsed_seconds)
        
        # Usa valores padrão se algum sensor não tiver dados
        self.temperature.append(data.get('temperature', self.temperature[-1] if self.temperature else 0))
        self.humidity.append(data.get('humidity', self.humidity[-1] if self.humidity else 0))
        self.accel_x.append(data.get('accel_x', self.accel_x[-1] if self.accel_x else 0))
        self.accel_y.append(data.get('accel_y', self.accel_y[-1] if self.accel_y else 0))
        self.accel_z.append(data.get('accel_z', self.accel_z[-1] if self.accel_z else 0))
    
    def start(self):
        """Inicia a aplicação"""
        print("=" * 65)
        print("BAT-SIGNAL • REAL-TIME MONITORING PLATFORM")
        print("Porta: COM3 | Baudrate: 115200")
        print("📈 MODO: 1 PONTO POR CICLO DE LEITURA")
        print("⏱️  Tempo começará em 0 no primeiro READ SENSORS")
        print("=" * 65)
        
        # Thread para ler dados
        data_thread = threading.Thread(target=self.read_serial_data, daemon=True)
        data_thread.start()
        
        # Animação dos gráficos
        ani = animation.FuncAnimation(
            self.fig, self.update_plots, interval=100, cache_frame_data=False
        )
        
        try:
            plt.show()
        except KeyboardInterrupt:
            print("\nBat-Signal monitoring stopped")
        finally:
            self.serial_running = False
            if self.ser:
                self.ser.close()

# Executar a aplicação
if __name__ == "__main__":
    monitor = BatSignalMonitor(port='COM3', baudrate=115200)
    monitor.start()