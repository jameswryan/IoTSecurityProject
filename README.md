# IoTSecurityProject
This is my midterm project for EE4953 IoT Security

This is a midterm project. There is some deliberately bad security practices here, mostly so that the second half of the project can be interesting.
Not all poor practices are documented. If something is bad with no explanation, please let me know so that I can either fix it, or just leave a note explaning why it is that way.



### PLEASE DO NOT USE THIS IN ANY SORT OF PRODUCTION!
If you want to use this in a long term project, fork it and modify what you need.
Again, this project is deliberatly insecure!!


## Project Description
This is a 'smart' parking lot tracking system.

There are two distance sensors on either side of an entrance, and when a vehicle passes by one sensor will be closer than the other.
By using this, we can determine whether the vehicle is leaving or entering.

There is also a display, which for now is a 2x23 LCD screen that displays the occupancy of a parking lot, and a message whenever a vehicle enters or exits.

This repository contains the code for the sensors and the display. 
There is also a server written in Python3 running on the Raspbery PI 4 that doubles as a LoRaWAN gateway.
