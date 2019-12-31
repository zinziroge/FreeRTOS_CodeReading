* version
  * ArduinoIDE: ver 1.8.10
  * FreeRTOS: ver 10.2.0-3
* src
  * Blink_AnalobRead
* 内容
  * code reading
  * code reading方法
    * vscodeの設定
  * build folder, IDEの設定
  * avr-gcc assembly lang
  * atmega328pの構成
  * elf のdisassemble
    * -d: asmのみ
    * -S: cソースとasm
  * elf のフォーマット


"C:\\Users\\shin\\AppData\\Local\\Arduino15\\packages\\arduino\\tools\\avr-gcc\\7.3.0-atmel3.6.1-arduino5/bin/avr-gcc" -c -w -Os -g -flto -fuse-linker-plugin -Wl,--gc-sections -mmcu=atmega328p "C:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832\\sketch\\Blink_AnalogRead.ino.cpp.o" "C:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832\\libraries\\FreeRTOS\\croutine.c.o" "C:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832\\libraries\\FreeRTOS\\event_groups.c.o" "C:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832\\libraries\\FreeRTOS\\heap_3.c.o" "C:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832\\libraries\\FreeRTOS\\list.c.o" "C:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832\\libraries\\FreeRTOS\\port.c.o" "C:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832\\libraries\\FreeRTOS\\queue.c.o" "C:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832\\libraries\\FreeRTOS\\stream_buffer.c.o" "C:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832\\libraries\\FreeRTOS\\tasks.c.o" "C:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832\\libraries\\FreeRTOS\\timers.c.o" "C:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832\\libraries\\FreeRTOS\\variantHooks.cpp.o" "C:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832/core\\core.a" "-LC:\\Users\\shin\\AppData\\Local\\Temp\\arduino_build_933832" -lm