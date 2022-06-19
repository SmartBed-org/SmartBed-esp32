# **Work Documantation**

## **26/03 - 02/04**

**Logistics and Planning Tasks**

- [x] Design 5 screens for application (initial sign up using phone number, login at shift start, push notification, opening notification screen and regular screen on opening the app). links to figma design:

1. [screens part 1](https://www.figma.com/file/vtczJ2zHBNRAVQ3KVYjtSi/Smart-Bed-Project-Screens?node-id=3%3A8)
2. [screens part 2](https://www.figma.com/file/JDiBIzdEYRwTZ5Z5PKktg3/Smart-Bed-Project-Screens-%232?node-id=0%3A1)

**HW Tasks**

- [x] Read & analyze information from 3 types of sensors (capacitance, force, pressure)

- Successful reading from [pressure sensor](https://www.aliexpress.com/item/33031549373.html). followed this [guide](https://www.electroniclinic.com/flexiforce-pressure-sensor-or-force-sensitive-resistor-fsr-programming-calibration-using-arduino/). Reading in stable state show 343-345g, deviation of -10g when mild force applied. Since the analog output from the sensor changes rapidly, to increase stable reading time it uses 47uF caps, I had only 100nF one's - got some good looking results, but should try using something from the uF scale, perhaps the readings will improve.

- Didn't succeed reading from the [force sensors](https://www.amazon.com/Half-bridge-Weighting-Sensor-Amplifier-Geekstory/dp/B079FTXR7Y), I think there's a module missing (HX711 AD Weight Module)
**Tom said the module was forgotten by mistake. Will be provided on sunday**

- For some reason, we didn't get capacitance sensors, already emailed to Tom & Yaron about it.
**Tom said we should start with the other two sensors and after we'll have some results we'll decide whether the capacitance sensors are relevant at all.**

- [x] Write from arduino to firebase db

- I'm following this [official arduino guide](https://github.com/mobizt/Firebase-ESP-Client/tree/main/examples/Firestore). Currently I'm stuck, waiting for Tomer to create Authentication for the project, which should produce API key required for firestore config in the cpp code to program the esp, I read it in this stackoverflow [issue](https://stackoverflow.com/questions/64690983/web-api-key-is-not-generated-in-firebase-while-creating-a-new-project-as-stated).

- So I added an Authentication method (email & password) and added myself as a user. Now I was able to complete the aforementioned guide, and indeed I succeeded to write to the firestore database using the esp32 board.

## **03/04 - 09/04**

**HW Tasks**

- [x] Tom should provide HX711 AD weight module on sunday. Then I need to successfully read data from it.

- First I need to meet with Tom to solder the module and the sensors.

- [x]  Perform measurements to identify patient trying to climb on the bed's barrier using the flexible pressure sensors. Check different variations to negate situations where patient only rolls on his side for example.

- I wasn't able to perform the measurements since the readings from the sensor are messed up. I have no idea why, since yesterday it worked perfectly and I didn't move any connections. Full problem description [here](https://docs.google.com/document/d/1LDruNqVpN6IS-AOigKtfhX7A0iwg1qz5cJxxVF7YbYY/edit#).

- [x] Perform measurements to identify patient trying to climb on the bed's barrier using the weight sensors. Check different variations to negate situations where patient only rolls on his side for example.

**SW Tasks**

- [ ]  Talk to Leon & Tomer and understand how _exactly_ they want the data to be stored in the firestore db, so it'll be convenient for them to read it from the application. Perhaps what I did is good?

### **06/04 Update**

- The problem with the flexible sensor was probably due to grounding one of the sensor's legs which enforces constant voltage in parallel to the sensor, so no resistance variations can be detected. under the new configuration which can be viewed below, the reading are good. Sensor activation video [link](https://photos.app.goo.gl/xSvtRRgVR1SxqvyEA)

<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/162001090-fe11ebb2-2ed0-43ee-accc-664b3781e378.png width="400" height="200">
</p>

- We've encountered a problem wiring the 50kg weight sensors to the HX711 module. apperantly, the sensors arrived with wrong wire colors, so it was problematic to match the wires in the real sensor to the one's in the guide. we followed [this](https://circuitjournal.com/50kg-load-cells-with-HX711) guide to detect the right wires, to solder the sensors & module and to code the board.

- After I finished wiring, the board stopped working. weird error message saying "A fatal error occurred: Timed out waiting for packet content". after presenting the problem to Tom it was decided that I will try different board that I have at home (DOIT esp32 DEVKIT V1) specs can be viewed [here](https://www.mischianti.org/2021/02/17/doit-esp32-dev-kit-v1-high-resolution-pinout-and-specs/) and it did work well, we think the board burnt for some reason. Sensor activation video [link](https://photos.app.goo.gl/5ZaizVf2jBCRCndx7)


### **07/04**

New insights about why the board burnt - the guide I followed for the flexible force sensor used different kind of sensor. To create the voltage divider with the sensor, 10 [Kohm] resistor was used. When I removed the resistor, the voltage drop across the sensor wad negligible, and most of the 5V input fell on the GPIO pin of the board, which made it burn.
The reason I didn't see any variations in he voltage when I used the 10k resistor was probably since it's resistance was too small in comparison to the resistance of the sensor (after measurement the sensor's resistance is ). Now, I rebuilt the configuration using 330 [Kohm] resistors, and I feed the circuit with 3.3V - works well. The configuration is given below:

<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/162128406-8c78f793-6710-47a2-b00c-d854513dbb6f.png width="300" height="300">
</p>

---
<h3 align="center"> Experiments Update </h3>

<h4 align="center"> Bed Structure </h4>
<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/162354943-cfcd0977-efc2-4604-894d-c1a0f08d6637.jpg width="480" height="450">
</p>



**Weight Sensor Results**

- steady state reading (with movement in bed) - 3-4 (3.4-3.6 on avg.)
- when climbing attempted - 1.9-3 (2.6-2.7 on avg.)

- climbing attempt detected when reading started to drop toward 3 and below

<h4 align="center"> Climbing Attempt Readings </h4>
<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/162216793-6a48f4a2-61bb-4aeb-8c05-0ab6006715e9.png width="480" height="450">
</p>

<h4 align="center"> Random Movement Readings </h4>
<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/162215090-cbfc3311-7a66-4fd1-8226-a359116a41f9.png width="360" height="450">
</p>

<h4 align="center"> Sitting in Bed Readings </h4>

<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/162218312-1ab9b934-6274-4a77-8a7b-8f513603508d.png width="360" height="450">
</p>

<h4 align="center"> Sensor Location </h4>
<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/162355192-81a2795b-dd67-4017-a5b8-edcc6f5348c3.jpg width="360" height="450">
</p>

<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/162356498-7a004d63-676a-44f9-935e-7dfb78b541fd.png width="580" height="350">
</p>



**Flexible Force Sensor Results**

- steady state reading (with movement in bed) - 30-60 (40 on avg.)
- when climbing attempted - 70-90 (80 on avg.)

<h4 align="center"> Sensor Location </h4>
<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/162356893-542fb549-2ad9-472b-94bd-fb211d88d9a4.png width="360" height="450">
</p>



#### Videos of the experiments can be viewed [here](https://drive.google.com/drive/folders/1aCJvnqv-2BqEwhJzU9by0vvKYv22XTE9?usp=sharing)

## **10/04 - 23/04**

### **16/04**

I've integrated both types of sensors (4 weight sensors and 2 flexible pressure sensors) into one full system. The model mimics the expected behaviour of the system, only instead of writing from the esp board to firebase when climbing attempt is detected, it switches on an LED.

Additionaly, I've connected the weight sensors, using dedicated mounts, onto a wooden board to allow better detection. The configuration is as follows:


<h4 align="center"> Weight Sensor Mounting to a Wooden Board </h4>
<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/163719978-82ddf4de-07aa-402f-b733-d70b783f5da8.jpeg width="360" height="450">
</p>


After performing some experiments, I understood that the weights sensors and the wooden board create a gap between the matress and the metalic bed skeleton, and harms the pressure sensor readings.

### **17/04**

To fix the previously mentioned problem, I've elevated the pressure sensors to be at the same plane as the top of the wooden board, and used the following configuration of the sensors:

<h4 align="center"> Full System Configuration </h4>
<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/163720023-2634cef7-e502-42fa-8697-fef409b5372e.jpeg width="360" height="450">
</p>

the results were very encouraging, the climbing attempt was detected in very high persicion and the response was very quick, as can be viewed [here](https://drive.google.com/drive/folders/1lhLVnGww8cEn1cMLH3RZlG8mfhjyOJLZ?usp=sharing)

#### *Next Tasks for 19/04:*
- [x]  Schedule a meeting with Tom and Yaron to get their approval regarding the results (after Passover)
- [x]  Finish LPWAN part for the seminar
- [ ] Plan the final video & final report

## **24/04 - 30/04**

### **27/04**
Meeting with Tom & Yaron. Positive feedback regarding results and seminar presentation.

### Tasks:
- [x] Need to switch wooden board for a bigger one so the system will be more robust.
- [x] Finish code so it will write to Firestore


### **28/04**

When I connected the board ti Wifi, Flexible force sensors stopped woring. debugging proccess:
- the weight sensors are working - the problem can't be with the PC or esp32 board
- used voltmeter to check sensor resistance - resistance changes with pressure - sensors are working
- tried to switch wires & resistors - still got nothing

After long reaserch proccess the answer was in the knowledge bank - esp32 has 2 types of analog pins: ADC1 & ADC2. ADC2 pins are used by wifi thus are not available when activating wifi. Solution: switched to ADC1 pins

### **30/04**

Switched to bigger wooden board so the system will be more robust as depicted below:

<h4 align="center"> Full System Configuration Using Wider Wooden Board </h4>
<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/167111327-c05a3d67-5b40-4d23-885d-46e3f849d6e7.jpeg width="360" height="450">
</p>


+ Updated code so it will be independent of calibration factor (normalized the input data and used threshold when the calue drops below 65%). more details in the code    documentation. 
+ Cleaned & added documentation to the code.
+ More videos documenting the results can be found [here](https://drive.google.com/drive/folders/18-_WjRkURPTSdkoMlGn1igrQaprKSmDA?usp=sharing)

### **Tasks:**
- [x] Solder wires nicely to metallic breadboard instead the plastic one


### **03/05**
Soldered the wires to Proto-board and ran few more tests

<h4 align="center"> Old Bread-Board Wiring </h4>
<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/167112013-5a95101a-7692-4546-9f86-edf978e78266.jpeg width="360" height="300">
</p>

<h4 align="center"> New Proto-Board Wiring </h4>
<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/167111933-4f0fe4e3-d493-49b0-89d9-f07feca317a6.jpeg width="360" height="250">
</p>

<h4 align="center"> Comparison between Bread-Board and Proto-Board Wiring </h4>
<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/167111875-822ec3e5-4766-4c1e-a73d-e954153bd37c.jpeg width="360" height="250">
</p>

More videos of the results may be viewed [here](https://drive.google.com/drive/folders/13qpj0Ir222XzYvJPqSrDwS6SJCgHgy94?usp=sharing)

## Tasks ##
- [x] Set up meeting with Tom to ensure HW part is done and continue working on the SW part


### **10/05**
After meeting with Tom - basically HW is done. We scheduled a meeting for Monday 16/05 to checkout some improvements for increasing pressure sensor range using sponges. Additionaly, Tom suggested that it is optional to find (5) volunteers that will test our system (5 repetitions - FALSE POSITIVE).

### **29/05**
After meeting again with Tom, we checked if extending sensors detection range is neccessary (using rigid sponges). concluded there's no need for further treatment, good as it is.

## Tasks ##
- [x] Fix strain relief of wires on proto-board
- [x] Decide how to connect the permenantly fix the wooden board to the bed (using neodymium magnets?)
- [x] Perform aformentioned experiments

## **31/05**
Performed 1st experiment with a volunteer, results were good. All the results will be updated during the week and may be viewed [here](https://drive.google.com/drive/folders/1EvLZEdo0fVP3fRZgbWB85z9uWv-5C-ld?usp=sharing)

## **01/06 - 12/06**
* Performed experiments with 4 volunteers, produced graphs & video documentation.
* Finished HW fixing and refinment
  - All boards & controllers are sealed within boxes using restrains for strain relief
  - Dedicated mounting devices were 3D printed to keep the wooden board from moving while performing experiments. The mounts were connected to the bed skeleton         using magnets

## **Tasks Before Submission**
- [ ] Video
- [x] Finish final report

## **15/06**
Tom printed dedicated mounts to keep the wooden board from moving. The mounts:
<h4 align="center"> Special 3D printed mounts to keep the wooden board from moving </h4>
<p align="center">
<img src=https://user-images.githubusercontent.com/48283282/174477371-d89c4e3e-d01f-4705-a21e-ec992920f82b.png width="450" height="250">
</p>!



