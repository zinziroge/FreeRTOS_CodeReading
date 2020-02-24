= hoge

== 対象バージョン
 * Arduino_FreeRTOS_Library
 ** https://github.com/feilipu/Arduino_FreeRTOS_Library.git
 *** 10.2.0-3
 * FreeRTOS
 ** 161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf
 ** FreeRTOS_Reference_Manual_V10.0.0.pdf
 * H/W
 ** Arduino UNO(ATMega328p)
 * Arduino IDE 1.8.10

//table[fig1][ファイル一覧と概要]{
name description
-------------------------
Arduino_FreeRTOS.h hoge
croutine.c         hoge
croutine.h||
event_groups.c||
event_groups.h||
FreeRTOSConfig.h||
FreeRTOSVariant.h||
heap_3.c||
list.c||
list.h||
message_buffer.h||
mpu_wrappers.h||
port.c||
portable.h||
portmacro.h||
projdefs.h||
queue.c||
queue.h||
semphr.h||
stack_macros.h||
stream_buffer.c||
stream_buffer.h||
task.h||
tasks.c||
timers.c||
timers.h||
variantHooks.cpp||
//}

== 変数命名規則
 * 'c' : char
 * 's' : int16_t
 * 'x' : BaseType_t
 * 'p' : ポインタ
 * 'u' : unsigned

== 関数命名規則
 * 'v' で始まる関数の戻り値は void
 * 'p' で始まる関数の戻り値は pointer
 * 'x' で始まる関数の戻り値は BaseType_t
 * Task で始まる関数の実体は、tasks.c に記述されている
 ** pvTimerGetTimerID() は void pointerを返す関数でtimers.cで定義されている
 * 'prv' で始まる関数は private 関数

