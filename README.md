# Blessyou

## 1.Introduction
During the COVID-19 pandemic, most of the school switches to online lectures. Unlike in-person lectures, online lectures lack normal interaction between students, such as the “bless you” from your classmates. To increase the engagement of the students during class, we come up with the idea to create an interactive sneeze detector that will say “bless you” after you sneeze. And it has the potential to be developed as a respiratory sickness prevention device.

---
## 2.Abstract
The objective of the project is to enable real-time sneeze detection using the microphone on Arduino Nano 33 BLE Sense. Fast Fourier Transform(FFT) was performed on pre-recorded sneeze samples to convert the data from the time domain to frequency domain. The samples were then filtered by the Finite Impulse Response(FIR) filter and ready for training classifiers. Two different machine learning classifier training approaches were implemented. The first one was using sklearn, and the highest training accuracy achieved was through RandomForest classifier with 97.4% and testing accuracy around 90%. The other one was using TensorFlow to implement a 3-layer neural network, and the training accuracy was around 89.7% with testing accuracy around 95%. After the sneeze is detected, a signal is sent from the Arduino board to the computer using Bluetooth and a message “Bless You!” will appear with a background voiceover.The project successfully detects different kinds of sneezes with no confusion on similar sound sources such as coughing and snapping fingers.

## 3.Related work
### Cover Your Cough: Detection of Respiratory Events with Confidence Using a Smartwatch
In this paper, detection of cough and sneezes was achieved through a hybrid method combining both sound recording and hand movement detection. A smartwatch is used and will record the environment sound after a quick hand movement is detected using the accelerometer. The sound recordings are then uploaded to the computer to train classification models using confidence machine learning. The relevant features were extracted with MFCC, CFA and ZCR. The trained model was evaluated and validated on a large ESC Sound Environment dataset as well as real-time dataset samples to prove its efficiency and accuracy. 

### FluSense: A Contactless Syndromic Surveillance Platform for Influenza-Like Illness in HospitalWaiting Areas
In this paper, the researchers developed a system (FluSense) to detect patients with flu like symptoms in a hospital waiting room. The system utilizes a thermal camera to capture thermal images for temperature measurement and a microphone array to capture audio data for symptoms detection. Then the data collected is fed to a neural computing engine to analyze at real-time. FluSence is deployed to the hospital of a large university for seven months. The result is compared to the gold standard laboratory-confirmed influenza case data collected from the same facility and shows strong correlation with the laboratory-confirmed influenza trend.

---
## 4.Technical approach
#### 4.1 System Overview
The sneeze detection system can be seperated in to three parts: hardware data capturing, data pre-processing, classifying. An overview for the entire system in shown in Fig 1 below.   
![FlowChart (1)](https://user-images.githubusercontent.com/72180511/102518683-703f0a00-4046-11eb-9254-7e0cfab224de.jpg)

#### 4.2 Hardware
The hardware for this project is the Arduino NANO 33 BLE Sense board, which is a cost-effective and power sufficient solution for the project. The board is based on a NINA B306 module that hosts a Nordic cRF52480 and contains a Cortex M4F microcontroller. The board comes with a digital microphone MP34DT05, which is the essential piece of sensor in the project and produces and outputs signal in PDM. The board also integrates with Bluetooth Low Energy (BLE) feature that allows direct communication from the board to the computer.

## 5.Goals
- Detect sneezes from different sound sources
- Deal with confusing cases such as talking loudly or coughing

