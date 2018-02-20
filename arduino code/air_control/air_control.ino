#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NTPtimeESP.h>

#define minut * 60000
#define header "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<!DOCTYPE html>\n"
#define viewport "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
#define h1 0
#define c1 4
#define T1 2
#define T2 5
#define ssid "248chilli.ro"
#define pass "carolinareaper66"

OneWire oneWire(T2);
DallasTemperature temperatureSensor(&oneWire);

NTPtime NTPch("ch.pool.ntp.org");

void heat(bool);
void cool(bool);
void check(void);
void readSensors(bool);

String request, ip;

strDateTime datetime;
DHT dht(T1, 22);

WiFiServer server(80);
WiFiClient client;

bool dht_err = 0, night = 0, heating = 0, cooling = 0, del_today = 0;
uint64_t m = 0, last_read = 0, last_post = 0, loops = 0;
float t[2] = {0, 0}, h = 0, high_thresh = 37, high_thresh2 = 35, low_thresh = 19, low_thresh2 = 25, dt = 2;
uint8_t ID = 0, retry_count = 0, err = 0, high_h = 22, low_h = 7, night_diff = 7;
int hour = -1, counter = 0;
unsigned int time_on = 0, time_on2 = 0;
unsigned long long last_add = 0, last_add2 = 0;

void setup() {
  // put your setup code here, to run once:
  EEPROM.begin(10);
  ID = EEPROM.read(0);
  low_thresh = EEPROM.read(1);
  high_thresh = EEPROM.read(2);
  low_h = EEPROM.read(3);
  high_h = EEPROM.read(4);
  night_diff = EEPROM.read(5);
  low_thresh2 = EEPROM.read(6);
  high_thresh2 = EEPROM.read(7);
  EEPROM.end();
  if (!low_thresh2 || low_thresh2 == 255)
    low_thresh2 = 25;
  if (!high_thresh2 || high_thresh2 == 255)
    high_thresh2 = 35;
  Serial.begin(9600);
  Serial.println("fwudcg ");
  dht.begin();
  temperatureSensor.begin();
  pinMode(h1, 1);
  pinMode(c1, 1);
  heat(0);
  cool(0);
  Serial.println("connecting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    yield();
    count++;
    if (count > 50)
      break;
  }
  ip = WiFi.localIP().toString();
  Serial.println(ip);
  server.begin();
  ArduinoOTA.onEnd([]() {
    ESP.restart();
  });
  ArduinoOTA.begin();
  Serial.println("done setup");
}

void loop() {
  // put your main code here, to run repeatedly:
  yield();
  m = millis();
  if (hour != -1) {
    if (hour >= low_h && hour <= high_h)
      night = 0;
    else
      night = 1;
  }
  if (m - last_read > 20000 || !h) {
    Serial.println("checking");
    readSensors(0);
    check();
  }
  if ((m - last_post >= 30 minut ) || !last_post ) {
    last_post = m = millis();
    readSensors(1);
    Serial.println("posting");
    HTTPClient http;
    String url = "http://192.168.1.101/upload/?";
    url += "type=temp_module";
    url += "&id=" + (String)ID;
    url += "&ip=" + ip;
    url += "&temperature1=" + (String)t[0];
    url += "&humidity=" + (String)h;
    url += "&temperature2=" + (String)t[1];
    url += "&err=" + (String)err;
    url += "&temperature_high=" + (String)high_thresh;
    url += "&temperature_low=" + (String)low_thresh;
    url += "&night_diff=" + (String)night_diff;
    url += "&high_hour=" + (String)high_h;
    url += "&low_hour=" + (String)low_h;
    url += "&time_on=" + (String)time_on;
    url += "&time_on2=" + (String)time_on2;
    url += "&count=" + (String)counter;
    int i = 0;
up:
    http.begin(url);
    if (http.GET() != HTTP_CODE_OK) {
      i++;
      if (i < 5)
        goto up;
      hour = -1;//*************************NTP****************************
      unsigned long long M = millis();
      datetime = NTPch.getNTPtime(2.0, 1);
      while (millis() - M < 20 * 1000 || datetime.valid)
        datetime = NTPch.getNTPtime(2.0, 1);
      if (datetime.valid) {
        hour = datetime.hour;
      }
    } else {
      String request = http.getString();
      loops = 0;
      http.end();
      if (request.indexOf("hour=") != -1) {
        hour = request.substring(request.indexOf("hour=") + 5, request.indexOf("!")).toInt();
      }
      Serial.println(request);
    }
    last_post = m = millis();
  } else if (loops >= 50) {
    loops = 0;
    last_post = m = millis();
  }

  if (heating && millis() - last_add > 1000 && hour >= 0) {
    time_on += (millis() - last_add) / 1000;
    last_add += ((millis() - last_add) / 1000) * 1000;
  } else if (millis() - last_add > 1000 && hour >= 0) {
    last_add += ((millis() - last_add) / 1000) * 1000;
  }
  if (cooling && millis() - last_add2 > 1000 && hour >= 0) {
    time_on2 += (millis() - last_add2) / 1000;
    last_add2 += ((millis() - last_add2) / 1000) * 1000;
  } else if (millis() - last_add2 > 1000 && hour > 0) {
    last_add2 += ((millis() - last_add2) / 1000) * 1000;
  }
  if (hour == 0 && del_today == 0) {
    time_on = 0;
    time_on2 = 0;
    del_today = 1;
    counter = 0;
  }
  if (hour > 0)
    del_today = 0;
  client = server.available();
  if (!client) {
    client.flush();
    return;
  }
  m = millis();
  Serial.println("got a client");
  while (!client.available()) {
    delay(1);
    if (millis() - m > 3000)
      break;
  }
  m = millis();
  request = client.readStringUntil('\r');
  client.flush();
  bool changed = 0;
  if (request.indexOf("favicon") != -1) {
    client.flush();
    client.stop();
    return;
  }
  if (request.indexOf("ota") != -1) {
    heat(0);
    cool(0);
    client.print(header);
    client.print("ota");
    client.flush();
    client.stop();
    while (1) {
      ArduinoOTA.handle();
      yield();
    }
    return;
  }
  if (request.indexOf("restart") != -1) {
    client.print(header);
    client.print("restart");
    client.flush();
    client.stop();
    ESP.restart();
  }
  if (request.indexOf("t_h=") != -1) {
    changed = 1;
    high_thresh = request.substring(request.indexOf("t_h=") + 4, request.indexOf(" HTTP")).toFloat();
    EEPROM.begin(10);
    EEPROM.write(2, (byte)high_thresh);
    EEPROM.commit();
  }
  if (request.indexOf("t_h2=") != -1) {
    changed = 1;
    high_thresh2 = request.substring(request.indexOf("t_h2=") + 5, request.indexOf(" HTTP")).toFloat();
    EEPROM.begin(10);
    EEPROM.write(7, (byte)high_thresh2);
    EEPROM.commit();
  }
  if (request.indexOf("t_l=") != -1) {
    changed = 1;
    low_thresh = request.substring(request.indexOf("t_l=") + 4, request.indexOf(" HTTP")).toFloat();
    EEPROM.begin(10);
    EEPROM.write(1, (byte)low_thresh);
    EEPROM.commit();
  }
  if (request.indexOf("t_l2=") != -1) {
    changed = 1;
    low_thresh2 = request.substring(request.indexOf("t_l2=") + 5, request.indexOf(" HTTP")).toFloat();
    EEPROM.begin(10);
    EEPROM.write(6, (byte)low_thresh2);
    EEPROM.commit();
  }
  if (request.indexOf("id=") != -1) {
    changed = 1;
    ID = request.substring(request.indexOf("id=") + 3, request.indexOf(" HTTP")).toInt();
    EEPROM.begin(10);
    EEPROM.write(0, ID);
    EEPROM.commit();
  }
  if (request.indexOf("low_hour=") != -1) {
    changed = 1;
    low_h = request.substring(request.indexOf("low_hour=") + 9, request.indexOf(" HTTP")).toInt();
    EEPROM.begin(10);
    EEPROM.write(3, (byte)low_h);
    EEPROM.commit();
    check();
  }
  if (request.indexOf("high_hour=") != -1) {
    changed = 1;
    high_h = request.substring(request.indexOf("high_hour=") + 10, request.indexOf(" HTTP")).toInt();
    EEPROM.begin(10);
    EEPROM.write(4, (byte)high_h);
    EEPROM.commit();
    check();
  }
  if (request.indexOf("night_thresh=") != -1) {
    changed = 1;
    night_diff = request.substring(request.indexOf("night_thresh=") + 13, request.indexOf(" HTTP")).toInt();
    EEPROM.begin(10);
    EEPROM.write(5, (byte)night_diff);
    EEPROM.commit();
    check();
  }
  readSensors(0);
  check();
  String page = header;
  page += "<html>\n";
  page += "<head>\n";
  page += "<title>ESP_";
  page += ID;
  page += "</title>\n";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n";
  if (changed) {
    page += "<meta http-equiv='refresh' content='0; url=http://";
    page += ip;
    page += "'/>";
    page += "</head></html>";
    client.print(page);
    client.flush();
    client.stop();
  } else {
    page += "<script>\n";
    page += "function new_low_h(){\n";
    page += "var new_h=prompt('Ora inceput zi:','";
    page += low_h;
    page += "');\n";
    page += "if(new_h!=null && new_h!=''){\n";
    page += "window.location.replace('http://";
    page += ip;
    page += "/low_hour='+new_h";
    page += ");\n}\n}\n";
    page += "function new_high_h(){\n";
    page += "var new_h=prompt('Ora sfarsit zi:','";
    page += high_h;
    page += "');\n";
    page += "if(new_h!=null && new_h!=''){\n";
    page += "window.location.replace('http://";
    page += ip;
    page += "/high_hour='+new_h";
    page += ");\n}\n}\n";
    page += "function new_n_thresh(){\n";
    page += "var new_h=prompt('Diferenta temperatura noapte:','";
    page += night_diff;
    page += "');\n";
    page += "if(new_h!=null && new_h!=''){\n";
    page += "window.location.replace('http://";
    page += ip;
    page += "/night_thresh='+new_h";
    page += ");\n}\n}\n";
    page += "function new_ID(){\n";
    page += "var new_id=prompt('ESP ID:','";
    page += ID;
    page += "');\n";
    page += "if(new_id!=null && new_id!=''){\n";
    page += "window.location.replace('http://";
    page += ip;
    page += "/id='+new_id";
    page += ");\n}\n}\n";
    page += "function t_h(){\n";
    page += "var new_temp=prompt('Prag sus:','";
    page += high_thresh;
    page += "');\n";
    page += "if(new_temp!=null && new_temp!=''){\n";
    page += "window.location.replace('http://";
    page += ip;
    page += "/t_h='+new_temp";
    page += ");\n}\n}\n";
    page += "function t_h(){\n";
    page += "var new_temp=prompt('Prag sus2:','";
    page += high_thresh2;
    page += "');\n";
    page += "if(new_temp!=null && new_temp!=''){\n";
    page += "window.location.replace('http://";
    page += ip;
    page += "/t_h2='+new_temp";
    page += ");\n}\n}\n";
    page += "function t_l(){\n";
    page += "var new_temp=prompt('Prag jos:','";
    page += low_thresh;
    page += "');\n";
    page += "if(new_temp!=null && new_temp!=''){\n";
    page += "window.location.replace('http://";
    page += ip;
    page += "/t_l='+new_temp";
    page += ");\n}\n}\n";
    page += "function t_l2(){\n";
    page += "var new_temp=prompt('Prag jos2:','";
    page += low_thresh2;
    page += "');\n";
    page += "if(new_temp!=null && new_temp!=''){\n";
    page += "window.location.replace('http://";
    page += ip;
    page += "/t_l2='+new_temp";
    page += ");\n}\n}\n";
    page += "</script>\n";
    client.print(page);
    page = "<style>\n";
    page += ".separator{width:100%;height:3px;background-color:black;}\n";
    page += ".big{font-size:36px;}\n";
    page += ".content{font-size:28px;}\n";
    page += "</style>\n";
    page += "</head>\n";
    client.print(page);
    page = "<body>\n";
    page += "<center>\n";
    page += "<h1 class='big' onclick='new_ID()'>Control Aer</h1>\n";
    page += "<div class='separator'></div>\n";
    page += "<h1 class='content'>\n";
    page += "Interior:";
    page += t[0];
    page += (char)176;
    page += "C<br/>";
    page += "Exterior:";
    page += t[1];
    page += (char)176;
    page += "C<br/>";
    page += "\n</h1>\n\n";
    page += "<div class='separator'></div>\n";
    page += "<h1 class='content'>\n";
    page += "Umiditate:";
    page += h;
    page += "%</h1>\n";
    page += "<div class='separator'></div>\n";
    page += "<br/><button class='content' onclick='t_h()'>Prag sus:";
    page += high_thresh;
    page += (char)176;
    page += "C";
    page += "\n</button><br/>\n\n";
    page += "<br/><button class='content' onclick='t_h2()'>Prag sus2:";
    page += high_thresh2;
    page += (char)176;
    page += "C";
    page += "\n</button>\n\n";
    client.print(page);
    page = "<br/><br/><button class='content' onclick='t_l2()'>Prag jos2:";
    page += low_thresh2;
    page += (char)176;
    page += "C";
    page += "\n</button>\n";
    page += "<br/><br/><button class='content' onclick='t_l()'>Prag jos:";
    page += low_thresh;
    page += (char)176;
    page += "C";
    page += "\n</button>\n<br/><br/>";
    page += "<div class='separator'></div>\n";
    page += "<h1 class='content' onclick='new_low_h()'>\n";
    page += "Inceput zi=";
    page += low_h;
    page += "</h1>\n";
    page += "<h1 class='content' onclick='new_high_h()'>\n";
    page += "Final zi=";
    page += high_h;
    page += "</h1>\n";
    page += "<div class='separator'></div>\n";
    page += "<h1 class='content' onclick='new_n_thresh()'>\n";
    page += "Diferenta temperatura noapte=";
    page += night_diff;
    page += (char)176;
    page += "C";
    page += "</h1>\n";
    page += "<div class='separator'></div>\n";
    page += "<h1 class='content' style='color:";
    if (t[0] < low_thresh) {
      page += "red;'>Incalzire</h1>\n</br>";
    } else if (t[0] > high_thresh) {
      page += "blue;'>Racire</h1>\n</br>";
    } else {
      page += "green;'>Ok</h1>\n</br>";
    }
    page += "</center>\n";
    page += "</body>\n";
    page += "</html>\n";
    client.print(page);
    client.flush();
    client.stop();
  }
}

void readSensors(bool all) {
  float T, H;
retry:
  last_read = m;
  T = dht.readTemperature();
  H = dht.readHumidity();
  if (isnan(t[0]) || isnan(h)) {
    retry_count++;
    if (retry_count > 1000) {
      dht_err = 1;
      err = 1;
    }
    Serial.println("nan");
    goto retry;
  }
  if (dht_err) {
    heat(0);
    cool(0);
    ESP.restart();
  }
  retry_count = 0;
  if (!isnan(T))
    t[0] = T;
  if (!isnan(H))
    h = H;
  if (all)
    for (int i = 0; i < 300; i++) {
      temperatureSensor.requestTemperatures();
      t[1] = temperatureSensor.getTempCByIndex(0);
      if (t[1] > -100 && t[1] < 70)
        break;
      delay(100);
    }
}

void check() {
  if (t[0] < low_thresh - night * night_diff) {
    Serial.println("heating");
    heat(1);
    cool(0);
  } else if (t[0] > high_thresh - night * night_diff) {
    Serial.println("cooling");
    heat(0);
    cool(1);
  } else if (t[0] > low_thresh2 - night * night_diff && t[0] < high_thresh2 - night * night_diff) {
    Serial.println("ok");
    heat(0);
    cool(0);
  }
}

void heat(bool a) {
  if (heating == 0 && a == 1)
    counter++;
  heating = a;
  digitalWrite(h1, !a);
}

void cool(bool a) {
  cooling = a;
  digitalWrite(c1, a);
}


