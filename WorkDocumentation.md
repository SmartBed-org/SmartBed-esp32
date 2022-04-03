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

- [ ] Tom should provide HX711 AD weight module on sunday. Then I need to successfully read data from it.
First I need to meet with tom and to solder the module and the sensors.

- [ ]  Perform measurements to identify patient trying to climb on the bed's barrier using the flexible pressure sensors. Check different variations to negate situations where patient only rolls on his side for example.
I wasn't able to perform the measurements since the readings from the sensor are messed up. I have no idea why, since yesterday it worked perfectly and I didn't move any connections. Full problem description [here](https://docs.google.com/document/d/1LDruNqVpN6IS-AOigKtfhX7A0iwg1qz5cJxxVF7YbYY/edit#).

- [ ] Perform measurements to identify patient trying to climb on the bed's barrier using the weight sensors. Check different variations to negate situations where patient only rolls on his side for example.

**SW Tasks**

- [ ]  Talk to Leon & Tomer and understand how _exactly_ they want the data to be stored in the firestore db, so it'll be convenient for them to read it from the application. Perhaps what I did is good?
