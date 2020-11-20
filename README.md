# Blessyou

## 1.Motivation
During the COVID-19 pandemic, most of the school switches to online lectures. Unlike in-person lectures, online lectures lack normal interaction between students, such as the “bless you” from your classmates. To increase the engagement of the students during class, we come up with the idea to create an interactive sneeze detector that will notify your classmates after you sneeze. And it has the potential to be developed as a respiratory sickness prevention device.

---
## 2.Abstract
The central idea of the project is to enable sneeze detection using the microphone on Arduino Nano 33 BLE Sense. Machine learning will be used to detect sneezes. Pre-recorded sneezing samples will be used to train and test the machine learning model. The sneezing samples will be pre-processed by filters to reduce background noises. The goal of the project is to detect different kinds of sneezes and deal with confusing cases, such as talking loudly or coughing. After the sneeze is detected, a signal will be sent from the board to the computer using Bluetooth and the text “Bless You!” will be shown.

## 3.Techniques Needed
- Voice signal processing
- Data filtering and noise reduction
- Machine learning
- Power optimization

---
## 4.Timeline
- Week 4: Research on the topic and basic test
- Week 5: Data collection and build filters
- Week 6: Build the machine learning model and train data
- Week 7: Robustness and power optimization
- Week 8: BLE communication 
- Week 9: Debug and test
- Week 10: Demo and build github website

## 5.Goals
- Detect sneezes from different sound sources
- Deal with confusing cases such as talking loudly or coughing

