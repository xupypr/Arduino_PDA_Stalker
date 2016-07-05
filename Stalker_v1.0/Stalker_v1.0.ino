//Ver 1.1 (c) xupypr vk.com/xupypr_grodno
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 6, 7, 8, 9);
//Id
byte Device_id=1, Group_id=1; //ID устройства, ID группировки 
//Мертв/жив
bool Death=false, DeathSignal=false; //Мертв ли сталкер, Повторение сигнала смерти пока не подтвердишь
byte DeathCause=0; //Причина смерти
long Health=100000;
long MaxHealth=100000,PrHealth=101000, PrHealthWrite=100000;; //Максимальное значение жизни
byte TreatKoef=100; //Коэффициент лечения в процентах
//Батарея
bool LowVolt=false; //низкий заряд батареи
byte Volt, BatICO=123; //Заряд батареи в процентах и адрес иконки батареи в зависимости от заряда
//Радиация и лечение
byte RadiationKoef=100, MinRadiationKoef=100; //Коэффициент воздействия радиации на игрока
unsigned int RadiationLevel=0, TreatLevel=0; //общий уровень радиации и лечилки
unsigned int BonusTreat=0; //Для реализации бонуса "Зализывая раны"
unsigned int TreatArtefact=0;
bool TreatBar=false; // бонус "Завсегдатай"
bool TreatBaza=false; //бонус "Член банды"
//Устройства, кнопки, пины
int OutDevice[13] = {70, 120, 198, 276, 361, 454, 532, 605, 676, 756, 859, 943, 953}; //АЦП артефактов и админ устройств
int KeysACP[5] = {890, 697, 512, 341, 776}; // АЦП кнопок
byte DeviceID=0, ArtefaktID[2]={0,0}; //Номер подключенного устройства
bool Device[2]={false,false}, Artefakt[2]={false, false}, Key=false; //Логика устройств внешних и нажатия кнопок меню
byte VbatPin=0, AdminDevPin[2]={6,3},ArtefaktPin[2]={7,2}, KeysPin=1, SpeakerPin=10, LedPin=9; //Пины контроллера
//Дисплей
int DisplayLedTime=30000; // Время подсветки дисплея
byte DisplayBright=200, DisplayPage=31; // Якрость подсветки дисплея и номер отображаемой страницы дисплея
bool DisplayChange=false; // Переключение дисплея при воздействии заразы
bool LCD_Sleep=false; //Спит ли дисплей
bool ArtInfo=false; //Показать инфо об артефакте
byte ArtPgNum=0; //Номер Артефакта, который отображает на дисплее
int ArtInfoTime=5000;
//СМС
byte Messages=0; //Кол-во непрочитанных сообщений
char M[70]={0}; //Для вывода сообщений на экран
//Болезнь
bool Zaraza=false;//Гадость
int Poison=0, PoisonAffect=0; // Уровень яда
byte PoisonKoef=100, MinPoisonKoef=100; //Коэффициент воздействия яда на игрока
//Динамик
int SpeakerTime=0,SpeakerWaitTime=0,SpeakerRepeat=0; //Время сигнала, между сигнлами, повторение сигналов
bool SpeakerWait=false, SpeakerReady=true, Mute=false; //запрет одновременного использования пищалки и время между сигналами, глушилка звука
//Отслеживать быстродействие
//bool Pin13=false;
//Статистика
unsigned int DeathCount=0;
//WiFI
unsigned int NewTreat=0, NewRadiation=0;
bool WiFiRefresh=false;
byte SignalLevel=0;
char buf[14];
//Счетчики
unsigned long PrMillisRad=0, PrMillisBat=0, PrMillisTreat=0, MillisSpeaker=0, PrDisplayLedTime=0, PrMillisChangeDisplayPg=0,PrMillisPoison=0,
PrMillisDisplayRefresh=0, PrArtInfoTime=0,PrMillisAdminDevice[2]={0,0},PrMillisArtefaktDevice[2]={0,0}, PrMillisPushKey=0, PrMillisWifiRefresh=0; //Для отсчета времени

void setup() {
if (EEPROM.read(82)!=58) {EEPROM_CLEAR();Signal(5);EEPROM.write(82,58);}
EEPROM_READ();
NoReadMessages();
//  pinMode(LedPin, OUTPUT);
//  analogWrite(LedPin, DisplayBright);
  pinMode(SpeakerPin, OUTPUT);
  pinMode(13, OUTPUT);
  display.begin();
  display.setContrast(60);
  delay(2000);
  PrintUserInfo();
  delay(5000);
  LCD(1);
 //   Serial.begin(9600);  // Debugging only
 //   Serial.println("setup"); 
 mySerial.begin(9600);
    Volt=GetVoltage(VbatPin, 6.5, 8.0, 2.0);
    digitalWrite(SpeakerPin, LOW);
    randomSeed(analogRead(4));
TreatLevel=BonusTreat;
}

void EEPROMWriteInt(int p_address, int p_value)
        {
        byte lowByte = ((p_value >> 0) & 0xFF);
        byte highByte = ((p_value >> 8) & 0xFF);

        EEPROM.write(p_address, lowByte);
        EEPROM.write(p_address + 1, highByte);
        }


unsigned int EEPROMReadInt(int p_address)
        {
        byte lowByte = EEPROM.read(p_address);
        byte highByte = EEPROM.read(p_address + 1);

        return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
        }

void PrintUserInfo() {
display.clearDisplay();
display.setCursor(0, 0);
M[0]='(';M[1]='с';M[2]=')';M[3]='x';M[4]='u';M[5]='p';M[6]='y';M[7]='p';M[8]='r';M[9]=' ';M[10]='2';M[11]='0';M[12]='1';M[13]='6';M[14]='v';M[15]='e';M[16]='r';M[17]='.';M[18]=' ';M[19]='1';M[20]='.';M[21]='0';M[22]=' ';M[23]=' ';M[24]=' ';M[25]=' ';M[26]=' ';M[27]=' ';M[28]='В';M[29]='л';M[30]='а';M[31]='д';M[32]='е';M[33]='л';M[34]='е';M[35]='ц';M[36]=' ';M[37]='П';M[38]='Д';M[39]='А';M[40]=':';M[41]=' ';
M[42]='С';M[43]='т';M[44]='а';M[45]='л';M[46]='к';M[47]='е';M[48]='р';M[49]=':';M[50]=' ';M[51]=' ';M[52]=' ';M[53]=' ';M[54]=' ';M[55]=' ';
M[56]='П';M[57]='р';M[58]='и';M[59]='з';M[60]='p';M[61]='а';M[62]='к';M[63]=' ';M[64]=' ';M[65]=' ';M[66]=' ';M[67]=' ';M[68]=' ';M[69]=' ';M[70]=0;
for (byte i=0;M[i]!=0;i++) display.write(M[i]);
display.display();
}
void EEPROM_CLEAR() {
  for (byte i=0;i<80;i++) EEPROM.write(i,0);
  for (byte i=83;i<101;i++) EEPROM.write(i,0);
  EEPROM.write(80,100);
  EEPROM.write(81,0);
}
void EEPROM_READ() {
DeathCount=EEPROMReadInt(84);
DeathCause=EEPROM.read(83);
//Health=EEPROM.read(80)*1000;
//Death=EEPROM.read(81);
PrHealthWrite=Health;
for (byte i=0;i<10;i++) if (EEPROM.read(i+30)==1) {Zaraza=true;Affect(i);break;};
}


void GetWifi() {

if (!WiFiRefresh) {mySerial.println("AT+CWLAP");PrMillisWifiRefresh=millis();WiFiRefresh=true;}

if (millis()-PrMillisWifiRefresh>5000 && WiFiRefresh) {
  NewRadiation=0;
  NewTreat=0;
  while (mySerial.available()) {
    for (int i=0;i<13;i++) {buf[i]=buf[i+1];}
    buf[13]=mySerial.read(); 
    if (buf[0]=='T' && buf[1]=='r' && buf[2]=='e' && buf[3]=='A') {
    NewTreat+=(buf[5]-48)*1000+(buf[6]-48)*100+(buf[7]-48)*10+buf[8]-48;
    }
      if (!Death) {
        if (buf[0]=='D' && buf[1]=='e' && buf[2]=='a' && buf[3]=='d' && buf[4]=='H') {DeathCause=3;Death_Stalker();} //Бомба
          if (buf[0]=='M' && buf[1]=='e' && buf[2]=='s' && buf[3]=='s' && buf[4]=='A') { //Сообщение
          NewSMS(buf[5], buf[6], buf[7], buf[8], buf[9], buf[10]);
          }
             if (buf[0]=='R' && buf[1]=='a' && buf[2]=='d' && buf[3]=='i' && buf[4]=='A') { //Радиация
             SignalLevel=100-((buf[12]-48)*10+buf[13]-48);
             NewRadiation+=((buf[5]-48)*1000+(buf[6]-48)*100+(buf[7]-48)*10+buf[8]-48) * SignalLevel / 10;
             }
      }
  }
  RadiationLevel=NewRadiation;
  TreatLevel=NewTreat+BonusTreat+TreatArtefact;
  WiFiRefresh=false;
}
}


void ReadNewSMS(byte MesNum) {
  /*
  if (MesNum<5) {
  if (EEPROM.read(MesNum+65)==0) {
  for (int i=0;i<5;i++) {if (i!=MesNum) {if (EEPROM.read(i+25)==1) EEPROM.write(i+25,0);if (EEPROM.read(i+65)==1) EEPROM.write(i+65,0);}
  else {EEPROM.write(i+25,1);EEPROM.write(i+65,1);}}
  byte cnt=0;
  for (byte i=11;buf[i]!=0;i++) {MA[cnt]=buf[i];cnt++;}
  MA[cnt]=0;
  Signal(6);
  NoReadMessages();
  }}
  */
  if (MesNum>4 && MesNum<30) {
   if (EEPROM.read(MesNum+35)==0) {
    EEPROM.write(MesNum-5,1);
    EEPROM.write(MesNum+35,1);
    Signal(6);
    NoReadMessages();
   }
  }
    if (MesNum>29 && MesNum<40) {
      if (EEPROM.read(MesNum+40)==0 && !Zaraza) {
        Zaraza=true;
        EEPROM.write(MesNum,1);
        EEPROM.write(MesNum+40,1);
        Affect(MesNum-30);
        Signal(9);
      }
    }
}
void NewSMS(byte MesNum, byte Rate, byte Id_group, byte Id_dev, byte Id_groupExclude, byte ID_deviceExclude) {
if (Device_id!=ID_deviceExclude || ID_deviceExclude==0) 
if (Group_id!=Id_groupExclude || Id_groupExclude==0)  
if (Id_dev==Device_id || Id_dev==0)
if (Id_group==Group_id || Id_group==0){
  byte rnd_rate=random(100);
  rnd_rate++;
  if (rnd_rate<=Rate) ReadNewSMS(MesNum);
  }
}


void Radiation_zone () { //Радиация
    if (!Death)
    if ((millis()-PrMillisRad)>=1000) {
      PrMillisRad=millis();
      Health-=RadiationLevel*RadiationKoef/100;
      DeathCause=0;
    }
}
void Treat_zone () { //Лечилка
    if ((millis()-PrMillisTreat)>=1000) {
    PrMillisTreat=millis();
    Health+=TreatLevel*TreatKoef/100;
    }
if (DeathSignal && TreatLevel>0) {DeathSignal=false;LCD(1);}
}

void Life() { // Проверка жизни сталкера
if (Health>MaxHealth) Health=MaxHealth;
if (Health<=0 && !Death) Death_Stalker();
if (Health>=MaxHealth && Death) Restore_Stalker();
if (Health>PrHealthWrite && Health-PrHealthWrite>=20000) {EEPROM.write(80, byte(Health/1000));PrHealthWrite=Health;}
if (Health<PrHealthWrite && PrHealthWrite-Health>=20000) {EEPROM.write(80, byte(Health/1000));PrHealthWrite=Health;}
}

void Death_Stalker() { // Сталкер умер
  DeathCount++;
  TreatArtefact=0;
  Artefakt[0]=false;Artefakt[1]=false;
  ArtefaktID[0]=0;ArtefaktID[1]=0;
  RadiationKoef=MinRadiationKoef;
  PoisonKoef=MinPoisonKoef;
  Poison=0;
  Death=true;
  DeathSignal=true;
  Messages=0;
  Zaraza=false;
  for (byte k=0;k<10;k++) if (EEPROM.read(k+30)==1) EEPROM.write(k+30,0);
  Health=0;
  EEPROM.write(80, 0);
  EEPROM.write(81,1);
  EEPROMWriteInt(84, DeathCount);
  EEPROM.write(83, DeathCause);
  PrHealthWrite=0;
  LCD(1);
  Signal(1);
}

void Restore_Stalker() { //Сталкер ожил
  Death=false;
  DeathSignal=false;
  LCD(1);
  Signal(2);
  EEPROM.write(80, MaxHealth);
  PrHealthWrite=MaxHealth;
  EEPROM.write(81,0);
  NoReadMessages();
}

void SignalRadiation() { //Подача сигнала радиации (счетчик гейгера)
  if (SpeakerReady && !Mute) {
  int SignMn=0;
  if (RadiationLevel<1000) SignMn=RadiationLevel*2 + random(250);
  if (RadiationLevel>=1000 && RadiationLevel<=3) SignMn=2000 + RadiationLevel*65/1000 + random(100);
  if (RadiationLevel>3000) SignMn=2200+random(100);
  if (millis()-MillisSpeaker > (2300 - SignMn)) {
    MillisSpeaker=millis();
    digitalWrite(SpeakerPin, HIGH);                         
    delay(3);          
    digitalWrite(SpeakerPin, LOW); 
  }
  }
}

void Signal(byte Val) { //Подача сигналов пищалкой
switch (Val) {
  case 0:
  if ((millis()-MillisSpeaker>=SpeakerTime) && !SpeakerReady && !SpeakerWait) {
    if (SpeakerRepeat>=1) SpeakerRepeat--;
    if (SpeakerRepeat==0) {SpeakerReady=true;SpeakerWait=false;} else SpeakerWait=true;
    MillisSpeaker=millis();
    digitalWrite(SpeakerPin, LOW); 
  }
  if ((millis()-MillisSpeaker>=SpeakerWaitTime) && !SpeakerReady && SpeakerWait) {
    SpeakerWait=false;
    MillisSpeaker=millis();
    digitalWrite(SpeakerPin, HIGH);
  }
  break;
  case 1:
  //умер
  digitalWrite(SpeakerPin, HIGH);                                                
  SpeakerTime=10000;
  SpeakerRepeat=1;
  SpeakerReady=false;MillisSpeaker=millis();
  break;
  case 2:
  //восстановился
  digitalWrite(SpeakerPin, HIGH);                         
  SpeakerTime=5000; 
  SpeakerRepeat=1;      
  SpeakerReady=false;MillisSpeaker=millis();
  break;
  case 5:
  //Села батарея
  digitalWrite(SpeakerPin, HIGH);
  SpeakerRepeat=5;
  SpeakerWaitTime=30;
  SpeakerTime=30; 
  SpeakerReady=false;MillisSpeaker=millis();
  break;
  case 8:
  //Писк кнопки
  digitalWrite(SpeakerPin, HIGH);                          
  SpeakerTime=50;
  SpeakerRepeat=1;
  SpeakerReady=false;MillisSpeaker=millis();
  break;
  case 9:
  //Подцепил заразу
  digitalWrite(SpeakerPin, HIGH);                         
  SpeakerRepeat=8;
  SpeakerTime=100; 
  SpeakerWaitTime=50;
  SpeakerReady=false;MillisSpeaker=millis();
  break;
}
  if (!Mute)  switch (Val) {
  case 3:
  //админ устройство Артефакт подключены
  digitalWrite(SpeakerPin, HIGH);                         
  SpeakerRepeat=2;
  SpeakerWaitTime=50;
  SpeakerTime=100; 
  SpeakerReady=false;MillisSpeaker=millis();
  break;
  case 4:
  //админ устройство Артефакт отключены
  digitalWrite(SpeakerPin, HIGH);                         
  SpeakerRepeat=2;
  SpeakerWaitTime=50;
  SpeakerTime=50; 
  SpeakerReady=false;MillisSpeaker=millis();
  break;
  case 6:
  //SMS
  digitalWrite(SpeakerPin, HIGH);                         
  SpeakerRepeat=3;
  SpeakerWaitTime=50;
  SpeakerTime=150; 
  SpeakerReady=false;MillisSpeaker=millis();
  break;
  case 7:
  //Дурной эффект
  digitalWrite(SpeakerPin, HIGH);                         
  SpeakerRepeat=1;
  SpeakerTime=300; 
  SpeakerReady=false;MillisSpeaker=millis();
  break; 
  }
}

void LCD_Refresh(){
  if (!LCD_Sleep) LCD(2);
}

int GetVoltage(int PinBat, float MinVolt, float MaxVolt, float KF) { //Проверка заряда батареи
  float Min=(MinVolt*204.8)/KF;
  float Max=(MaxVolt*204.8)/KF;
  int Volt1 = analogRead(PinBat);
  int VoltPerc=0;
  float Ind1 = (Max-Min) / 100;
  if (Volt1 < Min) {VoltPerc=0;}
  else {
  VoltPerc = (Volt1 - Min) / Ind1;
  if (VoltPerc>100) VoltPerc=100;}
  if ((VoltPerc<10) && !LowVolt) {LowVolt=true; DisplayLedTime=5000; DisplayBright=50;}
  if ((VoltPerc>10) && LowVolt) {LowVolt=false; DisplayLedTime=30000; DisplayBright=200;}
  if (VoltPerc/20 >= 5) BatICO=127;
  else BatICO=123+VoltPerc/20;
  if (LowVolt) Signal(5);
  return VoltPerc;
}

void PrintSMS() {
display.setCursor(0,8);
switch(DisplayPage) {
  case 0:
M[0]='Д';M[1]='о';M[2]='б';M[3]='р';M[4]='о';M[5]=' ';M[6]='п';M[7]='о';M[8]='ж';M[9]='а';M[10]='л';M[11]='о';M[12]='в';M[13]='а';M[14]='т';M[15]='ь';M[16]=' ';M[17]='в';M[18]=' ';M[19]='З';M[20]='о';M[21]='н';M[22]='у';M[23]='!';M[24]='!';M[25]='!';M[26]=0;
  break;
  case 1:
M[0]='К';M[1]='у';M[2]='п';M[3]='л';M[4]='ю';M[5]=' ';M[6]='х';M[7]='о';M[8]='б';M[9]='о';M[10]='т';M[11]='ы';M[12]=' ';M[13]='к';M[14]='р';M[15]='о';M[16]='в';M[17]='о';M[18]='с';M[19]='о';M[20]='с';M[21]='о';M[22]='в';M[23]='!';M[24]=' ';M[25]='Д';M[26]='о';
M[27]='р';M[28]='о';M[29]='г';M[30]='о';M[31]='!';M[32]=' ';M[33]='6';M[34]='8';M[35]='-';M[36]='7';M[37]='7';M[38]='-';M[39]='8';M[40]='9';M[41]=0;
  break;
  case 2:
M[0]='В';M[1]='с';M[2]='т';M[3]='у';M[4]='п';M[5]='а';M[6]='й';M[7]=' ';M[8]='в';M[9]=' ';M[10]='р';M[11]='я';M[12]='д';M[13]='ы';M[14]=' ';M[15]='с';M[16]='в';M[17]='о';M[18]='б';M[19]='о';M[20]='д';M[21]='ы';M[22]=' ';M[23]='с';M[24]='ы';M[25]='н';M[26]='о';
M[27]='к';M[28]='!';M[29]='!';M[30]='!';M[31]=0;
  break;
  case 3:
M[0]='П';M[1]='о';M[2]='г';M[3]='о';M[4]='в';M[5]='а';M[6]='р';M[7]='и';M[8]='в';M[9]='а';M[10]='ю';M[11]='т';M[12]=' ';M[13]='х';M[14]='о';M[15]='д';M[16]='о';M[17]='к';M[18]='и';M[19]=' ';M[20]='т';M[21]='а';M[22]='щ';M[23]='а';M[24]='т';M[25]=' ';M[26]='б';
M[27]='о';M[28]='м';M[29]='б';M[30]='у';M[31]=' ';M[32]='в';M[33]=' ';M[34]='з';M[35]='о';M[36]='н';M[37]='у';M[38]='!';M[39]=' ';M[40]='К';M[41]='а';M[42]='к';M[43]=' ';M[44]='б';M[45]='ы';M[46]=' ';M[47]='д';M[48]='е';M[49]='л';M[50]='о';M[51]='в';M[52]=' ';
M[53]='н';M[54]='е';M[55]=' ';M[56]='н';M[57]='а';M[58]='в';M[59]='о';M[60]='р';M[61]='о';M[62]='т';M[63]='и';M[64]='л';M[65]='и';M[66]='!';M[67]=0;
  break;
  case 4:
M[0]='П';M[1]='р';M[2]='и';M[3]='р';M[4]='у';M[5]='ч';M[6]='у';M[7]=' ';M[8]='к';M[9]='р';M[10]='о';M[11]='в';M[12]='о';M[13]='с';M[14]='о';M[15]='с';M[16]='а';M[17]='!';M[18]=' ';M[19]='Н';M[20]='е';M[21]=' ';M[22]='д';M[23]='о';M[24]='р';M[25]='о';M[26]='г';
M[27]='о';M[28]='!';M[29]=' ';M[30]='т';M[31]='е';M[32]='л';M[33]='.';M[34]=' ';M[35]='7';M[36]='7';M[37]='-';M[38]='6';M[39]='7';M[40]='-';M[41]='7';M[42]='7';M[43]=0;
  break;
  case 5:
M[0]='У';M[1]='д';M[2]='а';M[3]='ч';M[4]='н';M[5]='о';M[6]='й';M[7]=' ';M[8]='д';M[9]='о';M[10]='р';M[11]='о';M[12]='г';M[13]='и';M[14]=' ';M[15]='т';M[16]='е';M[17]='б';M[18]='е';M[19]=' ';M[20]='с';M[21]='т';M[22]='а';M[23]='л';M[24]='к';M[25]='е';M[26]='р';
M[27]=' ';M[28]='м';M[29]='и';M[30]='м';M[31]='о';M[32]=' ';M[33]='а';M[34]='н';M[35]='о';M[36]='м';M[37]='а';M[38]='л';M[39]='и';M[40]='й';M[41]=' ';M[42]='ч';M[43]='е';M[44]='р';M[45]='е';M[46]='з';M[47]=' ';M[48]='а';M[49]='р';M[50]='т';M[51]='е';M[52]='ф';
M[53]='а';M[54]='к';M[55]='т';M[56]='ы';M[57]='!';M[58]=0;
  break;
  case 6:

  break;
  case 7:

  break;
  case 8:

  break;
  case 9:

  break;
  case 10:

  break;
  case 11:

  break;
  case 12:

  break;
  case 13:

  break;
  case 14:

  break;
  case 15:

  break;
  case 16:

  break;
  case 17:

  break;
  case 18:

  break;
  case 19:

  break;
  case 20:

  break;
  case 21:

  break;
  case 22:

  break;
  case 23:

  break;
  case 24:

  break;
}
if (DisplayPage<25)  for (int i=0;M[i]!=0;i++) display.write(M[i]);
//else for (int i=0;MA[i]!=0;i++) display.write(MA[i]);
}

void PrintArtefaktInfo(byte Num) {
M[0]='А';M[1]='р';M[2]='т';M[3]='е';M[4]='ф';M[5]='а';M[6]='к';M[7]='т';M[8]=' ';M[10]=':';M[11]=0;
if (Num==0) M[9]='1'; else M[9]='2';
display.setCursor(0,8);
for (int i=0;M[i]!=0;i++) display.write(M[i]);
display.setCursor(0,16);
if (Artefakt[Num]) {
ArtInfoTime=5000;
 switch (ArtefaktID[Num]) {
 case 1:
  break;
  case 2:
  break;
  case 3:
  break;
  case 4:
  break;
  case 5:
  break;
  case 6:
  M[0]='К';M[1]='о';M[2]='л';M[3]='ю';M[4]='ч';M[5]='к';M[6]='а';M[7]=':';M[8]=' ';M[9]=' ';M[10]=' ';M[11]=' ';M[12]=' ';M[13]=' ';M[14]='+';M[15]='1';M[16]=' ';M[17]='з';M[18]='д';M[19]='/';M[20]='м';M[21]='и';M[22]='н';M[23]=' ';M[24]=' ';M[25]=' ';
  M[26]=' ';M[27]=' ';M[28]='-';M[29]='1';M[30]='5';M[31]='%';M[32]=' ';M[33]=' ';M[34]='р';M[35]='а';M[36]='д';M[37]='и';M[38]='а';M[39]='ц';M[40]='и';M[41]='и';M[42]='+';M[43]='2';M[44]='0';M[45]='%';M[46]=' ';M[47]='и';M[48]='м';M[49]='м';M[50]='у';
  M[51]='н';M[52]='и';M[53]='т';M[54]='е';M[55]='т';M[56]=0;
  break;
  case 7:
  break;
  case 8:
  break;
  case 9:
  break;
  case 10:
  break;
  case 11:
  break;
 }
} else {
  ArtInfoTime=1500;
  M[0]='Н';M[1]='е';M[2]='т';M[3]=' ';M[4]='а';M[5]='р';M[6]='т';M[7]='е';M[8]='ф';M[9]='а';M[10]='к';M[11]='т';M[12]='а';M[13]='!';M[14]=0;
}
for (int i=0;M[i]!=0;i++) display.write(M[i]);
}
void PrintZarazaMessage() {
byte Zar=0;
display.setCursor(0,8);
for (int i=0;i<10;i++) if (EEPROM.read(i+30)==1) Zar=i;
switch (Zar) {
  case 0:
M[0]='П';M[1]='о';M[2]='я';M[3]='в';M[4]='и';M[5]='л';M[6]='а';M[7]='с';M[8]='ь';M[9]=' ';M[10]='с';M[11]='т';M[12]='р';M[13]='а';M[14]='н';M[15]='н';M[16]='а';M[17]='я';M[18]=' ';M[19]='с';M[20]='ы';M[21]='п';M[22]='ь';M[23]='!';M[24]=' ';M[25]='Н';
M[26]='а';M[27]='д';M[28]='о';M[29]=' ';M[30]='б';M[31]='ы';M[32]=' ';M[33]='о';M[34]='б';M[35]='р';M[36]='а';M[37]='т';M[38]='и';M[39]='т';M[40]='ь';M[41]='с';M[42]='я';M[43]=' ';M[44]='к';M[45]=' ';M[46]='у';M[47]='ч';M[48]='е';M[49]='н';M[50]='ы';
M[51]='м';M[52]=',';M[53]=' ';M[54]='п';M[55]='о';M[56]='к';M[57]='а';M[58]=' ';M[59]='н';M[60]='е';M[61]=' ';M[62]='п';M[63]='о';M[64]='м';M[65]='е';M[66]='р';M[67]='!';M[68]=0;
  break;
  case 1:
M[0]='Х';M[1]='е';M[2]='р';M[3]='о';M[4]='в';M[5]='о';M[6]=' ';M[7]='с';M[8]='е';M[9]='б';M[10]='я';M[11]=' ';M[12]='ч';M[13]='у';M[14]='с';M[15]='т';M[16]='в';M[17]='у';M[18]='ю';M[19]='!';M[20]=' ';M[21]='П';M[22]='о';M[23]='х';M[24]='о';M[25]='ж';
M[26]='е';M[27]=' ';M[28]='н';M[29]='а';M[30]=' ';M[31]='в';M[32]='и';M[33]='р';M[34]='у';M[35]='с';M[36]=' ';M[37]='к';M[38]='р';M[39]='о';M[40]='в';M[41]='о';M[42]='с';M[43]='в';M[44]='и';M[45]='н';M[46]='к';M[47]='и';M[48]='!';M[49]=' ';M[50]='а';
M[51]='н';M[52]='т';M[53]='и';M[54]='д';M[55]='о';M[56]='т';M[57]=' ';M[58]='у';M[59]=' ';M[60]='б';M[61]='а';M[62]='р';M[63]='м';M[64]='е';M[65]='н';M[66]='а';M[67]='!';M[68]=0;
  break;
  case 2:
M[0]='К';M[1]='а';M[2]='к';M[3]='а';M[4]='я';M[5]='-';M[6]='т';M[7]='о';M[8]=' ';M[9]='х';M[10]='е';M[11]='р';M[12]='ь';M[13]=' ';M[14]='у';M[15]='к';M[16]='у';M[17]='с';M[18]='и';M[19]='л';M[20]='а';M[21]=' ';M[22]='в';M[23]=' ';M[24]='п';M[25]='р';
M[26]='а';M[27]='в';M[28]='у';M[29]='ю';M[30]=' ';M[31]='р';M[32]='у';M[33]='к';M[34]='у';M[35]='!';M[36]=' ';M[37]='Р';M[38]='у';M[39]='к';M[40]='а';M[41]=' ';M[42]='п';M[43]='а';M[44]='р';M[45]='а';M[46]='л';M[47]='и';M[48]='з';M[49]='о';M[50]='в';
M[51]='а';M[52]='н';M[53]='а';M[54]=' ';M[55]='д';M[56]='о';M[57]=' ';M[58]='п';M[59]='л';M[60]='е';M[61]='ч';M[62]='а';M[63]='!';M[64]=0;
  break;
  case 3:
M[0]='Я';M[1]=' ';M[2]='п';M[3]='о';M[4]='т';M[5]='е';M[6]='р';M[7]='я';M[8]='л';M[9]=' ';M[10]='з';M[11]='р';M[12]='е';M[13]='н';M[14]='и';M[15]='е';M[16]='!';M[17]=' ';M[18]='Н';M[19]='и';M[20]='ч';M[21]='е';M[22]='г';M[23]='о';M[24]=' ';M[25]='в';
M[26]='о';M[27]='о';M[28]='б';M[29]='щ';M[30]='е';M[31]=' ';M[32]='н';M[33]='е';M[34]=' ';M[35]='в';M[36]='и';M[37]='ж';M[38]='у';M[39]='!';M[40]=' ';M[41]='Г';M[42]='о';M[43]='в';M[44]='о';M[45]='р';M[46]='и';M[47]='л';M[48]='и';M[49]=' ';M[50]='ж';
M[51]='е';M[52]=' ';M[53]='З';M[54]='о';M[55]='н';M[56]='а';M[57]=' ';M[58]='т';M[59]='е';M[60]='б';M[61]='я';M[62]=' ';M[63]='п';M[64]='о';M[65]='г';M[66]='у';M[67]='б';M[68]='и';M[69]='т';M[70]=0;
  break;
  case 4: 

  break;
  case 5:

  break;
  case 6:

  break;
  case 7:

  break;
  case 8:

  break;
  case 9:

  break;
}
for (int i=0;M[i]!=0;i++) display.write(M[i]);

}
void PrintMessage() {
int k=0;
char KoefRad[3]={0,0,0};
sprintf(KoefRad, "%d", RadiationKoef);
char KoefPois[3]={0,0,0};
sprintf(KoefPois, "%d", PoisonKoef);
char PoisonLev[3]={0,0,0};
sprintf(PoisonLev, "%d", Poison/1000);
char Fon[4]={0,0,0,0};
sprintf(Fon, "%d", RadiationLevel);
for (k=0;Fon[3] == 0;k++) {
 Fon[3]=Fon[2];
 Fon[2]=Fon[1];
 Fon[1]=Fon[0];
 Fon[k]='0';
}
char TreatLev[4]={0,0,0,0};
sprintf(TreatLev, "%d", TreatLevel);
for (k=0;TreatLev[3] == 0;k++) {
 TreatLev[3]=TreatLev[2];
 TreatLev[2]=TreatLev[1];
 TreatLev[1]=TreatLev[0];
 TreatLev[k]='0';
}

if (!Death)
        if (DisplayPage==31) {
          if (!ArtInfo) {
            if (Zaraza && DisplayChange) {
            PrintZarazaMessage();
            } else {
            display.setCursor(0, 8);
            display.write('Ф');display.write('о');display.write('н');display.write(':');display.write(Fon[0]);display.write(Fon[1]);display.write(',');display.write(Fon[2]);display.write(Fon[3]);
            display.write('м');display.write('к');display.write('З');display.write(47);display.write('м');
            display.setCursor(0, 16);
            display.write('Я');display.write('д');display.write(':');display.write('-');display.write(PoisonLev[0]);display.write(PoisonLev[1]);display.write(PoisonLev[2]);display.write('ж');display.write('/');display.write('м');
            display.setCursor(0, 24);
            display.write('Б');display.write('л');display.write('а');display.write('г');display.write('о');display.write(':');display.write(TreatLev[0]);display.write(',');
            display.write(TreatLev[1]);display.write(TreatLev[2]);display.write(TreatLev[3]);display.write('ж');display.write('/');display.write('с');
            display.setCursor(0, 32);
            display.write('З');display.write('а');display.write('щ');display.write('.');display.write(' ');display.write('Р');display.write('а');display.write('д');display.write('.');display.write(':');
            display.write(KoefRad[0]);display.write(KoefRad[1]);display.write(KoefRad[2]);display.write('%');
            display.setCursor(0, 40);
            display.write('И');display.write('м');display.write('м');display.write('у');display.write('н');display.write('и');display.write('т');display.write('е');display.write('т');display.write(':');
            display.write(KoefPois[0]);display.write(KoefPois[1]);display.write(KoefPois[2]);display.write('%');
            } 
          } else PrintArtefaktInfo(ArtPgNum);
        } else {
            PrintSMS();
            }
if (Death) {
  if (TreatLevel>0) {
  display.setCursor(22,20);
  M[0]='Л';M[1]='е';M[2]='ч';M[3]='е';M[4]='н';M[5]='и';M[6]='е';M[7]=0;
 }
  else{
  display.setCursor(0,16);
  switch (DeathCause) {
    case 0:
    M[0]='Т';M[1]='ы';M[2]=' ';M[3]='у';M[4]='м';M[5]='е';M[6]='р';M[7]=' ';M[8]='о';M[9]='т';M[10]=' ';M[11]='р';M[12]='а';M[13]='д';M[14]='и';M[15]='а';M[16]='ц';M[17]='и';M[18]='и';M[19]='!';M[20]=' ';M[21]='И';M[22]='д';M[23]='и';M[24]=' ';
    M[25]='н';M[26]='а';M[27]=' ';M[28]='м';M[29]='е';M[30]='р';M[31]='т';M[32]='в';M[33]='я';M[34]='к';M[35]='!';M[36]=0;
    break;
    case 1:
    M[0]='Т';M[1]='ы';M[2]=' ';M[3]='у';M[4]='м';M[5]='е';M[6]='р';M[7]=' ';M[8]='о';M[9]='т';M[10]=' ';M[11]='с';M[12]='т';M[13]='р';M[14]='а';M[15]='н';M[16]='н';M[17]='о';M[18]='й';M[19]=' ';M[20]='б';M[21]='о';M[22]='л';M[23]='е';M[24]='з';
    M[25]='н';M[26]='и';M[27]='!';M[28]=' ';M[29]='И';M[30]='д';M[31]='и';M[32]=' ';M[33]='н';M[34]='а';M[35]=' ';M[36]='м';M[37]='е';M[38]='р';M[39]='т';M[40]='в';M[41]='я';M[42]='к';M[43]='!';M[44]=0;
    break;
    case 2:
    M[0]='Т';M[1]='ы';M[2]=' ';M[3]='у';M[4]='м';M[5]='е';M[6]='р';M[7]=' ';M[8]='о';M[9]='т';M[10]=' ';M[11]='с';M[12]='у';M[13]='и';M[14]='ц';M[15]='и';M[16]='д';M[17]='а';M[18]='!';M[19]=' ';M[20]='И';M[21]='д';M[22]='и';M[23]=' ';M[24]='н';
    M[25]='а';M[26]=' ';M[27]='м';M[28]='е';M[29]='р';M[30]='т';M[31]='в';M[32]='я';M[33]='к';M[34]='!';M[35]=0;
    break;
    case 3:
    M[0]='Т';M[1]='ы';M[2]=' ';M[3]='у';M[4]='м';M[5]='е';M[6]='р';M[7]=' ';M[8]='о';M[9]='т';M[10]=' ';M[11]='в';M[12]='з';M[13]='р';M[14]='ы';M[15]='в';M[16]='а';M[17]=' ';M[18]='б';M[19]='о';M[20]='м';M[21]='б';M[22]='ы';M[23]='!';M[24]=' ';
    M[25]='И';M[26]='д';M[27]='и';M[28]=' ';M[29]='н';M[30]='а';M[31]=' ';M[32]='м';M[33]='е';M[34]='р';M[35]='т';M[36]='в';M[37]='я';M[38]='к';M[39]='!';M[40]=0;
    break;
    case 4:
    break;
    case 5:
    break;
    case 6:
    break;
  }}
  for (byte i=0;M[i]!=0;i++) display.write(M[i]);
  }
}
void LCD(byte Val) { //Вывод инфы на дисплей
  if (Val==1) PrDisplayLedTime=millis();
  if (Val!=0) {
  char Heal[3]={0,0,0}, Mes[2]={0,0};
  //analogWrite(LedPin, DisplayBright);
  sprintf(Heal, "%d", byte(Health/1000));
  sprintf(Mes, "%d", Messages);
  LCD_Sleep=false;
  display.clearDisplay();
  display.setCursor(0, 0);
  //Хэлсы
  if (!Death) {
  display.write(95);display.setCursor(5, 0);display.write(96);}
  else display.write(92);
  display.write(Heal[0]);display.write(Heal[1]);display.write(Heal[2]);
  //Звук
  display.setCursor(36, 0);
  if (Mute) display.write(62); else display.write(60);
  //Сообщения
  display.setCursor(48, 0);display.write(93);display.setCursor(53, 0);display.write(94);display.setCursor(62, 0);display.write(Mes[0]);display.write(Mes[1]);
  //батарея
  display.setCursor(78, 0);display.write(BatICO);
  // Основной дисплей
  
PrintMessage();
  display.display(); 
  }   
  if (Val==0) {
//    analogWrite(LedPin, 0);
    display.clearDisplay();
    display.display();
    LCD_Sleep=true;
  }
}
void Affect (int Ind) { 
  switch (Ind) {
    case 0:
    Poison+=6000;
    PoisonAffect=6000;
    break;
    case 1:
    Poison+=6000;
    PoisonAffect=6000;
    break;
    case 2:
    Poison+=10000;
    PoisonAffect=10000;
    break;
    case 3:
    Poison+=10000;
    PoisonAffect=10000;
    break;
    case 4:
    Poison+=5000;
    PoisonAffect=5000;
    break;
    case 5:
    Poison+=5000;
    PoisonAffect=5000;
    break;
    case 6:
    Poison+=5;
    PoisonAffect=5;
    break;
    case 7:
    Poison+=5000;
    PoisonAffect=5000;
    break;
    case 8:
    Poison+=5000;
    PoisonAffect=5000;
    break;
    case 9:
    Poison+=5000;
    PoisonAffect=5000;
    break;
  }
}
void AdminDevice(byte PortNum){ //определение подключенного админ устройства
if (millis()-PrMillisAdminDevice[PortNum]>1000) {
int k;
int tmp3=analogRead(AdminDevPin[PortNum]);
if (tmp3>80) {
Signal(3);
Device[PortNum]=true;
for (k=0; k<13; k++) if (OutDevice[k] > tmp3) break;
  switch (k) {
    case 1:
    break;
    case 2:
    break;
    case 3:
    break;
    case 4:
    break;
    case 5:
    break;
    case 6:
    Health=MaxHealth;
    break;
    case 7:
    break;
    case 8:
    break;
    case 9:
    Death_Stalker();
    EEPROM_CLEAR();
    Restore_Stalker();
    break;
    case 10:
    Health=MaxHealth;
    break;
    case 11:
    Zaraza=false;
    Poison-=PoisonAffect;
    PoisonAffect=0;
    for (byte k=30;k<40;k++) if (EEPROM.read(k)==1) EEPROM.write(k,0);
    break;
  }
LCD(1);}
}
}
void ArtefaktDevice(byte PortNum){ //Определение подключенного артефакта
if (millis()-PrMillisArtefaktDevice[PortNum]>1000) {
int tmp3=analogRead(ArtefaktPin[PortNum]);
if (tmp3>80) {
ArtefaktID[PortNum]=0;
Signal(3); 
Artefakt[PortNum]=true;
for (ArtefaktID[PortNum]; ArtefaktID[PortNum]<13; ArtefaktID[PortNum]++) if (OutDevice[ArtefaktID[PortNum]] > tmp3) break;
  switch (ArtefaktID[PortNum]) {
    case 1:
    //1 артефакт
    break;
    case 2:
    //2 артефакт
    break;
    case 3:
    //3 артефакт
    break;
    case 4:
    //4 артефакт
    break;
    case 5:
    //5 артефакт
    break;
    case 6:
    //6 артефакт
    TreatArtefact+=17;
    RadiationKoef-=15;
    PoisonKoef-=20;
    break;
    case 7:
    //7 артефакт
    break;
    case 8:
    //8 артефакт

    break;
    case 9:
    //9 артефакт
    break;
    case 10:
    //10 артефакт

    break;
    case 11:
    //11 артефакт

    break;
     
  }
  ArtPgNum=PortNum;ArtInfo=true;PrArtInfoTime=millis();
LCD(1);}  
}
}
void ArtefaktEject(byte PortNum) { //Убираем воздействие артефакта при его изьятии
  Signal(4);
  switch (ArtefaktID[PortNum]) {
    case 1:
    //1 артефакт
    break;
    case 2:
    //2 артефакт
    break;
    case 3:
    //3 артефакт
    break;
    case 4:
    //4 артефакт
    break;
    case 5:
    //5 артефакт
    break;
    case 6:
    //6 артефакт
    TreatArtefact-=17;
    RadiationKoef+=15;
    PoisonKoef+=20;
    break;
    case 7:
    //7 артефакт
    break;
    case 8:
    //8 артефакт
    break;
    case 9:
    //9 артефакт
    break;
    case 10:
    //10 артефакт
    break;
    case 11:
    //11 артефакт
    break;   
  }
  ArtefaktID[PortNum]=0;
  Artefakt[PortNum]=false;
  LCD(1);
}
int ReadMessage() {
  int l;
  for (l=0;l<30;l++) {if (EEPROM.read(l)==1) break;}
  if (l==29 && EEPROM.read(29)==0) l=31; else {EEPROM.write(l,0);NoReadMessages();}
return l;
}
void NoReadMessages() {
Messages=0;
for (byte k=0;k<30;k++) if (EEPROM.read(k)==1) Messages++; 
}
void Push_Key() {
if (millis()-PrMillisPushKey>100) {
int tmp4=analogRead(KeysPin);
if (tmp4>100) {
DeathSignal=false;
  if (tmp4 < KeysACP[0]+15 && tmp4 > KeysACP[0]-15) {
    if (!LCD_Sleep && DisplayPage!=31 && ArtPgNum==1) {DisplayPage=31;ArtPgNum=2;}
    else if (ArtInfo && ArtPgNum==1) {ArtInfo=false;ArtPgNum=2;}
    else if (ArtInfo && ArtPgNum==0) {PrArtInfoTime=millis();ArtPgNum=1;}
    else {ArtInfo=true;PrArtInfoTime=millis();ArtPgNum=0;}
    LCD(1);
    Key=true;
  } 
  if (tmp4 < KeysACP[1]+15 && tmp4 > KeysACP[1]-15) {
    if (!LCD_Sleep) { 
      if (DisplayPage==31) LCD(0);
      if (Messages>0) {
        if (DisplayPage!=31) {
          if (Messages==0) DisplayPage=31;
          else DisplayPage=ReadMessage();
          LCD(1);
        }
        else {DisplayPage=ReadMessage();LCD(1);}
      } else {DisplayPage=31;}
    
    }
    else LCD(1);
    Key=true;} 
  if (tmp4 < KeysACP[2]+15 && tmp4 > KeysACP[2]-15) {if (Mute) {Mute=false;Signal(8);} else Mute=true;LCD(1);Key=true;} 
  if (tmp4 < KeysACP[3]+15 && tmp4 > KeysACP[3]-15) {if (DisplayBright>=230) DisplayBright=0; else DisplayBright+=50;LCD(1);Key=true;} 
  if (tmp4 < KeysACP[4]+15 && tmp4 > KeysACP[4]-15 && millis()-PrMillisPushKey>2000) {if (!Death) {DeathCause=2;Death_Stalker();};Key=true;} 
}
}
}
void loop() { //прога
Signal(0);

if (ArtInfo && millis() - PrArtInfoTime >= ArtInfoTime) ArtInfo=false;

if (millis() - PrMillisDisplayRefresh > 1000) {
  PrMillisDisplayRefresh=millis();LCD_Refresh();
}
  
 if (millis()-PrMillisChangeDisplayPg >= 3500) {
  PrMillisChangeDisplayPg=millis(); 
  if (DisplayChange) DisplayChange=false; else DisplayChange=true;
 }
 
  // Проверка батарейки
  if (millis()-PrMillisBat > 30000) {
    PrMillisBat=millis();
    Volt=GetVoltage(VbatPin, 6.5, 8.0, 2.0);
  }
  
  if (millis() - PrDisplayLedTime > DisplayLedTime && !LCD_Sleep) {
    LCD(0);
  }

if (!Death) {  
// Если есть радиация - воздействовать на игрока 
    if (RadiationLevel > 0) {Radiation_zone();SignalRadiation();}
}

    if (TreatLevel>0 && Health<MaxHealth) Treat_zone(); 
// Если есть лечилка - воздействовать на игрока 
      

// Отслеживание вредных воздействий
    if (Poison>0 && millis()-PrMillisPoison>=1000) {
      Health-=Poison/60;
      PrMillisPoison=millis();
      DeathCause=1;
    }

// Если изменилось значение жизни - отобразить на дисплее и проверить не умер ли или воскрес  
  if (byte(PrHealth/1000) > byte(Health/1000)) {Life();LCD_Refresh();PrHealth=Health;}
  if (byte(PrHealth/1000) < byte(Health/1000)) {Life();PrHealth=Health;LCD_Refresh();}
  
// Отслеживание кнопок
 if (analogRead(KeysPin) > 100) {
   if (!Key) Push_Key();
 }
 else {Key=false;PrMillisPushKey=millis();}
// Отслеживание админских модулей 1
  if (analogRead(AdminDevPin[0]) > 80) {
    if (!Device[0]) AdminDevice(0);
  }
  else {Device[0]=false;PrMillisAdminDevice[0]=millis();}
// Отслеживание админских модулей 2
  if (analogRead(AdminDevPin[1]) > 80) {
    if (!Device[1]) {AdminDevice(1);PrMillisAdminDevice[1]=millis();}
  }
  else Device[1]=false;

  
 // Отслеживание Артефактов 1 разьема
  if (analogRead(ArtefaktPin[0]) > 80 && !Death) {
    if (!Artefakt[0]) ArtefaktDevice(0);
  }
  else {
  Artefakt[0]=false; 
  PrMillisArtefaktDevice[0]=millis();
  if (ArtefaktID[0]!=0) ArtefaktEject(0);
  }
  // Отслеживание Артефактов 2 разьема
  if (analogRead(ArtefaktPin[1]) > 80 && !Death) {
    if (!Artefakt[1]) ArtefaktDevice(1);
  }
  else {
  PrMillisArtefaktDevice[1]=millis();
  Artefakt[1]=false;
  if (ArtefaktID[1]!=0) ArtefaktEject(1);
  }
GetWifi();
//Pin13=!Pin13;
//digitalWrite(13, Pin13);
}
