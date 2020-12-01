/*libraries*/
#include "simpleDSP_fir.h"
#include "tf_lite_model.h"
#include "Classifier.h"
#include <EloquentTinyML.h>
#include <PDM.h>
#include <arduinoFFT.h>
#include <ArduinoBLE.h>

/*global constants*/
#define SAMPLING_FREQUENCY 16000
#define SOUND_THRESHOLD 400
#define SAMPLES 128
#define PREDIC_THRESHOLD   0.6
#define NUMBER_OF_LABELS   2
#define RAW_OUTPUT         true  // output prediction probability of each label
#define NUMBER_OF_INPUTS   SAMPLES
#define NUMBER_OF_OUTPUTS  NUMBER_OF_LABELS
#define TENSOR_ARENA_SIZE  4 * 1024

/* This value was also used in the PDM example, seems like a good enough reason to
   continue using it. With this value and 16kHz sampling frequency, the RMS sampling
   period will be 16mS */
#define MICROPHONE_BUFFER_SIZE_IN_WORDS (256U)
#define MICROPHONE_BUFFER_SIZE_IN_BYTES (MICROPHONE_BUFFER_SIZE_IN_WORDS * sizeof(int16_t))

const String words[NUMBER_OF_LABELS] = {"noise", "sneeze"};

float coef[46] =
{
  0.00113053589111100,
  0.00101072486672204,
  0.000468847235288906,
  0.000603489038578611,
  -0.00208429064928862,
  -0.00340892103405782,
  -0.00362003403497889,
  -0.00179251511006564,
  0.00229274890344763,
  0.00758093034996730,
  0.0117041456801663,
  0.0117198866352666,
  0.00550311571071171,
  -0.00673606239661105,
  -0.0215570909070923,
  -0.0326442238558300,
  -0.0325783280789586,
  -0.0155583311041979,
  0.0199164513221244,
  0.0695391190116097,
  0.123656990515743,
  0.169767912370886,
  0.196291877716615,
  0.196291877716615,
  0.169767912370886,
  0.123656990515743,
  0.0695391190116097,
  0.0199164513221244,
  -0.0155583311041979,
  -0.0325783280789586,
  -0.0326442238558300,
  -0.0215570909070923,
  -0.00673606239661105,
  0.00550311571071171,
  0.0117198866352666,
  0.0117041456801663,
  0.00758093034996730,
  0.00229274890344763,
  -0.00179251511006564,
  -0.00362003403497889,
  -0.00340892103405782,
  -0.00208429064928862,
  -0.000603489038578611,
  0.000468847235288906,
  0.00101072486672204,
  0.00113053589111100
};


/*ML Models*/
Eloquent::TinyML::TfLite<NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> tf_model;
Eloquent::ML::Port::RandomForest clf;

/*BLE*/
// Device name
const char* nameOfPeripheral = "MicrophoneMonitor";
const char* uuidOfService = "00001101-0000-1000-8000-00805f9b34fb";
const char* uuidOfRxChar = "00001142-0000-1000-8000-00805f9b34fb";
const char* uuidOfTxChar = "00001143-0000-1000-8000-00805f9b34fb";

// BLE Service
BLEService microphoneService(uuidOfService);

// Setup the incoming data characteristic (RX).
const int WRITE_BUFFER_SIZE = 256;
bool WRITE_BUFFER_FIZED_LENGTH = false;

// RX / TX Characteristics
BLECharacteristic rxChar(uuidOfRxChar, BLEWriteWithoutResponse | BLEWrite, WRITE_BUFFER_SIZE, WRITE_BUFFER_FIZED_LENGTH);
BLEByteCharacteristic txChar(uuidOfTxChar, BLERead | BLENotify | BLEBroadcast);

/*global variables*/
volatile int samplesRead;
double vReal[MICROPHONE_BUFFER_SIZE_IN_WORDS];
double vImag[MICROPHONE_BUFFER_SIZE_IN_WORDS];
bool calibFlag;
int32_t offset;
float threshold_upper;
float threshold_avg;
float features[SAMPLES];
int counter = 0;
int16_t microphoneBuffer[MICROPHONE_BUFFER_SIZE_IN_WORDS];
int32_t microphoneRMSValue;
bool microphoneBufferReadyFlag;
bool recordFlag;
bool voice_detected;

arduinoFFT XXD = arduinoFFT();
FIR fir1;

/*global function prototypes*/
void Microphone_availablePDMDataCallback(void);
void Micophone_computeRMSValue(void);
void Microphone_calibration(void);

/*
   LEDS
*/
void connectedLight() {
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, HIGH);
}


void disconnectedLight() {
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, LOW);
}


/*
    BLUETOOTH
*/
void startBLE() {
  if (!BLE.begin())
  {
    Serial.println("starting BLE failed!");
    while (1);
  }
}

void onRxCharValueUpdate(BLEDevice central, BLECharacteristic characteristic) {
  // central wrote new value to characteristic, update LED
  Serial.print("Characteristic event, read: ");
  byte test[256];
  int dataLength = rxChar.readValue(test, 256);

  for (int i = 0; i < dataLength; i++) {
    Serial.print((char)test[i]);
  }
  Serial.println();
  Serial.print("Value length = ");
  Serial.println(rxChar.valueLength());
}

void onBLEConnected(BLEDevice central) {
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
  connectedLight();
}

void onBLEDisconnected(BLEDevice central) {
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  disconnectedLight();
}


void setup()
{
  /* Serial setup for UART debugging */
  Serial.begin(115200);

  /* Wait for serial to be available */
  while (!Serial);

  // Prepare LED pins.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);

  // Start BLE.
  startBLE();

  // Create BLE service and characteristics.
  BLE.setLocalName(nameOfPeripheral);
  BLE.setAdvertisedService(microphoneService);
  microphoneService.addCharacteristic(rxChar);
  microphoneService.addCharacteristic(txChar);
  BLE.addService(microphoneService);

  // Bluetooth LE connection handlers.
  BLE.setEventHandler(BLEConnected, onBLEConnected);
  BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);

  // Event driven reads.
  rxChar.setEventHandler(BLEWritten, onRxCharValueUpdate);

  // Let's tell devices about us.
  BLE.advertise();

  // Print out full UUID and MAC address.
  Serial.println("Peripheral advertising info: ");
  Serial.print("Name: ");
  Serial.println(nameOfPeripheral);
  Serial.print("MAC: ");
  Serial.println(BLE.address());
  Serial.print("Service UUID: ");
  Serial.println(microphoneService.uuid());
  Serial.print("rxCharacteristic UUID: ");
  Serial.println(uuidOfRxChar);
  Serial.print("txCharacteristics UUID: ");
  Serial.println(uuidOfTxChar);
  Serial.println("Bluetooth device active, waiting for connections...");

  /* PDM setup for MP34DT05 microphone */
  /* configure the data receive callback to transfer data to local buffer */
  PDM.onReceive(Microphone_availablePDMDataCallback);
  calibFlag = false;
  /* Initialise single PDM channel with a 16KHz sample rate (only 16kHz or 44.1kHz available */
  if (!PDM.begin(1, 16000))
  {
    Serial.println("Failed to start PDM!");
    /* Hacky way of stopping program executation in event of failure. */
    while (1);
  }
  else
  {
    /* Gain values can be from 0 to 80 (around 38db). Check out nrf_pdm.h
       from the nRF528x-mbedos core to confirm this. */
    /* This has to be done after PDM.begin() is called as begin() always
        sets the gain as the default PDM.h value (20).
    */
    PDM.setGain(50);
  }
  tf_model.begin((unsigned char*) model_data);
  /* Initialise micophone buffer ready flag */
  microphoneBufferReadyFlag = false;
  recordFlag = false;
  firInit(&fir1, 46, coef);
  Microphone_calibration();
}

void loop()
{

  BLEDevice central = BLE.central();
  if (central)
  {
    while (central.connected()) {
      connectedLight();


      if (microphoneBufferReadyFlag)
      {
        Micophone_computeRMSValue();
        microphoneBufferReadyFlag = false;
      }
      if ((microphoneRMSValue > SOUND_THRESHOLD)) {
        if (recordFlag == false && counter == 0) {
          recordFlag = true;
        }
      }

      if (recordFlag) {
        features[counter] = microphoneRMSValue;
        counter++;
        delay(5);
      }

      if (counter == SAMPLES) {
        recordFlag = false;
        float prediction[NUMBER_OF_LABELS];
        tf_model.predict(features, prediction);
        Serial.println("Predicting the word:");
        if (RAW_OUTPUT) {
          for (int i = 0; i < NUMBER_OF_LABELS; i++) {
            Serial.print("Label ");
            Serial.print(i);
            Serial.print(" = ");
            Serial.println(prediction[i]);
          }
        }
        voice_detected = false;
        for (int i = 0; i < NUMBER_OF_LABELS; i++) {
          if (prediction[i] >= PREDIC_THRESHOLD) {
            Serial.print("Word detected: ");
            Serial.println(words[i]);
            Serial.println("");

            if (words[i] == "sneeze")
            {
              txChar.writeValue((byte)0x01);
              txChar.writeValue((byte)0x00);
            }
            voice_detected = true;
          }
        }
        if (!voice_detected && !RAW_OUTPUT) Serial.println("Word not recognized\n");

        //    //Print_features();
        //Serial.print("Clf,You said: ");
        //Serial.println(clf.predictLabel(features));
        counter = 0;
      }
    }
  } else {
    disconnectedLight();
  }

}

void Microphone_availablePDMDataCallback()
{
  // query the number of bytes available
  int bytesAvailable = PDM.available();

  if (bytesAvailable == MICROPHONE_BUFFER_SIZE_IN_BYTES)
  {
    PDM.read(microphoneBuffer, bytesAvailable);
    microphoneBufferReadyFlag = true;
  }
}

void Micophone_computeRMSValue(void)
{
  for (int i = 0; i < MICROPHONE_BUFFER_SIZE_IN_WORDS; i++) {
    vReal[i] = microphoneBuffer[i];
    vImag[i] = 0;
  }
  XXD.Windowing(vReal, MICROPHONE_BUFFER_SIZE_IN_WORDS, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  XXD.Compute(vReal, vImag, MICROPHONE_BUFFER_SIZE_IN_WORDS, FFT_FORWARD);
  XXD.ComplexToMagnitude(vReal, vImag, MICROPHONE_BUFFER_SIZE_IN_WORDS);
  for (int i = 0; i < MICROPHONE_BUFFER_SIZE_IN_WORDS; i++) {
    vReal[i] = firFilt(&fir1, vReal[i]);
    vImag[i] = 0;
  }
  microphoneRMSValue = XXD.MajorPeak(vReal, MICROPHONE_BUFFER_SIZE_IN_WORDS, SAMPLING_FREQUENCY) - threshold_avg;
}

void Calibration_Micophone_computeRMSValue(void)
{
  for (int i = 0; i < MICROPHONE_BUFFER_SIZE_IN_WORDS; i++) {
    vReal[i] = microphoneBuffer[i];
    vImag[i] = 0;
  }
  XXD.Windowing(vReal, MICROPHONE_BUFFER_SIZE_IN_WORDS, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  XXD.Compute(vReal, vImag, MICROPHONE_BUFFER_SIZE_IN_WORDS, FFT_FORWARD);
  XXD.ComplexToMagnitude(vReal, vImag, MICROPHONE_BUFFER_SIZE_IN_WORDS);
  for (int i = 0; i < MICROPHONE_BUFFER_SIZE_IN_WORDS; i++) {
    vReal[i] = firFilt(&fir1, vReal[i]);
    vImag[i] = 0;
  }
  microphoneRMSValue = XXD.MajorPeak(vReal, MICROPHONE_BUFFER_SIZE_IN_WORDS, SAMPLING_FREQUENCY);
}

void Microphone_calibration() {
  int counter = 0;
  for (int i = 0; i < 1000; i++) {
    if (microphoneBufferReadyFlag)
    {
      counter++;
      Calibration_Micophone_computeRMSValue();
      threshold_avg += microphoneRMSValue;

      if ((microphoneRMSValue > threshold_upper)) {
        threshold_upper = microphoneRMSValue;
      }
      microphoneBufferReadyFlag = false;
    }
    delay(1);
  }
  threshold_avg /= counter;

  Serial.print("calibration fininshed");
  Serial.printf("upper: %f", threshold_upper);
  Serial.println();

}

void Print_features() {
  for (int i = 0; i < SAMPLES; i++) {
    Serial.printf("%f", features[i]);
    Serial.print(i == SAMPLES - 1 ? '\n' : ',');
  }
}
