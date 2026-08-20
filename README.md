# Tablette_GraphicOLED

Un tel projet repose sur quatre briques fondamentales : le composant matériel, l'architecture réseau sans fil, le code embarqué sur l'ESP8266 et l'application mobile.

1. Composants matériels requis

    ESP8266 (NodeMCU V3 ou Wemos D1 Mini)
    Écran OLED SSD1306 (0,96" ou 1,3", résolution 128x64 pixels, bus I2C)
    Plaque d'essai (breadboard) et câbles Dupont (Fremelle-Femelle ou Mâle-Femelle)

   2. Câblage (I2C)

   Écran OLED (SSD1306)ESP8266 (NodeMCU)
   VCC    3.3V
   GND    GND
   SDA    D2 (GPIO4)
   SCL    D1 (GPIO5)

   3. Architecture réseau & communication

   Pour obtenir un affichage temps réel fluide :
     Point d'accès Wi-Fi (SoftAP) : L'ESP8266 génère son propre réseau Wi-Fi (ex: Tablette-OLED). Le téléphone s'y connecte directement.
     Protocole WebSockets : Contrairement au protocole HTTP standard (trop lent), le WebSocket garde un canal TCP ouvert en permanence pour transmettre les coordonnées (X, Y) du dessin instantanément       avec une latence minimale.

   4. Code ESP8266 (Arduino IDE)Installe les bibliothèques Adafruit SSD1306, Adafruit GFX et WebSockets par Markus Sattler dans le gestionnaire de bibliothèques Arduino.
