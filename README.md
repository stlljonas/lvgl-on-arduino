# Setup

There are currently three versions to use in this repo.

1. SensiHub demo
	A sketch that scans and displays nearby Sensirion BLE Gadgets which you can cycle through
	using the buttons.
	This is currently on the main branch and works with the TTGO T-Display S3. 
	The correct TFT_eSP library needs to be in the Arduino/libraries/ folder. 
	It can be found [here](https://github.com/Xinyuan-LilyGO/T-Display-S3/tree/main/lib).
	Make sure to use the correct board if you are using the Arduino IDE. I had to update
	the "additional board manager" url for it to show up.

	This sketch also needs the montserrat_30 font to be enabled. Do this by setting the LV_FONT_MONTSERRAT_30 macro to 1. It is found in the `lv_conf.h` file next to the lvgl directory (or the `lvgl/lv_conf.h` file if that doesn't work).

2. Display image
	This is a separate sketch also on the main branch. It shows how to display and image that
	was converted through LVGLs online [image converter tool](https://lvgl.io/tools/imageconverter).
	Note that this only runs on the original T-Display board. For an upgrade to the S3, you will need
	to adjust the screen dimensions and the pinout of Button two (have a look at the proof-of-concept sketch).
3. Bare Bones
	This is a separate branch (bare-bones) that contains a minimal running sketch that displays "hello world".
	Again, this is currently only setup for the original T-Display board and needs to be adjusted as before. 

# PlatformIO hints

You will need to clone https://github.com/Xinyuan-LilyGO/T-Display-S3 into your home directory. For another location (e.g. `~/Arduino/libraries`), change `~/T-Display-S3/lib` to your path in `platformio.ini` (e.g. `~/Arduino/libraries/T-Display-S3/lib`).