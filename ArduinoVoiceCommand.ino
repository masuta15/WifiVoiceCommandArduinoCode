#include <SPI.h>
#include <String.h>
#include <Ethernet.h>
#include <Servo.h>
#include <Wire.h>

const int LM35 = A5; // Define o pino que lera a saída do LM35
float temperatura; // Variável que armazenará a temperatura medida

int pinoSensor =A0;
 
int sensorValue_aux = 0;
float valorSensor = 0;
float valorCorrente = 0;
float voltsporUnidade = 0.004887586;// 5%1023
// Para ACS712 de  5 Amperes use 0.185
// Para ACS712 de 10 Amperes use 0.100
//  Para ACS712 de 5 Amperes use 0.066
float sensibilidade = 0.066;
 
//Tensao da rede AC 110 Volts e na verdade (127 volts)
int tensao = 127;

byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x83, 0xEA };
byte ip [] = {192,168,1,103};
EthernetServer server(80);
int led1 =5;
int aberto=0;
Servo microservo;
int pos=0;

String readString = String(30);
String statusLed;
String statusServo;
void setup()
{
  //Serial.begin(9600); // inicializa a comunicação serial
  Ethernet.begin(mac, ip);
  pinMode(led1, OUTPUT);
  microservo.attach(7);
}
void loop()
{

temperatura = (float(analogRead(LM35))*5/(1023))/0.01;
//Serial.print("Temperatura: ");
//Serial.println(temperatura);
//delay(2000);
  
  for(int i=100; i>0; i--){
    // le o sensor na pino analogico A0 e ajusta o valor lido ja que a saída do sensor é (1023)vcc/2 para corrente =0
    sensorValue_aux = (analogRead(pinoSensor) -510);
    // somam os quadrados das leituras.
    valorSensor += pow(sensorValue_aux,2);
    delay(1);
  }
  int valorLido=analogRead(A5);

  // finaliza o calculo da média quadratica e ajusta o valor lido para volts
  valorSensor = (sqrt(valorSensor/ 10000)) * voltsporUnidade;
  // calcula a corrente considerando a sensibilidade do sernsor (185 mV por amper)
  valorCorrente = (valorSensor/sensibilidade);
 
  //tratamento para possivel ruido
  //O ACS712 para 30 Amperes é projetado para fazer leitura
  // de valores alto acima de 0.25 Amperes até 30.
  // por isso é normal ocorrer ruidos de até 0.20A
  //por isso deve ser tratado

  EthernetClient client = server.available();
 
  if(client)
  {
    while(client.connected())
    {
     if (client.available())
      {
      char c = client.read();
      
      if(readString.length() < 30) {
    readString += (c);   
    }
    
    
    if(c == '\n')
    {
      
      if(readString.indexOf("led1") >0) 
      {
        digitalWrite(led1, !digitalRead(led1));
      }

        
      // cabeçalho http padrão
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<!doctype html>");
          client.println("<html>");
          client.println("<head>");
          client.println("<title>Acionando led com Arduino</title>");
          client.println("<meta name=\"viewport\" content=\"width=320\">");
          client.println("<meta name=\"viewport\" content=\"width=device-width\">");
          client.println("<meta charset=\"utf-8\">");
          client.println("<meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\">");
          client.println("</head>");
          client.println("<body>");
          client.println("<center>");
          
          client.println("<font size=\"5\" face=\"verdana\" color=\"Black\">Sistema</font>");
          client.println("<font size=\"5\" face=\"verdana\" color=\"black\"> de Automacao com </font>");
          client.println("<font size=\"5\" face=\"verdana\" color=\"blue\"> ARDUINO</font><br />");
                   
          client.print("<font size=\"5\" face=\"verdana\" color=\"red\">Corrente - ");
          client.print(valorCorrente*6.8*0.01);
          client.println(" Amperes </font><br />");
          client.print("<font size=\"5\" face=\"verdana\" color=\"green\">Temperatura - ");
          client.print(temperatura);
          client.println(" Celsius </font><br />");
          
          
         

          
          if(digitalRead(led1)) 
          {
            statusLed ="Desligado";
          }
         else
        {
          statusLed = "Ligado";
        }

           if(pos==0) 
          {
            statusServo ="aberta";

          }
         if(pos==90)
        {
          statusServo = "fechada";
        }
        
          client.println("<form action=\"led1\" method=\"get\">");
          client.println("<button type=submit style=\"width:200px;\">Led 1 -"+statusLed+" </button> ");
          client.println("</form> <br />");

          client.println("<form action=\"abrir\" method=\"get\">");
          client.println("<button type=submit style=\"width:200px;\"> Abrir porta</button> ");
          client.println("</form> <br />");
          
          client.println("<form action=\"fechar\" method=\"get\">");
          client.println("<button type=submit style=\"width:200px;\"> Fechar porta </button> ");
          client.println("</form> <br />");
          
          client.println("</center>");
          client.println("</body>");
          client.print(" <meta http-equiv=\"refresh\" content=\"2; url=http://192.168.1.103/\"> ");
          client.println("</html>");  

          if (readString.indexOf("abrir") >0){
          for(pos = 0; pos < 90; pos += 3) // goes from 0 degrees to 180 degrees 
          { // in steps of 1 degree 
          microservo.write(pos); // tell servo to go to position in variable 'pos' 
          delay(15); // waits 15ms for the servo to reach the position 
          } 
          }
          if (readString.indexOf("fechar") >0){
          for(pos = 90; pos>=1; pos-=3) // goes from 180 degrees to 0 degrees 
          { 
          microservo.write(pos); // tell servo to go to position in variable 'pos' 
          delay(15); // waits 15ms for the servo to reach the position 
          } 
        
} 
     
      readString = "";
      client.stop();     
      
    }
    }
  }
}
}
