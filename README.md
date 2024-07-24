# ESP32 GEMINI
Chat with Gemini on ESP32-2432S028 CYD (cheap yellow display)
This project uses LVGL and Gemini API, its a simple AI project with ESP32-2432S028

## Examples:
![](https://github.com/MYusufY/lvgl-gemini/blob/main/examples/example_1.gif)
![](https://github.com/MYusufY/lvgl-gemini/blob/main/examples/example_2.gif)
## YouTube video:
[You can watch it here.](https://www.youtube.com/watch?v=O8mf3UPJdV8)

# How to: Build and Upload!
## Setup Libraries
The requried libraries with their configuration files are given under the "arduino_lib" folder. Change the libraries path in platformio.ini file to the path of arduino_lib folder.

## Building & Uploading
*I didnt test if it works by simply opening the main.cpp file with Arduino IDE and uploading it* (you should choose ESP32 Dev Module as board) but you can try it yourself. If it works:
1. Setup libraries like shown above.
2. Set your wifi ssid, password and api key settings in main.ino
3. Select ESP32 Dev Module as Board
4. Upload
5. Done!
   
*IF ARDUINO IDE DOESNT WORK or YOU ARE ALREDY USING PLATFORMIO IN YOUR PROJECTS:*
1. Setup libraries like shown above. 
2. Open the "platformio_project" project folder in platformio.
3. Change your wifi ssid, password and api key settings in main.ino
4. Upload
5. Done!

# What it does?
You can send messages to Gemini using the textarea on top of the screen, and get answers quickly!

# Contact
You can contact me here: 
tachion.software@gmail.com 
