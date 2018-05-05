#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <NTPtimeESP.h>

#define ON 1
#define OFF 0
#define HOME 1
#define header "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<!DOCTYPE html>\n"
#define viewport "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
#if HOME == 1
#define ssid "1"
#define pass "Cristina18."
#else
#define ssid "248chilli.ro"
#define pass "carolinareaper66"
#endif

struct Solar {
  bool active, on1, on2, done;
  int h1, h2, t1, t2, day;
  uint64_t last;
} solar[10];

bool ok[10][2];

void Read(int);
void check();

WiFiClient client;
WiFiServer server(80);
NTPtime NTPch("ro.pool.ntp.org");
strDateTime datetime;
String request, ip;

int reset_today, now_on;
uint64_t current_millis, started_on, time_on, last_post, last_check, last_hour;
int q[100];
int v[] = {1, 2, 0, 4, 5 , 16, 14, 12, 13, 15};

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 9; i++)
    pinMode(v[i], OUTPUT), turn(v[i], OFF);
  EEPROM.begin(100);
  for (int i = 1; i <= 9; i++) {
    Read(i);
  }
  EEPROM.end();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.begin(9600);
  Serial.println("wfsdxubu");
  Serial.println(ssid);
  Serial.println(pass);
  Serial.print("connecting");
  int loops = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    loops++;
    if (loops > 50)
      break;
    delay(500);
  }
  Serial.println();
  Serial.println(WiFi.localIP());
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
  ip = WiFi.localIP().toString();
  if (millis() - last_check > 5000) {
    check();
    last_check = millis();
  }
  if (datetime.hour == 2 and !reset_today) {
    reset_today = 1;
    for (int i = 0; i < 10; i++)
      ok[i][0] = ok[i][1] = false;
  }else if(datetime.hour != 2)
    reset_today = 0;
  if (millis() - last_post >= 1 * 1000 * 60 or !last_post) {
    Serial.println("posting");
    last_post = millis();
    for (int i = 1; i < 10; i++) {
      if (!solar[i].active)
        continue;
      HTTPClient http;
      String url = "http://192.168.43.31/solar/upload/?";
      url += "ip=" + ip;
      url += "&no=" + String(i);
      url += "&ok1=" + String(ok[i][0]);
      url += "&ok2=" + String(ok[i][1]);
      int I = 0;
up:
      http.begin(url);
      if (http.GET() != HTTP_CODE_OK) {
        I++;
        if (I < 5)
          goto up;
        while (millis() - last_hour > 60 * 1000) {
          last_hour -= 60 * 1000;
          datetime.hour++;
          if(datetime.hour > 23)
            datetime.hour -= 24;
        }
      } else {
        Serial.println("success");
        String request = http.getString();
        http.end();
        if (request.indexOf("h=") != -1) {
          datetime.hour = request.substring(request.indexOf("h=") + 2, request.indexOf("!")).toInt();
          last_hour = millis();
        }
      }
    }
  }
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
    client.print(header);
    client.print("restart");
    client.flush();
    client.stop();
    ESP.restart();
    return;
  }
  if (request.indexOf("?solar=") != -1) {
    Serial.println("got");
    int NR = request.substring(request.indexOf("?solar=") + 7, request.indexOf("&active=")).toInt();
    solar[NR].active = request.substring(request.indexOf("&active=") + 8, request.indexOf("&on1=")).toInt();
    solar[NR].on1 = request.substring(request.indexOf("&on1=") + 5, request.indexOf("&on2=")).toInt();
    solar[NR].on2 = request.substring(request.indexOf("&on2=") + 5, request.indexOf("&h1=")).toInt();
    solar[NR].h1 = request.substring(request.indexOf("&h1=") + 4, request.indexOf("&h2=")).toInt();
    solar[NR].h2 = request.substring(request.indexOf("&h2=") + 4, request.indexOf("&t1=")).toInt();
    solar[NR].t1 = request.substring(request.indexOf("&t1=") + 4, request.indexOf("&t2=")).toInt();
    solar[NR].t2 = request.substring(request.indexOf("&t2=") + 4, request.indexOf("&day=")).toInt();
    solar[NR].day = request.substring(request.indexOf("&day=") + 4, request.indexOf(" HTTP")).toInt();
    EEPROM.begin(100);
    Write(NR);
    EEPROM.commit();
    client.print(header);
    client.print("ok");
    client.flush();
    client.stop();
  }
  client.println(header);
  client.println(datetime.hour);
  client.println(solar[1].h1);
  client.println(now_on);
  client.flush();
  client.stop();
}

void check() {
  for (int i = 1; i <= 9; i++) {
    if (solar[i].active) {
      if (millis() - solar[i].last > 1000 * 60 * 60 * (24 * solar[i].day - 1 - abs(solar[i].h1 - solar[i].h2)) or !solar[i].last) {
        int found = 0;
        if (datetime.hour == solar[i].h1 and solar[i].on1) {
          for (int j = 1; j <= q[0]; j++)
            if (q[j] == i)
              found = 1;
          if (found)
            continue;
          q[0]++;
          q[q[0]] = i;
        } else if (datetime.hour == solar[i].h2 and solar[i].on2) {
          for (int j = 1; j <= q[0]; j++)
            if (q[j] == i + 20)
              found = 1;
          if (found)
            continue;
          q[0]++;
          q[q[0]] = i + 20;
        }
        //Serial.println(solar[i].h1);
      }
    }
  }
  Serial.println(q[0]);
  if (millis() - started_on > time_on) {
    started_on = 0;
    now_on = 0;
    for (int i = 1; i <= 9; i++)
      turn(v[i], OFF);
  }
  if (q[0] and !started_on) {
    if (q[q[0]] > 20) {
      q[q[0]] -= 20;
      time_on = solar[q[q[0]]].t2;
      ok[q[q[0]]][1] = true;
    } else {
      time_on = solar[q[q[0]]].t1;
      ok[q[q[0]]][0] = true;
    }
    Serial.print(q[q[0]]);
    Serial.print(" on for ");
    Serial.println((int)time_on);
    time_on *= 1000 * 60;
    now_on = q[q[0]];
    turn(v[q[q[0]]], ON);
    solar[q[q[0]]].last = millis();
    started_on = millis();
    q[0]--;
  }
}

void turn(int i, bool x) {
  //Serial.println("turn" + String(i) + " " + String(x == ON));
  if (i == 5)
    digitalWrite(i, x);
  else
    digitalWrite(i, !x);
}

void Read(int i) {
  solar[i].active = EEPROM.read(8 * i);
  solar[i].on1 = EEPROM.read(8 * i + 1);
  solar[i].on2 = EEPROM.read(8 * i + 2);
  solar[i].h1 = EEPROM.read(8 * i + 3);
  solar[i].h2 = EEPROM.read(8 * i + 4);
  solar[i].t1 = EEPROM.read(8 * i + 5);
  solar[i].t2 = EEPROM.read(8 * i + 6);
  solar[i].day = EEPROM.read(8 * i + 7);
}

void Write(int i) {
  EEPROM.write(8 * i, (byte)solar[i].active);
  EEPROM.write(8 * i + 1, (byte)solar[i].on1);
  EEPROM.write(8 * i + 2, (byte)solar[i].on2);
  EEPROM.write(8 * i + 3, (byte)solar[i].h1);
  EEPROM.write(8 * i + 4, (byte)solar[i].h2);
  EEPROM.write(8 * i + 5, (byte)solar[i].t1);
  EEPROM.write(8 * i + 6, (byte)solar[i].t2);
  EEPROM.write(8 * i + 7, (byte)solar[i].day);
}

