# RTOS_Assignments
Healthcare system design using RTOS:
https://docs.google.com/document/d/1BXU7wduERB2gXLKHvBevEfalvn8D_3wIfmur4czQB4s/edit

#System Requirements:

#Design an healthcare system using RTOS with the following requirements:

1- A touch LCD as input that can control the system and give commands.
    Every LCD command is represented in 4 bytes.
    The LCD is connected to the Mu through UART with speed 9600 bps[Bit per Second].
    (Reading 4 bytes and processing the command takes 2ms).

2-Blood pressure sensor with new data every 25ms.
(Reading the sensor and processing its data takes 3ms)

3-Heart beat detector with new data every 100ms.
(Reading the sensor and processing its data takes 1.5ms)

4-Temperature sensor with new data every 10ms.
(Reading the sensor and processing its data takes 3ms)

5-Alert siren.( Activate or Deactivate the siren takes 3ms)
