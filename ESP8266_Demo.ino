/*
    ESP8266 Demonstrateur
    This program allow to test different functionality of ESP8266 NodeMCU module
    Based on different example
    Initial version in 2016 by Thierry Dezormeaux
*/

#include "ESP8266WiFi.h"
#include <dht.h>
#include "pwd.h" //ssid & password in separate file

dht DHT;            /* Create DHT object */
#define DHT11_PIN 5 //The data I/O pin connected to the DHT11 sensor : GPIO5 = D1 of NodeMCU ESP8266 Board

WiFiServer server(80); //declare a web server running on port 80

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done");
}

void loop() {
  Serial.println("Enter a request  : ");
  listRequestAvailable();

  while (true) { // On boucle sans fin
    // si des ordres sont envoyés sur le port série, on analyse
    if (Serial.available() > 0) {
      int command = Serial.parseInt();
      Serial.print(" Command received: "); Serial.print(command);
      switch (command) {
        case 1:
          Serial.println(" = scanWifi requested ");
          scanWifi();
          Serial.println("=> Command 1 Done");
          break;

        case 2:
          Serial.println(" = Blink requested ");
          ledBlink();
          Serial.println("=> Command 2 Done");
          break;

        case 3:
          Serial.println(" = connectWifi ");
          connectWifi();
          Serial.println("=> Command 3 Done");
          break;

        case 4:
          Serial.println(" = connexion web server ");
          connectWebServer();
          Serial.println("=> Command 4 Done");
          break;

        case 5:
          Serial.println(" = connexion thingSpeak");
          connectThingSPeak();
          Serial.println("=> Command 5 Done");
          break;

        case 6:
          Serial.println(" = temperature");
          temperature();
          Serial.println("=> Command 6 Done");
          break;

        case 7:
          Serial.println(" = temperature en boucle");
          temperatureBcle();
          Serial.println("=> Command 7 Done");
          break;

        case 8:
          Serial.println(" = web server to remote control LED");
          webServer();
          Serial.println("=> Command 8 Done");
          break;

        default:
          Serial.println(" => Command not found");
          break;
      }
      Serial.println(""); Serial.println("Enter a new request  : ");
      listRequestAvailable();
    }

    // Wait a bit before loop again
    delay(50);
  }
}

void listRequestAvailable() {
  Serial.println("  1 = scanWifi requested ");
  Serial.println("  2 = Blink requested ");
  Serial.println("  3 = connectWifi ");
  Serial.println("  4 = connexion web server ");
  Serial.println("  5 = connexion thingSpeak");
  Serial.println("  6 = temperature");
  Serial.println("  7 = temperature en boucle (entrez un caractère pour arrêter");
  Serial.println("  8 = Web Serveur to control LED");
}

void scanWifi() {
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
}

// blink 10 times
void ledBlink() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
    delay(250);                      // Wait for 250ms
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    delay(250);                      // Wait for 250ms (to demonstrate the active low LED)
  }
}

void connectWifi() {

  // We start by connecting to a WiFi network

  Serial.print("Connexion au WiFi ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);   // On se connecte

  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i < 40) { // On attend max 20 s
    delay(500);
    Serial.print(".");
    i++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");  // on affiche les paramÃ¨tres
    Serial.println("WiFi connecté");
    Serial.print("Adresse IP du module EPC: ");
    Serial.println(WiFi.localIP());
    Serial.print("Adresse IP de la box : ");
    Serial.println(WiFi.gatewayIP());
  } else {
    Serial.println("");
    Serial.println("WiFi Not connected");
    Serial.println("");
  }
}

/*
    This function sends data via HTTP GET requests to api.tom.tools service.

*/

void connectWebServer() {
  // valeurs pour le serveur Web
  const char* host     = "api.tom.tools";

  Serial.print("Connexion au serveur : ");
  Serial.println(host);

  // On se place dans le rôle du  client en utilisant WifiClient
  WiFiClient client;

  // le serveur Web attend tradionnellement sur le port 80
  const int httpPort = 80;

  // Si la connexio échoue ca sera pour la prochaine fois
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }


  // La connexion a réussie on forme le chemin
  String url = String("/hits/");

  Serial.print("demande URL: ");
  Serial.println(url);

  // On l'envoie au serveur sur plusieurs lignes
  // GET /compteur.php HTTP/1.1
  // Hosts: outils.plido.net
  // Connection: close
  //
  // La première ligne précise à la fin version du protocole attendu
  // La deuxième rappelle au serveur sous quel nom on l'appelle, en
  // effet, à une même adresse IP on peut avoire différents serveurs
  // repondant à des noms différents.
  // La troisième ligne indique que le serveur doit fermer la
  // connexion apres la réponse et ne pas attendre d'autres requêtes.
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  // On attend 10 mili-secondes
  delay(1000);

  // On lit les données reçues, s'il y en a
  while (client.available()) {
    String line = client.readStringUntil('\r'); // découpe ligne par ligne
    Serial.print(line);
  }

  // plus de données
  Serial.println();
  Serial.println("connexion fermée");

}

void connectThingSPeak() {
  // valeurs pour le serveur Web
  const char* host     = "api.thingspeak.com";
  const char* apikey   = "your API key";
  const char* town     = "Rennes,fr";

  String keyword = String("\"latitude\":"); //chaîne que l'on recherche dans le JSON

  // drapeau indiquant pendant l'analyse de la réponse du serveur
  // si on est dans l'en-tête HTTP (false) ou dans le contenu de
  // la ressource.
  bool inBody = false;
  float temperature = 0; /* lue du fichier JSON reçu */

  Serial.print("Connexion au serveur : ");
  Serial.println(host);

  // On se place dans le rôle du  client en utilisant WifiClient
  WiFiClient client;

  // le serveur Web attend tradionnellement sur le port 80
  const int httpPort = 80;

  // Si la connexion échoue ca sera pour la prochaine fois
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // La connexion a réussie on forme le chemin
  // URL  complexe composé du chemin et de deux
  // questions contenant le nom de ville et l'API key

  String url = String("/channels/9/feeds.json?results=10");// + town + "&appid=" + apikey;

  Serial.print("demande URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  // On attend 1 seconde
  delay(10000);

  inBody = false; // on est dans l'en-tête

  // On lit les données reçues, s'il y en a
  while (client.available()) {
    String line = client.readStringUntil('\r');

    if (line.length() == 1) inBody = true; /* passer l'en-tête jusqu'à une ligne vide */
    if (inBody) {  // ligne du corps du message, on cherche le mot clé
      int pos = line.indexOf(keyword);

      if (pos > 0) { /* mot clÃ© trouvÃ© */
        // indexOf donne la position du début du mot clé, en ajoutant sa longueur
        // on se place à la fin.
        pos += keyword.length();

        Serial.println (&line[pos]);

        temperature = atof(&line[pos]);

      } /* fin récupération du flotant */
    } /* fin de la recherche du mot clé */
  } /* fin data avalaible */

  Serial.println();//Serial.print ("Temperature = "); Serial.println(temperature-273.15); // temp en Kelvin

  Serial.println("connexion fermée");
}

void temperatureBcle() {
  while (!Serial.available())
  {
    Serial.print(millis());
    Serial.print(", \t");
    temperature();
    delay(10000); //toutes les 10 secondes
  }
}

void temperature() {
  // READ DATA
  Serial.print("DHT11, \t");
  int chk = DHT.read11(DHT11_PIN);

  // DISPLAY DATA
  Serial.print(DHT.humidity, 1);
  Serial.print(",\t");
  Serial.println(DHT.temperature, 1);

}

void webServer() {

  // Start the server
  server.begin();
  Serial.println("Server started");

  while (true)
  { 
    WiFiClient client;
    
    // Check if a client has connected
    while (!client) {
      client = server.available();
    }

    // Wait until the client sends some data
    Serial.println("new client");
    while (!client.available()) {
      delay(1);
    }

    // Read the first line of the request
    String req = client.readStringUntil('\r');
    Serial.println(req);
    client.flush();

    // Match the request
    if (req.indexOf("/on") != -1) {
      digitalWrite(LED_BUILTIN, LOW);
    }
    else if (req.indexOf("/off") != -1) {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (req.indexOf("/quitter") != -1) {
      return;
    }

    client.flush();

    // Prepare the response
    String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    s += "<head>";
    s += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    s += "<script src=\"https://code.jquery.com/jquery-2.1.3.min.js\"></script>";
    s += "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css\">";
    s += "</head>";
    s += "<div class=\"container\">";
    s += "<h1>Lamp Control</h1>";
    s += "<div class=\"row\">";
    s += "<div class=\"col-md-2\"><input class=\"btn btn-block btn-lg btn-primary\" type=\"button\" value=\"On\" onclick=\"on()\"></div>";
    s += "<div class=\"col-md-2\"><input class=\"btn btn-block btn-lg btn-danger\" type=\"button\" value=\"Off\" onclick=\"off()\"></div>";
    s += "<div class=\"col-md-2\"><input class=\"btn btn-block btn-lg btn-info\" type=\"button\" value=\"Quitter\" onclick=\"quitter()\"></div>";
    s += "</div></div>";
    s += "<script>function on() {$.get(\"/on\");}</script>";
    s += "<script>function off() {$.get(\"/off\");}</script>";
    s += "<script>function quitter() {$.get(\"/quitter\");}</script>";
    
    // Send the response to the client
    client.print(s);
    delay(1);
    Serial.println("Client disconnected");

    // The client will actually be disconnected
    // when the function returns and 'client' object is detroyed
  }
}

