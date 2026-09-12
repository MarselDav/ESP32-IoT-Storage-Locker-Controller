#include "connectionmanager.h"
#include "impactchecker.h"
#include "buzzeralarm.h"
#include "lockmanager.h"

#define BUZZER_PIN 23
#define THRESH_SHOCK 3
#define DURATION 20
#define THRESH_TEMPERATURE 30

#define SEND_STATE_DELAY 10000
#define MIN_SEND_STATE_DELAY 500
#define GET_STATE_DELAY 250

#define ALARM_DURATION 5000
#define ALARM_SEND_DELAY 500

#define RELAY_PIN 4

BuzzerAlarm buzzer(BUZZER_PIN);  // Пин D4
ConnectionManager connManager;
ImpactChecker impCheker;
LockManager lockManager;

unsigned long sendStatePrevMillis = 0;
unsigned long getStatePrevMillis = 0;
unsigned long getAlarmStatePrevMillis = 0;

int melody_alarm[] = {NOTE_F5, NOTE_DS5, NOTE_D5, NOTE_C5, NOTE_DS5, NOTE_F5};
int melody_alarm_duration[] = {8, 8, 8, 8, 8, 8};
const int melody_length = sizeof(melody_alarm) / sizeof(int);

unsigned long openPrevMillis = 0;
unsigned long autoclose_timer = 5000;

int send_state_delay = SEND_STATE_DELAY;

bool prevLockState = false;

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT); 
  digitalWrite(RELAY_PIN, LOW);

  impCheker.setup(THRESH_SHOCK, DURATION, THRESH_TEMPERATURE);
  connManager.setup();
}

String getCurrentDateTime()
{
  String datetime = 
      String(day())     +  ":" +
      String(month())   +  ":" +
      String(year())    +  ":" +
      String(hour())    +  ":" +
      String(minute())  +  ":" +
      String(second());
  
  return datetime;
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Сообщение получено [");
  Serial.print(topic);
  Serial.print("]: ");

  String jsonStr;
  for (unsigned int i = 0; i < length; i++) {
    jsonStr += (char)payload[i];  // Собираем JSON-строку
  }
  Serial.println(jsonStr);

  JSONVar data = JSON.parse(jsonStr); // Парсим JSON

  if (data.hasOwnProperty("settings"))
  {
    Serial.println("DEBUG: settings");
    if (data["settings"].hasOwnProperty("auto_close_timer"))
    {
      unsigned int seconds = data["settings"]["auto_close_timer"];
      lockManager.setAutocloseTime(seconds);
      Serial.printf("DEBUG: Установлено время закрытия %d\n", seconds);
    }
    else
    {
      Serial.println("ERROR: Время автозакрытия отсутствует в JSON");
    }

    if (data["settings"].hasOwnProperty("temperature_threshold"))
    {
      unsigned int temp_threshold = data["settings"]["temperature_threshold"];
      impCheker.setTemperatureThresh(temp_threshold);
      Serial.printf("DEBUG: Установлен temperature_threshold %d\n", temp_threshold);
    }
    else
    {
      Serial.println("ERROR: Максимальная температура отсутствует в JSON");
    }

    if (data["settings"].hasOwnProperty("delay"))
    {
      unsigned int delay = data["settings"]["delay"];
      delay *= 1000;
      if (delay < MIN_SEND_STATE_DELAY)
        send_state_delay = MIN_SEND_STATE_DELAY;
      else
        send_state_delay = delay;
        
      Serial.printf("DEBUG: Установлен delay %d\n", send_state_delay);
    }
    else
    {
      Serial.println("ERROR: Значение задержки отсутствует в JSON");
    }
  }
  else if (data.hasOwnProperty("lock_is_open"))
  {
    JSONVar lockStateJSON = impCheker.getStatus();
    lockStateJSON["timestamp"] = getCurrentDateTime();

    if (data["lock_is_open"])
    {
      if (lockManager.getLockState())
      {
        lockStateJSON["status"] = "already opened";
      }
      else
      {
        if (!buzzer.isActive())
        {
          lockManager.open();
          lockStateJSON["lock_is_open"] = true;
          lockStateJSON["status"] = "successfully opened";
        }
        else
        {
          lockStateJSON["lock_is_open"] = false;
          lockStateJSON["status"] = "alarm is active, cant open";
        }
      }

      if (data.hasOwnProperty("auto_close_timer"))
      {
        unsigned int seconds = data["auto_close_timer"];
        lockManager.setAutocloseTime(seconds);
      }
      else
      {
        Serial.println("DEBUG: Время автозакрытия отсутствует в JSON");
      }

      lockStateJSON["auto_close_timer"] = lockManager.getAutocloseTime();
    }
    else
    {
      if (!lockManager.getLockState())
      {
        lockStateJSON["status"] = "already closed";
      }
      else
      {
        lockManager.close();
        lockStateJSON["status"] = "successfully closed";
      }
      lockStateJSON["lock_is_open"] = false;
    }

    String jsonString = JSON.stringify(lockStateJSON);
    connManager.sendJSON(jsonString);
  }
  else
  {
    Serial.println("ERROR: Не правильная структура JSON");
    return;
  }
}
// {
//   "lock_is_open" : true,
//   "auto_close_timer" : 5
// }

void loop()
{
  connManager.update();
  lockManager.update();
  buzzer.update();

  if (!connManager.isWifiConnect() || !connManager.isMqttConnect())
  {
    lockManager.close();
  }

  unsigned long currentMillis = millis();

  if (currentMillis - getStatePrevMillis >= GET_STATE_DELAY)
  {
    JSONVar statusJSON = impCheker.getStatus();

    bool shock_impact = statusJSON["shock_impact"]["shock_detected"];
    bool temp_impact = statusJSON["temperature_impact"]["overheat_detected"];

    if (shock_impact || temp_impact)
    {
      int impactType = 0;

      if (shock_impact & temp_impact)
      {
        impactType = 2;
      }
      else if (shock_impact)
      {
        impactType = 1;
      }

      buzzer.startAlarm(ALARM_DURATION, melody_alarm, melody_alarm_duration, melody_length, impactType);
      lockManager.close();
    }

    if ((shock_impact || temp_impact) || buzzer.isActive() ||
     currentMillis - sendStatePrevMillis >= send_state_delay ||
      prevLockState > lockManager.getLockState())
    {
      if (buzzer.isActive())
      {
        if (buzzer.getImpactType() >= 1)
        {
          statusJSON["shock_impact"]["shock_detected"] = true;
        }

        if (buzzer.getImpactType() == 0 || buzzer.getImpactType() == 2)
        {
          statusJSON["temperature_impact"]["overheat_detected"] = true;
        }
        
      }
      statusJSON["timestamp"] = getCurrentDateTime();
      statusJSON["lock_is_open"] = lockManager.getLockState();
      String jsonString = JSON.stringify(statusJSON);
      connManager.sendJSON(jsonString);
      sendStatePrevMillis = currentMillis;
    }

    getStatePrevMillis = currentMillis;
    getAlarmStatePrevMillis = currentMillis;
    prevLockState = lockManager.getLockState();
  }
}