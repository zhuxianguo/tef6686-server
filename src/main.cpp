#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>

#include "secrets.h"

byte DSP_I2C = (0x64U);
ESP8266WebServer server(80);

uint8_t Write(uint8_t *buf, uint8_t len){
  uint8_t i=0;
  Wire.beginTransmission(DSP_I2C);
  for (i = 0; i < len; i++)Wire.write(buf[i]);
  return Wire.endTransmission();
}

uint8_t Read(uint8_t *buf, uint8_t len){
  uint8_t i=0;
  Wire.requestFrom(DSP_I2C, len);
  if (Wire.available() == len) {
    for (i = 0; i < len; i++)buf[i] = Wire.read();
    return 0;
  }
  return 1;
}

uint8_t DelimCount(String str,char delim){
  uint8_t len=str.length(),count=0,i=0;
  for(i=0;i<len;i++){
    if(str[i]==delim)count++;
  }
  return count;
}

void Split(String str,char delim,String result[]){
  uint8_t len=str.length(),i=0,j=0;
  for(i=0;i<len;i++){
    if(str[i]==delim){
      j++;
    }else{
      result[j]+=str[i];
    }
  }
}

uint8_t WriteString(String args){
    char delim=',';
    uint8_t count=DelimCount(args,delim)+1,buf[count]={0},i=0;
    String strs[count];
    Split(args,delim,strs);
    for(i=0;i<count;i++){
      buf[i]=strs[i].toInt();
    }
    return Write(buf,count);
}

void setup(void)
{
  Wire.begin();
  // wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passPhrase);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
  }
  //server
  server.on("/", []() {
    server.send(200, "text/plain", "tef6686 server");
  });
  server.on("/read", []() {
    String arg=server.arg(0),str;
    uint8_t len=arg.toInt(),buf[len]={0},r=0,i=0;
    r=Read(buf,len);
    if(r==0){
      for(i=0;i<len;i++){
        if(i==0){
          str=buf[i];
        }else{
          str=str+','+buf[i];
        }
      }
      server.send(200, "text/plain", str);
    }else{
      server.send(500);
    }
  });
  server.on("/write", []() {
    String arg=server.arg(0);
    uint8_t r=WriteString(arg);
    if(r==0){
      server.send(200, "text/plain", String(r));
    }else{
      server.send(500);
    }
  });
  server.enableCORS(true);
  server.begin();
}

void loop(void){
  server.handleClient();
}