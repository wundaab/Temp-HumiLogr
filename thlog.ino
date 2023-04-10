#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "DHT.h"
#include <RtcDS1302.h>

//SSID and Password of your WiFi router
const char* ssid = "HUAWEI-246B";
const char* password = "05129867";

#define LED 2       //On board LED
#define DHTTYPE DHT11 // DHT 11
uint8_t DHTPin = 14; 
DHT dht(DHTPin, DHTTYPE); 

ThreeWire myWire(12, 5, 13); // IO, SCLK, CE 6.1.7
RtcDS1302<ThreeWire> Rtc(myWire);

float humidity, temperature;
ESP8266WebServer server(80); //Server on port 80
const char MAIN_page[] PROGMEM = R"=====(
<!doctype html>
<html>
<head>
  <title>Temperature and Humidity Logger</title>
  <h1 style="text-align:center; color:rgb(99, 208, 168);font-family: sans-serif;">Temp&HumiLogr</h1>
  <style>
  canvas{
    -moz-user-select: none;
    -webkit-user-select: none;
    -ms-user-select: none;
  }
  /* Data Table Styling*/ 
  #dataTable {
    font-family: sans-serif;
    font-weight: 600;
    border-collapse: collapse;
    width: 100%;
    text-align: center;
    background-color: rgb(223, 223, 223);
  }
  #dataTable td, #dataTable th {
    border: 1px solid #070707;
    padding: 8px;
  }
  /* #dataTable tr:nth-child(even){background-color: #f2f2f2;} */
  /* #dataTable tr:hover {background-color: #ddd;} */
  #dataTable th {
    padding-top: 12px;
    padding-bottom: 12px;
    text-align: center;
    background-color: rgb(49, 58, 73);
    color: white;
  }
  </style>
</head>
<body style="background-color: rgb(31, 38, 49);">
<div>
  <table id="dataTable">
    <tr><th>Time</th><th>Temperature (&deg;C)</th><th>Humidity (%)</th></tr>
  </table>
</div>
<br>
<br>  
<script>
var Tvalues = [];
var Hvalues = [];
var timeStamp = [];
setInterval(function() {
  // Call a function repetatively with 5 Second interval
  getData();
}, 5000); //5000mSeconds update rate
 function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
     //Push the data in array
  var time = new Date().toLocaleTimeString();
  var txt = this.responseText;
  var obj = JSON.parse(txt); 
      Tvalues.push(obj.Temperature);
      Hvalues.push(obj.Humidity);
      timeStamp.push(time);
  //Update Data Table
    var table = document.getElementById("dataTable");
    var row = table.insertRow(1); //Add after headings
    var cell1 = row.insertCell(0);
    var cell2 = row.insertCell(1);
    var cell3 = row.insertCell(2);
    cell1.innerHTML = obj.Time;
    cell2.innerHTML = obj.Temperature;
    cell3.innerHTML = obj.Humidity;
  // Check temperature range
      if (obj.Temperature < 25 || obj.Temperature > 35) {
        cell2.style.backgroundColor = "rgb(216, 0, 0)";
      } else {
        cell2.style.backgroundColor = "";
      }
      
      // Check humidity range
      if (obj.Humidity < 65 || obj.Humidity > 75) {
        cell3.style.backgroundColor = "rgb(216, 0, 0)";
      } else {
        cell3.style.backgroundColor = "";
      }
    }
  };
  xhttp.open("GET", "readData", true); //Handle readData server on ESP8266
  xhttp.send();
}
</script>
</body>
</html>

)=====";
 

void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}

void readData() {
// Get the current time from the DS1302 real-time clock module and format it
RtcDateTime now = Rtc.GetDateTime();
String timeString = "";
timeString += now.Day();
timeString += "/";
timeString += now.Month();
timeString += "/";
timeString += now.Year();
timeString += " ";
timeString += now.Hour();
timeString += ":";
timeString += now.Minute();
timeString += ":";
timeString += now.Second();


  
  // Read the temperature and humidity from the DHT sensor
  temperature = dht.readTemperature(); 
  humidity = dht.readHumidity(); 
  
  // Construct the data string with the temperature, humidity, and time
  String data = "{\"Temperature\":\""+ String(temperature) +"\", \"Humidity\":\""+ String(humidity) +"\", \"Time\":\""+ timeString +"\"}";
  
  digitalWrite(LED,!digitalRead(LED)); //Toggle LED on data request ajax
  server.send(200, "text/plane", data); //Send ADC value, temperature and humidity JSON to client ajax request
  delay(2000);
}



void setup ()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(DHTPin, INPUT);
  dht.begin();
 
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
  //Onboard LED port Direction output
  pinMode(LED,OUTPUT); 
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
 
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/readData", readData); //This page is called by java Script AJAX
 
  server.begin();                  //Start server
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();          //Handle client requests
}
