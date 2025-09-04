# ESP32 + Dallas DS18B20 + AWS IoT Core MQTT

Este projeto demonstra como conectar um ESP32 com sensor de temperatura Dallas DS18B20 ao AWS IoT Core usando protocolo MQTT para envio de dados de telemetria.

## 📋 Índice

- [Características](#características)
- [Hardware Necessário](#hardware-necessário)
- [Bibliotecas Utilizadas](#bibliotecas-utilizadas)
- [Configuração da AWS](#configuração-da-aws)
- [Configuração do Projeto](#configuração-do-projeto)
- [Instalação](#instalação)
- [Uso](#uso)
- [Estrutura dos Dados](#estrutura-dos-dados)
- [Monitoramento](#monitoramento)
- [Troubleshooting](#troubleshooting)
- [Contribuição](#contribuição)
- [Licença](#licença)

## ✨ Características

- **Leitura de temperatura** em tempo real usando sensor Dallas DS18B20
- **Conexão segura** com AWS IoT Core via MQTT over TLS
- **Envio automático** de dados de telemetria a cada 5 segundos
- **Recepção de comandos** via tópicos MQTT
- **Reconexão automática** em caso de perda de conexão
- **Logs detalhados** para debug via Serial Monitor

## 🛠️ Hardware Necessário

| Componente | Quantidade | Descrição |
|------------|------------|-----------|
| ESP32 DevKit | 1 | Microcontrolador principal |
| Dallas DS18B20 | 1 | Sensor de temperatura digital |
| Resistor 4.7kΩ | 1 | Pull-up para o barramento OneWire |
| Protoboard | 1 | Para conexões |
| Jumpers | Vários | Para ligações |

### 🔌 Esquema de Conexões

```
ESP32          Dallas DS18B20
-----          --------------
3.3V     ----> VDD (vermelho)
GND      ----> GND (preto)
GPIO 15  ----> DATA (amarelo) + Resistor 4.7kΩ para 3.3V
```

## 📚 Bibliotecas Utilizadas

```ini
lib_deps = 
    knolleary/PubSubClient@^2.8
    paulstoffregen/OneWire@^2.3.7
    milesburton/DallasTemperature@^3.11.0
```

## ☁️ Configuração da AWS

### 1. Criar uma Thing no AWS IoT Core

1. Acesse o [AWS IoT Console](https://console.aws.amazon.com/iot/)
2. Vá em **Manage** > **Things** > **Create**
3. Escolha **Create a single thing**
4. Nome: `ESP32_DHT11`
5. Clique em **Next**

### 2. Criar Certificados

1. Escolha **Auto-generate a new certificate**
2. **Download** todos os certificados:
   - Device certificate (`.pem.crt`)
   - Private key (`.pem.key`)
   - Root CA certificate ([Amazon Root CA 1](https://www.amazontrust.com/repository/AmazonRootCA1.pem))

### 3. Criar Policy de Segurança

```json
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": "iot:Connect",
      "Resource": "arn:aws:iot:us-east-2:*:client/ESP32_DHT11"
    },
    {
      "Effect": "Allow",
      "Action": "iot:Publish",
      "Resource": "arn:aws:iot:us-east-2:*:topic/esp32/pub"
    },
    {
      "Effect": "Allow",
      "Action": "iot:Subscribe",
      "Resource": "arn:aws:iot:us-east-2:*:topicfilter/esp32/sub"
    },
    {
      "Effect": "Allow",
      "Action": "iot:Receive",
      "Resource": "arn:aws:iot:us-east-2:*:topic/esp32/sub"
    }
  ]
}
```

### 4. Anexar Policy ao Certificado

1. Vá em **Secure** > **Certificates**
2. Selecione seu certificado
3. **Actions** > **Attach policy**
4. Selecione a policy criada

## 🔧 Configuração do Projeto

### 1. Estrutura de Arquivos

```
projeto/
├── platformio.ini
├── src/
│   └── main.cpp
├── include/
│   └── secrets.h
└── README.md
```

### 2. Arquivo `platformio.ini`

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps = 
    knolleary/PubSubClient@^2.8
    paulstoffregen/OneWire@^2.3.7
    milesburton/DallasTemperature@^3.11.0
```

### 3. Arquivo `include/secrets.h`

```cpp
#include <pgmspace.h>
#define SECRET
#define THINGNAME "ESP32_DHT11"

const char WIFI_SSID[] = "SEU_WIFI_AQUI";
const char WIFI_PASSWORD[] = "SUA_SENHA_WIFI_AQUI";
const char AWS_IOT_ENDPOINT[] = "seu-endpoint-ats.iot.us-east-2.amazonaws.com";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
COLE_AQUI_SEU_ROOT_CA_CERTIFICATE
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
COLE_AQUI_SEU_DEVICE_CERTIFICATE
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
COLE_AQUI_SUA_PRIVATE_KEY
-----END RSA PRIVATE KEY-----
)KEY";
```

> ⚠️ **Importante**: Adicione `include/secrets.h` ao seu `.gitignore` para não versionar credenciais!

## 🚀 Instalação

1. **Clone o repositório**
   ```bash
   git clone https://github.com/seu-usuario/esp32-aws-iot-dallas.git
   cd esp32-aws-iot-dallas
   ```

2. **Configure o arquivo secrets.h**
   - Edite `include/secrets.h`
   - Adicione suas credenciais Wi-Fi
   - Cole os certificados AWS baixados

3. **Compile e upload**
   ```bash
   pio run --target upload
   ```

4. **Monitor serial**
   ```bash
   pio device monitor
   ```

## 📱 Uso

### Inicialização
Após o upload, o ESP32 irá:
1. Conectar ao Wi-Fi configurado
2. Estabelecer conexão segura com AWS IoT Core
3. Inicializar o sensor Dallas DS18B20
4. Começar a enviar dados automaticamente

### Logs Esperados
```
ESP32 Dallas DS18B20 + AWS IoT Starting...
Connecting to Wi-Fi
........
Wi-Fi connected!
Dallas Temperature sensor initialized
Connecting to AWS IOT
.....
AWS IoT Connected!
Temperature: 25.5°C / 77.9°F
Message published successfully!
```

## 📊 Estrutura dos Dados

### Dados Enviados (Publish)
**Tópico**: `esp32/pub`

```json
{
  "device": "ESP32_Dallas",
  "temperature_celsius": 25.5,
  "temperature_fahrenheit": 77.9,
  "timestamp": 123456789
}
```

### Comandos Recebidos (Subscribe)
**Tópico**: `esp32/sub`

```json
{
  "message": "Comando ou informação enviada para o dispositivo"
}
```

## 📈 Monitoramento

### AWS IoT Console
1. Vá para **Test** > **MQTT test client**
2. **Subscribe** ao tópico `esp32/pub` para ver os dados
3. **Publish** no tópico `esp32/sub` para enviar comandos

### Exemplo de Subscription
```bash
# Tópico: esp32/pub
# Qualidade de serviço: 0
```

### Exemplo de Publish (Comando)
```json
{
  "message": "Teste de comando do AWS Console"
}
```

## 🔧 Troubleshooting

### Problemas Comuns

| Problema | Solução |
|----------|---------|
| **Falha na conexão Wi-Fi** | Verifique SSID e senha em `secrets.h` |
| **Erro de certificados** | Confirme se certificados foram colados corretamente |
| **Sensor não detectado** | Verifique conexões e resistor pull-up de 4.7kΩ |
| **AWS IoT timeout** | Confirme endpoint e policy de segurança |
| **Falha no publish** | Verifique se o tópico está correto na policy |

### Debug Avançado

1. **Ativar logs detalhados** no PlatformIO:
   ```ini
   build_flags = -DCORE_DEBUG_LEVEL=4
   ```

2. **Verificar conexão MQTT**:
   - Use ferramentas como MQTT Explorer
   - Monitore o AWS CloudWatch Logs

3. **Testar certificados**:
   ```bash
   openssl x509 -in certificate.pem.crt -text -noout
   ```

## 🤝 Contribuição

1. Fork o projeto
2. Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

## 📄 Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

## 👨‍💻 Autor

**Thiago de Sena**

<table> <tr> <td align="center"> <img src="https://github.com/user-attachments/assets/65acf7c7-8a0d-4dbb-9472-d22afee52b31" width="120px;" alt="Foto de Thiago de Sena"/><br> <sub><b></b></sub><br>  </td> </tr> </table>

- GitHub: [@ThiagoDeSena](https://github.com/ThiagoDeSena)
- LinkedIn: [Thiago De Sena](https://www.linkedin.com/in/thiago-de-sena-developer/)

## 🙏 Agradecimentos

- [AWS IoT Core Documentation](https://docs.aws.amazon.com/iot/)
- [PubSubClient Library](https://github.com/knolleary/pubsubclient)
- [DallasTemperature Library](https://github.com/milesburton/Arduino-Temperature-Control-Library)

---
⭐ Se este projeto te ajudou, considere dar uma estrela no repositório!
