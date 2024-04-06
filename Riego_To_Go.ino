#define TdsSensorPin A1
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
#include <Servo.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  //Crear el objeto LCD con los números correspondientes (rs, en, d4, d5, d6, d7)
Servo servoMotor;// Declaramos la variable para controlar el servo
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;

volatile int NumPulsos; //variable para la cantidad de pulsos recibidos
int PinSensor = 1;    //Sensor conectado en el pin 2 movido al pin 1
float factor_conversion=7.5; //para convertir de frecuencia a caudal

//---Función que se ejecuta en interrupción---------------
void ContarPulsos ()  
{ 
  NumPulsos++;  //incrementamos la variable de pulsos
} 

//---Función para obtener frecuencia de los pulsos--------
int ObtenerFrecuencia() 
{
  
  int frecuencia;
  NumPulsos = 0;   //Ponemos a 0 el número de pulsos
  
  
  

      interrupts();    //Habilitamos las interrupciones
      delay(1000);   //muestra de 1 segundo
      noInterrupts(); //Deshabilitamos  las interrupciones
      
      
  
  frecuencia=NumPulsos; //Hz(pulsos por segundo)
  return frecuencia;
}

void setup() 
{ 

  Serial.begin(115200);

  //Serial.begin(9600); ///////////////////////////////////////////////////////////////////////////////////////////

  lcd.begin(16, 2);// Inicializar el LCD con el número de  columnas y filas del LCD
  
  pinMode(TdsSensorPin,INPUT);
  pinMode(PinSensor, INPUT); 
  attachInterrupt(0,ContarPulsos,RISING);//(Interrupción 0(Pin2),función,Flanco de subida)
  servoMotor.attach(10);//////////////////////////////////////////////////////////////////////////////////////////////cambio del 9 al 10
  // Desplazamos a la posición 0º
  servoMotor.write(0);
  // Esperamos 1 segundo

  lcd.print("hola");
  delay(2000);
} 


/////////////INICIO DEL CODIGO PRINCIPAL/////////////////////////////////

void loop ()    
{
  

  //-------------------------- AQUI SE MIDE LA PUREZA DEL AGUA TDS ---------------------
  static unsigned long analogSampleTimepoint = millis();
  static unsigned long printTimepoint = millis();
  float volumen = 0;
  long dt = 0; //variación de tiempo por cada bucle
  long t0 = 0; //millis() del bucle anterior

  if (millis() - analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }


  if (millis() - printTimepoint > 800U)
  {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++){
      
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge = averageVoltage / compensationCoefficient;  //temperature compensation
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //convert voltage value to tds value
    Serial.print("Calidad (TDS):");
    Serial.print(tdsValue, 0);
    Serial.println("ppm");
    
  // Escribimos el Mensaje en el LCD.





///////////////////////////////////////////////////////////////////////////////////////


// -------------------- FIN DE CODIGO CALIDAD DEL AGUA



//-------------------------- AQUI SE MIDE CUANTA AGUA PASA  ---------------------
  float frecuencia = ObtenerFrecuencia(); //obtenemos la frecuencia de los pulsos en Hz
  float caudal_L_m = frecuencia / factor_conversion; //calculamos el caudal en L/m
  dt = millis() - t0; //calculamos la variación de tiempo
  t0 = millis();
  volumen = volumen + (caudal_L_m / 60) * (dt / 1000); // volumen(L)=caudal(L/s)*tiempo(s)

  //-----Enviamos por el puerto serie---------------
  Serial.print("Caudal de bomba de agua: ");
  Serial.print(caudal_L_m, 3);
  Serial.print("L/min\tVolumen: ");
  Serial.print(volumen, 3);
  Serial.print(" L\t");
   delay(3000);
  /////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
  // Escribimos el Mensaje en el LCD.

   lcd.clear();// borrar pantalla lcd

    lcd.setCursor(0,0);
    lcd.print("Calidad: ");
    lcd.print(tdsValue);
   

    delay(1000);

        if  (tdsValue < 600)
    {
      servoMotor.write(90);  // abre llave
     lcd.setCursor(0,1);
    lcd.print("agua sucia");
    
   }

   else {
    servoMotor.write(180); /// cierra llave
     lcd.setCursor(0,1);
    lcd.print("agua limpia");
   }

    delay(1000);



//-------------------- FIN CODIGO CUANTA AGUA PASA



////-------------------------aqui se mueve el servomotor

           // Desplazamos a la posición 180º
//  servoMotor.write(180);
//  delay(1000);
//   servoMotor.write(0);
//   delay(1000);

// ------------ condiciones de la llave del agua
 

 }
     }
/////////////////////////////////////////////////////////////////////////////////////


//   if(tdsValue < 550)
 //   {
//      // Desplazamos a la posición 180º
  //servoMotor.write(180);
  // Esperamos 1 segundo
  //delay(1000);
    //}
    //else{
//      servoMotor.write(90);
//  // Esperamos 1 segundo
//  delay(1000);
  //  }
  //}

  //if (Serial.available()) {
   // if (Serial.read() == 'r') volumen = 0; //restablecemos el volumen si recibimos 'r'
  //}





}
///////FIN DEL CODIGO PRINCIPAL




////////////// FUNCIONES EXTRA/////////////////


int getMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}