#include <SoftwareSerial.h>

SoftwareSerial serialGSM(10, 11); // RX, TX

bool temSMS = false;
String telefoneSMS;
String dataHoraSMS;
String mensagemSMS;
String comandoGSM = "";
String ultimoGSM = "";
uint8_t REL1 = 2; //Conexao PIN 2
uint8_t REL2 = 3; //Conexao PIN 3
uint8_t REL3 = 5; //Conexao PIN 5
uint8_t REL4 = 4; //Conexao PIN 4

#define COM1 "UnlockCar"
#define COM2 "LockCar"
#define COM3 "StartCar"
#define COM4 "MyGps"

void leGSM();
void enviaSMS(String telefone, String mensagem);
void configuraGSM();
void (*funcReset)() = 0;

void setup() {

  Serial.begin(9600);
  Serial.println("Sketch Iniciado!");
  digitalWrite(REL1, LOW); //PIN 2
  digitalWrite(REL2, LOW); //PIN 3
  digitalWrite(REL3, LOW); //PIN 5
  digitalWrite(REL4, LOW); //PIN 4
  
  serialGSM.begin(9600);

  pinMode(REL1, OUTPUT);
  pinMode(REL2, OUTPUT);
  pinMode(REL3, OUTPUT);
  pinMode(REL4, OUTPUT);

  
  configuraGSM();

}

void loop() {

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
   //Renicia o arduino automaticamente
    if (millis() == 15000) {
      Serial.println ("O Arduino foi Reniciado com sucesso.");
      funcReset();
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