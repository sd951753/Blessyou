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
  The sneeze detection system can be seperated in to four parts: hardware data capturing, data pre-processing, classifying, wireless communication. An overview for the entire system in shown in Fig 1 below.
![FlowChart (1)](https://user-images.githubusercontent.com/72180511/102518683-703f0a00-4046-11eb-9254-7e0cfab224de.jpg)

#### 4.2 Hardware
  The hardware for this project is the Arduino NANO 33 BLE Sense board, which is a cost-effective and power sufficient solution for the project. The board is based on a NINA B306 module that hosts a Nordic cRF52480 and contains a Cortex M4F microcontroller. The board comes with a digital microphone MP34DT05, which is the essential piece of sensor in the project and produces and outputs signal in PDM. The board also integrates with Bluetooth Low Energy (BLE) feature that allows direct communication from the board to the computer.
![20201217_095812](https://user-images.githubusercontent.com/72180511/102525471-23abfc80-404f-11eb-9a5b-635af8ac849f.jpg)

#### 4.3 Data collection
  Sneeze samples were selected through the webpage “FreeSounds”, which contains around 200 samples from male, female and children. Each sample was recorded by the digital microphone on the Arduino 33 BLE Sense board. The microphone sampling frequency was set to 16kHz, and the data sampling frequency was set to 200Hz. The threshold for the microphone to start recording is 400Hz. 

  During the data collection process, the sneeze samples were played with regard to slightly different distances from the board and slightly different volumes to add more variance to the dataset, taking care of the corner cases. The output signal of the microphone from a sneeze was decoded to an integer array of size 128 from PDM and was stored into a csv file. 

#### 4.4 Data Preprocessing

##### 4.4.1 Feature Extraction
  At the start of each sneeze recording session, the Arduino microphone was calibrated for the first 3 seconds to get rid of background noises and offsets. After each sneeze was captured, FFT was performed to convert the data from the time domain to frequency domain for feature extraction: 
# FFT image
  The reason was that the feature that was intended to be used for sneeze detection is the frequency instead of amplitude, since the frequency of the sneeze is significantly higher than other activities such as coughing, finger snapping and talking. The waveforms of sneeze as well as other activities are plotted in frequency domain as below. The peak frequency of the sneeze is around 5000-6000Hz.
- Sneezing:
![sneeze](https://user-images.githubusercontent.com/72180511/102526103-0cb9da00-4050-11eb-8f42-ae07646af700.JPG)
- Coughing:
![coughing](https://user-images.githubusercontent.com/72180511/102526181-25c28b00-4050-11eb-94ee-3f4ca12650e2.JPG)
- Finger snapping:
![snapping](https://user-images.githubusercontent.com/72180511/102526231-3a068800-4050-11eb-8c26-7f4be060c8d1.JPG)
- Talking:
![talk](https://user-images.githubusercontent.com/72180511/102526295-4f7bb200-4050-11eb-9a8d-3859ee472d54.JPG)

##### 4.4.2 Filtering
The data was then filtered by the Finite Impulse Response(FIR) filter. The FIR filter was chosen because it is stable and easy to implement. The FIR filter structure is shown below:
# FIR image
The filter input is applied to a sequence of delays, and the output from each delay is then applied to the input of multipliers, which has a set of coefficients. The number of coefficients that were used in this project is 46. The output of each multiplier is then added together using an adder, and the output of the adder is the filtered output.

#### 4.5 Classification
The classifier was implemented using two types of Python libraries: sklearn and TensorFlow.
In sklearn, several different classifiers were used and compared: SVM, RandomForest,  DecisionTree and LogisticRegression. Below are the specific parameters that were used for each of the classifier:
| Classifier  | SVM | RandomForest  | DecisionTree | LogisticRegression  |
| --- | ---  | --- | --- | --- |
| Parameters |kernel='rbf', gamma = 0.2, C=100 | max_depth=3, random_state=0 | default | random_state=0 |

In TensorFlow, the neural network based on TinyML was implemented and below are the  related parameters:
| Model Type  | Sequential |
| ------------- | ------------- |
| Number of layers | 3 |
| Activation functions | ‘relu’, ‘relu’, ‘softmax’  |
| Optimizer | ‘adam’ |
| Loss | ‘sparse_categorical_crossentropy’ |
| Metrics | ‘accuracy’ |
| Iteration  | 2000 |
| Batch size | 16 |

After each of the models had been trained, it was deployed to plain C code using micromlgen Python library so that the model could be interpreted by the Arduino board and perform sneeze detection in real-time using the microphone input.

#### 4.6 Wireless Communication
The ArduinoBLE and the bleak library are used to establish BLE communication between the board and the computer. Every time a sneeze is being detected, a byte array containing ‘0x01’ will be sent from the Arduino board to the computer. The computer is always ready and awaits receiving the signal from the board. Upon receiving ‘0x01’, the computer will print out a message “Bless you!” on the screen, at the same time a sound snippet of “bless you” will be played.

---
## 5.Experimental Results
Below are the model training accuracy results for each of the classifier:
| Classifier  | SVM | RandomForest  | DecisionTree | LogisticRegression  | Convoluted Neural Network |
| ---  | --- | ---  | --- | ---  | --- |
| Training Accuracy  | 64.1% | 97.4%  | 89.0% | 82.1%  | 89.7% |

RandomForest and Neural Network classifiers come with the highest training accuracy, with respect to 97.4% and 89.7%. Below are the confusion matrices for Random Forest Classifier and Neural Network Classifier:
- Random Forest Classifier
![CM_RFC](https://user-images.githubusercontent.com/72180511/102528284-345e7180-4053-11eb-8717-1abdaaaf2ba9.png)
- Neural Network Classifier
![CM_NN](https://user-images.githubusercontent.com/72180511/102528293-36283500-4053-11eb-8c69-1019b02a57b9.png)

According to the matrices, Random Forest Classifier generates better results when detecting sneezes compared with Neural Network Classifier in theory. However, when testing with real-time sneeze data on Arduino, the testing accuracy of the Neural Network Classifier is around 95%, while the testing accuracy of the Random Forest Classifier is only 80%. 

---
## 6.Conclusion & Future work
In this project,  a robust and reliable sneeze detecting system is implemented with an embedded machine learning model. The convoluted neural network ML model is chosen as the primary classifier with around 95% testing accuracy. The classification algorithms are optimized using FFT and FIR filters to deal with confusing cases such as snapping fingers and coughing. The BLE communication between the Arduino board and the computer is also established. 

However, this is not the end point of the project. First, the classifier can be further optimized by incorporating sensors such as motion sensors to detect the position of the device and add weight variables. Second, better filtering methods are considered when capturing audio signals to deal with unwanted spikes. Lastly, since the project only uses the microphone and MCU on the Arduino 33 BLE Sense board, the size of the device can be further decreased by designing a customized PCB board that contains only essential parts of the project.



