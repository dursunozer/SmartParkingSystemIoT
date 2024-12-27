#define BLYNK_TEMPLATE_ID "TMPL6_v7LKdZx"
#define BLYNK_TEMPLATE_NAME "OtoparkProjeNodemcu"
#define BLYNK_AUTH_TOKEN "AVPqQPQ91PBD97czJo4_S-UYUgIKAqZk"

#include <Servo.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define TRIG_PIN D1       // Trig pin HC-SR04
#define ECHO_PIN D2       // Echo pin HC-SR04
#define BUZZER_PIN D4     // Buzzer pin
#define SERVO_PIN D5      // Servo motor pin
#define RED_LED_PIN D6    // Kırmızı LED pin
#define GREEN_LED_PIN D7  // Yeşil LED pin

Servo myServo;            // Servo motor objesi

long duration;
int distance;
int servoPosition = 0;    // Servo motor pozisyonu
bool barrierControl = false;  // Blynk bariyer kontrolü (manuel/otomatik)

// WiFi bilgileri
char ssid[] = "tunahan";
char pass[] = "12345677";

void connectToWiFi() {
  Serial.print("WiFi'ye bağlanılıyor: ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  // WiFi bağlanana kadar bekle
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    retryCount++;

    // Eğer çok uzun süre bağlanamazsa yeniden başlat
    if (retryCount > 10) {
      Serial.println("\nWiFi'ye bağlanılamadı. ESP yeniden başlatılıyor...");
      ESP.restart();
    }
  }

  Serial.println("\nWiFi bağlantısı başarılı!");
  Serial.print("IP Adresi: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);    // Kırmızı LED pinini çıkış olarak ayarla
  pinMode(GREEN_LED_PIN, OUTPUT);  // Yeşil LED pinini çıkış olarak ayarla

  myServo.attach(SERVO_PIN);
  myServo.write(servoPosition);  // Başlangıçta bariyer kapalı

  // Başlangıçta LED'leri kapalı tut
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  // WiFi'ye bağlan
  connectToWiFi();

  // Blynk başlat
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

// Blynk üzerinden manuel kontrol (V1 butonu)
BLYNK_WRITE(V1) {
  int buttonState = param.asInt();  // Buton durumu (0: Kapalı, 1: Açık)
  barrierControl = (buttonState == 1);  // Blynk bariyer kontrol durumunu güncelle
}

void loop() {
  Blynk.run();  // Blynk işlemlerini çalıştır

  // Ultrasonik sensörden mesafe ölçümü
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;  // Mesafeyi santimetre olarak hesapla

  // Mesafeyi Blynk'e gönder (V0 üzerinden)
  Blynk.virtualWrite(V0, distance);

  // Eğer manuel kontrol açıksa (Blynk butonuyla)
  if (barrierControl) {
    myServo.write(180);           // Bariyeri aç
    digitalWrite(RED_LED_PIN, HIGH);   // Kırmızı ışığı yak
    digitalWrite(GREEN_LED_PIN, LOW);  // Yeşil ışığı kapat
    Blynk.virtualWrite(V2, 0);    // Kırmızı LED'in durumu
    return;  // Otomatik sensör kontrolünü atla
  }

  // Eğer mesafe 10 cm'den küçükse, araba algılanmış demektir
  if (distance < 10) {
    // Servo motoru 180 derece döndürerek bariyeri kaldır
    myServo.write(180);
    // Buzzer'ı çaldır
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);  // Buzzer sesini 0.5 saniye boyunca çaldır
    digitalWrite(BUZZER_PIN, LOW);
    delay(500);   // Biraz bekleyelim

    // Kırmızı ışığı yak
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);  // Yeşil ışığı kapat
    Blynk.virtualWrite(V2, 0);    // Kırmızı LED'in durumu
  } else {
    // Eğer araba yoksa, bariyeri kapalı tut
    myServo.write(0);

    // Yeşil ışığı yak
    digitalWrite(RED_LED_PIN, LOW);   // Kırmızı ışığı kapat
    digitalWrite(GREEN_LED_PIN, HIGH);
    Blynk.virtualWrite(V2, 1);    // Yeşil LED'in durumu
  }

  delay(100);  // Bir sonraki ölçüm için kısa bir gecikme
}
