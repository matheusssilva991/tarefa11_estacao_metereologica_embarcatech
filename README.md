# 🌡️ Estação Meteorológica Embarcada

Uma estação meteorológica completa desenvolvida para **Raspberry Pi Pico W** com interface web moderna e monitoramento em tempo real.

## 📋 Descrição do Projeto

Este projeto implementa uma estação meteorológica embarcada utilizando **Raspberry Pi Pico W** que coleta dados de sensores ambientais e disponibiliza uma interface web responsiva para visualização e configuração em tempo real. O sistema oferece um dashboard moderno com gráficos interativos, configurações personalizáveis e histórico de dados.

## ⚡ Funcionalidades

### 📊 **Monitoramento em Tempo Real**
- **Temperatura** com suporte a offset de calibração
- **Umidade relativa** do ar
- **Pressão atmosférica**
- **Altitude estimada** (baseada na pressão)

### 🎨 **Interface Web Moderna**
- Dashboard responsivo com **Tailwind CSS**
- Gráficos interativos com **ApexCharts**
- Tema escuro otimizado
- Ícones **Font Awesome**
- Atualizações automáticas a cada segundo

### ⚙️ **Configurações Avançadas**
- Limites mínimo e máximo de temperatura
- Offset de calibração para sensores
- Persistência de configurações
- Validação de dados em tempo real

### 📈 **Visualização de Dados**
- Gráficos históricos das últimas 20 medições
- Alternância entre temperatura, umidade e pressão
- Animações suaves e responsivas
- Indicadores visuais de status

### 🌐 **Conectividade**
- Servidor web integrado
- API REST para dados JSON
- Suporte a múltiplas conexões simultâneas
- WiFi integrado do Pico W

## 🛠️ Hardware Utilizado

- **Microcontrolador**: Raspberry Pi Pico W
- **Sensor de Pressão/Temperatura**: BMP280
- **Sensor de Umidade/Temperatura**: AHT20
- **Conectividade**: WiFi integrado (CYW43)

## 🚀 Como Rodar

### **Pré-requisitos**
- Raspberry Pi Pico W
- Sensores BMP280 e AHT20
- Ambiente de desenvolvimento C/C++ configurado
- SDK do Pico instalado

### **1. Clone o Repositório**
```bash
git clone https://github.com/matheusssilva991/tarefa11_estacao_metereologica_embarcatech
cd tarefa11_estacao_metereologica_embarcatech
```

### **2. Configuração do Hardware**
```
Pico W  →  BMP280
VCC     →  3.3V
GND     →  GND
SDA     →  GP4 (I2C SDA)
SCL     →  GP5 (I2C SCL)

Pico W  →  AHT20
VCC     →  3.3V
GND     →  GND
SDA     →  GP4 (I2C SDA)
SCL     →  GP5 (I2C SCL)
```

### **3. Configuração WiFi**
Edite o arquivo de configuração com suas credenciais:
```c
#define WIFI_SSID "Sua_Rede_WiFi"
#define WIFI_PASSWORD "Sua_Senha"
```

### **4. Compilação**
```bash
mkdir build
cd build
cmake ..
make
```

### **5. Upload para o Pico W**
```bash
# Conecte o Pico W em modo BOOTSEL
cp main.uf2 /media/RPI-RP2/
```

### **6. Acesso à Interface**
1. Abra o monitor serial para ver o IP atribuído
2. Acesse `http://IP_DO_PICO` no navegador
3. Visualize os dados em tempo real!

### **7. Teste Local (Desenvolvimento)**
Para testar a interface localmente:
```bash
cd public
npm install express
node server.js
# Acesse http://localhost:3000
```

## 📱 Interface

### **Dashboard Principal**
- Cards com métricas atuais
- Status de conexão em tempo real
- Indicadores visuais coloridos

### **Configurações**
- Campos para limites de temperatura
- Offset de calibração (-10°C a +10°C)
- Botão de salvar com validação

### **Gráficos**
- Histórico das últimas 20 medições
- Alternância entre métricas
- Cores diferenciadas por tipo de dado

## 🎥 Vídeo de Demonstração

[![Demonstração da Estação Meteorológica](https://drive.google.com/file/d/1q2SlqfeNkkMLK1iP5iJRek6tT3ToE3Eb/view?usp=drive_link)](https://drive.google.com/file/d/1q2SlqfeNkkMLK1iP5iJRek6tT3ToE3Eb/view?usp=drive_link)

*Clique na imagem acima para assistir ao vídeo de demonstração completo*

### **O que você verá no vídeo:**
- ✅ Inicialização do sistema
- ✅ Conexão WiFi automática
- ✅ Interface web responsiva
- ✅ Dados atualizando em tempo real
- ✅ Configuração de limites e offset
- ✅ Gráficos interativos
- ✅ Múltiplas conexões simultâneas

## 🏗️ Arquitetura do Sistema

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Sensores      │    │   Raspberry      │    │   Interface     │
│                 │◄──►│   Pico W         │◄──►│   Web           │
│ • BMP280        │    │                  │    │                 │
│ • AHT20         │    │ • Servidor HTTP  │    │ • Dashboard     │
│                 │    │ • API REST       │    │ • Gráficos      │
│                 │    │ • WiFi           │    │ • Configurações │
└─────────────────┘    └──────────────────┘    └─────────────────┘
```

## 📁 Estrutura do Projeto

```
tarefa11_estacao_metereologica_embarcatech/
├── 📁 config/
│   └── lwipopts_examples_common.h    # Configurações de rede
├── 📁 public/
│   ├── html_data.h                   # HTML embarcado
│   ├── index.html                    # Interface de desenvolvimento
│   └── server.js                     # Servidor de teste
├── 📁 src/
│   ├── main.c                        # Código principal
│   ├── sensors.c                     # Funções dos sensores
│   └── web_server.c                  # Servidor HTTP
├── CMakeLists.txt                    # Configuração de build
├── pico_sdk_import.cmake             # SDK do Pico
└── README.md                         # Este arquivo
```

## 🔧 API Endpoints

| Método | Endpoint | Descrição |
|--------|----------|-----------|
| `GET` | `/` | Interface web principal |
| `GET` | `/api/weather` | Dados dos sensores (JSON) |
| `POST` | `/api/limits` | Salvar configurações |
| `GET` | `/api/status` | Status do sistema |

### **Exemplo de Resposta da API:**
```json
{
  "temperature": 25.3,
  "humidity": 65.8,
  "pressure": 1013.25,
  "altitude": 850,
  "minTemperature": 10,
  "maxTemperature": 70,
  "tempOffset": 0.5,
  "timestamp": "2025-01-21T10:30:00.000Z"
}
```

## 🏆 Desenvolvedores

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/matheusssilva991">
        <img src="https://avatars.githubusercontent.com/matheusssilva991" width="100px;" alt="Foto do Matheus"/><br>
        <sub>
          <b>Matheus Santos Silva</b>
        </sub>
      </a><br>
      <span title="Código">💻</span>
      <span title="Hardware">🔧</span>
      <span title="Interface">🎨</span>
    </td>
  </tr>
</table>
