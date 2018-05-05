#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <NTPtimeESP.h>

#define used 1
#define HOME 1
#define header "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<!DOCTYPE html>\n"
#define viewport "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
#define minut *60000.0
#define r1 16
#define r2 5
#define h1 0
#define h2 4
#define t 2
#define TIME_BETWEEN 16 * 60 * 60 * 1000
#if HOME == 1
#define ssid "1"
#define pass "Cristina18."
#else
#define ssid "248chilli.ro"
#define pass "carolinareaper66"
#endif

String ip;

NTPtime NTPch("ro.pool.ntp.org");

OneWire oneWire(t);
DallasTemperature temperatureSensor(&oneWire);
WiFiServer server(80);
WiFiClient post_client, client;

strDateTime datetime;
String request;

void water(bool a);
void heat(bool a);
int readHumidity(int a);
float readTemperature(void);
void readSensors(void);
void check(void);

bool watering = 0, heating = 0, changed = 0, del_today = 0, now = 0;
int humidity[2];
float temperature, temperature_threshold = 30, humidity_threshold = 540, liters;
byte ID = 1, err = 0;
int loops = 0, hour = -1 ;
unsigned long long current_millis, last_post, last_sensors_read, last_change, last_temp_read, watering_time;
unsigned int time_on = 0;
unsigned long long last_add = 0;

void setup() {
  // put your setup code here, to run once:
  EEPROM.begin(7);
  ID = EEPROM.read(0);
  temperature_threshold = EEPROM.read(1);
  humidity_threshold = EEPROM.read(2);
  liters = EEPROM.read(4);
  liters /= 100;
  liters += EEPROM.read(3);
  EEPROM.end();
  if (ID == 255)
    ID = 1;
  if (!humidity_threshold || humidity_threshold == 255)
    humidity_threshold = 140;
  if (!temperature_threshold || temperature_threshold == 255)
    temperature_threshold = 30;
  humidity_threshold *= 4;
  WiFi.mode(WIFI_STA);
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  pinMode(A0, INPUT);
  if (used == 2) {
    pinMode(h1, OUTPUT);
    pinMode(h2, OUTPUT);
  }
  pinMode(r1, OUTPUT);
  pinMode(r2, OUTPUT);
  heat(0);
  water(0);
  temperatureSensor.begin();
  Serial.print("connecting");
  int loops = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    loops++;
    if (loops > 50)
      break;
    delay(500);
  }
  readSensors();
  Serial.println();
  ip = WiFi.localIP().toString();
  Serial.println(ip);
  server.begin();
  ArduinoOTA.onEnd([]() {
    ESP.restart();
  });
  ArduinoOTA.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  yield();
  current_millis = millis();
  if (watering and current_millis - watering_time > 20 * 1000 * liters)
    water(0);
  if (current_millis > last_temp_read + 20000 || !last_temp_read) {
    if (used == 2)
      readSensors();
    else
      readTemperature();
    check();
  }
  if ((current_millis - last_post > 30 minut) || !last_post ) {
    ip = WiFi.localIP().toString();
    last_post = current_millis = millis();
    readSensors();
    Serial.println("posting");
    HTTPClient http;
    String url;
    if (HOME)
      url = "http://192.168.43.31/upload/?";
    else
      url = "http://192.168.1.101/upload/?";
    url += "type=normal_module";
    url += "&id=" + (String)ID;
    url += "&ip=" + ip;
    url += "&temperature=" + (String)temperature;
    url += "&humidity1=" + (String)humidity[0];
    url += "&humidity2=" + (String)humidity[1];
    url += "&err=" + (String)err;
    url += "&threshold=" + (String)temperature_threshold;
    url += "&time_on=" + (String)time_on;
    url += "&critical_humidity=" + (String)humidity_threshold;
    url += "&now=" + (String)now;
    now = 0;
    int i = 0;
up:
    http.begin(url);
    if (http.GET() != HTTP_CODE_OK) {
      i++;
      if (i < 5)
        goto up;
      hour = -1;
      unsigned long long M = millis();
      datetime = NTPch.getNTPtime(2.0, 1);
      while (millis() - M < 20 * 1000 || datetime.valid)
        datetime = NTPch.getNTPtime(2.0, 1);
      if (datetime.valid) {
        hour = datetime.hour;
      }
    } else {
      String request = http.getString();
      if ( request.indexOf("hour") != -1) {
        hour = request.substring(request.indexOf("hour=") + 5, request.indexOf("!")).toInt();
        Serial.print("got hour:");
        Serial.println(hour);
      }
      loops = 0;
      Serial.print("Response:");
      Serial.println(request);
    }
    last_post = current_millis = millis();
  } else if (loops >= 50) {
    loops = 0;
    last_post = current_millis = millis();
  }
  if (heating and millis() - last_add > 1000 and hour >= 0) {
    time_on += (millis() - last_add) / 1000;
    last_add += ((millis() - last_add) / 1000) * 1000;
    Serial.println("adding");
  } else if (millis() - last_add > 1000 and hour >= 0) {
    last_add += ((millis() - last_add) / 1000) * 1000;
    Serial.println("not adding");
  }
  if (hour == 0 and del_today == 0) {
    time_on = 0;
    del_today = 1;
  }
  if (hour > 0)
    del_today = 0;
  client = server.available();
  if (!client) {
    client.flush();
    return;
  }
  current_millis = millis();
  Serial.println("got a client");
  while (!client.available()) {
    delay(1);
    if (millis() - current_millis > 3000) {
      Serial.println("client disconnected");
      break;
    }
  }
  Serial.println("client connected");
  current_millis = millis();
  request = client.readStringUntil('\r');
  client.flush();
  if (request.indexOf("favicon") != -1) {
    client.flush();
    client.stop();
    return;
  }
  if (request.indexOf("ota") != -1) {
    heat(0);
    water(0);
    client.print(header);
    client.print("OTA");
    client.flush();
    client.stop();
    while (1) {
      ArduinoOTA.handle();
      yield();
    }
    return;
  }
  if (request.indexOf("restart") != -1) {
    heat(0);
    water(0);
    client.print(header);
    client.print("restart");
    client.flush();
    client.stop();
    ESP.restart();
    return;
  }
  changed = 0;
  if (request.indexOf("ID=") != -1) {
    changed = 1;
    ID = request.substring(request.indexOf("ID=") + 3, request.indexOf(" HTTP")).toInt();
    EEPROM.begin(5);
    EEPROM.write(0, ID);
    EEPROM.commit();
  }
  if (request.indexOf("temperature_threshold=") != -1) {
    changed = 1;
    temperature_threshold = request.substring(request.indexOf("temperature_threshold=") + 22, request.indexOf(" HTTP")).toFloat();
    EEPROM.begin(5);
    EEPROM.write(1, (byte)temperature_threshold);
    EEPROM.commit();
    check();
  }
  if (request.indexOf("water") != -1)
    water(1);
  if (request.indexOf("humidity_threshold=") != -1) {
    changed = 1;
    humidity_threshold = request.substring(request.indexOf("humidity_threshold=") + 19, request.indexOf(" HTTP")).toInt();
    EEPROM.begin(5);
    EEPROM.write(2, (byte)(humidity_threshold / 4));
    EEPROM.commit();
  }
  if (request.indexOf("liters=") != -1) {
    changed = 1;
    liters = request.substring(request.indexOf("liters=") + 7, request.indexOf(" HTTP")).toFloat();
    EEPROM.begin(5);
    EEPROM.write(3, (byte)liters);
    EEPROM.write(4, (byte)((liters - (int)liters) * 100));
    EEPROM.commit();
  }
  if (request.indexOf("upload") != -1) {
    last_post = 0;
  }
  String page = header;
  page += "<html>\n";
  page += "<head>\n";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1.0'/>";
  if (changed) {
    page += "<meta http-equiv='refresh' content='0; url=http://";
    page += ip;
    page += "'/>";
    page += "</head></html>";
    client.print(page);
    client.flush();
    client.stop();
  } else {
    readSensors();
    page += "<title>ESP_";
    page += ID;
    page += "</title>\n";
    client.print(page);
    page = "<script>\n";
    page += "function new_ID(){\n";
    page += "var new_id=prompt('Tava nr:','";
    page += ID;
    page += "');\n";
    page += "if(new_id!=null && new_id!=''){\n";
    page += "window.location.replace('http://";
    page += ip;
    page += "/ID='+new_id";
    page += ");\n}\n}\n";
    page += "function temp(){\n";
    page += "var new_temp=prompt('Prag temperatura:','";
    page += (int)temperature_threshold;
    page += "');\n";
    page += "if(new_temp!=null && new_temp!=''){\n";
    page += "window.location.replace('http://";
    page += ip;
    page += "/temperature_threshold='+new_temp";
    page += ");\n}\n}\n";
    page += "</script>\n";
    client.print(page);
    page = "<style>\n";
    page += ".big{font-size:36px;}\n";
    page += ".content{font-size:28px;}\n";
    page += ".separator{width:100%;height:3px;background-color:black;}\n";
    page += "</style>\n";
    page += "</head>\n";
    page += "<body>\n";
    page += "<center>\n";
    page += "<h1 class='big' onclick='new_ID()'>\nTava ";
    page += ID;
    page += "\n</h1>\n<div class='separator'></div>\n\n";
    page += "<h1 class='content' onclick='temp()'>\n";
    page += "Temperatura=";
    page += temperature;
    page += (char)176;
    page += "C";
    page += "\n</h1>\n<div class='separator'></div>\n";
    page += "<h1 class='content' onclick='hum()'>\n";
    page += "Umiditate 1=";
    page += humidity[0];
    page += "\n</h1>\n";
    page += "<h1 class='content' onclick='hum()'>\n";
    page += "Umiditate 2=";
    page += humidity[1];
    page += "\n</h1>\n";
    page += "<div class='separator'></div>\n";
    page += "<h1 class='content' ";
    if (heating)
      page += "style='color:red'>\nIncalzire\n";
    else
      page += "style='color:green'>\nOk\n";
    page += "</h1>\n";
    page += "<h1 style='display:none;'>critical_humidity=" + (String)humidity_threshold;
    page += "\n<br/>temperature_threshold=" + (String)temperature_threshold;
    page += "\n<br/>time_on=" + (String)(liters * 20);
    if (watering_time)
      page += "\n<br/>diff=" + (String)((int)(millis() - watering_time) / 1000);
    page += "\n<br/>now=" + (String)now;
    page += "\n</h1>";
    page += "</center>\n";
    page += "</body>\n";
    page += "</html>\n";
    client.print(page);
    client.flush();
    client.stop();
  }
}

void water(bool a) {
  watering = a;
  digitalWrite(r1, !a);
  if (a == 1){
    watering_time = millis();
    now = 1;
  }
}

void heat(bool a) {
  heating = a;
  last_change = millis();
  digitalWrite(r2, !a);
}

int readHumidity(int a) {
  int val;
  if (used == 2) {
    if (a == 1) {
      digitalWrite(h1, 1);
      digitalWrite(h2, 0);
    } else {
      digitalWrite(h1, 0);
      digitalWrite(h2, 1);
    }
    delay(100);
    val = analogRead(A0);
    digitalWrite(h1, 1);
    digitalWrite(h2, 1);
  } else
    val = analogRead(A0);
  return val;
}

float readTemperature() {
  last_temp_read = millis();
  for (int i = 0; i < 500; i++) {
    temperatureSensor.requestTemperatures();
    float Temp = temperatureSensor.getTempCByIndex(0);
    if (Temp > -80 && Temp < 80)
      return Temp;
    delay(100);
  }
  err = 2;
  return -127;
}

void readSensors() {
  Serial.println("REading");
  last_sensors_read = current_millis;
  temperature = readTemperature();
  humidity[0] = 1024 - readHumidity(1);
  if (used == 2) {
    delay(150);
    humidity[1] = 1024 - readHumidity(2);
  } else
    humidity[1] = humidity[0];
  Serial.println("temp=" + (String)temperature);
  Serial.println("hum1=" + (String)humidity[0]);
  Serial.println("hum2=" + (String)humidity[1]);
  if (humidity[0] > humidity[1] + 100) {
    err = 1;
    humidity[1] = humidity[0];
    Serial.println("invailid hum2");
  }
  if (humidity[1] > humidity[0] + 100) {
    err = 1;
    humidity[0] = humidity[1];
    Serial.println("invalid hum1");
  }
  if (humidity[0] < 230 || humidity[1] < 230) {
    err = 1;
    Serial.println("invalid values");
  }
  //if (err)
  //  return;
}

void check() {
  if (temperature < temperature_threshold - 0.5 )
    heat(1);
  if (temperature > temperature_threshold + 2)
    heat(0);
  if (humidity[0] < humidity_threshold or humidity[1] < humidity_threshold)
    if (current_millis > watering_time + TIME_BETWEEN or (watering_time == 0 /*and millis() > 1 * 1000 * 60 * 60*/))
      water(1);
}
