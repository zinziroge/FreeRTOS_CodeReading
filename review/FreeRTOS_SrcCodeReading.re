= Blink_AnalogRead.ino

#@# ここからソースコードリーディング ##########################################################

== どこから読むか

FreeRTOSをインストールするとFreeRTOSを使用したサンプルもいくつかインストールされます。
その中から Blink_AnalogRead(example/Blink_AnalogRead.ino) を読みながら、
FreeRTOSのソースコードリーディングを進めていきます。

example/Blink_AnalogRead.ino をまず見てみましょう。

//listnum[Blink_AnalogRead_1][example/Blink_AnalogRead.ino]{
#include <Arduino_FreeRTOS.h>

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void TaskAnalogRead( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);

  while (!Serial) {
    ;
  }

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink
    ,  (const portCHAR *)"Blink"   // A name just for humans
    ,  128  @<embed>$|latex|\linebreak\hspace*{5ex}$// This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  @<embed>$|latex|\linebreak\hspace*{5ex}$// Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the @<embed>$|latex|\linebreak\hspace*{5ex}$lowest.
    ,  NULL );

  xTaskCreate(
    TaskAnalogRead
    ,  (const portCHAR *) "AnalogRead"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual @<embed>$|latex|\linebreak\hspace*{5ex}$tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void TaskAnalogRead(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;)
  {
    int sensorValue = analogRead(A0);
    Serial.println(sensorValue);
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}
//}

Arduinoのスケッチでは、setup()関数とloop()関数が必要です。
そのsetup()関数とloop()関数は下記ソースコードから呼ばれています。

//listnum[main][C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\main.cpp::main()][c]{
int main(void)
{
	init();

	initVariant();

#if defined(USBCON)
	USBDevice.attach();
#endif

	setup();

	for (;;) {
		loop();
		if (serialEventRun) serialEventRun();
	}

	return 0;
}
//}

#@# なぜ、variantHook.cpp から読み始めるか
#@# https://feilipu.me/2015/11/24/arduino_freertos/ に

C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\wiring.c:init()

それではinitVariant()関数から読んでいきましょう。

//listnum[initVariant][variantHook.cpp::initVariant()][c]{
void initVariant(void) __attribute__ ((OS_main));
void initVariant(void)
{
#if defined(USBCON)
    USBDevice.attach();
#endif

    setup();        // the normal Arduino setup() function is run here.
    vTaskStartScheduler(); @<embed>$|latex|\linebreak\hspace*{5ex}$// initialise and run the freeRTOS scheduler. Execution should never return here.
}
//}

https://gcc.gnu.org/onlinedocs/gcc/AVR-Function-Attributes.html によると、
OS_mainアトリビュートを宣言すると、この関数に入るときには割り込みが禁止されていることが保証されていて、
スタックへの退避や復帰が行われないので、スタック領域の使用量を増やさない効果があるようです。
USBCON は未定義です。setup()は、Arduino スケッチファイル(.ino) でユーザが記述するsetup()関数です。

次はvTaskStartScheduler()のコードを見ていきましょう。

//listnum[vTaskStartScheduler_1][tasks.c::void vTaskStartScheduler(), 1/7][c]{
void vTaskStartScheduler( void )
{
BaseType_t xReturn;

    /* Add the idle task at the lowest priority. */
    #if( configSUPPORT_STATIC_ALLOCATION == 1 )
    {
        StaticTask_t *pxIdleTaskTCBBuffer = NULL;
        StackType_t *pxIdleTaskStackBuffer = NULL;
        configSTACK_DEPTH_TYPE ulIdleTaskStackSize;

        /* The Idle task is created using user provided RAM - obtain the
        address of the RAM then create the idle task. */
        vApplicationGetIdleTaskMemory( &pxIdleTaskTCBBuffer, @<embed>$|latex|\linebreak\hspace*{5ex}$&pxIdleTaskStackBuffer, &ulIdleTaskStackSize );
        xIdleTaskHandle = xTaskCreateStatic(    prvIdleTask,
                                                configIDLE_TASK_NAME,
                                                ulIdleTaskStackSize,
                                                ( void * ) NULL, @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e961.  The cast is not redundant for all compilers. */
                                                portPRIVILEGE_BIT, @<embed>$|latex|\linebreak\hspace*{5ex}$/* In effect ( tskIDLE_PRIORITY | portPRIVILEGE_BIT ), but tskIDLE_PRIORITY is @<embed>$|latex|\linebreak\hspace*{5ex}$zero. */
                                                pxIdleTaskStackBuffer,
                                                pxIdleTaskTCBBuffer ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e961 MISRA exception, justified as it is not a redundant explicit cast to @<embed>$|latex|\linebreak\hspace*{5ex}$all supported compilers. */

        if( xIdleTaskHandle != NULL )
        {
            xReturn = pdPASS;
        }
        else
        {
            xReturn = pdFAIL;
        }
    }
    #else
    {
        /* The Idle task is being created using dynamically allocated RAM. */
        xReturn = xTaskCreate(  prvIdleTask,
                                configIDLE_TASK_NAME,
                                configMINIMAL_STACK_SIZE,
                                ( void * ) NULL,
                                portPRIVILEGE_BIT, @<embed>$|latex|\linebreak\hspace*{5ex}$/* In effect ( tskIDLE_PRIORITY | portPRIVILEGE_BIT ), but tskIDLE_PRIORITY is @<embed>$|latex|\linebreak\hspace*{5ex}$zero. */
                                &xIdleTaskHandle ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e961 MISRA exception, justified as it is not a redundant explicit cast to @<embed>$|latex|\linebreak\hspace*{5ex}$all supported compilers. */
    }
    #endif /* configSUPPORT_STATIC_ALLOCATION */

    ...

}
//}

BaseType_t は、ハードウェアアーキテクチャで最も効率的に扱えるデータタイプです。
Arduino Unoで使用されているMPUであるATMega328pでは、singed char(portmacro.hで定義されている)と定義されています。

configSUPPORT_STATIC_ALLOCATION は 0 なので、 #else節が有効になります。
(以降、特に断りなく#if definedで無効なブロックは引用せず省略する場合があります。)

configSUPPORT_STATIC_ALLOCATIONはFreeRTOSConfig.hで定義されています。
configで始まるマクロ変数は、FreeRTOSConfig.hで定義されています。
portで始まるマクロ変数は、portmacro.hで定義されています。


== タスク作成API xTaskCreate() 前半部

タスク作成のAPIである`xTaskCreate()`のコードを読んでいきます。
長いので2つに分けて読んでいきます。まず前半部です。

//listnum[xTaskCreate_1][tasks.c::BaseType_t xTaskCreate(), 1/2][c]{
    BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                            const char * const pcName, @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e971 Unqualified char types are allowed for strings and single characters @<embed>$|latex|\linebreak\hspace*{5ex}$only. */
                            const configSTACK_DEPTH_TYPE usStackDepth,
                            void * const pvParameters,
                            UBaseType_t uxPriority,
                            TaskHandle_t * const pxCreatedTask )
    {
    TCB_t *pxNewTCB;
    BaseType_t xReturn;

        /* If the stack grows down then allocate the stack then the TCB so the stack
        does not grow into the TCB.  Likewise if the stack grows up then allocate
        the TCB then the stack. */
        #if( portSTACK_GROWTH > 0 )
        {
            ...
        }
        #else /* portSTACK_GROWTH */
        {
        StackType_t *pxStack;

            /* Allocate space for the stack used by the task being created. */
            pxStack = ( StackType_t * ) pvPortMalloc( @<embed>$|latex|\linebreak\hspace*{5ex}$( ( ( size_t ) usStackDepth ) * sizeof( StackType_t ) ) ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9079 All values returned by pvPortMalloc() have at least the alignment @<embed>$|latex|\linebreak\hspace*{5ex}$required by the MCU's stack and this allocation is the stack. */

    ...
}
//}

TCB_t は、データ構造で説明済みのタスク制御バッファー（Task Control Buffer）のための構造体です。
portSTACK_GROWTH は -1なので、#else節が有効です。 スタックのアドレスは小さい方に伸びていきます。
StackType_t は uint8_t です(portmacro.hで定義されています)。
usStackDepth はこの関数の引数で、configMINIMAL_STACK_SIZEが値渡しされています。
configMINIMAL_STACK_SIZEの値は192であり、StackType_t は uint8_t なので 192byte のスタック領域を確保します。

C言語に馴染みのある方なら名前から想像される通り pvPortMalloc()関数で動的にメモリをヒープ領域から確保しています。
pvPortMalloc()のコードを読んでいきましょう。


//listnum[pvPortMalloc_1][heap_3.c::pvPortMalloc(), 1/2]{
void *pvPortMalloc( size_t xWantedSize )
{
void *pvReturn;

    vTaskSuspendAll();
    {
        pvReturn = malloc( xWantedSize );
        traceMALLOC( pvReturn, xWantedSize );
    }
    ( void ) xTaskResumeAll();

    ...
}
//}

malloc()でヒープ領域からメモリを確保する前に vTaskSuspendAll() ですべてのタスクをサスペンド(一時停止)し、
メモリ確保後に xTaskResumeAll() ですべてのタスクをレジューム(復帰)しています。
vTaskSuspendAll()、xTaskResumeAll() はこの関数の説明のあとで読んでいきます。

malloc()は stdlib.h で定義されています。
malloc()の実装までは追いませんが、malloc動画(https://youtu.be/0-vWT-t0UHg)などを参考にすると よいかもしれません。
#@# * traceMALLOC( pvReturn, xWantedSize ); はどこで定義されている？ <!-- TODO -->

ヒープ関連のソースファイルは今回参照した heap_3.c の他に heap_1.c, heap_2.c, heap_4.c が含まれる場合があるようです。
@<b>{freertos_mastering} によると、使い分けは以下のとおりです。

 * heap_1.c : スケジューラ起動前にヒープを割り当てる。制約の厳しい組込み向けのようです。
 * heap_2.c : 配列を小さな領域に分割する。新規デザインでは推奨されておらず、heap_4の使用が推奨されています。
 * heap_3.c : 標準ライブラリのmalloc(), free()を使用します。
 * heap_4.c : 配列を小さな領域に分割します。静的に確保されたconfigTOTAL_HEAP_SIZEの大きさのメモリをヒープとして消費していきます。

この文面だけ読むとArduino Unoはheap_1.cに該当しそうですが、heap_3.cが使われています。

続いてタスクのサスペンド(一時停止)処理を見ていきます。
メモリーを確保(malloc())する直前にほかのタスクが実行状態にならないようにサスペンド状態に遷移させます。

//listnum[vTaskSuspendAll][tasks.c::vTaskSuspendAll()]{
void vTaskSuspendAll( void )
{
    /* A critical section is not required as the variable is of type
    BaseType_t.  Please read Richard Barry's reply in the following link to a
    post in the FreeRTOS support forum before reporting this as a bug! -
    http://goo.gl/wu4acr */
    ++uxSchedulerSuspended;
}
//}

コードは1行だけですが、長めのコメントとURIが記載されています。
コメントによると、ここの記述はクリティカルセクションにする必要がないと説明しているようです。
クリティカルセクション（割り込み禁止）は、このあと読んでいくxTaskResumeAll()でも出てきますが、
taskENTER_CRITICAL() と taskEXIT_CRITICAL() でブロックを囲む必要があります。
しかし、ここではそれが必要ないと説明しているようです。
その理由が記載された http://goo.gl/wu4acr の内容を要約してみます。

//quote{
キーポイントは、各タスクはそれぞれ自分自身のコンテキストを管理するので、
変数(uxSchedulerSuspended)がもしゼロだとしてもコンテキストスイッチは起こらない。
従って、レジスタバックからメモリへ書き込むことはアトミック
(訳注：不可分で2つ以上の命令に分割できない。1つのstore命令である。)であり、
問題にならない。
//}

以下のとおりでこの記述で問題ないようです。XXX捕捉説明。

vTaskSuspendAll()と対になるxTaskResumeAll()もここで見ていきましょう。

//listnum[xTaskResumeAll_1][tasks.c::vTaskSuspendAll(), 1/5]{
BaseType_t xTaskResumeAll( void )
{
TCB_t *pxTCB = NULL;
BaseType_t xAlreadyYielded = pdFALSE;

    /* If uxSchedulerSuspended is zero then this function does not match a
    previous call to vTaskSuspendAll(). */
    configASSERT( uxSchedulerSuspended );

    /* It is possible that an ISR caused a task to be removed from an event
    list while the scheduler was suspended.  If this was the case then the
    removed task will have been added to the xPendingReadyList.  Once the
    scheduler has been resumed it is safe to move all the pending ready
    tasks from this list into their appropriate ready list. */
    taskENTER_CRITICAL();
    {
        --uxSchedulerSuspended;

        if( uxSchedulerSuspended == ( UBaseType_t ) pdFALSE )
        {
            if( uxCurrentNumberOfTasks > ( UBaseType_t ) 0U )
            {
                /* Move any readied tasks from the pending list into the
                appropriate ready list. */
                while( listLIST_IS_EMPTY( &xPendingReadyList ) == pdFALSE )
                {
                    pxTCB = listGET_OWNER_OF_HEAD_ENTRY( ( &xPendingReadyList ) ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9079 void * is used as this macro is used with timers and co-routines too.@<embed>$|latex|\linebreak\hspace*{5ex}$  Alignment is known to be fine as the type of the pointer stored and retrieved is @<embed>$|latex|\linebreak\hspace*{5ex}$the same. */
                    ( void ) uxListRemove( &( pxTCB->xEventListItem ) );
                    ( void ) uxListRemove( &( pxTCB->xStateListItem ) );
                    prvAddTaskToReadyList( pxTCB );

                    /* If the moved task has a priority higher than the current
                    task then a yield must be performed. */
                    if( pxTCB->uxPriority >= pxCurrentTCB->uxPriority )
                    {
                        xYieldPending = pdTRUE;
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }
                }

        ...
    }
    taskEXIT_CRITICAL();

    return xAlreadyYielded;
}
//}

taskENTER_CRITICAL() と taskEXIT_CRITICAL() で挟まれたブロックがあります。
このブロック内の処理は割り込み禁止状態で実行されます。
taskENTER_CRITICAL() を読んで、割り込み禁止がどのように実装されているか見ていきましょう。

//listnum[taskENTER_CRITICAL][task.h]{
#define taskENTER_CRITICAL()        portENTER_CRITICAL()
//}

taskENTER_CRITICAL() は portENTER_CRITICAL() の別名定義です。

//listnum[portENTER_CRITICAL][portmacro.h]{
#define portENTER_CRITICAL()    __asm__ __volatile__ (                              \
                                        "in __tmp_reg__, __SREG__"        "\n\t"    \
                                        "cli"                             "\n\t"    \
                                        "push __tmp_reg__"                "\n\t"    \
                                        ::: "memory"                                \
                                        )
//}

アセンブリコードがでてきましたので、1行ずつ読んでいきましょう。
__asm__ はインライン・アセンブラ(高水準言語(ここではC言語)の処理系中に埋込まれているアセンブラ)であることを意味します。
実際はasmがインライン・アセンブラのを表す__asm__ は asm の別名定義です。
__volatile__ は、volatileの別名定義です。

2行目のin命令は、Load an I/O Location to Register の意味でここでは、
ステータスレジスタ__SREG__を__tmp__reg にロードしています(コピーしています)。
3行目のcliは割り込み禁止命令です。xxx外部割り込み、内部割込み
4行目のpush命令で __tmp_reg__ を、つまりロードした__SREG__を、スタックの一番上に積んでいます(コピーしています）。

 * __SREG__	: Status register at address 0x3F
 * __tmp_reg__	: Register r0, used for temporary storage
 * clobber "memory"
 ** コンパイラに対して、アセンブラコードはメモリ配置を修正するかもしれないことを知らせる
 ** それは、コンパイラにレジスタに保持されている全てのコンテンツを保持(退避)することを、アセンブラコード実行前に、強制する。アセンブラコード実行後に書き戻す。
 *** http://www.nongnu.org/avr-libc/user-manual/inline_asm.html

次にportENTER_CRITICAL()と対になるtaskEXIT_CRITICAL()も見ていきましょう。

//listnum[taskEXIT_CRITICAL][task.h.]{
#define taskEXIT_CRITICAL()         portEXIT_CRITICAL()
//}

taskEXIT_CRITICAL() も同様に portEXIT_CRITICAL() の別名定義です。

//listnum[portEXIT_CRITICAL_define][task.h.]{
#define taskEXIT_CRITICAL()         portEXIT_CRITICAL()
//}

//listnum[portEXIT_CRITICAL][portmacro.h]{
#define portEXIT_CRITICAL()     __asm__ __volatile__ (                              \
                                        "pop __tmp_reg__"                 "\n\t"    \
                                        "out __SREG__, __tmp_reg__"       "\n\t"    \
                                        ::: "memory"                                \
                                        )
//}

portENTER_CRITICAL() と逆の操作をします。
2行目のpop命令で、スタックの一番上に積まれた値を __tmp_reg__ にコピーし、スタックアドレスを1つ増やします。
3行目のout命令は、Store an I/O Location to Register の意味で、__SREG__に＿tmp_reg__の値をストア(コピー)します。
結果、portENTER_CRITICAL()で退避した __SREG__ の値が __SREG__ に書き戻されます。
つまり、ステータスレジスタの値を元に戻しています。

vTaskSuspendAll()の続きを読んでいきましょう。

//listnum[xTaskResumeAll_2][tasks.c::vTaskSuspendAll(), 2/5]{
BaseType_t xTaskResumeAll( void )
{
    ...

    taskENTER_CRITICAL();
    {
        --uxSchedulerSuspended;

        if( uxSchedulerSuspended == ( UBaseType_t ) pdFALSE )
        {
            if( uxCurrentNumberOfTasks > ( UBaseType_t ) 0U )
            {
                /* Move any readied tasks from the pending list into the
                appropriate ready list. */
                while( listLIST_IS_EMPTY( &xPendingReadyList ) == pdFALSE )
                {
                    pxTCB = listGET_OWNER_OF_HEAD_ENTRY( ( &xPendingReadyList ) ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9079 void * is used as this macro is used with timers and co-routines too.@<embed>$|latex|\linebreak\hspace*{5ex}$  Alignment is known to be fine as the type of the pointer stored and retrieved is @<embed>$|latex|\linebreak\hspace*{5ex}$the same. */
                    ( void ) uxListRemove( &( pxTCB->xEventListItem ) );
                    ( void ) uxListRemove( &( pxTCB->xStateListItem ) );
                    prvAddTaskToReadyList( pxTCB );

                    /* If the moved task has a priority higher than the current
                    task then a yield must be performed. */
                    if( pxTCB->uxPriority >= pxCurrentTCB->uxPriority )
                    {
                        xYieldPending = pdTRUE;
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }
                }

                if( pxTCB != NULL )
                {

        ...
    }
    taskEXIT_CRITICAL();

    ...
}
//}

スケジューラがサスペンド状態でなく、かつ現在のタスク数がゼロより多いときに、タスク毎にサスペンド状態にしていきます。
#@# ペンディングレディーリストとは？
ペンディング・レディーリストとは、スケジューラがサスペンド中にレディー状態になったタスクのリストです。
それらのタスクをレディーリスト(レディー状態）に移動(遷移)させます。
そのタスクの優先度が現在のタスクより高ければ、そのタスクをxxx実行状態にする？

//listnum[xTaskResumeAll_3][tasks.c::vTaskSuspendAll(), 3/5]{
BaseType_t xTaskResumeAll( void )
{
    ...

                if( pxTCB != NULL )
                {
                    /* A task was unblocked while the scheduler was suspended,
                    which may have prevented the next unblock time from being
                    re-calculated, in which case re-calculate it now.  Mainly
                    important for low power tickless implementations, where
                    this can prevent an unnecessary exit from low power
                    state. */
                    prvResetNextTaskUnblockTime();
                }

    ...

    return xAlreadyYielded;
}
//}

スケジューラがサスペンド中にブロック解除されたタスク。次のアンブロック時間を
低電力消費実装では大事。

//listnum[prvResetNextTaskUnblockTime][tasks.c::prvResetNextTaskUnblockTime()]{
static void prvResetNextTaskUnblockTime( void )
{
TCB_t *pxTCB;

    if( listLIST_IS_EMPTY( pxDelayedTaskList ) != pdFALSE )
    {
        /* The new current delayed list is empty.  Set xNextTaskUnblockTime to
        the maximum possible value so it is    extremely unlikely that the
        if( xTickCount >= xNextTaskUnblockTime ) test will pass until
        there is an item in the delayed list. */
        xNextTaskUnblockTime = portMAX_DELAY;
    }
    else
    {
        /* The new current delayed list is not empty, get the value of
        the item at the head of the delayed list.  This is the time at
        which the task at the head of the delayed list should be removed
        from the Blocked state. */
        ( pxTCB ) = listGET_OWNER_OF_HEAD_ENTRY( pxDelayedTaskList ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9079 void * is used as this macro is used with timers and co-routines too. @<embed>$|latex|\linebreak\hspace*{5ex}$ Alignment is known to be fine as the type of the pointer stored and retrieved is @<embed>$|latex|\linebreak\hspace*{5ex}$the same. */
        xNextTaskUnblockTime = listGET_LIST_ITEM_VALUE( @<embed>$|latex|\linebreak\hspace*{5ex}$&( ( pxTCB )->xStateListItem ) );
    }
}
//}



//listnum[xTaskResumeAll_4][tasks.c::vTaskSuspendAll(), 4/5]{
BaseType_t xTaskResumeAll( void )
{
    ...

                /* If any ticks occurred while the scheduler was suspended then
                they should be processed now.  This ensures the tick count does
                not    slip, and that any delayed tasks are resumed at the correct
                time. */
                {
                    UBaseType_t uxPendedCounts = uxPendedTicks; /@<embed>$|latex|\linebreak\hspace*{5ex}$* Non-volatile copy. */

                    if( uxPendedCounts > ( UBaseType_t ) 0U )
                    {
                        do
                        {
                            if( xTaskIncrementTick() != pdFALSE )
                            {
                                xYieldPending = pdTRUE;
                            }
                            else
                            {
                                mtCOVERAGE_TEST_MARKER();
                            }
                            --uxPendedCounts;
                        } while( uxPendedCounts > ( UBaseType_t ) 0U );

                        uxPendedTicks = 0;
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }
                }

    ...

    return xAlreadyYielded;
}
//}



//listnum[xTaskResumeAll_5][tasks.c::vTaskSuspendAll(), 5/5]{
BaseType_t xTaskResumeAll( void )
{
    ...

                if( xYieldPending != pdFALSE )
                {
                    #if( configUSE_PREEMPTION != 0 )
                    {
                        xAlreadyYielded = pdTRUE;
                    }
                    #endif
                    taskYIELD_IF_USING_PREEMPTION();
                }
                else
                {
                    mtCOVERAGE_TEST_MARKER();
                }
            }
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
    taskEXIT_CRITICAL();

    return xAlreadyYielded;
}
//}

xTaskResumeAll()を読み終わりました。

===[column] コラム：始めと後始末

xTaskResumeAll()は、xTaskSuspendAll()よりも随分コード量が多かったですね。
始めることよりも、その後始末の方がいつも大変な気がします。
日常生活においては、何かを始められる人は決断力や勇気があって称えられるべきと思いますが、
後始末する人も正当な評価がされるといいなと思います。

===[/column]


では、pvPortMalloc() の続きを見ていきます。

//listnum[pvPortMalloc_2][heap_3.c::pvPortMalloc(), 2/2]{
void *pvPortMalloc( size_t xWantedSize )
{
    ...

    #if( configUSE_MALLOC_FAILED_HOOK == 1 )
    {
        if( pvReturn == NULL )
        {
            extern void vApplicationMallocFailedHook( void );
            vApplicationMallocFailedHook();
        }
    }
    #endif

    return pvReturn;
}
//}

pvReturnがNULL、つまりmalloc()によるメモリ確保に失敗したときの処理です。
この本では基本的にはエラー処理や例外処理は読み飛ばしていきますが、ここではvApplicationMallocFailedHook()も読んでみましょう。

//listnum[vApplicationMallocFailedHook][variantHooks.c::vApplicationMallocFailedHook()]{
void vApplicationMallocFailedHook( void )
{
#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) @<embed>$|latex|\linebreak\hspace*{5ex}$|| defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) @<embed>$|latex|\linebreak\hspace*{5ex}$|| defined(__AVR_ATmega2561__) // Arduino Mega with 2560
...
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) @<embed>$|latex|\linebreak\hspace*{5ex}$|| defined(__AVR_ATmega8__) @<embed>$|latex|\linebreak\hspace*{5ex}$// assume we're using an Arduino Uno with 328p
    DDRB  |= _BV(DDB5);
    PORTB |= _BV(PORTB5);       // Main (red PB5) LED on. Main LED on.

...

#endif

    ...

    for(;;)
    {
        _delay_ms(50);

#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) @<embed>$|latex|\linebreak\hspace*{5ex}$|| defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) @<embed>$|latex|\linebreak\hspace*{5ex}$|| defined(__AVR_ATmega2561__)  // Mega with 2560
        PINB  |= _BV(PINB7);       // Main (red PB7) LED toggle. Main LED fast blink.

#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) @<embed>$|latex|\linebreak\hspace*{5ex}$|| defined(__AVR_ATmega8__) @<embed>$|latex|\linebreak\hspace*{5ex}$// assume we're using an Arduino Uno with 328p
        PINB  |= _BV(PINB5);       // Main (red PB5) LED toggle. Main LED fast blink.

...

#endif
    }
}
//}

Arduino Uno はATmega328Pを使用しているので、#if defined((__AVR_ATmega328P__) が真になります。
malloc()失敗時は、PORTB5のレジスタに直接書き込みを行い、50ミリ秒毎にArudino UnoのPB5ピンの出力をトグルさせて、接続されているLEDを点滅させます。
これで、pvPortMalloc() を見終わりました。

#@#############################################################################
== xTaskCreate() 後半部

xTaskCreate()の後半部を見ていきます。

//listnum[xTaskCreate_2][tasks.c::xTaskCreate(), 2/2]{
    BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                            const char * const pcName, @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e971 Unqualified char types are allowed for strings and single characters @<embed>$|latex|\linebreak\hspace*{5ex}$only. */
                            const configSTACK_DEPTH_TYPE usStackDepth,
                            void * const pvParameters,
                            UBaseType_t uxPriority,
                            TaskHandle_t * const pxCreatedTask )
    {
        ...

            if( pxStack != NULL )
            {
                /* Allocate space for the TCB. */
                pxNewTCB = ( TCB_t * ) pvPortMalloc( sizeof( TCB_t ) ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9087 !e9079 All values returned by pvPortMalloc() have at least the @<embed>$|latex|\linebreak\hspace*{5ex}$alignment required by the MCU's stack, and the first member of TCB_t is always a@<embed>$|latex|\linebreak\hspace*{5ex}$ pointer to the task's stack. */

                if( pxNewTCB != NULL )
                {
                    /* Store the stack location in the TCB. */
                    pxNewTCB->pxStack = pxStack;
                }
                else
                {
                    /* The stack cannot be used as the TCB was not created.  Free
                    it again. */
                    vPortFree( pxStack );
                }
            }
            else
            {
                pxNewTCB = NULL;
            }
        }

        ...
    }
//}

タスク(今回はスケジューラタスク)のためのタスク制御バッファ(TCB: Task Control Buffer)のためのメモリ領域を確保し、
その領域をタスク制御バッファのスタックとしてpxStackを割り当てています。
タスク制御バッファを作成できなかった場合は、確保したスタック領域も開放しています。

結局、xTaskCreate()は前半部でタスク制御バッファの領域を確保し、後半部でそのタスクが使用するスタック領域を確保しています。

xTaskCreate()も読み終わったので、最初に見ていた vTaskStartScheduler() に戻り続きを見ていきましょう。

//listnum[vTaskStartScheduler_2][tasks.c::vTaskStartScheduler(), 2/7]{
void vTaskStartScheduler( void )
{
    ...

    #if ( configUSE_TIMERS == 1 )
    {
        if( xReturn == pdPASS )
        {
            xReturn = xTimerCreateTimerTask();
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
    #endif /* configUSE_TIMERS */

    ...
}
//}

configUSE_TIMERSは1(FreeRTOSConfig.hで定義)なので、
タスク作成が成功していたら(xReturn == pdPASS)、xTimerCreateTimerTask() でタイマータスクを作ります。
タイマータスクとはなんでしょうか？
タイマータスク作成関数(xTimerCreateTimerTask())の前半部を読んで理解していきましょう。

//listnum[xTimerCreateTimerTask_1][timers.c::xTimerCreateTimerTask(), 1/2]{
BaseType_t xTimerCreateTimerTask( void )
{
BaseType_t xReturn = pdFAIL;

    /* This function is called when the scheduler is started if
    configUSE_TIMERS is set to 1.  Check that the infrastructure used by the
    timer service task has been created/initialised.  If timers have already
    been created then the initialisation will already have been performed. */
    prvCheckForValidListAndQueue();

    ...
//}

コメントを見ると以下のことが書かれています。
 * この関数はスケジューラが始まったときに呼ばれる関数である。
 * タイマーサービスタスクによって使用されるインフラを検査する
 * タイマーサービスタスクが生成済みならば、初期化もすでに実施されているはずである

タイマーサービスタスクによって使用されるインフラを検査する、という関数prvCheckForValidListAndQueue()の中身を見ていきましょう。

//listnum[prvCheckForValidListAndQueue_1][timers.c::prvCheckForValidListAndQueue(), 1/3]{
static void prvCheckForValidListAndQueue( void )
{
    /* Check that the list from which active timers are referenced, and the
    queue used to communicate with the timer service, have been
    initialised. */
    taskENTER_CRITICAL();
    {
        ...
    }
    taskEXIT_CRITICAL();
}
//}

コメントによると、アクティブなタイマーがこのリストで管理される、とあります。
タイマーは時間が切れる順に参照される、最も最初に切れるタイマーがリストの先頭にある

また、taskENTER_CRITICAL() と taskEXIT_CRITICAL() で囲まれたブロックがでてきました。
既に読んだこのブロック内では割り込みが禁止されています。
このブロック内の処理を見ていきましょう。

//listnum[prvCheckForValidListAndQueue_2][timer.c::prvCheckForValidListAndQueue, 2/3]{
static void prvCheckForValidListAndQueue( void )
{
    ...

    taskENTER_CRITICAL();
    {
        if( xTimerQueue == NULL )
        {
            vListInitialise( &xActiveTimerList1 );
            vListInitialise( &xActiveTimerList2 );
            pxCurrentTimerList = &xActiveTimerList1;
            pxOverflowTimerList = &xActiveTimerList2;
//}

xActiveTimerList1、xActiveTimerList2という2つのリストを初期化しています。
pxCurrentTimerList, pxOverflowTimerList はそれぞれtimer.cで宣言されている変数で、
現在のタイマーリストとオーバーフローしたタイマーリストの２つを作成しています。
現在のタイマーリストはともかく、オーバーフローしたタイマーリストとはなんでしょう。

XXX

prvCheckForValidListAndQueue()に戻ります。

//listnum[prvCheckForValidListAndQueue_3][timer.c::prvCheckForValidListAndQueue, 3/3]{
static void prvCheckForValidListAndQueue( void )
{
    ...

    taskENTER_CRITICAL();
    {
        ...

            pxOverflowTimerList = &xActiveTimerList2;

            #if( configSUPPORT_STATIC_ALLOCATION == 1 )
            {
                ...
            }
            #else
            {
                xTimerQueue = xQueueCreate( @<embed>$|latex|\linebreak\hspace*{5ex}$( UBaseType_t ) configTIMER_QUEUE_LENGTH, sizeof( DaemonTaskMessage_t ) );
            }
            #endif
        ...
    }
    taskEXIT_CRITICAL();
}
//}

configSUPPORT_STATIC_ALLOCATIONは0なので(FreeRTOSConfig.hで定義されている)、#else節だけ見ていきます。
タイマー用のキューxTimerQueueを作成しています。

== キューの作成

xQueueCreate() は xQueueGenericCreate() で別名定義されています。
xQueueGenericCreate() を見ていきましょう。
uxQueueLength, uxItemSize の値は引き継がれますが、ucQueueType は queueQUEUE_TYPE_BASE が渡されます。

//listnum[xQueueGenericCreate:1/3][queue.c]{
    QueueHandle_t xQueueGenericCreate( @<embed>$|latex|\linebreak\hspace*{5ex}$const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, @<embed>$|latex|\linebreak\hspace*{5ex}$const uint8_t ucQueueType )
    {
    Queue_t *pxNewQueue;
    size_t xQueueSizeInBytes;
    uint8_t *pucQueueStorage;

        configASSERT( uxQueueLength > ( UBaseType_t ) 0 );

        if( uxItemSize == ( UBaseType_t ) 0 )
        {
            /* There is not going to be a queue storage area. */
            xQueueSizeInBytes = ( size_t ) 0;
        }
        else
        {
            /* Allocate enough space to hold the maximum number of items that
            can be in the queue at any time. */
            xQueueSizeInBytes = ( size_t ) ( uxQueueLength * uxItemSize ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e961 MISRA exception as the casts are only redundant for some ports. */
        }

//}

 * タイマーのためのキューを作成している。サイズは10。listとの使い分けは？

//listnum[xQueueGenericCreate:2/3][queue.c]{
        /* Allocate the queue and storage area.  Justification for MISRA
        deviation as follows:  pvPortMalloc() always ensures returned memory
        blocks are aligned per the requirements of the MCU stack.  In this case
        pvPortMalloc() must return a pointer that is guaranteed to meet the
        alignment requirements of the Queue_t structure - which in this case
        is an int8_t *.  Therefore, whenever the stack alignment requirements
        are greater than or equal to the pointer to char requirements the cast
        is safe.  In other cases alignment requirements are not strict (one or
        two bytes). */
        pxNewQueue = ( Queue_t * ) pvPortMalloc( @<embed>$|latex|\linebreak\hspace*{5ex}$sizeof( Queue_t ) + xQueueSizeInBytes ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9087 !e9079 see comment above. */
//}

リンターのメッセージe9087、 e9079に対する説明がコメントとして記載されています。

//quote{
pvPortMalloc()はメモリーブロックがMCUスタックが要求するアライメントされているかを常に確認する必要がある。
この場合は、pvPortMalloc()はQueue_t構造体のアライメントを満たすことが保証されている。ここではint8_t*で定義されている。avrのasmなのであとで確認する。
それゆえ、スタックアライメント要求がcharへのポインターと同じか大きい限り、キャストは安全である。
//}
ということで、( Queue_t * )へのキャストは安全であることを説明している。
Queue_t構造体のアライメントがint16_t*やint32_t*の移植では安全ではない。

xQueueGenericCreate()の続きを見ていこう。

//listnum[xQueueGenericCreate:3/3][queue.c]{
        if( pxNewQueue != NULL )
        {
            /* Jump past the queue structure to find the location of the queue
            storage area. */
            pucQueueStorage = ( uint8_t * ) pxNewQueue;
            pucQueueStorage += sizeof( Queue_t ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9016 Pointer arithmetic allowed on char types, especially when it assists @<embed>$|latex|\linebreak\hspace*{5ex}$conveying intent. */

            #if( configSUPPORT_STATIC_ALLOCATION == 1 )
            {
                /* Queues can be created either statically or dynamically, so
                note this task was created dynamically in case it is later
                deleted. */
                pxNewQueue->ucStaticallyAllocated = pdFALSE;
            }
            #endif /* configSUPPORT_STATIC_ALLOCATION */

            prvInitialiseNewQueue(@<embed>$|latex|\linebreak\hspace*{5ex}$ uxQueueLength, uxItemSize, pucQueueStorage, ucQueueType, pxNewQueue );
        }
        else
        {
            traceQUEUE_CREATE_FAILED( ucQueueType );
            mtCOVERAGE_TEST_MARKER();
        }

        return pxNewQueue;
    }
//}

Queueへのメモリ割り当てが成功していたら(pxNewQueue != NULL)、
pucQueueStorage(Pointer to Unsigned Char)が確保したキューを指すようにして、ポインタをQueue_t分進めています。
図があるとよい

新規作成したキューの初期化処理(prvInitialiseNewQueue())も見ていきましょう。

//listnum[prvInitialiseNewQueue][queue.c::prvInitialiseNewQueue(), 1/2]{
static void prvInitialiseNewQueue( @<embed>$|latex|\linebreak\hspace*{5ex}$const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, @<embed>$|latex|\linebreak\hspace*{5ex}$uint8_t *pucQueueStorage, const uint8_t ucQueueType, Queue_t *pxNewQueue )
{
    /* Remove compiler warnings about unused parameters should
    configUSE_TRACE_FACILITY not be set to 1. */
    ( void ) ucQueueType;

    if( uxItemSize == ( UBaseType_t ) 0 )
    {
        /* No RAM was allocated for the queue storage area, but PC head cannot
        be set to NULL because NULL is used as a key to say the queue is used as
        a mutex.  Therefore just set pcHead to point to the queue as a benign
        value that is known to be within the memory map. */
        pxNewQueue->pcHead = ( int8_t * ) pxNewQueue;
    }
    else
    {
        /* Set the head to the start of the queue storage area. */
        pxNewQueue->pcHead = ( int8_t * ) pucQueueStorage;
    }

    /* Initialise the queue members as described where the queue type is
    defined. */
    pxNewQueue->uxLength = uxQueueLength;
    pxNewQueue->uxItemSize = uxItemSize;
    ( void ) xQueueGenericReset( pxNewQueue, pdTRUE );

    ...
}
//}

キューのリセット関数xQueueGenericReset()も読んでいきましょう。

//listnum[xQueueGenericReset][queue.c]{
BaseType_t xQueueGenericReset( QueueHandle_t xQueue, BaseType_t xNewQueue )
{
Queue_t * const pxQueue = xQueue;

    configASSERT( pxQueue );

    taskENTER_CRITICAL();
    {
        pxQueue->u.xQueue.pcTail = pxQueue->pcHead @<embed>$|latex|\linebreak\hspace*{5ex}$+ ( pxQueue->uxLength * pxQueue->uxItemSize ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9016 Pointer arithmetic allowed on char types, especially when it assists @<embed>$|latex|\linebreak\hspace*{5ex}$conveying intent. */
        pxQueue->uxMessagesWaiting = ( UBaseType_t ) 0U;
        pxQueue->pcWriteTo = pxQueue->pcHead;
        pxQueue->u.xQueue.pcReadFrom = @<embed>$|latex|\linebreak\hspace*{5ex}$pxQueue->pcHead + ( ( pxQueue->uxLength - 1U ) * pxQueue->uxItemSize ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9016 Pointer arithmetic allowed on char types, especially when it assists @<embed>$|latex|\linebreak\hspace*{5ex}$conveying intent. */
        pxQueue->cRxLock = queueUNLOCKED;
        pxQueue->cTxLock = queueUNLOCKED;

        if( xNewQueue == pdFALSE )
        {
            ...
        }
        else
        {
            /* Ensure the event queues start in the correct state. */
            vListInitialise( &( pxQueue->xTasksWaitingToSend ) );
            vListInitialise( &( pxQueue->xTasksWaitingToReceive ) );
        }
    }
    taskEXIT_CRITICAL();

    /* A value is returned for calling semantic consistency with previous
    versions. */
    return pdPASS;
}
//}

また、taskENTER_CRITICAL()、taskEXIT_CRITICAL() で囲まれたブロックがでてきました。
このブロック内は割り込み禁止です。
キューが保持するメッセージをゼロにし、次にキューが使うストレージ領域はキューのストレージ領域の先頭に指定し、
キューのストレージ領域の最後の位置は、キューのストレージ領域の先頭からキューの保持できる要素数×要素毎のサイズだけずらした位置にしている。

次に vListInitialise() でキューの送信待ちタスクリスト、受信待ちタスクリストの初期化を行う。

 listの最後pxIndexはそのリストの最後xListEndを割り当てる
 リストの最後xListEndの値は、割り当て可能な最大値portMAX_DELAY(0xffff)を設定する。リストの終端であることを示すために設定しているらしい。
 リストの終端の1つ前と1つ後がリスト終端自身を指すことで、リストがカラであることが分かる。
 リストの要素数もゼロにする。
 listSET_LIST_INTEGRITY_CHECK_1_VALUE(), listSET_LIST_INTEGRITY_CHECK_2_VALUE() マクロは何もしない。

//listnum[prvInitialiseNewQueue_2][queue.c::prvInitialiseNewQueue(), 2/2]{
static void prvInitialiseNewQueue( @<embed>$|latex|\linebreak\hspace*{5ex}$const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, @<embed>$|latex|\linebreak\hspace*{5ex}$uint8_t *pucQueueStorage, const uint8_t ucQueueType, Queue_t *pxNewQueue )
{
    ...

    #if ( configUSE_TRACE_FACILITY == 1 )
    {
        pxNewQueue->ucQueueType = ucQueueType;
    }
    #endif /* configUSE_TRACE_FACILITY */

    #if( configUSE_QUEUE_SETS == 1 )
    {
        pxNewQueue->pxQueueSetContainer = NULL;
    }
    #endif /* configUSE_QUEUE_SETS */

    traceQUEUE_CREATE( pxNewQueue );
}
//}

これらは何もしません。


== xTimerCreateTimerTask())の後半部

大分遠回りしましたが、 次はタイマータスク作成の関数(xTimerCreateTimerTask())の後半部です。
ようやくタイマータスクを作成します。

//listnum[xTimerCreateTimerTask_2][timers.c:: BaseType_t xTimerCreateTimerTask( void ), 2/2]{
BaseType_t xTimerCreateTimerTask( void )
{
    ...

        #else
        {
            xReturn = xTaskCreate(    prvTimerTask,
                                    configTIMER_SERVICE_TASK_NAME,
                                    configTIMER_TASK_STACK_DEPTH,
                                    NULL,
                                    ( ( UBaseType_t ) configTIMER_TASK_PRIORITY ) | @<embed>$|latex|\linebreak\hspace*{5ex}$portPRIVILEGE_BIT,
                                    &xTimerTaskHandle );
        }
        #endif /* configSUPPORT_STATIC_ALLOCATION */
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }

    configASSERT( xReturn );
    return xReturn;
}
//}

xTaskCreate()でスケジューラタスクのタイマーのタスクを作ります。
xTaskCreate()は説明済みなので、引数だけ確認します。XXX


#@# vTaskStartScheduler ##########################################################

== vTaskStartScheduler() との再会

やっとvTaskStartScheduler()に戻ってきました。

//listnum[vTaskStartScheduler_3][tasks.c::vTaskStartScheduler(), 3/7]{
void vTaskStartScheduler( void )
{
    ...

    if( xReturn == pdPASS )
    {
        /* freertos_tasks_c_additions_init() should only be called if the user
        definable macro FREERTOS_TASKS_C_ADDITIONS_INIT() is defined, as that is
        the only macro called by the function. */
        #ifdef FREERTOS_TASKS_C_ADDITIONS_INIT
        {
            ...
        }
        #endif

        /* Interrupts are turned off here, to ensure a tick does not occur
        before or during the call to xPortStartScheduler().  The stacks of
        the created tasks contain a status word with interrupts switched on
        so interrupts will automatically get re-enabled when the first task
        starts to run. */
        portDISABLE_INTERRUPTS();

        #if ( configUSE_NEWLIB_REENTRANT == 1 )
        {
            ...
        }
        #endif /* configUSE_NEWLIB_REENTRANT */

    ...
}
//}

 割り込み禁止して、xPortStartScheduler()を呼ぶ前と呼んでいる間に割り込みは発生しないようにします。
 生成されたタスクは割り込みスイッチの状態を含むので、最初のタスクが開始されたら自動的に再度割り込み可能になる。
 * tickとは？
 1tickはデフォルトは15ms(16?), 128kHz動作のWDTの2048カウント分。
1000 / (128000 >> (0+11)) = 16ms

//listnum[configTICK_RATE_HZ][FreeRTOSVariant.h]{
// System Tick - Scheduler timer
// Use the Watchdog timer, and choose the rate at which scheduler interrupts will occur.

#define portUSE_WDTO            WDTO_15MS    // portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick

/* Watchdog period options:     WDTO_15MS
                                WDTO_30MS
                                WDTO_60MS
                                WDTO_120MS
                                WDTO_250MS
                                WDTO_500MS
*/
//    xxx Watchdog Timer is 128kHz nominal, but 120 kHz at 5V DC and 25 degrees is actually more accurate, from data sheet.
#define configTICK_RATE_HZ      ( (TickType_t)( (uint32_t)128000 >> (portUSE_WDTO + 11) ) )  // 2^11 = 2048 WDT scaler for 128kHz Timer
//}

//listnum[pdMS_TO_TICKS][projdef.h]{
#ifndef pdMS_TO_TICKS
    #define pdMS_TO_TICKS( xTimeInMs ) ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeInMs ) * ( TickType_t ) configTICK_RATE_HZ ) / ( TickType_t ) 1000 ) )
#endif
//}

portDISABLE_INTERRUPTS() は portmacro.h で定義されている。

//listnum[portDISABLE_INTERRUPT][portmacro.h]{
#define portDISABLE_INTERRUPTS()        __asm__ __volatile__ ( "cli" ::: "memory")
//}
 * cli: 割り込み禁止
 * xPortStartScheduler()を呼ぶ前、あるいは呼んでいる途中でCPUクロックが進まないように？割り込みが起きないように?
 * 最初のタスクが実行されると自動的に割り込みは有効になる。

//listnum[vTaskStartScheduler_4][taskc.c::void vTaskStartScheduler( void ), 4/7]{
void vTaskStartScheduler( void )
{
    ...

        #if ( configUSE_NEWLIB_REENTRANT == 1 )
        {
            ...
        }

        #endif /* configUSE_NEWLIB_REENTRANT */
        xNextTaskUnblockTime = portMAX_DELAY;
        xSchedulerRunning = pdTRUE;
        xTickCount = ( TickType_t ) configINITIAL_TICK_COUNT;

    ...
}
//}

 * 右辺の値はそれぞれportmacro.h, projdefs.h, Arduino_FreeRTOS.h に記述されている。

//listnum[vTaskStartScheduler_5][taskc.c::void vTaskStartScheduler( void ), 5/7]{
void vTaskStartScheduler( void )
{
    ...

        /* If configGENERATE_RUN_TIME_STATS is defined then the following
        macro must be defined to configure the timer/counter used to generate
        the run time counter time base.   NOTE:  If configGENERATE_RUN_TIME_STATS
        is set to 0 and the following line fails to build then ensure you do not
        have portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() defined in your
        FreeRTOSConfig.h file. */
        portCONFIGURE_TIMER_FOR_RUN_TIME_STATS();

        traceTASK_SWITCHED_IN();

    ...
}
//}

FreeRTOSConfig.h で configGENERATE_RUN_TIME_STATS が定義されていないので、 portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() は何もしない
同様にtraceTASK_SWITCHED_IN()も何もしない。

vTaskStartScheduler()の最後までたどり着きました。

//listnum[vTaskStartScheduler_6][tasks.c::void vTaskStartScheduler( void ), 6/7]{
void vTaskStartScheduler( void )
{
    ...

        /* Setting up the timer tick is hardware specific and thus in the
        portable interface. */
        if( xPortStartScheduler() != pdFALSE )
        {
            /* Should not reach here as if the scheduler is running the
            function will not return. */
        }
        else
        {
            /* Should only reach here if a task calls xTaskEndScheduler(). */
        }

    ...
}
//}

スケジューラを開始します(xPortStartScheduler())。
ハードウェア固有の処理です。

//listnum[xPortStartScheduler_1][port.c::xPortStartScheduler(void), 1/3]{
BaseType_t xPortStartScheduler( void )
{
    /* Setup the relevant timer hardware to generate the tick. */
    prvSetupTimerInterrupt();

    /* Restore the context of the first task that is going to run. */
    portRESTORE_CONTEXT();

    /* Simulate a function call end as generated by the compiler.  We will now
    jump to the start of the task the context of which we have just restored. */
    __asm__ __volatile__ ( "ret" );

    /* Should not get here. */
    return pdTRUE;
}
//}

//listnum[prvSetupTimerInterrupt][port.c::void prvSetupTimerInterrupt( void )]{
//initialize watchdog
void prvSetupTimerInterrupt( void )
{
    //reset watchdog
    wdt_reset();

    //set up WDT Interrupt (rather than the WDT Reset).
    wdt_interrupt_enable( portUSE_WDTO );
}
//}

//listnum[wdt_reset][wdt.h::wdt_reset()]{
#define wdt_reset() __asm__ __volatile__ ("wdr")
//}

ウォッチドッグタイマーはカウントし続けるカウンタで一定期間ごとにリセットする必要があります。
逆に言うと一定期間ごとにリセットされない場合はMPUが暴走している可能性があります。

ウォッチドッグタイマーをリセットします。
wdr はavrのアセンブラコードでウォッチドッグタイマーをリセットします。

//listnum[wdt_interrupt_enable][portable.h::void wdt_interrupt_enable (const uint8_t value)]{
static __inline__
__attribute__ ((__always_inline__))
void wdt_interrupt_enable (const uint8_t value)
{
    if (_SFR_IO_REG_P (_WD_CONTROL_REG))
    {
        __asm__ __volatile__ (
                "in __tmp_reg__,__SREG__"   "\n\t"
                "cli"                       "\n\t"
                "wdr"                       "\n\t"
                "out %0, %1"                "\n\t"
                "out __SREG__,__tmp_reg__"  "\n\t"
                "out %0, %2"                "\n\t"
                : /* no outputs */
                : "I" (_SFR_IO_ADDR(_WD_CONTROL_REG)),
                "r" ((uint8_t)(_BV(_WD_CHANGE_BIT) | _BV(WDE))),
                "r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) |
                        _BV(WDIF) | _BV(WDIE) | (value & 0x07)) )
                : "r0"
        );
    }
    else
    {
        __asm__ __volatile__ (
                "in __tmp_reg__,__SREG__"   "\n\t"
                "cli"                       "\n\t"
                "wdr"                       "\n\t"
                "sts %0, %1"                "\n\t"
                "out __SREG__,__tmp_reg__"  "\n\t"
                "sts %0, %2"                "\n\t"
                : /* no outputs */
                : "n" (_SFR_MEM_ADDR(_WD_CONTROL_REG)),
                "r" ((uint8_t)(_BV(_WD_CHANGE_BIT) | _BV(WDE))),
                "r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) |
                        _BV(WDIF) | _BV(WDIE) | (value & 0x07)) )
                : "r0"
        );
    }
}
//}

ウォッチドッグタイマーによる割り込みを有効にします。

//listnum[xPortStartScheduler_2][port.c::xPortStartScheduler(void), 2/3]{
BaseType_t xPortStartScheduler( void )
{
    ...

    /* Restore the context of the first task that is going to run. */
    portRESTORE_CONTEXT();

    ...
}
//}
コンテキストをリストアします。
ここでのコンテキストはレジスタセットとTCBのポインタ。？

//listnum[portRESTORE_CONTEXT][port.c]{
#define portRESTORE_CONTEXT()                                                       \
        __asm__ __volatile__ (  "lds    r26, pxCurrentTCB                   \n\t"   \
                                "lds    r27, pxCurrentTCB + 1               \n\t"   \
                                "ld     r28, x+                             \n\t"   \
                                "out    __SP_L__, r28                       \n\t"   \
                                "ld     r29, x+                             \n\t"   \
                                "out    __SP_H__, r29                       \n\t"   \
                                "pop    r31                                 \n\t"   \
                                "pop    r30                                 \n\t"   \
                                ...
                                "pop    r3                                  \n\t"   \
                                "pop    r2                                  \n\t"   \
                                "pop    __zero_reg__                        \n\t"   \
                                "pop    __tmp_reg__                         \n\t"   \
                                "out    __SREG__, __tmp_reg__               \n\t"   \
                                "pop    __tmp_reg__                         \n\t"   \
                             );
#endif
//}
 * lds: LDS – Load Direct from Data Space
 * pxCurrentTCBのアドレスをコピー
 * __SP_H__	Stack pointer high byte at address 0x3E
 * __SP_L__	Stack pointer low byte at address 0x3D
 * stackポインタが16bitある？のでHigh,Lowに分けて書き戻す
 * r2～r31の汎用レジスタを書き戻す(pop)
 * __tmp_reg__(r0)(実際はステータスレジスタの値), __zero_reg__(r1)を書き戻す
 * __tmp_reg__に保持していたステータスレジスタの値をステータスレジスタに書き戻す
 * __tmp_reg__(r0)を書き戻す

参考まで portSAVE_CONTEXT() も見ておく。

//listnum[portSAVE_CONTEXT][port.c]{
#define portSAVE_CONTEXT()                                                          \
        __asm__ __volatile__ (  "push   __tmp_reg__                         \n\t"   \
                                "in     __tmp_reg__, __SREG__               \n\t"   \
                                "cli                                        \n\t"   \
                                "push   __tmp_reg__                         \n\t"   \
                                "push   __zero_reg__                        \n\t"   \
                                "clr    __zero_reg__                        \n\t"   \
                                "push   r2                                  \n\t"   \
                                "push   r3                                  \n\t"   \

                                ...

                                "push   r30                                 \n\t"   \
                                "push   r31                                 \n\t"   \
                                "lds    r26, pxCurrentTCB                   \n\t"   \
                                "lds    r27, pxCurrentTCB + 1               \n\t"   \
                                "in     __tmp_reg__, __SP_L__               \n\t"   \
                                "st     x+, __tmp_reg__                     \n\t"   \
                                "in     __tmp_reg__, __SP_H__               \n\t"   \
                                "st     x+, __tmp_reg__                     \n\t"   \
                             );
#endif
//}

 * __tmp_reg__(r0) を退避
 * 割り込み禁止
 * __tmp_reg__にコピーしたステータスレジスタ, __zero_reg__を退避
 * __zero_reg__をゼロクリア
 * r2～r31の汎用レジスタを退避
 * lds: LDS – Load Direct from Data Space
 * pxCurrentTCBのアドレスをコピー
 * __SP_H__	Stack pointer high byte at address 0x3E
 * __SP_L__	Stack pointer low byte at address 0x3D
 * stackポインタが16bitある？のでHigh,Lowに分けて退避

xPortStartScheduler() の続きを見ていきます。

//listnum[xPortStartScheduler_3][port.c::xPortStartScheduler(void), 3/3]{
BaseType_t xPortStartScheduler( void )
{
    ...

    /* Simulate a function call end as generated by the compiler.  We will now
    jump to the start of the task the context of which we have just restored. */
    __asm__ __volatile__ ( "ret" );

    /* Should not get here. */
    return pdTRUE;
}
//}
 * __asm__ __volatile__ ( "ret" );
 ** returnじゃダメなのはなんで??? __asm__ __volatile__ ( "ret" );

xPortStartScheduler()が読み終わったので、vTaskStartScheduler()に戻ってきました。

//listnum[vTaskStartScheduler_7][tasks.c::void vTaskStartScheduler( void ), 7/7]{
void vTaskStartScheduler( void )
{
    ...

    /* Prevent compiler warnings if INCLUDE_xTaskGetIdleTaskHandle is set to 0,
    meaning xIdleTaskHandle is not used anywhere else. */
    ( void ) xIdleTaskHandle;
}
//}

 * `INCLUDE_xTaskGetIdleTaskHandle` が`0`に設定されている時でも、コンパイラがワーニング(宣言されたが一度も使用されていない)を出さないように記載されている。
 * FreeRTOSの思想として、いろんなCPUに移植できるようにしているが、ノーエラー、ノーワーニングで提供する（要確認）

ここまでで初期化完了

#@# ここから .ino ファイルに戻る #########################################################

== 終わりそうで終わらない

example/Blink_AnalogRead.ino を改めてみてみる。ただし、コメントは適宜削除している。

//listnum[Blink_AnalogRead_2][example/Blink_AnalogRead.ino]{
#include <Arduino_FreeRTOS.h>

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void TaskAnalogRead( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);

  while (!Serial) {
    ;
  }

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink
    ,  (const portCHAR *)"Blink"   // A name just for humans
    ,  128  @<embed>$|latex|\linebreak\hspace*{5ex}$// This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  @<embed>$|latex|\linebreak\hspace*{5ex}$// Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the @<embed>$|latex|\linebreak\hspace*{5ex}$lowest.
    ,  NULL );

  xTaskCreate(
    TaskAnalogRead
    ,  (const portCHAR *) "AnalogRead"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual @<embed>$|latex|\linebreak\hspace*{5ex}$tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void TaskAnalogRead(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;)
  {
    int sensorValue = analogRead(A0);
    Serial.println(sensorValue);
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}
//}

まだコードを見ていないvTaskDelay()があるのでこれを読んでいきます。

== vTaskDelay() ちょっと待ちます

//listnum[vTaskDelay_1][tasks.c::vTaskDelay(), 1/2]{
    void vTaskDelay( const TickType_t xTicksToDelay )
    {
    BaseType_t xAlreadyYielded = pdFALSE;

        /* A delay time of zero just forces a reschedule. */
        if( xTicksToDelay > ( TickType_t ) 0U )
        {
            configASSERT( uxSchedulerSuspended == 0 );
            vTaskSuspendAll();
            {
                traceTASK_DELAY();

                /* A task that is removed from the event list while the
                scheduler is suspended will not get placed in the ready
                list or removed from the blocked list until the scheduler
                is resumed.

                This task cannot be in an event list as it is the currently
                executing task. */
                prvAddCurrentTaskToDelayedList( xTicksToDelay, pdFALSE );
            }
            xAlreadyYielded = xTaskResumeAll();
        }
        else
        {
            ...
        }

        ...
    }
//}

vTaskSuspendAll() と xTaskResumeAll() で囲まれたブロックで実行されるので、
ほかのタスクはRun状態にはなりません。

prvAddCurrentTaskToDelayedList() を見ていく。
現在のタスクを遅延タスクリストに追加します。

//listnum[prvAddCurrentTaskToDelayedList_1][tasks.c::prvAddCurrentTaskToDelayedList(), 1/2]{
static void prvAddCurrentTaskToDelayedList( @<embed>$|latex|\linebreak\hspace*{5ex}$TickType_t xTicksToWait, const BaseType_t xCanBlockIndefinitely )
{
TickType_t xTimeToWake;
const TickType_t xConstTickCount = xTickCount;

    #if( INCLUDE_xTaskAbortDelay == 1 )
    {
        ...
    }
    #endif

    /* Remove the task from the ready list before adding it to the blocked list
    as the same list item is used for both lists. */
    if( uxListRemove( &( pxCurrentTCB->xStateListItem ) ) == ( UBaseType_t ) 0 )
    {
        /* The current task must be in a ready list, so there is no need to
        check, and the port reset macro can be called directly. */
        portRESET_READY_PRIORITY( pxCurrentTCB->uxPriority, uxTopReadyPriority ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e931 pxCurrentTCB cannot change as it is the calling task.  @<embed>$|latex|\linebreak\hspace*{5ex}$pxCurrentTCB->uxPriority and uxTopReadyPriority cannot change as called with @<embed>$|latex|\linebreak\hspace*{5ex}$scheduler suspended or in a critical section. */
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }

    ...
}
//}

同じリストアイテムが両方のリストで使用されるので、
ブロックリストに追加する前に、準備リストからタスクを削除する。
現在のタスクは準備リストにいる必要があるので、チェックする必要はない。また、ポートリセットマクロが直接呼ばれる。

//listnum[portRESET_READY_PRIORITY][tasks.c::portRESET_READY_PRIORITY]{
    /* Define away taskRESET_READY_PRIORITY() and portRESET_READY_PRIORITY() as
    they are only required when a port optimised method of task selection is
    being used. */
    #define taskRESET_READY_PRIORITY( uxPriority )
    #define portRESET_READY_PRIORITY( uxPriority, uxTopReadyPriority )
//}
portRESET_READY_PRIORITY()は、なにも実行しない。

//listnum[prvAddCurrentTaskToDelayedList_2][tasks.c::prvAddCurrentTaskToDelayedList(), 2/2]{
static void prvAddCurrentTaskToDelayedList( @<embed>$|latex|\linebreak\hspace*{5ex}$TickType_t xTicksToWait, const BaseType_t xCanBlockIndefinitely )
{
    ...

    #if ( INCLUDE_vTaskSuspend == 1 )
    {
        if( ( xTicksToWait == portMAX_DELAY ) @<embed>$|latex|\linebreak\hspace*{5ex}$&& ( xCanBlockIndefinitely != pdFALSE ) )
        {
            /* Add the task to the suspended task list instead of a delayed task
            list to ensure it is not woken by a timing event.  It will block
            indefinitely. */
            vListInsertEnd( &xSuspendedTaskList, &( pxCurrentTCB->xStateListItem ) );
        }
    ...
}
//}
INCLUDE_vTaskSuspendは1と定義されている(FreeRTOSConfig.h)。
xTicksToWaitが最大値で、xCanBlockIndefinitelyがTrueなら、遅延タスクリストの代わりに、サスペンドタスクリストに入れて、
タイミングイベントで起こされるまでブロックされる。

サスペンドタスクリストに現在のタスクを追加します。

//listnum[vListInsertEnd][list.c::vListInsertEnd()]{
void vListInsertEnd( List_t * const pxList, ListItem_t * const pxNewListItem )
{
ListItem_t * const pxIndex = pxList->pxIndex;

    /* Only effective when configASSERT() is also defined, these tests may catch
    the list data structures being overwritten in memory.  They will not catch
    data errors caused by incorrect configuration or use of FreeRTOS. */
    listTEST_LIST_INTEGRITY( pxList );
    listTEST_LIST_ITEM_INTEGRITY( pxNewListItem );

    /* Insert a new list item into pxList, but rather than sort the list,
    makes the new list item the last item to be removed by a call to
    listGET_OWNER_OF_NEXT_ENTRY(). */
    pxNewListItem->pxNext = pxIndex;
    pxNewListItem->pxPrevious = pxIndex->pxPrevious;

    /* Only used during decision coverage testing. */
    mtCOVERAGE_TEST_DELAY();

    pxIndex->pxPrevious->pxNext = pxNewListItem;
    pxIndex->pxPrevious = pxNewListItem;

    /* Remember which list the item is in. */
    pxNewListItem->pxContainer = pxList;

    ( pxList->uxNumberOfItems )++;
}
//}

//listnum[listTEST_LIST_ITEM_INTEGRITY][list.h::listTEST_LIST_ITEM_INTEGRITY()]{
#if( configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES == 0 )
    /* Define the macros to do nothing. */
    ...
    #define listTEST_LIST_ITEM_INTEGRITY( pxItem )
    #define listTEST_LIST_INTEGRITY( pxList )
//}

pxList がサスペンドタスクリスト、pxNewListItemが追加するタスクです。
listTEST_LIST_INTEGRITY()、listTEST_LIST_ITEM_INTEGRITY()は何もしません。
pxIndex は pxList の最後の要素を指します。XXX
つまり最後の要素にpxNewListItemを挿入しています。
pxNewListItem->pxContainer = pxList はこの要素がどのリストに含まれるかを記憶しておきます。

//listnum[prvAddCurrentTaskToDelayedList_3][tasks.c::prvAddCurrentTaskToDelayedList(), 3/4]{
static void prvAddCurrentTaskToDelayedList( @<embed>$|latex|\linebreak\hspace*{5ex}$TickType_t xTicksToWait, const BaseType_t xCanBlockIndefinitely )
{
    ...

            vListInsertEnd( &xSuspendedTaskList, &( pxCurrentTCB->xStateListItem ) );
        }
        else
        {
            /* Calculate the time at which the task should be woken if the event
            does not occur.  This may overflow but this doesn't matter, the
            kernel will manage it correctly. */
            xTimeToWake = xConstTickCount + xTicksToWait;

            /* The list item will be inserted in wake time order. */
            listSET_LIST_ITEM_VALUE( @<embed>$|latex|\linebreak\hspace*{5ex}$&( pxCurrentTCB->xStateListItem ), xTimeToWake );

            ...
//}

xTimeToWakeにイベントが発生しなかったときに、タスクが起こされる時間を設定する。
listSET_LIST_ITEM_VALUE() でリストの値として、タスクが起こされる時間を設定する。

//listnum[prvAddCurrentTaskToDelayedList_4][tasks.c::prvAddCurrentTaskToDelayedList(), 4/4]{
static void prvAddCurrentTaskToDelayedList( @<embed>$|latex|\linebreak\hspace*{5ex}$TickType_t xTicksToWait, const BaseType_t xCanBlockIndefinitely )
{
    ...

            if( xTimeToWake < xConstTickCount )
            {
                /* Wake time has overflowed.  Place this item in the overflow
                list. */
                vListInsert( pxOverflowDelayedTaskList, @<embed>$|latex|\linebreak\hspace*{5ex}$&( pxCurrentTCB->xStateListItem ) );
            }
            else
            {
                /* The wake time has not overflowed, so the current block list
                is used. */
                vListInsert( pxDelayedTaskList, &( pxCurrentTCB->xStateListItem ) );

                /* If the task entering the blocked state was placed at the
                head of the list of blocked tasks then xNextTaskUnblockTime
                needs to be updated too. */
                if( xTimeToWake < xNextTaskUnblockTime )
                {
                    xNextTaskUnblockTime = xTimeToWake;
                }
                else
                {
                    mtCOVERAGE_TEST_MARKER();
                }
            }
        }
    }
    #else /* INCLUDE_vTaskSuspend */
    {
        ...
    }
    #endif /* INCLUDE_vTaskSuspend */
}
//}


//listnum[vTaskDelay_2][tasks.c::vTaskDelay(), 1/2]{
    void vTaskDelay( const TickType_t xTicksToDelay )
    {
        ...

        /* Force a reschedule if xTaskResumeAll has not already done so, we may
        have put ourselves to sleep. */
        if( xAlreadyYielded == pdFALSE )
        {
            portYIELD_WITHIN_API();
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
//}

xTaskResumeAll() がまだ実施されていなければ、強制リスケジュールして、このタスク自身スリープさせる。

//listnum[vPortYield][port.c::vPortYield()]{
void vPortYield( void ) __attribute__ ( ( hot, flatten, naked ) );
void vPortYield( void )
{
    portSAVE_CONTEXT();
    vTaskSwitchContext();
    portRESTORE_CONTEXT();

    __asm__ __volatile__ ( "ret" );
}
//}

portSAVE_CONTEXT()とportRESTORE_CONTEXT()は既に読んでいる。

//listnum[vTaskSwitchContext][task.c::vTaskSwitchContext()]{
void vTaskSwitchContext( void )
{
    if( uxSchedulerSuspended != ( UBaseType_t ) pdFALSE )
    {
        /* The scheduler is currently suspended - do not allow a context
        switch. */
        xYieldPending = pdTRUE;
    }
    else
    {
        xYieldPending = pdFALSE;
        traceTASK_SWITCHED_OUT();

        #if ( configGENERATE_RUN_TIME_STATS == 1 )
        {

            ...

        }
        #endif /* configGENERATE_RUN_TIME_STATS */

        /* Check for stack overflow, if configured. */
        taskCHECK_FOR_STACK_OVERFLOW();

//}
traceTASK_SWITCHED_OUT()は何もしない。

//listnum[taskCHECK_FOR_STACK_OVERFLOW][stack_macros.h::taskCHECK_FOR_STACK_OVERFLOW()]{
    /* Only the current stack state is to be checked. */
    #define taskCHECK_FOR_STACK_OVERFLOW()                                                              \
    {                                                                                                   \
        /* Is the currently saved stack pointer within the stack limit? */                              \
        if( pxCurrentTCB->pxTopOfStack <= pxCurrentTCB->pxStack )                                       \
        {                                                                                               \
            vApplicationStackOverflowHook( @<embed>$|latex|\linebreak\hspace*{5ex}$( TaskHandle_t ) pxCurrentTCB, pxCurrentTCB->pcTaskName );   \
        }                                                                                               \
    }
//}

xYieldPending はコンテキストスイッチが要求されたときにラッチされる(pdTrue)。

//listnum[vTaskSwitchContext_2][task.c::vTaskSwitchContext()]{
void vTaskSwitchContext( void )
{
    ...

        /* Before the currently running task is switched out, save its errno. */
        #if( configUSE_POSIX_ERRNO == 1 )
        {
            ...
        }
        #endif

        /* Select a new task to run using either the generic C or port
        optimised asm code. */
        taskSELECT_HIGHEST_PRIORITY_TASK(); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9079 void * is used as this macro is used with timers and co-routines too.  @<embed>$|latex|\linebreak\hspace*{5ex}$Alignment is known to be fine as the type of the pointer stored and retrieved @<embed>$|latex|\linebreak\hspace*{5ex}$is the same. */
//}

//listnum[taskSELECT_HIGHEST_PRIORITY_TASK][task.c::taskSELECT_HIGHEST_PRIORITY_TASK()]{
    #define taskSELECT_HIGHEST_PRIORITY_TASK()                                                          \
    {                                                                                                   \
    UBaseType_t uxTopPriority = uxTopReadyPriority;                                                     \
                                                                                                        \
        /* Find the highest priority queue that contains ready tasks. */                                \
        while( listLIST_IS_EMPTY( &( pxReadyTasksLists[ uxTopPriority ] ) ) )                           \
        {                                                                                               \
            configASSERT( uxTopPriority );                                                              \
            --uxTopPriority;                                                                            \
        }                                                                                               \
                                                                                                        \
        /* listGET_OWNER_OF_NEXT_ENTRY indexes through the list, so the tasks of                        \
        the    same priority get an equal share of the processor time. */                               \
        listGET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB,@<embed>$|latex|\linebreak\hspace*{5ex}$&( pxReadyTasksLists[ uxTopPriority ] ) );           \
        uxTopReadyPriority = uxTopPriority;                                                             \
    } /* taskSELECT_HIGHEST_PRIORITY_TASK */
//}

xXXレディ状態のタスクの中で優先度が最も高いタスクを選択し、レディ状態のタスクの中で優先度が最も高いタスクを新たにuxTopReadyPriorityに設定する。

//listnum[vTaskSwitchContext_3][task.c::vTaskSwitchContext()]{
void vTaskSwitchContext( void )
{
    ...

        traceTASK_SWITCHED_IN();

        /* After the new task is switched in, update the global errno. */
        #if( configUSE_POSIX_ERRNO == 1 )
        {
            ...
        }
        #endif

        #if ( configUSE_NEWLIB_REENTRANT == 1 )
        {
            ...
        }
        #endif /* configUSE_NEWLIB_REENTRANT */
    }
}
//}

traceTASK_SWITCHED_IN() は何もしない。


これで一通りのコードに目を通したことになる。
ここまでに登場していない関数の説明

