/* 
Alfano Francesco Paolo Classe 4^C
Progetto mirato ad emulare il comportamento della lavatrice domestica 

*/
#include <LiquidCrystal.h>

//dc motor
#define ENABLE 5
#define DIRA 3
#define DIRB 4


const int en=49,rs=48,d4=52,d5=53,d6=50,d7=51; 
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);

const int pompa=8;//x switchare il relay e attivare pompa
const int sound=12; // x passive buzzer


bool go=false; // per avviamento iniziale
bool vai=false;  //per partenza 
const int greenLed=13; // per avviamento iniziale

// sezione bottoni

const int pulsante_pause_play=40; // pause or play  x avviamento iniziale
const int pulsante_parti=41;// avvia finalmente tutto
const int nbottoni = 4;
const int vet_Bottoni[nbottoni] = {30,31,32,33};  //numero di input, bottoni
 
bool salta[nbottoni] = {LOW,LOW,LOW,LOW}; // vedi spiegazione nella relativa funzione 
bool premuto[nbottoni] = {LOW,LOW,LOW,LOW};// bottoni premuti

long lastPressed[nbottoni] = {0,0,0,0}; 
long delayRimbalzi = 15; // x problema rimbalzo

// menu visualizzato
int display_corrente = 0;  
const int numSchermi = 4;
int parametri[numSchermi];
String schermi[numSchermi][2] = {{"Lavaggio","tipo di capi"}, {"Temperatura", "gradi"}, {"Giri","al minuto"},{"Timer","ore"}};

// sezione tempi

int tempotemposo=1500; // tempo di pressione necessario per accendere la lavatrice e per avviare il programma , cioè devi premere per tempotemposo tempo il bottone  (funzione emulata dalla lavatrice domestica)
long x;//avvio
long t;// t che serve per debounce alla conferma modalità iniziale 
long tempino; // t che serve per debonce alla conferma modalità finale
int tacqua=5000; // tempo acqua che pompa la pompa

double ttsupport; //pompa;
double tsupport;// lavatrice
 


void setup() {
  pinMode(greenLed,OUTPUT);
  pinMode(pompa,OUTPUT);
  pinMode(sound, OUTPUT);//buzzer

// setto i parametri iniziali
   parametri[0]=1;
   parametri[1]=0;
   parametri[2]=0;
   parametri[3]=3;

 digitalWrite(pompa,HIGH);// SWITCH relay aperto
   
  pinMode(ENABLE,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(DIRB,OUTPUT);
 
     lcd.print("Accensione? ");
  for(int i = 0; i < nbottoni; i++) 
  {
    pinMode(vet_Bottoni[i], INPUT);  
  }

  Serial.begin(9600);
  lcd.begin(16, 2);
}


void loop() {
 if(go==false)  accensione();   // sta qui finchè non viene premuto il bottone play/pause per più di 3 secondi e poi rilascia 
 if(go==true)
 {  
    inputbottoni();// sente se un bottone è premuto
    bottonipremuti(); // se un bottone è stato premuto fa qualcosa a seconda del bottone (usando un'altra funzione)
    Pronti_Partenza_Via(); // se si vuole avviare la lavatrice è necessario premere il corrispettivo bottone(quello estremo a destra) , se premuto vai==true

    if(vai==true)
    {
      avvia();// in base a certe scelte i valori dei parametri verranno contenuti in un certo range es con valori random( scegli capo lana     max temp 90  min 30    se metti 20 gradi sarà cambiato a 30 , se metti 100 diverrà 90 se metti un valore all'interno del range rimarrà tale ) 
      boom(parametri[2],parametri[3]); // parte la lavatrice , viene passata la velocità e il tempo 
      imdone();// suono finale e resetto tutto come all'inizio
    }
  
 }

}



// FUNZIONI  //


/*ACCENSIONE */
void accensione()
{ 
    lcd.clear();
    lcd.print("  Accensione? ");
    Serial.println("Accensione? ");
    Serial.println(digitalRead(pulsante_pause_play));
    
    if(digitalRead(pulsante_pause_play)==HIGH)
    {
        delay(delayRimbalzi); // x rimbalzi
        Serial.println(digitalRead(pulsante_pause_play));
        x=millis();
   
          while(digitalRead(pulsante_pause_play)==HIGH  && millis()-x<=tempotemposo)
          
          {
        
            if(digitalRead(pulsante_pause_play)==LOW) { return ; }
           }
             
            
                  
          while(digitalRead(pulsante_pause_play)==HIGH)
            {
               Serial.println("Accensione?!?! ");
               Serial.println("Accensione?! ");  
            }

         
             // e rilascia
             lcd.clear();
             lcd.setCursor(0,0);
             lcd.print("   Premi  per ");      
             lcd.setCursor(0,1);   
             lcd.print("    iniziare"); 
             Serial.println("Premi per iniziare ");    
             digitalWrite(greenLed,HIGH);    
             go=true;   
    } 
   
}




void inputbottoni() 
{
  for(int i = 0; i <nbottoni; i++)// controllo 
 {   
    int input_bottone = digitalRead(vet_Bottoni[i]); // tutti gli input

    if(salta[i]==LOW)  // ** if ((millis() - lastPressed[i]) > delayRimbalzi) se non è passato il tempo dei rimbalzi non voglio resettare il tempo in cui ho premuto il bottone 
     {
      if (input_bottone==HIGH)  lastPressed[i] = millis(); // prendo il tempo di quando il pulsante [i]  è stato premuto      
     }
       
    if ((millis() - lastPressed[i]) > delayRimbalzi) // se è passato abbastanza tempo (15ms per problema di rimbalzo) controllo ancora l'input // tutto questo x sostituire delay che messo nel for rallenterebbe tantissimo tutto il processo 
    { 
      salta[i]=HIGH;
      if (input_bottone==HIGH){ premuto[i] = HIGH; salta[i]=LOW;}   
    }
 }
   
}



void bottonipremuti() {  // vede quale bottone è stato premuto e passa tale valore ad una funzione (che in base al bottone fa cose differenti)
  for(int i = 0; i < nbottoni; i++) 
  {  
    if(premuto[i] == HIGH)
    {
      cambiamenti_menu(i);//cambia parametri o pagina a schermo
      premuto[i] = LOW;// k è stato premuto ed ho fatto i miei cambiamenti 
      lcdRefresh();//stampa effettivamente i cambiamenti 
    }
  }
}




void cambiamenti_menu(int input) {
  if(input == 1) 
  {
    if (display_corrente == 0)  display_corrente = numSchermi-1; // se sono al primo e vado indietro vado allo'ultimo (es : menu di 5 parti     l'ultimo sarebbe il numero vet[4] perchè il vet parte da 0.... 0 1 2 3 4(l'ultimo)) 
    else display_corrente--;// vado indietro di schermo
    
  }else if(input == 0) 
  {
   if (display_corrente == numSchermi-1) display_corrente = 0;
   else display_corrente++;
    
  }else if(input == 2) Cambio_Valori(0);// se premi il 4 da sinistra aumenta il parametro dello schermo corrente
  else if(input == 3)  Cambio_Valori(1);// se premi il penultimo bottone da destra diminuisce il parametro dello schermo corrente
  
  
}



void Cambio_Valori(int key) // cambiano i parametri
{
  
    if(key == 0)   //scorre avanti di  pagina
    {
      switch(display_corrente) 
      {
        case(0): // tipo di capi 1 .... 6
         if(parametri[display_corrente]==6) parametri[display_corrente]=1;
         else parametri[display_corrente]++;
        break;
            
        case(1): // temperature 0 10 20 30 40 50 60 70 80 90
          if(parametri[display_corrente]==90) parametri[display_corrente]=20;
          else parametri[display_corrente]+=10;
        break;
        
        case(2):  // speed 0 100 200 300 400  500  600 700 800 ... 1200
          if(parametri[display_corrente]==1200) parametri[display_corrente]=0;
          else parametri[display_corrente]+=100;        
        break;
        
        
        case(3):  // hrs    3 6 9 12
          if(parametri[display_corrente]==12) parametri[display_corrente]=3;
          else parametri[display_corrente]+=3;       
        break;
                 
     }
       
      
   }
    
  
  
  else if(key == 1) //scorre dietro di  pagina
  {
     switch(  display_corrente ) 
     {
        case(0): // tipo di capi 1 .... 6
         if(parametri[display_corrente]==1) parametri[display_corrente]=6;
         else parametri[display_corrente]--;
        break;
            
        case(1): // temperatura 0 20 30 40 50 60 70 80 90
          if(parametri[display_corrente]==0) parametri[display_corrente]=90;
          else parametri[display_corrente]-=10;
        break;
        
        case(2):  // speed 0 ...400  500  600 700 800 ... 1200
          if(parametri[display_corrente]==0) parametri[display_corrente]=1200;
          else parametri[display_corrente]-=100;
        break;
          
        case(3):  // hrs    3 6 9 12
          if(parametri[display_corrente]==3) parametri[display_corrente]=12;
          else parametri[display_corrente]-=3;
        
        break;
  
     }
      
  }
  
  
}


void lcdRefresh() // per refreshare i parametri
{ 
  lcd.clear();
  lcd.print(schermi[display_corrente][0]);
  Serial.println(schermi[display_corrente][0]);
  lcd.setCursor(0,1);
  lcd.print(parametri[display_corrente]);
  Serial.print(parametri[display_corrente]);
  lcd.print(" ");
  Serial.print("");
  lcd.print(schermi[display_corrente][1]);
  Serial.println(schermi[display_corrente][1]);
}


/*
 avvia
// 1 cotone standard 1000 max giri  60 gradi max
// 2 cotone con prelavaggio  1200 max giri 90 gradi max 
// 3 sintetico energico     800 max giri  max 40 gradi max
// 4 sport intensivo   400 max giri    50 gradi max
// 5 special shoes    200 giri max  (velocità 50) 20 gradi max
// 6 lana, seta e tende 0 giri 0 gradi fredda no centrifuga

*/

void avvia() // in base alle scelte il programma pone una guida  in base al tipo di capo scelto(a seconda del capo scelto vi è un range di valori in cui bisogna stare)
{
  int maxtemperature;
  int mintemperature;
  int maxgiri;
  int mingiri;
  int maxtempo;
  int mintempo;
  
  // capi
  switch(parametri[0])
  {
    case(1): // 1 cotone standard 
      maxtemperature=60;
      mintemperature=50;
      maxgiri=1000;
      mingiri=800;
      maxtempo=12;
      mintempo=12;
    break;
    case(2):// 2 cotone con prelavaggio  1200 max giri 90 gradi max  
      maxtemperature=90;
      mintemperature=90;
      maxgiri=1200;
      mingiri=1000;
      maxtempo=12;
      mintempo=9;
    break;
    case(3):// 4 sport intensivo     
      maxtemperature=90;
      mintemperature=40;
      maxgiri=800;
      mingiri=700;
      maxtempo=3;
      mintempo=3;
    break;
    
    case(4)://4  shoes     
      maxtemperature=70;
      mintemperature=50;
      maxgiri=600;
      mingiri=600;
      maxtempo=12;
      mintempo=3;
    break;
    case(5):// 5 special shoes    
      maxtemperature=30;//freddo
      mintemperature=20;
      maxgiri=700;
      mingiri=600;
      maxtempo=9;
      mintempo=3;
    break;
    case(6):// 6 lana, seta e tende 0 giri 0 gradi fredda no centrifuga    
      maxtemperature=20;
      mintemperature=0;
      maxgiri=0;
      mingiri=0;
      maxtempo=6;
      mintempo=6;
    break;
    
  }
 //   {{"Lavaggio","tipo di capi"}, {"Temperatura", "gradi"}, 
//{"Giri","al minuto"},{"Timer","ore"}};

  // se selezioni una temperatura , un numero di giri o un tempo fuori dal range consigliato il relativo parametro sarà rimesso nel range
  if(parametri[1]>maxtemperature)  parametri[1]=maxtemperature; //vai a maxtemperature
  else if(parametri[1]<mintemperature)  parametri[1]=mintemperature;
  
  if(parametri[2]>maxgiri)  parametri[2]=maxgiri;
  else if(parametri[2]<mingiri)parametri[2]=mingiri;
  
  if(parametri[3]>maxtempo) parametri[3]=maxtempo; //vai a maxtempo
  else if(parametri[3]<mintempo)parametri[3]=mintempo;
     
}

void Pronti_Partenza_Via()
{
  if(digitalRead(pulsante_parti)==HIGH)
    {
       delay(delayRimbalzi); // x rimbalzi
        Serial.println(digitalRead(pulsante_parti));
       tempino=millis();
    
       
          while(digitalRead(pulsante_parti)==HIGH  && millis()-tempino<=tempotemposo)
          {
                  Serial.println("Qui");
              if(digitalRead(pulsante_parti)==LOW) {  
            Serial.println("Qua");  return ; }
           }
                              
          while(digitalRead(pulsante_parti)==HIGH)
            {
               Serial.println("Partooo?!?! ");
                 
            }
        vai=true;
    } 
     
}


void boom(float speeed,int t) // parte la lavatrice
{

  Serial.print("Speed :");
  Serial.println(speeed);
  Serial.print("Tempo :");
  Serial.println(t);
  Serial.print("Temperatura :");
  Serial.println(parametri[1]);

 //delay(4000) ; // SOLO  X DIMOSTRAZIONE VIDEO


  
  // 255 max speed == 1200 giri   255/1200 = 0.21 circa
  speeed=speeed*0.21;
  t=t*1000; // si dovrebbe fare x 3600000 perchè sarebbero ore 
  ttsupport=millis();
  

 // PARTENZA POMPA 
  digitalWrite(pompa,LOW);// switch rele attivo, la pompa parte
   lcd.clear();
   lcd.print("     Acqua  ");
      while(millis()-ttsupport<=tacqua)
      {
     Serial.println(" la pompa vaaaa ");
      }
     digitalWrite(pompa,HIGH); // fermo la pompa tramite relay


// PARTENZA CENTRIFUGA
 
   lcd.clear();
   lcd.print("   Centrifuga");
   tsupport=millis();   
  
      while(millis()-tsupport<t/3)
    {
    analogWrite(ENABLE,speeed); //enable on 
    digitalWrite(DIRA,HIGH); //cambio verso giro
    digitalWrite(DIRB,LOW);  

   //Serial.println("centrifuga");
    }
   tsupport=millis();
      while(millis()-tsupport<t/3)
    {
    analogWrite(ENABLE,speeed); //enable on 
    digitalWrite(DIRA,LOW); //cambio verso giro
    digitalWrite(DIRB,HIGH);  

    //Serial.println("centrifuga");

    }
    tsupport=millis();
   
    while(millis()-tsupport<t/3)
    {
    analogWrite(ENABLE,speeed); //enable on 
    digitalWrite(DIRA,HIGH); //cambio verso giro
    digitalWrite(DIRB,LOW);  

   // Serial.println("centrifuga");

    }
/*
  while(millis()-tsupport<t) // solo con lattina perchè non ha sempre la  potenza per cambiare rapidamente direzione, note: ATTENTO CHE LA LATTINA NON TOCCHI QUALCHE BORDO o farà veramente fatica a partire, fino a necessitare una spinta iniziale molto lieve
    {
    analogWrite(ENABLE,speeed); //enable on 
    digitalWrite(DIRA,HIGH); //cambio verso giro
    digitalWrite(DIRB,LOW);  

   //Serial.println("centrifuga");
    }

  
   digitalWrite(ENABLE,LOW);
*/
}


void imdone()  // suono finale
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("     Fine");
  tone(sound,300,500);
  delay(1000);
  tone(sound,300,500);
  delay(1000);
  tone(sound,450,500);
  delay(2200);
  tone(sound,550,1000);
  delay(5000);  
  lcd.clear();

  //resetto allo stato d'avvio iniziale
  go=false;
  vai=false;
  digitalWrite(greenLed,LOW);    
}
