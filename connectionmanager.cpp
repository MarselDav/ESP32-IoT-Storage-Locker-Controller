#include "connectionmanager.h"

ConnectionManager::ConnectionManager() : client(secureClient), timeClient(ntpUDP, "pool.ntp.org", 10800) {} // GMT+3 (3 * 3600 сек)  {}

void ConnectionManager::connectWIFI()
{
  WiFi.begin(SSID, PASSWORD); // Начинаем подключение к WiFi по заданному SSID и паролю
  Serial.println("DEBUG MESSAGE: Connecting to WiFi:");

  while(WiFi.status() != WL_CONNECTED) // Ожидаем подключения
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("DEBUG MESSAGE: Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP()); // Выводим IP-адрес, выданный устройству
}

void ConnectionManager::setCurrentDateTime()
{
  timeClient.begin();
  timeClient.update();

  // Получаем Unix-время (секунды с 1970)
  time_t epochTime = timeClient.getEpochTime();  // Используем time_t вместо unsigned long

  // Конвертируем в struct tm
  struct tm *ptm = gmtime(&epochTime);  // Теперь тип аргумента корректен

  // Извлекаем дату и время
  int hour = timeClient.getHours();
  int minute = timeClient.getMinutes();
  int second = timeClient.getSeconds();
  int day = ptm->tm_mday;
  int month = ptm->tm_mon + 1;    // tm_mon = 0-11
  int year = ptm->tm_year + 1900;  // tm_year = годы с 1900

  // Устанавливаем системное время
  setTime(hour, minute, second, day, month, year);
}

bool ConnectionManager::isConnected()
{
  return client.connected();
}

void ConnectionManager::sendJSON(String impactState)
{
  if (client.connected())
  {
    client.publish(INPUT_TOPIC, impactState.c_str());
  }
}


void ConnectionManager::setup()
{
  secureClient.setInsecure();  // <<<<< ОТКЛЮЧАЕМ проверку сертификата

  client.setServer(MQTT_BROKER_URL, MQTT_PORT);
  client.setCallback(callback); // Установка колбэк-функции
}

void ConnectionManager::reconnect()
{
  Serial.print("Подключение к HiveMQ...");
  if (client.connect("ESP32Client", MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println("успешно!");
    client.subscribe(OUTPUT_TOPIC);
  } else {
    Serial.print("Ошибка, код ");
    Serial.print(client.state());
    Serial.println(". Повтор...");
  }
}

bool ConnectionManager::isWifiConnect()
{
  return WiFi.status() == WL_CONNECTED;
}

bool ConnectionManager::isMqttConnect()
{
  return client.connected();
}

void ConnectionManager::update()
{
  if (!isWifiConnect())
  {
    if (!startConnectingWifi)
    {
      WiFi.begin(SSID, PASSWORD);
      startConnectingWifi = true;
    }
  }
  else
  {
    if (startConnectingWifi)
      setCurrentDateTime();

    startConnectingWifi = false;
    if (!client.connected()) {
      reconnect();
    }

    client.loop();
  }
}