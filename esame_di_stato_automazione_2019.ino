#include <Arduino.h>

const int startButtonPin = 4; //Pulsante di Start

//pin encoder MAT X e MAT Y

const int encoder_MAT_X_Pin = 2;
const int encoder_MAT_Y_Pin = 3;

// controllo Motori asincroni trifase X e Y

const int mat_X_Forward_ControlPin = 5; 
const int mat_X_Backward_ControlPin = 6; 
const int mat_Y_Forward_ControlPin = 7; 
const int mat_Y_Backward_ControlPin = 8; 

//pin controllo operazioni tramoggia

const int saracinesca_immissione_liquido = 10 ;
const int saracinesca_scarico_liquido = 11;
const int sensore_livello_tramoggia = 12;
const int sensore_panetti_pin = PC11;
const int sensore_presenza_cassa_pin = PC15;
const int mat_Tramoggia_ControlPin = 9;
const int mat_nastro_panetti_ControlPin = 13;
const int piatto_Riscaldante_ControlPin = PC14;

//contatori impulsi encoder

volatile int conta_impulsi_X = 0;
volatile int conta_impulsi_Y = 0;
volatile int panetti_immessi = 0;

// 10 minuti in millisecondi di mescola in tramoggia

long tempo_limite_mescola = 10*60*1000; 

// 10 minuti in millisecondi per trattamento termico 1

long tempo_limite_1 = 10*60*1000;

// 5 minuti in millisecondi per trattamento termico 2

long tempo_limite_2 = 5*60*1000; 

//Conteggio delle casse immagazzinate

int casse_immagazzinate = 0; 

//Variabile di stop 

bool stop = false;

/**************************************************************************/

// funzioni di incremento innescate dagli interrupt sui piedini encoder

void incrementa_impulsi_X()
{
     conta_impulsi_X++;
    }

void incrementa_impulsi_Y()
{
     conta_impulsi_Y++;
    }

void incrementa_panetti()
{
     panetti_immessi++;
    }



void setup() {

pinMode(startButtonPin, INPUT);

pinMode(mat_X_Forward_ControlPin, OUTPUT);
pinMode(mat_X_Backward_ControlPin, OUTPUT);
pinMode(mat_Y_Forward_ControlPin, OUTPUT);
pinMode(mat_Y_Backward_ControlPin, OUTPUT);

 //turn on pullup resistor for encoder lines

pinMode(encoder_MAT_X_Pin, INPUT_PULLUP);
pinMode(encoder_MAT_Y_Pin, INPUT_PULLUP);
pinMode(sensore_panetti_pin, INPUT);

//Ad ogni impulso encoder incremento il contatore degli impulsi per i due motori


attachInterrupt(digitalPinToInterrupt(encoder_MAT_X_Pin), incrementa_impulsi_X, RISING);
attachInterrupt(digitalPinToInterrupt(encoder_MAT_Y_Pin), incrementa_impulsi_Y, RISING); 

//ad ogni rilevazione di passaggio di un panetto incrementa il contatore dei panetti

attachInterrupt(digitalPinToInterrupt(sensore_panetti_pin), incrementa_panetti, RISING); 

}

void loop() {
  
while(!digitalRead(startButtonPin)); //pull-up resistor -> se non premuto HIGH   

//immetto il liquido nella tramoggia

digitalWrite(saracinesca_immissione_liquido, HIGH);

// aspetto che si riempia

while(sensore_livello_tramoggia != true);

//chiudo la saracinesca di immissione liquido

digitalWrite(saracinesca_immissione_liquido, LOW);

//Immetto i 5 panetti solidi 

digitalWrite(mat_nastro_panetti_ControlPin, HIGH);

// aspetto che si riempia

while(panetti_immessi <=5); //il conteggio si incrementa ad ogni interrupt sul pin del sensore

//spengo il nastro dei panetti

digitalWrite(mat_nastro_panetti_ControlPin, LOW);

// aziono il motore della tramoggia per il mescolamento per 10 minuti

digitalWrite(mat_Tramoggia_ControlPin, HIGH);

long inizio_mescola = millis();

//Aspetto 10 minuti senza fare nulla

while((millis() - inizio_mescola) < tempo_limite_mescola);

//Spengo il mescolatore della tramoggia

digitalWrite(mat_Tramoggia_ControlPin, LOW);


/**********4 SCARICHI CONSECUTIVI CON IMMAGAZZINAMENTO DI 4 CASSE *****************/


//Ciclo da ripetere 4 volte

while(casse_immagazzinate<4)
{

  //Controllo se è presente la cassa sotto la tramoggia: se assente aspetto

  while (digitalRead(sensore_presenza_cassa_pin) != HIGH);

  //apro la saracinesca di scarico del composto per 5 secondi (portata 2l/s)

  digitalWrite(saracinesca_scarico_liquido, HIGH);
  delay(5000);
  digitalWrite(saracinesca_scarico_liquido, LOW);

//Trattamento termico 1 del composto nella cassa: alimento al 60% del valore massimo di tensione
// ossia al 66% di 120 gradi pari a 90° per 10 minuti

  analogWrite(piatto_Riscaldante_ControlPin, 0.66*255);

  long inizio_trattamento_termico_1 = millis();

  //Aspetto 10 minuti senza fare nulla

  while((millis() - inizio_trattamento_termico_1) < tempo_limite_1);

  //Trattamento termico 2 del composto nella cassa: alimento al 33% del valore massimo di tensione
  // ossia al 50% di 120 gradi pari a 50° per 5 minuti

  analogWrite(piatto_Riscaldante_ControlPin, 0.5*255);

  long inizio_trattamento_termico_2 = millis();

  while((millis() - inizio_trattamento_termico_1) < tempo_limite_2);

  //Spengo la resistenza di riscaldamento

  analogWrite(piatto_Riscaldante_ControlPin, 0);

  //Immagazzinamento delle casse

  switch(casse_immagazzinate)
  {
    case 0: 
      //Aziono MAT_X per 4 secondi
      digitalWrite(mat_X_Forward_ControlPin, HIGH);
      delay(4000);
      digitalWrite(mat_X_Forward_ControlPin, LOW);
      casse_immagazzinate++;
    break;
 
    case 1:
      digitalWrite(mat_X_Forward_ControlPin, HIGH);
      delay(7000);
      digitalWrite(mat_X_Forward_ControlPin, LOW);
      casse_immagazzinate++;
    break;

    case 2:
      //Aziono MAT_X per 4 secondi
      digitalWrite(mat_X_Forward_ControlPin, HIGH);
      delay(4000);
      digitalWrite(mat_X_Forward_ControlPin, LOW);
       //Aziono MAT_Y per 3 secondi
      digitalWrite(mat_Y_Forward_ControlPin, HIGH);
      delay(3000);
      digitalWrite(mat_Y_Forward_ControlPin, LOW);
      casse_immagazzinate++;
      break;

    case 3:
      //Aziono MAT_X per 7 secondi
      digitalWrite(mat_X_Forward_ControlPin, HIGH);
      delay(7000);
      digitalWrite(mat_X_Forward_ControlPin, LOW);
       //Aziono MAT_Y per 3 secondi
      digitalWrite(mat_Y_Forward_ControlPin, HIGH);
      delay(3000);
      digitalWrite(mat_Y_Forward_ControlPin, LOW);
      casse_immagazzinate++;


    stop = true;
    break;
  
    default: 
    stop = true;
  } 

  //
  Nel caso di fine magazzino ritorno al setup e mi blocco sullo START
  if(stop == true) setup(); 
}
//da ripetere 4 volte


}
