#ifndef BUZZER_ALARM_H
#define BUZZER_ALARM_H

#include <Arduino.h>
#include "pitches.h"  // Файл с нотами 

class BuzzerAlarm {
private:
    const uint8_t buzzerPin;  // Пин бузера
    bool isAlarmActive;       // Флаг активности
    unsigned long startTime;  // Время начала сигнала
    unsigned long duration;   // Длительность (мс)
    unsigned long lastNoteTime; // Время последней ноты
    int currentNote;         // Текущая нота в мелодии
    const int* melody;       // Указатель на массив нот
    const int* noteDurations; // Длительности нот
    int melodyLength;        // Длина мелодии
    int noteDuration;        // Длительность текущей ноты (мс)
    int impactType;

public:
    // Конструктор
    BuzzerAlarm(uint8_t pin) : 
        buzzerPin(pin), isAlarmActive(false), 
        startTime(0), duration(0), lastNoteTime(0),
        currentNote(0), melody(nullptr), noteDurations(nullptr),
        melodyLength(0), noteDuration(0) {
        pinMode(buzzerPin, OUTPUT);
    }

    // Запуск сигнала с мелодией (длительность в мс)
    void startAlarm(unsigned long durationMs, const int* melodyArray, 
                   const int* durationsArray, int length, int impType) {
        if (durationMs == 0 || length == 0) return;
        isAlarmActive = true;
        duration = durationMs;
        startTime = millis();
        melody = melodyArray;
        noteDurations = durationsArray;
        melodyLength = length;
        currentNote = 0;
        impactType = impType;
        playNextNote();
    }

    // Принудительное выключение
    void endAlarm() {
        noTone(buzzerPin);
        isAlarmActive = false;
    }

    // Обновление состояния (вызывать в loop())
    void update() {
        if (!isAlarmActive) return;

        // Проверка времени сигнала
        if (millis() - startTime >= duration) {
            endAlarm();
            return;
        }

        // Переключение нот
        if (millis() - lastNoteTime >= noteDuration) {
            playNextNote();
        }
    }

    // Воспроизведение следующей ноты
    void playNextNote() {
        if (currentNote >= melodyLength) {
            currentNote = 0;  // Зацикливаем мелодию
        }

        noteDuration = 1000 / noteDurations[currentNote];  // Длительность в мс
        tone(buzzerPin, melody[currentNote], noteDuration * 0.9); // 90% длительности
        lastNoteTime = millis();
        currentNote++;
    }

    bool isActive() const {
        return isAlarmActive;
    }

    bool getImpactType() const
    {
        return impactType;
    }
};

#endif