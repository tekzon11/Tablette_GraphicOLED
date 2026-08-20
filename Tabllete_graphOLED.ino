#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Configuration de l'écran OLED (I2C)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Serveur Web sur le port 80 et WebSocket sur le port 81
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Interface HTML/CSS/JS stockée dans la mémoire Flash (PROGMEM)
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Tablette OLED ESP8266</title>
  <style>
    * { box-sizing: border-box; touch-action: none; }
    body {
      margin: 0; padding: 20px; background-color: #121212; color: #ffffff;
      font-family: Arial, sans-serif; display: flex; flex-direction: column;
      align-items: center; justify-content: center; min-height: 100vh;
    }
    h2 { margin-bottom: 10px; font-size: 1.2rem; }
    #status { font-size: 0.9rem; margin-bottom: 15px; color: #ff5252; }
    #status.connected { color: #4caf50; }
    canvas {
      border: 2px solid #333; border-radius: 8px; background-color: #000000;
      width: 100%; max-width: 384px; aspect-ratio: 2 / 1; cursor: crosshair;
    }
    .controls { margin-top: 20px; width: 100%; max-width: 384px; }
    button {
      width: 100%; padding: 15px; font-size: 1rem; font-weight: bold;
      color: #ffffff; background-color: #d32f2f; border: none; border-radius: 6px; cursor: pointer;
    }
    button:active { background-color: #9a0007; }
  </style>
</head>
<body>
  <h2>Tablette Graphique OLED</h2>
  <div id="status">Déconnecté</div>

  <canvas id="paintCanvas" width="128" height="64"></canvas>

  <div class="controls">
    <button onclick="clearCanvas()">Effacer l'écran</button>
  </div>

  <script>
    const canvas = document.getElementById('paintCanvas');
    const ctx = canvas.getContext('2d');
    const statusDiv = document.getElementById('status');
    let isDrawing = false, lastX = 0, lastY = 0, socket;

    ctx.strokeStyle = '#FFFFFF';
    ctx.lineWidth = 1;
    ctx.lineCap = 'round';

    function initWebSocket() {
      // Connexion au WebSocket sur le même hôte que la page web, port 81
      socket = new WebSocket('ws://' + window.location.hostname + ':81/');

      socket.onopen = () => {
        statusDiv.textContent = 'Connecté à l\'ESP8266';
        statusDiv.classList.add('connected');
      };

      socket.onclose = () => {
        statusDiv.textContent = 'Déconnecté - Tentative...';
        statusDiv.classList.remove('connected');
        setTimeout(initWebSocket, 2000);
      };
    }

    function getCanvasCoordinates(e) {
      const rect = canvas.getBoundingClientRect();
      const clientX = e.touches ? e.touches[0].clientX : e.clientX;
      const clientY = e.touches ? e.touches[0].clientY : e.clientY;
      const scaleX = canvas.width / rect.width;
      const scaleY = canvas.height / rect.height;
      return {
        x: Math.floor((clientX - rect.left) * scaleX),
        y: Math.floor((clientY - rect.top) * scaleY)
      };
    }

    function startDrawing(e) {
      isDrawing = true;
      const coords = getCanvasCoordinates(e);
      lastX = coords.x;
      lastY = coords.y;
    }

    function draw(e) {
      if (!isDrawing) return;
      e.preventDefault();
      const coords = getCanvasCoordinates(e);

      ctx.beginPath();
      ctx.moveTo(lastX, lastY);
      ctx.lineTo(coords.x, coords.y);
      ctx.stroke();

      if (socket && socket.readyState === WebSocket.OPEN) {
        socket.send(`${lastX},${lastY},${coords.x},${coords.y}`);
      }

      lastX = coords.x;
      lastY = coords.y;
    }

    function stopDrawing() { isDrawing = false; }

    function clearCanvas() {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      if (socket && socket.readyState === WebSocket.OPEN) {
        socket.send('CLEAR');
      }
    }

    canvas.addEventListener('touchstart', startDrawing, { passive: false });
    canvas.addEventListener('touchmove', draw, { passive: false });
    canvas.addEventListener('touchend', stopDrawing);
    canvas.addEventListener('mousedown', startDrawing);
    canvas.addEventListener('mousemove', draw);
    canvas.addEventListener('mouseup', stopDrawing);
    canvas.addEventListener('mouseleave', stopDrawing);

    window.onload = initWebSocket;
  </script>
</body>
</html>
)rawliteral";

// Gestion des messages reçus via WebSockets
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String data = String((char*)payload);
    if (data == "CLEAR") {
      display.clearDisplay();
      display.display();
    } else {
      int x1, y1, x2, y2;
      if (sscanf(data.c_str(), "%d,%d,%d,%d", &x1, &y1, &x2, &y2) == 4) {
        display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
        display.display();
      }
    }
  }
}

// Service de la page HTML
void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void setup() {
  // Initialisation de l'écran OLED (Adresse I2C 0x3C)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;); // Bloque le programme si l'écran n'est pas détecté
  }
  display.clearDisplay();
  display.display();

  // Création du point d'accès Wi-Fi
  WiFi.softAP("Tablette-OLED", "12345678");

  // Démarrage du serveur Web HTTP
  server.on("/", handleRoot);
  server.begin();

  // Démarrage du serveur WebSockets
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}