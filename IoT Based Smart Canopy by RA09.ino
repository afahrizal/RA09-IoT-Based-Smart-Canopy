#include <WiFi.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Servo.h>

#define ldrA 25
#define ldrD 4
#define rainD 32
#define lamp 5
#define pir 14

#define BOTtoken "5833217743:AAGoiPayXD8rzc8k8pm4Ji8KVbAeZKidGso"

const char* ssid = "idphy";
const char* password = "12345678";
Servo rotor;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(13, DHT11);
float t;
bool status_kanopi = 0; //0 = tertutup, 1 = terbuka
bool motorStats = 0;
int state = LOW;
int val = 0;
int cycle = 0;
bool ledState = 0;
int pos = 55;
bool fc = 1; // 1 = mati, 0 = otomatis
bool fcled = 0;
String temp = "Temperatur: " + String(t) + "C";

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
 Serial.println("handleNewMessages");
 Serial.println(String(numNewMessages));

 for (int i=0; i<numNewMessages; i++) {
  // Chat id of the requester
  String chat_id = String(bot.messages[i].chat_id);

  // Print the received message
  String text = bot.messages[i].text;
  Serial.println(text);

  String from_name = bot.messages[i].from_name;

  if (text == "/start") {
    String welcome = "Selamat Datang, " + from_name + ".\n";
    welcome += "Gunakan command dibawah untuk mengontrol.\n\n";
    welcome += "/kanopi_buka untuk Kanopi Terbuka \n";
    welcome += "/kanopi_tutup untuk Kanopi Tertutup \n";
    welcome += "/led_on untuk Lampu Menyala \n";
    welcome += "/led_off untuk Lampu Mati \n";
    welcome += "/sensor_on untuk menyalakan otomatisasi  \n";
    welcome += "/sensor_off untuk menyalakan otomatisasi  \n";
    welcome += "/get_status untuk melihat status kanopi & led  \n";
    
    bot.sendMessage(chat_id, welcome, "");
  }


  if (text == "/kanopi_buka") {
    bot.sendMessage(chat_id, "Kanopi Terbuka", "");
    membuka();
    fc = 1;
    Serial.print("fc: ");
    Serial.println(fc);
  }


  if (text == "/kanopi_tutup") {
    bot.sendMessage(chat_id, "Kanopi Tertutup", "");
    menutup();
    fc = 1;
    Serial.print("fc: ");
    Serial.println(fc);
  }

  if (text == "/led_on") {
    bot.sendMessage(chat_id, "Lampu Menyala", "");
    fcled = 1;
    ledState = 1;
    Serial.print("fcled: ");
    Serial.println(fcled);
  }

  if (text == "/led_off") {
    bot.sendMessage(chat_id, "Lampu Mati", "");
    ledState = 0;
    fcled = 0;
    Serial.print("fcled: ");
    Serial.println(fcled);
  }

  if (text == "/sensor_on") {
    bot.sendMessage(chat_id, "Menyalakan Otomatisasi", "");
    fc = 0;
  }

  if (text == "/sensor_off") {
    bot.sendMessage(chat_id, "Mematikan Otomatisasi", "");
    fc = 1;
  }

  if (text == "/get_status") {
    if (status_kanopi){
      bot.sendMessage(chat_id, "Kanopi Sedang Terbuka", "");
    } else if(!status_kanopi){
      bot.sendMessage(chat_id, "Kanopi Sedang Tertutup", "");
    }
    if(!ledState){
      bot.sendMessage(chat_id, "Lampu Mati", "");
    }else{
      bot.sendMessage(chat_id, "Lampu Hidup", "");
    }
    if(digitalRead(ldrD) == LOW){
      bot.sendMessage(chat_id, "Cahaya Terang", "");
    }else{
      bot.sendMessage(chat_id, "Cahaya Gelap", "");
    }
    if(digitalRead(rainD) == LOW){
      bot.sendMessage(chat_id, "Cuaca Hujan", "");
    }else{
      bot.sendMessage(chat_id, "Cuaca Cerah", "");
    }
    if(!fc){
      bot.sendMessage(chat_id, "Otomatisasi Aktif", "");
    }else{
      bot.sendMessage(chat_id, "Otomatisasi Tidak Aktif", "");
    }
    bot.sendMessage(chat_id, temp, "");
    
    /*if (led){
    bot.sendMessage(chat_id, "led", "");
    }
    else if(!led){
    bot.sendMessage(chat_id, "led", "");
    }*/
    }



      //  if (text == "/option") {
      //  String keyboardJson = "[[\"/kanopi_buka\", \"/kanopi_tutup\", \"/led_on\", \"/led_off\"],[\"/get_status\"]]";
      //  bot.sendMessageWithReplyKeyboard(chat_id, "Choose one of the following options", "", keyboardJson, true);
      //  }
 }
}

void membuka(){
  if(!status_kanopi){
    for(pos; pos <= 120; pos++){
      rotor.write(pos);
      motorStats = 1;
      Serial.print("status_motor : ");
      Serial.println(motorStats);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sedang Membuka...");
      delay(80);
    }
    status_kanopi = 1;
    motorStats = 0 ;
  }
}

void menutup(){
  if(status_kanopi){
    for(pos; pos >= 55; pos--){
      rotor.write(pos);
      motorStats = 1;
      Serial.print("status_motor : ");
      Serial.println(motorStats);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sedang Menutup...");
      delay(80);
    }
    status_kanopi = 0;
    motorStats = 0;
  }
}

void cekLDRPIR(){
  val = digitalRead(pir);
  if(!fc){
    if(!fcled){
      if(digitalRead(ldrD) == LOW){
      ledState = 0;
      } else {
        if(val == HIGH){
          ledState = 1;
        } else {
          ledState = 0;
        }
      }
    }
  }
  digitalWrite(lamp, ledState);
}

void serial_out(){
  Serial.print("Suhu = ");
  Serial.print(t);
  Serial.println(" C");

  Serial.print("Rain = ");
  Serial.println(digitalRead(rainD));

  Serial.print("Cahaya = ");
  Serial.println(digitalRead(ldrD));

  Serial.print("Gerakan = ");
  Serial.println(val);

  Serial.print("status_kanopi : ");
  Serial.println(status_kanopi);

  Serial.print("status_motor : ");
  Serial.println(motorStats);
  Serial.println("");
}

void screenout(){
  lcd.setCursor(0, 0);
  lcd.print("Status Kanopi : ");
  lcd.setCursor(0, 1);
  if(status_kanopi){
    lcd.print("Terbuka         ");
  } else if(!status_kanopi){
    lcd.print("Tertutup        ");
  }
}

void setup() {
  pinMode(ldrA, INPUT);
  pinMode(ldrD, INPUT);
  pinMode(rainD, INPUT);
  pinMode(lamp, OUTPUT);
  pinMode(pir, INPUT);
  rotor.attach(27);
  rotor.write(pos);
  lcd.init();
  lcd.backlight();
  dht.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    lcd.setCursor(0, 0);
    lcd.print("Connecting to ");
    lcd.setCursor(0, 1);
    lcd.print("WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  lcd.clear();
  delay(100);
  lcd.setCursor(0, 0);
  lcd.print("Terhubung!");
  delay(1000);
  lcd.clear();
}

void loop() {
  t = dht.readTemperature();
  Serial.print("fc (loop): ");
  Serial.println(fc);
  screenout();
  // Serial.print("suhu: ");
  // Serial.println(t);

  // if(!status_kanopi){
  //   lcd.setCursor(0, 0);
  //   lcd.print("Status Kanopi : ");
  //   lcd.setCursor(0, 1);
  //   lcd.print("Terbuka         ");
  // } else {
  //   lcd.setCursor(0, 0);
  //   lcd.print("Status Kanopi : ");
  //   lcd.setCursor(0, 1);
  //   lcd.print("Tertutup        ");
  // }
  if(!fc){
    if(t > 35){
      menutup();
      cekLDRPIR();
    } else if(t < 20){
      menutup();
      cekLDRPIR();
    } else if(digitalRead(rainD) == LOW ){
      menutup();
      cekLDRPIR();
    } else if(t > 20 || t < 35){
      if(digitalRead(rainD) == LOW){
        menutup();
        cekLDRPIR();
      } else{
        membuka();
        cekLDRPIR();
      }
    } 
  }
  
  if(cycle == 20){
    serial_out();
    cycle = 0;
  }
  cycle++;
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    } 
  lastTimeBotRan = millis();
  }
  delayMicroseconds(500);
}

