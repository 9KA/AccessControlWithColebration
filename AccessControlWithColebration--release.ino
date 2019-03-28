#include <SPI.h> // Библиотека для работы по SPI (Serial Peripheral Interface)
#include <MFRC522.h> //Библиотека для работы с модулем MFRC522
#include <Thread.h>

// Прекомпиляторы для удобства
#define SS_PIN 10
#define RST_PIN 9

#define RED_LED 6
#define GREEN_LED 7

MFRC522 rfid(SS_PIN, RST_PIN); // Передаем к конструктор пины подключения

MFRC522::MIFARE_Key key;

int* code; // Массив с UID карты разблокировки (приложеной при калибровке)

void setup() {
    // Устанавливаем режим работы пинов с светодиодами как выход
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);

    SPI.begin(); // Инициализация SPI
    rfid.PCD_Init(); // Инициализация MFRC522
    Calibration(); //Начало калибровки
}

void loop() {
    if (rfid.PICC_IsNewCardPresent()) { // PICC_IsNewCardPresent() метод обьекта rfid, возвращает true если приложена новая карта
        readAndCheckRFID();
    }
}

void readAndCheckRFID() { // Функция чтения и обработки (сравнения) UID приложенной карты с UID карты разблокировки

    rfid.PICC_ReadCardSerial(); // Читаем тип карты
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

    // Проверяем карту на MIFARE Classic
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        //Карта не типа  MIFARE Classic
        BlinkSec(1000, RED_LED);
        return;
    }

    boolean match = true;
    for (int i = 0; i < rfid.uid.size; i++) { //Сравнения UID приложенной карточки с UID карточки разблокировки

        if (!(int(rfid.uid.uidByte[i]) == int(code[i]))) {
            match = false;
        }
    }

    if (match) { // Проверка на соответсвие
        // UID приложенной карточки совпало с UID карточки разблокировки
        BlinkSec(1000, GREEN_LED);
    }
    else {
        // UID приложенной карточки НЕ сходится с UID карточки разблокировки
        BlinkSec(1000, RED_LED);
    }

    // Останавливаем PICC
    rfid.PICC_HaltA();

    // Останавливаем шифрование на PCD
    rfid.PCD_StopCrypto1();
}

void Calibration() { // Функция калибровки (записи) uid карты разблокировки
    bool checked = false;
    
    //Подаём ток на светодиоды
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    while (!checked) {
        if (rfid.PICC_IsNewCardPresent()) {
            if (NeededType(GetType())) {
                checked = true;
            }
        }
    }
    
    // Динамический массив с размером UID
    code = new int[rfid.uid.size];
    for (int i = 0; i < rfid.uid.size; i++) {
        code[i] = int(rfid.uid.uidByte[i]); //Заполнение массива
    }

    //Перестаем подавать ток на светодиоды
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    
    delay(1500);  //Задержка для удобства работы с устройства
    //Калибровка завершена
}

bool NeededType(MFRC522::PICC_Type piccType) { //Функция проверки типа приложеной карты на тип MIFARE Classic
    if (piccType == MFRC522::PICC_TYPE_MIFARE_MINI || piccType == MFRC522::PICC_TYPE_MIFARE_1K || piccType == MFRC522::PICC_TYPE_MIFARE_4K) {
        //Карта типа  MIFARE Classic
        return true;
    }
    else {
      //Карта не типа  MIFARE Classic
        return false;
    }
}
MFRC522::PICC_Type GetType() { // Функция возвращающая тип приложеной карты 
    rfid.PICC_ReadCardSerial(); // Читаем тип карты
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    return piccType; // Возвращаем тип карты
}

void BlinkSec(int sec, int pin) { //Фукнция, подает ток и через указанное время (int sec) перестает подавать ток на указанный пин (pin)
    digitalWrite(pin, HIGH);
    delay(sec);
    digitalWrite(pin, LOW);
}
