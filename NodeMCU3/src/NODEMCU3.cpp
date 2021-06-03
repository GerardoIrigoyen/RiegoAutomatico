#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SoftwareSerial.h>
#include <RTClib.h>
#include <Time.h>
#include <Hash.h>
#include "LittleFS.h"
#include <iostream>
using namespace std;

#define RELAY_NC false
#define NUM_RELAYS 3
#define NUM_SECTORES 2
int relayGPIOs[NUM_RELAYS] = {13, 14, 12};
int Sector[NUM_SECTORES] = {1, 2};

const char *PARAM_INPUT_1 = "relay";
const char *PARAM_INPUT_2 = "state";
const char *PARAM_INT = "inputInt";
String inputMessage;
String inputParam;
String inputMessage2;
String inputParam2;

const char *ssid = "Patio";
const char *password = "12033986";


IPAddress local_IP(192, 168, 0, 14);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);


IPAddress subnet(255, 255, 255, 0);

IPAddress primaryDNS(192, 168, 1, 1);   //optional
IPAddress secondaryDNS(0, 0, 0, 0); //optional

#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float t = 0.0;
float h = 0.0;

AsyncWebServer server(80);

unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
const long interval = 10000;

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
DateTime now;
DateTime UltimoRiego;
String UltimoRiegoDia;
String Ultimavez = "Todavia no se rego";
String MensajeRiego = "";
char UltimoMesRiego2digitos[12];
char UltimoDia2digitos[12];
char UltimaHoraRiego2digitos[12];
char UltimoMinutoRiego2digitos[12];

unsigned long InicioRiegoSector1 = 0;
unsigned long InicioRiegoSector2 = 0;
unsigned long TiempodeRiego = 0;
unsigned long TiempodeRiego2 = 0;
float DuracionRiego;
String tiempominutos = "";
int Seconds_To_Convert;

const int LitrosHoraBomba = 5000; //CAMBIAR POR LOS LITROS HORA DE LA BOMBA
int yourInputInt;

boolean PAUSA = 0;

boolean sector1 = 0;
boolean sector2 = 0;
boolean RIEGOOK = 0;

String mlriego = "1";
String valorboton1;
String valorboton2;
String valorboton3;

int seconds;
int minute;

int rssi;
String conexion;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1 ">
   <link rel="icon" href="data:,">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    h2 { font-size: 2rem; color:white}
    h3 { font-size: 3rem; color:white; padding:0;}
    .units { font-size: 1rem; }
    .dht-labels{
      font-size: 1rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    input[type=number], select {
  width: 20%;
  padding: 10px 15px;
  margin: 8px 0;
  display: inline-block;
  border: 1px solid #ccc;
  border-radius: 4px;
  box-sizing: border-box;
}

input[type=submit] {
  width: 25%;
  background-color: #215e2a;
  color: white;
  padding: 5px 5px;
  margin: 8px 0;
  border: none;
  border-radius: 4px;
  cursor: pointer;
}

    .logo{text-align: right; font-size: 1rem;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #949494; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2AAD0C}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
    .grande{font-size: 2rem; color:green}   
   .topnav { overflow: hidden; background-color: #1b78e2;}
   .topnav2 {
            border-radius: 10px;
            background: #73AD21;
            padding: 12px; 
            width: 40px;
            height: 1px;
            font-weight: bold;
            font-size: 1rem;  
            color:white}
    .content { padding: 20px;}
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);}
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));}
    .card-bme{ color: #000000;}
    .slider2 {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #949494; border-radius: 34px}
    .slider2:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider2 {background-color: #ad0c0c}
    input:checked+.slider2:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}

  </style>
</head>
<body>
  <div class="topnav">
  <h2>RIEGO AUTOMATICO</h2>
  </div>
    <div class="content">
      <div class="cards">
        <div class="card card-bme">
          <i class="fas fa-wifi"></i><span class="dht-labels">&nbsp;&nbsp;Conectado a red WiFi:&nbsp;</span><span id="SSID">%ssid%</span>
            <span class="dht-labels">&nbsp;&nbsp;&nbsp;&nbsp;Se&#241al:&nbsp;</span><span id="CONEXION">%conexion%</span>
        </div>
      <div class="card card-bme">
        <i class="fas fa-thermometer-half" style="color:#059e8a;"></i><span class="dht-labels">&nbsp;&nbsp;Temperatura:&nbsp;</span><span id="temperature">%TEMPERATURE%</span><span class="units">&#x2103 &nbsp; &nbsp;</span>
        <i class="fas fa-tint" style="color:#00add6;"></i><span class="dht-labels">&nbsp;&nbsp;Humedad:&nbsp;</span><span id="humidity">%HUMIDITY%</span><span class="units">&#x25</span>
       </div>
        <div class="card card-bme">
          <iclass="dht-labels">&nbsp;&nbsp;Riego Automatico:&nbsp;</span><span id="PROXIMO">%proximoriego%</span>
        </div>
          <div class="card card-bme">
            <iclass="dht-labels">&nbsp;&nbsp;Ultimo Riego:&nbsp;</span><span id="ULTIMO">%ultimavez%</span>
          </div>
          <div class="card card-bme">
            <form action="\get" target="hidden-form"  onsubmit="return validateForm()">Ingresar ml a regar: <input type="number" name="inputInt"value="" min="1" max="5" id="userInput">
            <IFRAME style="display:none" name="hidden-form"></IFRAME>
            <input type="submit" value="INGRESAR" onclick="" id="INPUT">
        
           <p>
            <iclass="dht-labels">Por sector se va a regar:&nbsp;</span>
             <span class="topnav2">
                <span id="CANTIDADRIEGO"> %MLRIEGO%</span>
               <span>
                   ml
              </span>
             </span>
           </p>
           <br /> 
          <p>
            <iclass="dht-labels">Tiempo de riego por sector:&nbsp;</span>
            <span class="topnav2">  
              <span id="TIEMPORIEGO">%tiempominutos%</span>
              <span>
                min
              </span>
            </span>
          </p>
        
        </div>
      <div> 
      <span id="botones">%BUTTONPLACEHOLDER%</span>
      </div>
    
         <div class="logo">
         <i class="fab fa-glide-g"></i>
        </div>
    </div>
  
</body>
  <script>
    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
    };
    xhttp.open("GET", "/temperature", true);
    xhttp.send();
    }, 10000 ) ;

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
    };
    xhttp.open("GET", "/humidity", true);
    xhttp.send();
    }, 10000 ) ;

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("SSID").innerHTML = this.responseText;
    }
    };
    xhttp.open("GET", "/SSID", true);
    xhttp.send();
    }, 10000 ) ;

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ULTIMO").innerHTML = this.responseText;
    }
    };
    xhttp.open("GET", "/ULTIMO", true);
    xhttp.send();
    }, 2000 ) ;

     setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("PROXIMO").innerHTML = this.responseText;
    }
    };
    xhttp.open("GET", "/PROXIMO", true);
    xhttp.send();
    }, 2000 ) ;

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("CANTIDADRIEGO").innerHTML = this.responseText;
    }
    };
    xhttp.open("GET", "/CANTIDADRIEGO", true);
    xhttp.send();
    }, 1000 ) ;

     setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("1").checked = this.responseText;
    }
    };
    xhttp.open("GET", "/valorboton1", true);
    xhttp.send();
    }, 1000 ) ;

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("2").checked = this.responseText;
    }
    };
    xhttp.open("GET", "/valorboton2", true);
    xhttp.send();
    }, 1000 ) ;

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("3").checked = this.responseText;
    }
    };
    xhttp.open("GET", "/valorboton3", true);
    xhttp.send();
    }, 1000 ) ;

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("PROXIMO").innerHTML = this.responseText;
    }
    };
    xhttp.open("GET", "/PROXIMO", true);
    xhttp.send();
    }, 2000 ) ;

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("CONEXION").innerHTML = this.responseText;
    }
    };
    xhttp.open("GET", "/CONEXION", true);
    xhttp.send();
    }, 1000 ) ;


    function toggleCheckbox(element) {
    var xhr = new XMLHttpRequest();
    if(element.checked){ xhr.open("GET", "/update?relay="+element.id+"&state=1", true); }
    else { xhr.open("GET", "/update?relay="+element.id+"&state=0", true);}
    xhr.send();
    }

    setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("TIEMPORIEGO").innerHTML = this.responseText;
    }
    };
    xhttp.open("GET", "/TIEMPORIEGO", true);
    xhttp.send();
    }, 1000 ) ;

    function validateForm() {
    var a = document.getElementById("userInput").value;
    if (a == null || a == "" || a=="0") {
      alert("Completa todos los campos Jorgito");
      return false;
        }
      }

  </script>

</html>)rawliteral";

void Convert(int value, int &minute, int &seconds)
{
  minute = (value % 3600) / 60; // Minute component
  seconds = value % 60;         // Second component
}

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

String readFile(fs::FS &fs, const char *path)
{

  File file = fs.open(path, "r");
  if (!file || file.isDirectory())
  {
    Serial.println("- empty file or failed to open file");
    return String();
  }
  String fileContent;
  while (file.available())
  {
    fileContent += String((char)file.read());
  }
  return fileContent;
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- write failed");
  }
}

String relayState(int numRelay)
{
  if (RELAY_NC)
  {
    if (digitalRead(relayGPIOs[numRelay - 1]))
    {
      return "checked";
    }
    else
    {
      return "";
    }
  }
  else
  {
    if (digitalRead(relayGPIOs[numRelay - 1]))
    {
      return "";
    }
    else
    {
      return "checked";
    }
  }
  return "";
}

String relayState2(int numRelay) // ESTO LO HICE PARA QUE EL BOTON DE PAUSAR RIEGO QUEDE APRETADO CUANDO SE ACTUALIZA LA PAGINA
{
  if (RELAY_NC)
  {
    if (PAUSA == 1)
    {
      return "";
    }
    else
    {
      return "checked";
    }
  }
  else
  {
    if (PAUSA == 1)
    {
      valorboton3 = "checked";
      return "checked";
    }
    else
    {
      valorboton3 = "";
      return "";
    }
  }
  return "";
}

String processor(const String &var)
{
  if (var == "TEMPERATURE")
  {
    return String(t);
  }
  else if (var == "HUMIDITY")
  {
    return String(h);
  }
  else if (var == "ssid")
  {
    return String(ssid);
  }
  else if (var == "ultimavez")
  {
    return String(Ultimavez);
  }
  else if (var == "proximoriego")
  {
    return String(MensajeRiego);
  }
  else if (var == "MLRIEGO")
  {
    return String(mlriego);
  }
  else if (var == "tiempominutos")
  {
    return String(tiempominutos);
  }
  else if (var == "conexion")
  {
    return String(conexion);
  }

  else if (var == "BUTTONPLACEHOLDER")
  {
    String buttons = "";
    for (int i = 1; i <= (NUM_RELAYS); i++)
    {
      String relayStateValue = relayState(i);
      if (i == 3)
      {
        String relayStateValue2 = relayState2(3);
        buttons += "<h4>PAUSAR RIEGO AUTOMATICO "
                   "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" +
                   String(i) + "\" " + relayStateValue2 + "><span class=\"slider2\"></span></label>";
      }
      else
      {
        buttons += "<h4>SECTOR " + String(i) + "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\" " + relayStateValue + "><span class=\"slider\"></span></label>";
      }
    }

    return buttons;
  }
  return String();
}

void FECHAULTIMORIEGO()
{
  UltimoRiego = rtc.now();
  UltimoRiegoDia = daysOfTheWeek[UltimoRiego.dayOfTheWeek()];
  sprintf(UltimoMesRiego2digitos, "%02u", UltimoRiego.month());
  sprintf(UltimoDia2digitos, "%02u", UltimoRiego.day());
  sprintf(UltimaHoraRiego2digitos, "%02u", UltimoRiego.hour());
  sprintf(UltimoMinutoRiego2digitos, "%02u", UltimoRiego.minute());
}

void printDate()
{
  Serial.print(now.year());
  Serial.print('/');
  Serial.print(now.month());
  Serial.print('/');
  Serial.print(now.day());
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour());
  Serial.print(':');
  Serial.print(now.minute());
  Serial.print(':');
  Serial.print(now.second());
  Serial.println();
}

void REGAR()
{
  int PAUSA2 = 0;
  int PAUSA3 = 0;
  PAUSA2 = inputMessage.toInt();
  PAUSA3 = inputMessage2.toInt();
  if (!(PAUSA == 1))
  {
    InicioRiegoSector1 = millis();
    TiempodeRiego = InicioRiegoSector1 + DuracionRiego;
    while (!(PAUSA2 == 3 && PAUSA3 == 1) && (InicioRiegoSector1 < TiempodeRiego))
    {
      PAUSA2 = inputMessage.toInt();
      PAUSA3 = inputMessage2.toInt();
      RIEGOOK = 1;
      valorboton1 = "checked";
      valorboton2 = "";
      digitalWrite(relayGPIOs[0], LOW);
      digitalWrite(relayGPIOs[1], HIGH);
      digitalWrite(relayGPIOs[2], LOW);
      InicioRiegoSector1 = millis();
      yield();
      MensajeRiego = "ACTIVADO";
    }
    valorboton1 = "";
    digitalWrite(relayGPIOs[0], HIGH);
    InicioRiegoSector2 = millis();
    TiempodeRiego2 = InicioRiegoSector2 + DuracionRiego;
    while (!(PAUSA2 == 3 && PAUSA3 == 1) && (InicioRiegoSector2 < TiempodeRiego2))
    {
      PAUSA2 = inputMessage.toInt();
      PAUSA3 = inputMessage2.toInt();
      RIEGOOK = 1;
      valorboton2 = "checked";
      valorboton1 = "";
      digitalWrite(relayGPIOs[0], HIGH);
      digitalWrite(relayGPIOs[1], LOW);
      digitalWrite(relayGPIOs[2], LOW);
      InicioRiegoSector2 = millis();
      yield();
      MensajeRiego = "ACTIVADO";
    }
    valorboton2 = "";
    RIEGOOK = 0;
    digitalWrite(relayGPIOs[1], HIGH);
    digitalWrite(relayGPIOs[2], HIGH);
    FECHAULTIMORIEGO();
    Ultimavez = UltimoRiegoDia + " " + UltimoDia2digitos + "/" + UltimoMesRiego2digitos + " a las " + UltimaHoraRiego2digitos + ":" + UltimoMinutoRiego2digitos;
  }
}

void TIEMPORIEGO()
{
  // 200 por los 20 metros cudrados; 60 por minutos; 60 por segundos; 1000 por milisegundos
  DuracionRiego = (mlriego.toFloat() * 200 * 60 * 60 * 1000) / LitrosHoraBomba;
  Seconds_To_Convert = DuracionRiego / 1000;
  Convert(Seconds_To_Convert, minute, seconds);
  String minute1 = String(minute);
  String seconds2 = String(seconds);
  if (seconds == 0)
  {
    tiempominutos = minute1;
  }
  else
  {
    tiempominutos = minute1 + ":" + seconds2;
  }
}

void RIEGOAUTOMATICO()
{
  rssi = WiFi.RSSI();
  if (rssi > -10)
  {
    conexion = "Excelente";
  }
  if ((rssi < -10) && (rssi > -30))
  {
    conexion = "Muy buena";
  }
  if ((rssi < -30) && (rssi > -60))
  {
    conexion = "Buena";
  }
  if ((rssi < -60) && (rssi > -70))
  {
    conexion = "Mala";
  }
  if (rssi < -70)
  {
    conexion = "Muy mala";
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
    previousMillis = currentMillis;
    now = rtc.now();
    if (now.month() == 11 || now.month() == 12 || now.month() == 1 || now.month() == 2 || now.month() == 3 || now.month() == 10)
    {
      if (PAUSA == 0)
      {
        MensajeRiego = "Lunes/Miercoles/Viernes a las 06:00";
      }
      if (now.dayOfTheWeek() == 1 || now.dayOfTheWeek() == 3 || now.dayOfTheWeek() == 6)
      {
        if (now.hour() == 16 && now.minute() >= 41 && now.minute() < 42)
        {
          Serial.println("Regando");
          REGAR();
        }
      }
    }
    else
    {
      if (PAUSA == 0)
      {
        MensajeRiego = "Miercoles a las 06:00";
      }
      if (now.dayOfTheWeek() == 3)
      {
        if (now.hour() == 6 && now.minute() >= 0 && now.minute() < 5)
        {
          REGAR();
        }
      }
    }
    printDate();
  }
}

void CONTROLBOTONES()
{
  if (RIEGOOK == 0)
  {
    if (inputMessage.toInt() == 1 && inputMessage2.toInt() == 1)
    {
      digitalWrite(relayGPIOs[0], LOW);
      sector1 = 1;
      valorboton1 = "checked";
    }
    if (inputMessage.toInt() == 1 && inputMessage2.toInt() == 0)
    {
      digitalWrite(relayGPIOs[0], HIGH);
      sector1 = 0;
      valorboton1 = "";
    }
    if (inputMessage.toInt() == 2 && inputMessage2.toInt() == 1)
    {
      valorboton2 = "checked";
      digitalWrite(relayGPIOs[1], LOW);
      sector2 = 1;
    }
    if (inputMessage.toInt() == 2 && inputMessage2.toInt() == 0)
    {
      valorboton2 = "";
      digitalWrite(relayGPIOs[1], HIGH);
      sector2 = 0;
    }
    if ((sector1) || (sector2))
    {
      digitalWrite(relayGPIOs[2], LOW);
      FECHAULTIMORIEGO();
      Ultimavez = UltimoRiegoDia + " " + UltimoDia2digitos + "/" + UltimoMesRiego2digitos + " a las " + UltimaHoraRiego2digitos + ":" + UltimoMinutoRiego2digitos;
    }
    else
    {
      digitalWrite(relayGPIOs[2], HIGH);
    }
  }
  if (!(inputMessage.toInt() == 3))
  {
    RIEGOAUTOMATICO();
  }
  else
  {
    if (inputMessage2.toInt() == 1)
    {
      if ((valorboton1 == "") && (valorboton2 == ""))
      {
        sector1 = 0;
        sector2 = 0;
      }
      PAUSA = 1;
      valorboton3 = "checked";
      Serial.println("ACTIVADO");
      FECHAULTIMORIEGO();
      MensajeRiego = "Pausado el dia " + UltimoRiegoDia + " " + UltimoDia2digitos + "/" + UltimoMesRiego2digitos + " a las " + UltimaHoraRiego2digitos + ":" + UltimoMinutoRiego2digitos;
    }
    else
    {
      valorboton3 = "";
      PAUSA = 0;
      RIEGOAUTOMATICO();
    }
  }
}

void TYH()
{
  unsigned long currentMillis2 = millis();
  if (currentMillis2 - previousMillis2 >= interval)
  {
    previousMillis2 = currentMillis2;
    float newT = dht.readTemperature();
    if (isnan(newT))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
      t = newT;
      Serial.println(t);
    }
    float newH = dht.readHumidity();
    if (isnan(newH))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
      h = newH;
      Serial.println(h);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  if (!rtc.begin())
  {
    Serial.println("Modulo RTC no encontrado !");
    while (1)
      ;
  }

  for (int i = 1; i <= NUM_RELAYS; i++)
  {
    pinMode(relayGPIOs[i - 1], OUTPUT);
    if (RELAY_NC)
    {
      digitalWrite(relayGPIOs[i - 1], LOW);
    }
    else
    {
      digitalWrite(relayGPIOs[i - 1], HIGH);
    }
  }

  dht.begin();
  
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
  Serial.println("STA Failed to configure");
}

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(h).c_str());
  });
  server.on("/SSID", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(ssid).c_str());
  });
  server.on("/ULTIMO", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(Ultimavez).c_str());
  });
  server.on("/PROXIMO", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(MensajeRiego).c_str());
  });
  server.on("/valorboton1", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(valorboton1).c_str());
  });
  server.on("/valorboton2", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(valorboton2).c_str());
  });
  server.on("/valorboton3", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(valorboton3).c_str());
  });
  server.on("/PROXIMO", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(MensajeRiego).c_str());
  });
  server.on("/CANTIDADRIEGO", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(mlriego).c_str());
  });
  server.on("/TIEMPORIEGO", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(tiempominutos).c_str());
  });
  server.on("/CONEXION", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(conexion).c_str());
  });
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2))
    {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
      if (RELAY_NC)
      {
        Serial.print("NO ");
        digitalWrite(relayGPIOs[inputMessage.toInt() - 1], !inputMessage2.toInt());
      }
      else
      {
        Serial.print("NC ");
        Serial.println(inputMessage);
        Serial.println(inputMessage2);
      }
    }
    else
    {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage;
    // GET inputInt value on <ESP_IP>/get?inputInt=<inputMessage>
    if (request->hasParam(PARAM_INT))
    {
      inputMessage = request->getParam(PARAM_INT)->value();
      writeFile(LittleFS, "/inputInt.txt", inputMessage.c_str());
      yourInputInt = readFile(LittleFS, "/inputInt.txt").toInt();
      mlriego = yourInputInt;
    }
    else
    {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop()
{
  TYH();
  CONTROLBOTONES();
  TIEMPORIEGO();
  RIEGOAUTOMATICO();
}
