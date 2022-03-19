#include <SoftwareSerial.h>
#include <TinyGPS.h>

SoftwareSerial serialGSM(10, 11); // RX, TX
SoftwareSerial serialGPS(6, 7);

TinyGPS gps;

bool temSMS = false;
String telefoneSMS;
String dataHoraSMS;
String mensagemSMS;
String comandoGSM = "";
String ultimoGSM = "";
uint8_t REL1 = 2; //Conexao RELE 1
uint8_t REL2 = 3; //Conexao RELE 2
uint8_t REL3 = 5; //Conexao RELE 4
uint8_t REL4 = 4; //Conexao RELE 3
#define FREIO 9
#define CARON 8
#define COM1 "UnlockCar"
#define COM2 "LockCar"
#define COM3 "StartCar"
#define COM4 "MyGps"


void leGSM();
void leGPS();
void enviaSMS(String telefone, String mensagem);
void configuraGSM();

unsigned long delay2 = 0;

void setup() {

  digitalWrite(REL1, LOW);
  digitalWrite(REL2, LOW);
  digitalWrite(REL3, LOW);
  digitalWrite(REL4, LOW);
  Serial.begin(9600);
  serialGPS.begin(9600);
  serialGSM.begin(9600);

  pinMode(FREIO, INPUT_PULLUP); //Quando acionar o freio desliga o carro e obriga a ligar pela chave.
  pinMode(CARON, INPUT_PULLUP); //Se o carro estiver ligado, mesmo que receba o SMS, nao executa o comando.
  pinMode(REL1, OUTPUT);
  pinMode(REL2, OUTPUT);
  pinMode(REL3, OUTPUT);
  pinMode(REL4, OUTPUT);

  Serial.println("Sketch Iniciado!");
  configuraGSM();

  leGPS();
}

void loop() {
  static unsigned long delayLeGPS = millis();

  if ( (millis() - delayLeGPS) > 10000 ) {
    leGPS();
    delayLeGPS = millis();
  }

  leGSM();

  if (comandoGSM != "") {
    Serial.println(comandoGSM);
    ultimoGSM = comandoGSM;
    comandoGSM = "";
  }

  if (temSMS) {

    Serial.println("Chegou Mensagem!!");
    Serial.println();

    Serial.print("Remetente: ");
    Serial.println(telefoneSMS);
    Serial.println();

    Serial.print("Data/Hora: ");
    Serial.println(dataHoraSMS);
    Serial.println();

    Serial.println("Mensagem:");
    Serial.println(mensagemSMS);
    Serial.println();

    mensagemSMS.trim();
    if ( mensagemSMS == COM4 ) {
      Serial.println("Enviando SMS de Resposta.");
      leGPS();

      float flat, flon;
      unsigned long age;

      gps.f_get_position(&flat, &flon, &age);

      if ( (flat == TinyGPS::GPS_INVALID_F_ANGLE) || (flon == TinyGPS::GPS_INVALID_F_ANGLE) ) {
        enviaSMS(telefoneSMS, "GPS WITHOUT SIGNAL  !!!");
      } else {
        String urlMapa = "Local Identificado: https://maps.google.com/maps/?&z=10&q=";
        urlMapa += String(flat, 6);
        urlMapa += ",";
        urlMapa += String(flon, 6);

        enviaSMS(telefoneSMS, urlMapa);
      }
    }
    //Desbloqueia as portas do carro
    if ( mensagemSMS == COM1) {
      Serial.println("Enviando SMS de Resposta.");
      digitalWrite(REL1, HIGH);
      delay(1000);
      digitalWrite(REL1, LOW);
      enviaSMS(telefoneSMS, "Car Unlocked");
    }
    //Bloqueia as portas do carro
    if ( mensagemSMS == COM2 ) {
      Serial.println("Enviando SMS de Resposta.");
      digitalWrite(REL2, HIGH);
      delay(1000);
      digitalWrite(REL2, LOW);
      enviaSMS(telefoneSMS, "Car Locked!!!");
    }
    //Liga o carro
    if ( mensagemSMS == COM3 ) {
      Serial.println("Enviando SMS de Resposta.");
      digitalWrite(REL4, HIGH);
      delay(2000);
      digitalWrite(REL4, LOW);
      enviaSMS(telefoneSMS, "Ariana seu carro foi ligado!!!");
    }

    temSMS = false;
  }
}

void leGSM()
{
  static String textoRec = "";
  static unsigned long delay1 = 0;
  static int count = 0;
  static unsigned char buffer[64];

  serialGSM.listen();
  if (serialGSM.available()) {

    while (serialGSM.available()) {

      buffer[count++] = serialGSM.read();
      if (count == 64)break;
    }

    textoRec += (char*)buffer;
    delay1   = millis();

    for (int i = 0; i < count; i++) {
      buffer[i] = NULL;
    }
    count = 0;
  }


  if ( ((millis() - delay1) > 100) && textoRec != "" ) {

    if ( textoRec.substring(2, 7) == "+CMT:" ) {
      temSMS = true;
    }

    if (temSMS) {

      telefoneSMS = "";
      dataHoraSMS = "";
      mensagemSMS = "";

      byte linha = 0;
      byte aspas = 0;
      for (int nL = 1; nL < textoRec.length(); nL++) {

        if (textoRec.charAt(nL) == '"') {
          aspas++;
          continue;
        }

        if ( (linha == 1) && (aspas == 1) ) {
          telefoneSMS += textoRec.charAt(nL);
        }

        if ( (linha == 1) && (aspas == 5) ) {
          dataHoraSMS += textoRec.charAt(nL);
        }

        if ( linha == 2 ) {
          mensagemSMS += textoRec.charAt(nL);
        }

        if (textoRec.substring(nL - 1, nL + 1) == "\r\n") {
          linha++;
        }
      }
    } else {
      comandoGSM = textoRec;
    }

    textoRec = "";
  }
}


void enviaSMS(String telefone, String mensagem) {
  serialGSM.print("AT+CMGS=\"" + telefone + "\"\n");
  serialGSM.print(mensagem + "\n");
  serialGSM.print((char)26);
}


void configuraGSM() {
  serialGSM.print("AT+CMGF=1\n;AT+CNMI=2,2,0,0,0\n;ATX4\n;AT+COLP=1\n");
}