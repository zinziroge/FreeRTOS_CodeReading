
= (hoga)

== 動機
 * RTOS(Real Time OS)について知りたい
 * Arduino UNOで動くRTOSがあるらしい。実行ファイルのサイズが7kBらしい。これなら読めるのでは？

== 対象バージョン
 * Arduino_FreeRTOS_Library
 ** https://github.com/feilipu/Arduino_FreeRTOS_Library.git
 *** 10.1.1-1
 * FreeRTOS
 ** 161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf
 ** FreeRTOS_Reference_Manual_V10.0.0.pdf
 * H/W
 ** Arduino UNO(ATMega328p)

== 方針
 * 最初にコードを引用し、そのあと解説を加える
 * コメントは適宜削除
 * #if defined() ~ #endif は該当する箇所のみ記述し、適宜削除。
 * Config は FreeRTOSConfig.h のデフォルト値に従う。デフォルト値以外は、補足であつかうかも。

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
 * p
 * r
 * v

== 関数命名規則
 * 'v' で始まる関数の戻り値は void
 * 'p' で始まる関数の戻り値は pointer
 * 'x' で始まる関数の戻り値は BaseType_t
 * Task で始まる関数の実体は、tasks.c に記述されている

== ソースコードの引用方法
 * 下記のような関数があるとき、すべてを一度に引用すると見にくいので、部分的に引用して説明する。

//emlist{
void func(void) {
    aaa()
    bbb()
    ccc()
}
//}

例えば, aaa(), bbb(), ccc() を別々に引用し説明するときは下記のように ... で省略していることを明記する。

//emlist{
void func(void) {
    aaa()

    ...
}
//}
 * aaa() についての説明

//emlist{
void func(void) {
    ...

    bbb()

    ...
}
//}
 * bbb() についての説明

//emlist{
void func(void) {
    ...

    ccc()
}
//}
 * ccc() についての説明


== ここからソースコードリーディング

なぜ、variantHook.cpp から読み始めるか
161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf
に書いてある

//listnum[hoge][variantHook.cpp::void initVariant(void)][c]{
void initVariant(void) __attribute__ ((OS_main));
void initVariant(void)
{
#if defined(USBCON)
    USBDevice.attach();
#endif

    setup();        // the normal Arduino setup() function is run here.
    vTaskStartScheduler(); // initialise and run the freeRTOS scheduler. Execution should never return here.
}
//}

 * __attribute__ ((OS_main)) は、最初にコールされる関数を指定していると思うが要確認。 <!-- TODO -->
 * USBCON は未定義。
 * `setup()`は、.ino でユーザが記述する`setup()`関数。

//listnum[vTaskStartScheduler_1][tasks.c::void vTaskStartScheduler( void ), 1/6][c]{
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
        vApplicationGetIdleTaskMemory( &pxIdleTaskTCBBuffer, &pxIdleTaskStackBuffer, &ulIdleTaskStackSize );
        xIdleTaskHandle = xTaskCreateStatic(    prvIdleTask,
                                                configIDLE_TASK_NAME,
                                                ulIdleTaskStackSize,
                                                ( void * ) NULL, /*lint !e961.  The cast is not redundant for all compilers. */
                                                portPRIVILEGE_BIT, /* In effect ( tskIDLE_PRIORITY | portPRIVILEGE_BIT ), but tskIDLE_PRIORITY is zero. */
                                                pxIdleTaskStackBuffer,
                                                pxIdleTaskTCBBuffer ); /*lint !e961 MISRA exception, justified as it is not a redundant explicit cast to all supported compilers. */

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
                                portPRIVILEGE_BIT, /* In effect ( tskIDLE_PRIORITY | portPRIVILEGE_BIT ), but tskIDLE_PRIORITY is zero. */
                                &xIdleTaskHandle ); /*lint !e961 MISRA exception, justified as it is not a redundant explicit cast to all supported compilers. */
    }
    #endif /* configSUPPORT_STATIC_ALLOCATION */

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

    if( xReturn == pdPASS )
    {
        /* freertos_tasks_c_additions_init() should only be called if the user
        definable macro FREERTOS_TASKS_C_ADDITIONS_INIT() is defined, as that is
        the only macro called by the function. */
        #ifdef FREERTOS_TASKS_C_ADDITIONS_INIT
        {
            freertos_tasks_c_additions_init();
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
            /* Switch Newlib's _impure_ptr variable to point to the _reent
            structure specific to the task that will run first. */
            _impure_ptr = &( pxCurrentTCB->xNewLib_reent );
        }
        #endif /* configUSE_NEWLIB_REENTRANT */

        xNextTaskUnblockTime = portMAX_DELAY;
        xSchedulerRunning = pdTRUE;
        xTickCount = ( TickType_t ) configINITIAL_TICK_COUNT;

        /* If configGENERATE_RUN_TIME_STATS is defined then the following
        macro must be defined to configure the timer/counter used to generate
        the run time counter time base.   NOTE:  If configGENERATE_RUN_TIME_STATS
        is set to 0 and the following line fails to build then ensure you do not
        have portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() defined in your
        FreeRTOSConfig.h file. */
        portCONFIGURE_TIMER_FOR_RUN_TIME_STATS();

        traceTASK_SWITCHED_IN();

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
    }
    else
    {
        /* This line will only be reached if the kernel could not be started,
        because there was not enough FreeRTOS heap to create the idle task
        or the timer task. */
        configASSERT( xReturn != errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY );
    }

    /* Prevent compiler warnings if INCLUDE_xTaskGetIdleTaskHandle is set to 0,
    meaning xIdleTaskHandle is not used anywhere else. */
    ( void ) xIdleTaskHandle;
}
//}


 * BaseType_t は，H/Wアーキテクチャで最も効率的に扱えるデータタイプ。ATMega328pでは8bit?
 ** 1.5 Data Types and Coding Style Guide
 * configSUPPORT_STATIC_ALLOCATION = 0 なので `#else` が有効。(以降、特に断りなく#if definedで該当しない行は引用せず省略することがある。)

//listnum[xTaskCreate_1][tasks.c::BaseType_t xTaskCreate(), 1/2][c]{
    BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                            const char * const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
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
            pxStack = ( StackType_t * ) pvPortMalloc( ( ( ( size_t ) usStackDepth ) * sizeof( StackType_t ) ) ); /*lint !e9079 All values returned by pvPortMalloc() have at least the alignment required by the MCU's stack and this allocation is the stack. */

    ...
}
//}

 * TCB_t は、Task Control Bufferのための構造体。
 * portSTACK_GROWTH = -1 (@portmacro.h) なので、`#else`が有効。
 * StackType_t は uint8_t である(@portmacro.h)
 * usStackDepth はこの関数の引数で、configMINIMAL_STACK_SIZE が値渡しされている。StackType_t は uint8_t なので 192byte のstack領域を確保する。
 * pvPortMalloc() でメモリ確保している。


//listnum[tskTCB][tasks.c::tskTCB, TCB_t][c]{
/*
 * Task control block.  A task control block (TCB) is allocated for each task,
 * and stores task state information, including a pointer to the task's context
 * (the task's run time environment, including register values)
 */
typedef struct TaskControlBlock_t
{
    volatile StackType_t    *pxTopOfStack;  /*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE TCB STRUCT. */

    #if ( portUSING_MPU_WRAPPERS == 1 )
        xMPU_SETTINGS   xMPUSettings;       /*< The MPU settings are defined as part of the port layer.  THIS MUST BE THE SECOND MEMBER OF THE TCB STRUCT. */
    #endif

    ListItem_t          xStateListItem;     /*< The list that the state list item of a task is reference from denotes the state of that task (Ready, Blocked, Suspended ). */
    ListItem_t          xEventListItem;     /*< Used to reference a task from an event list. */
    UBaseType_t         uxPriority;         /*< The priority of the task.  0 is the lowest priority. */
    StackType_t         *pxStack;           /*< Points to the start of the stack. */
    char                pcTaskName[ configMAX_TASK_NAME_LEN ];/*< Descriptive name given to the task when created.  Facilitates debugging only. */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */

    #if ( ( portSTACK_GROWTH > 0 ) || ( configRECORD_STACK_HIGH_ADDRESS == 1 ) )
        StackType_t     *pxEndOfStack;      /*< Points to the highest valid address for the stack. */
    #endif

    #if ( portCRITICAL_NESTING_IN_TCB == 1 )
        UBaseType_t     uxCriticalNesting;  /*< Holds the critical section nesting depth for ports that do not maintain their own count in the port layer. */
    #endif

    #if ( configUSE_TRACE_FACILITY == 1 )
        UBaseType_t     uxTCBNumber;        /*< Stores a number that increments each time a TCB is created.  It allows debuggers to determine when a task has been deleted and then recreated. */
        UBaseType_t     uxTaskNumber;       /*< Stores a number specifically for use by third party trace code. */
    #endif

    #if ( configUSE_MUTEXES == 1 )
        UBaseType_t     uxBasePriority;     /*< The priority last assigned to the task - used by the priority inheritance mechanism. */
        UBaseType_t     uxMutexesHeld;
    #endif

    #if ( configUSE_APPLICATION_TASK_TAG == 1 )
        TaskHookFunction_t pxTaskTag;
    #endif

    #if( configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0 )
        void            *pvThreadLocalStoragePointers[ configNUM_THREAD_LOCAL_STORAGE_POINTERS ];
    #endif

    #if( configGENERATE_RUN_TIME_STATS == 1 )
        uint32_t        ulRunTimeCounter;    /*< Stores the amount of time the task has spent in the Running state. */
    #endif

    #if ( configUSE_NEWLIB_REENTRANT == 1 )
        /* Allocate a Newlib reent structure that is specific to this task.
        Note Newlib support has been included by popular demand, but is not
        used by the FreeRTOS maintainers themselves.  FreeRTOS is not
        responsible for resulting newlib operation.  User must be familiar with
        newlib and must provide system-wide implementations of the necessary
        stubs. Be warned that (at the time of writing) the current newlib design
        implements a system-wide malloc() that must be provided with locks. */
        struct    _reent xNewLib_reent;
    #endif

    #if( configUSE_TASK_NOTIFICATIONS == 1 )
        volatile uint32_t ulNotifiedValue;
        volatile uint8_t ucNotifyState;
    #endif

    /* See the comments above the definition of
    tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE. */
    #if( tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE != 0 ) /*lint !e731 !e9029 Macro has been consolidated for readability reasons. */
        uint8_t    ucStaticallyAllocated;         /*< Set to pdTRUE if the task is a statically allocated to ensure no attempt is made to free the memory. */
    #endif

    #if( INCLUDE_xTaskAbortDelay == 1 )
        uint8_t ucDelayAborted;
    #endif

    #if( configUSE_POSIX_ERRNO == 1 )
        int iTaskErrno;
    #endif

} tskTCB;

/* The old tskTCB name is maintained above then typedefed to the new TCB_t name
below to enable the use of older kernel aware debuggers. */
typedef tskTCB TCB_t;
//}

//listnum[pvPortMalloc][heap_3.c::void *pvPortMalloc( size_t xWantedSize )]{
void *pvPortMalloc( size_t xWantedSize )
{
void *pvReturn;

    vTaskSuspendAll();
    {
        pvReturn = malloc( xWantedSize );
        traceMALLOC( pvReturn, xWantedSize );
    }
    ( void ) xTaskResumeAll();

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

 * malloc() は stdlib.h で定義されている
  * 中身は要確認 <!-- TODO -->
 * traceMALLOC( pvReturn, xWantedSize ); はどこで定義されている？ <!-- TODO -->
 * メモリ確保する前に vTaskSuspendAll() ですべてのタスクをサスペンドし、メモリ確保後に xTaskResumeAll() ですべてのタスクをレジュームしている。
 * heap_3.c の他に heap_1.c, heap_2.c, heap_4.c がある場合がる。

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

 * コンテキストスイッチをそのままにしておく。
 * http://goo.gl/wu4acr の要約
 ** キーポイントは、各タスクはそれぞれ自分自身のコンテキストを管理するので、変数(uxSchedulerSuspended)がもしゼロだとしてもコンテキストスイッチは起こらない。従って、レジスタバックからメモリへ書き込むことはatomic(訳注：不可分で2つ以上の命令に分割できない。1つのstore命令である。)であり、問題にならない。

//listnum[vApplicationMallocFailedHook][variantHooks.c::vApplicationMallocFailedHook()]{
void vApplicationMallocFailedHook( void )
{
#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) // Arduino Mega with 2560
...
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega8__) // assume we're using an Arduino Uno with 328p
    DDRB  |= _BV(DDB5);
    PORTB |= _BV(PORTB5);       // Main (red PB5) LED on. Main LED on.

...

#endif

    ...

    for(;;)
    {
        _delay_ms(50);

#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)  // Mega with 2560
        PINB  |= _BV(PINB7);       // Main (red PB7) LED toggle. Main LED fast blink.

#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega8__) // assume we're using an Arduino Uno with 328p
        PINB  |= _BV(PINB5);       // Main (red PB5) LED toggle. Main LED fast blink.

...

#endif
    }
}
//}

  * Arduino UNO は ATmega328P。malloc失敗時はLEDを点滅

//listnum[vTaskResumeAll][tasks.c::vTaskResumeAll()]{
//}

//listnum[xTaskCreate_2][tasks.c::BaseType_t xTaskCreate(), 2/2]{
    BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                            const char * const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                            const configSTACK_DEPTH_TYPE usStackDepth,
                            void * const pvParameters,
                            UBaseType_t uxPriority,
                            TaskHandle_t * const pxCreatedTask )
    {
        ...

            if( pxStack != NULL )
            {
                /* Allocate space for the TCB. */
                pxNewTCB = ( TCB_t * ) pvPortMalloc( sizeof( TCB_t ) ); /*lint !e9087 !e9079 All values returned by pvPortMalloc() have at least the alignment required by the MCU's stack, and the first member of TCB_t is always a pointer to the task's stack. */

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

 * このタスクのためのTCB(Task Control Buffer)のためのメモリ領域を確保し、確保したスタック領域pxStackを割り当てる。TCBを作成できなかった場合は、確保したスタック領域も開放する。

//listnum[vTaskStartScheduler_2][tasks.c::void vTaskStartScheduler( void ), 2/6]{
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

* タスク作成が成功していたら、タイマータスクを作る。

//listnum[xTimerCreateTimerTask_1][timers.c::BaseType_t xTimerCreateTimerTask( void ), 1/2]{
BaseType_t xTimerCreateTimerTask( void )
{
BaseType_t xReturn = pdFAIL;

    /* This function is called when the scheduler is started if
    configUSE_TIMERS is set to 1.  Check that the infrastructure used by the
    timer service task has been created/initialised.  If timers have already
    been created then the initialisation will already have been performed. */
    prvCheckForValidListAndQueue();
BaseType_t xTimerCreateTimerTask( void )

    ...
//}

//listnum[prvCheckForValidListAndQueue][ timers.c::prvCheckForValidListAndQueue()]{
static void prvCheckForValidListAndQueue( void )
{
    /* Check that the list from which active timers are referenced, and the
    queue used to communicate with the timer service, have been
    initialised. */
    taskENTER_CRITICAL();
    {
        if( xTimerQueue == NULL )
        {
            vListInitialise( &xActiveTimerList1 );
            vListInitialise( &xActiveTimerList2 );
            pxCurrentTimerList = &xActiveTimerList1;
            pxOverflowTimerList = &xActiveTimerList2;

            #if( configSUPPORT_STATIC_ALLOCATION == 1 )
            {
                /* The timer queue is allocated statically in case
                configSUPPORT_DYNAMIC_ALLOCATION is 0. */
                static StaticQueue_t xStaticTimerQueue; /*lint !e956 Ok to declare in this manner to prevent additional conditional compilation guards in other locations. */
                static uint8_t ucStaticTimerQueueStorage[ ( size_t ) configTIMER_QUEUE_LENGTH * sizeof( DaemonTaskMessage_t ) ]; /*lint !e956 Ok to declare in this manner to prevent additional conditional compilation guards in other locations. */

                xTimerQueue = xQueueCreateStatic( ( UBaseType_t ) configTIMER_QUEUE_LENGTH, ( UBaseType_t ) sizeof( DaemonTaskMessage_t ), &( ucStaticTimerQueueStorage[ 0 ] ), &xStaticTimerQueue );
            }
            #else
            {
                xTimerQueue = xQueueCreate( ( UBaseType_t ) configTIMER_QUEUE_LENGTH, sizeof( DaemonTaskMessage_t ) );
            }
            #endif

            #if ( configQUEUE_REGISTRY_SIZE > 0 )
            {
                if( xTimerQueue != NULL )
                {
                    vQueueAddToRegistry( xTimerQueue, "TmrQ" );
                }
                else
                {
                    mtCOVERAGE_TEST_MARKER();
                }
            }
            #endif /* configQUEUE_REGISTRY_SIZE */
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
    taskEXIT_CRITICAL();
}
//}


//listnum[prvCheckForValidListAndQueue_2][timer.h::prvCheckForValidListAndQueue, 2/?]{
static void prvCheckForValidListAndQueue( void )
{
    /* Check that the list from which active timers are referenced, and the
    queue used to communicate with the timer service, have been
    initialised. */
    taskENTER_CRITICAL();
//}

 * コメントによると、アクティブなタイマーがこのリストで管理される。タイマーは時間が切れる順に参照される、最も最初に切れるタイマーがリストの先頭にある

//listnum[taskENTER_CRITICAL][task.h]{
#define taskENTER_CRITICAL()        portENTER_CRITICAL()
//}

//listnum[portENTER_CRITICAL][portmacro.h]{
#define portENTER_CRITICAL()    __asm__ __volatile__ (                              \
                                        "in __tmp_reg__, __SREG__"        "\n\t"    \
                                        "cli"                             "\n\t"    \
                                        "push __tmp_reg__"                "\n\t"    \
                                        ::: "memory"                                \
                                        )
//}

 * ステータスレジスタをSTACKに退避している

 * in : Load an I/O Location to Register
 ** __tmp_reg__ に __SREG__ をコピーする
 * cli : 割り込み禁止
 * __tmp_reg__ の値を STACK に積む(push)
 * __SREG__	: Status register at address 0x3F
 * __tmp_reg__	: Register r0, used for temporary storage
 * clobber "memory"
 ** コンパイラに対して、アセンブラコードはメモリ配置を修正するかもしれないことを知らせる
 ** それは、コンパイラにレジスタに保持されている全てのコンテンツを保持(退避)することを、アセンブラコード実行前に、強制する。アセンブラコード実行後に書き戻す。
 *** http://www.nongnu.org/avr-libc/user-manual/inline_asm.html

//listnum[prvCheckForValidListAndQueue_3][timer.h::prvCheckForValidListAndQueue, 3/?]{
    taskENTER_CRITICAL();
    {
        if( xTimerQueue == NULL )
        {
            vListInitialise( &xActiveTimerList1 );
            vListInitialise( &xActiveTimerList2 );
            pxCurrentTimerList = &xActiveTimerList1;
            pxOverflowTimerList = &xActiveTimerList2;
//}
 * vListInitialiseは双方向リストの初期化API
 * pxCurrentTimerList, pxOverflowTimerList は tasks.c で定義されている変数
 * 現在のタイマーリストと、オーバーフローしたタイマーリストの２つを作成している。 
 * @todo : オーバーフローしたタイマーリストとは？


//listnum[prvCheckForValidListAndQueue_4][timer.h::prvCheckForValidListAndQueue, 4/?]{
            pxOverflowTimerList = &xActiveTimerList2;

            #if( configSUPPORT_STATIC_ALLOCATION == 1 )
            {
                ...
            }
            #else
            {
                xTimerQueue = xQueueCreate( ( UBaseType_t ) configTIMER_QUEUE_LENGTH, sizeof( DaemonTaskMessage_t ) );
            }
            #endif
//}

//listnum[xQueueCreate][queue.h]
#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
    #define xQueueCreate( uxQueueLength, uxItemSize ) xQueueGenericCreate( ( uxQueueLength ), ( uxItemSize ), ( queueQUEUE_TYPE_BASE ) )
#endif
//}

//listnum[xQueueGenericCreate_define][queue.h]
#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
    #define xQueueCreate( uxQueueLength, uxItemSize ) xQueueGenericCreate( ( uxQueueLength ), ( uxItemSize ), ( queueQUEUE_TYPE_BASE ) )
#endif
//}

//listnum[xQueueGenericCreate][queue.c]
    QueueHandle_t xQueueGenericCreate( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, const uint8_t ucQueueType )
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
            xQueueSizeInBytes = ( size_t ) ( uxQueueLength * uxItemSize ); /*lint !e961 MISRA exception as the casts are only redundant for some ports. */
        }

        /* Allocate the queue and storage area.  Justification for MISRA
        deviation as follows:  pvPortMalloc() always ensures returned memory
        blocks are aligned per the requirements of the MCU stack.  In this case
        pvPortMalloc() must return a pointer that is guaranteed to meet the
        alignment requirements of the Queue_t structure - which in this case
        is an int8_t *.  Therefore, whenever the stack alignment requirements
        are greater than or equal to the pointer to char requirements the cast
        is safe.  In other cases alignment requirements are not strict (one or
        two bytes). */
        pxNewQueue = ( Queue_t * ) pvPortMalloc( sizeof( Queue_t ) + xQueueSizeInBytes ); /*lint !e9087 !e9079 see comment above. */

        if( pxNewQueue != NULL )
        {
            /* Jump past the queue structure to find the location of the queue
            storage area. */
            pucQueueStorage = ( uint8_t * ) pxNewQueue;
            pucQueueStorage += sizeof( Queue_t ); /*lint !e9016 Pointer arithmetic allowed on char types, especially when it assists conveying intent. */

            #if( configSUPPORT_STATIC_ALLOCATION == 1 )
            {
                /* Queues can be created either statically or dynamically, so
                note this task was created dynamically in case it is later
                deleted. */
                pxNewQueue->ucStaticallyAllocated = pdFALSE;
            }
            #endif /* configSUPPORT_STATIC_ALLOCATION */

            prvInitialiseNewQueue( uxQueueLength, uxItemSize, pucQueueStorage, ucQueueType, pxNewQueue );
        }
        else
        {
            traceQUEUE_CREATE_FAILED( ucQueueType );
            mtCOVERAGE_TEST_MARKER();
        }

        return pxNewQueue;
    }
//}

 * タイマーのためのキューを作成している。サイズは10。listとの使い分けは？


//listnum[prvCheckForValidListAndQueue_5][timer.h::prvCheckForValidListAndQueue, 5/?]{
    taskEXIT_CRITICAL();
//}

//listnum[taskEXIT_CRITICAL][task.h.]
#define taskEXIT_CRITICAL()         portEXIT_CRITICAL()
//}

//listnum[portEXIT_CRITICAL_define][task.h.]
#define taskEXIT_CRITICAL()         portEXIT_CRITICAL()
//}

//listnum[portEXIT_CRITICAL][portmacro.h]
#define portEXIT_CRITICAL()     __asm__ __volatile__ (                              \
                                        "pop __tmp_reg__"                 "\n\t"    \
                                        "out __SREG__, __tmp_reg__"       "\n\t"    \
                                        ::: "memory"                                \
                                        )
//}
 * __tmp_reg__ の値を STACK から取り出す(pop)
 * out : Store an I/O Location to Register
 * ステータスレジスタ(__SREG__)に退避した値を書き戻す


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
                                    ( ( UBaseType_t ) configTIMER_TASK_PRIORITY ) | portPRIVILEGE_BIT,
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

 * タイマーのタスクを作る
 ** スケジューラタスクのタイマーのタスクを作る


//listnum[vTaskStartScheduler_3][tasks.c::vTaskStartScheduler(), 3/6]{
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

        xNextTaskUnblockTime = portMAX_DELAY;
        xSchedulerRunning = pdTRUE;
        xTickCount = ( TickType_t ) configINITIAL_TICK_COUNT;

        /* If configGENERATE_RUN_TIME_STATS is defined then the following
        macro must be defined to configure the timer/counter used to generate
        the run time counter time base.   NOTE:  If configGENERATE_RUN_TIME_STATS
        is set to 0 and the following line fails to build then ensure you do not
        have portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() defined in your
        FreeRTOSConfig.h file. */
        portCONFIGURE_TIMER_FOR_RUN_TIME_STATS();

        traceTASK_SWITCHED_IN();

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
    }
    else
    {
        /* This line will only be reached if the kernel could not be started,
        because there was not enough FreeRTOS heap to create the idle task
        or the timer task. */
        configASSERT( xReturn != errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY );
    }

    ...
}
//}

 * portDISABLE_INTERRUPTS() は portmacro.h で定義されている。avrのasmなのであとで確認する。

//listnum[portDISABLE_INTERRUPT][portmacro.h]{
#define portDISABLE_INTERRUPTS()        __asm__ __volatile__ ( "cli" ::: "memory")
//}
 * cli: 割り込み禁止
 * xPortStartScheduler()を呼ぶ前、あるいは呼んでいる途中でCPUクロックが進まないように？割り込みが起きないように?
 * 最初のタスクが実行されると自動的に割り込みは有効になる。

//listnum[vTaskStartScheduler_4][taskc.c::void vTaskStartScheduler( void ), 4/6]{
        xNextTaskUnblockTime = portMAX_DELAY;
        xSchedulerRunning = pdTRUE;
        xTickCount = ( TickType_t ) configINITIAL_TICK_COUNT;
//}

 * 右辺の値はそれぞれportmacro.h, projdefs.h, Arduino_FreeRTOS.h に記述されている。

//listnum[vTaskStartScheduler_5][taskc.c::void vTaskStartScheduler( void ), 5/6]{
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

//listnum[Arduino_FreeRTOS_h][Arduino_FreeRTOS.h]{
...

#ifndef portCONFIGURE_TIMER_FOR_RUN_TIME_STATS
    #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#endif

...

#ifndef traceTASK_SWITCHED_IN
    /* Called after a task has been selected to run.  pxCurrentTCB holds a pointer
    to the task control block of the selected task. */
    #define traceTASK_SWITCHED_IN()
#endif

...
//}
 * どちらも中身はない。

//listnum[vTaskStartScheduler_6][tasks.c::void vTaskStartScheduler( void ), 6/6]{
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

//listnum[xPortStartScheduler][port.c::xPortStartScheduler(void)]{
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

 * prvSetupTimerInterrupt() で割り込み禁止
 * 

//listnum[portRESTORE_CONTEXT][port.c]
#define portRESTORE_CONTEXT()                                                           \
        __asm__ __volatile__ (  "lds    r26, pxCurrentTCB                       \n\t"   \
                                "lds    r27, pxCurrentTCB + 1                   \n\t"   \
                                "ld     r28, x+                                 \n\t"   \
                                "out    __SP_L__, r28                           \n\t"   \
                                "ld     r29, x+                                 \n\t"   \
                                "out    __SP_H__, r29                           \n\t"   \
                                "pop    r31                                     \n\t"   \
                                "pop    r30                                     \n\t"   \
                                ...
                                "pop    r3                                      \n\t"   \
                                "pop    r2                                      \n\t"   \
                                "pop    __zero_reg__                            \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
                                "out    __SREG__, __tmp_reg__                   \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
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

 * __asm__ __volatile__ ( "ret" );
 ** returnじゃダメなのはなんで??? __asm__ __volatile__ ( "ret" );

//listnum[portSAVE_CONTEXT][port.c]
#define portSAVE_CONTEXT()                                                              \
        __asm__ __volatile__ (  "push   __tmp_reg__                             \n\t"   \
                                "in     __tmp_reg__, __SREG__                   \n\t"   \
                                "cli                                            \n\t"   \
                                "push   __tmp_reg__                             \n\t"   \
                                "push   __zero_reg__                            \n\t"   \
                                "clr    __zero_reg__                            \n\t"   \
                                "push   r2                                      \n\t"   \
                                "push   r3                                      \n\t"   \

                                ...

                                "push   r30                                     \n\t"   \
                                "push   r31                                     \n\t"   \
                                "lds    r26, pxCurrentTCB                       \n\t"   \
                                "lds    r27, pxCurrentTCB + 1                   \n\t"   \
                                "in     __tmp_reg__, __SP_L__                   \n\t"   \
                                "st     x+, __tmp_reg__                         \n\t"   \
                                "in     __tmp_reg__, __SP_H__                   \n\t"   \
                                "st     x+, __tmp_reg__                         \n\t"   \
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

//listnum[vTaskStartScheduler_7][tasks.c::void vTaskStartScheduler( void ), 7/6]{
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

//listnum[Blink_AnalogRead][example/Blink_AnalogRead.ino]{
#include <Arduino_FreeRTOS.h>

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void TaskAnalogRead( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink
    ,  (const portCHAR *)"Blink"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskAnalogRead
    ,  (const portCHAR *) "AnalogRead"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, LEONARDO, MEGA, and ZERO 
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN takes care 
  of use the correct LED pin whatever is the board used.
  
  The MICRO does not have a LED_BUILTIN available. For the MICRO board please substitute
  the LED_BUILTIN definition with either LED_BUILTIN_RX or LED_BUILTIN_TX.
  e.g. pinMode(LED_BUILTIN_RX, OUTPUT); etc.
  
  If you want to know what pin the on-board LED is connected to on your Arduino model, check
  the Technical Specs of your board  at https://www.arduino.cc/en/Main/Products
  
  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
  
  modified 2 Sep 2016
  by Arturo Guadalupi
*/

  // initialize digital LED_BUILTIN on pin 13 as an output.
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
  
/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/

  for (;;)
  {
    // read the input on analog pin 0:
    int sensorValue = analogRead(A0);
    // print out the value you read:
    Serial.println(sensorValue);
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}
//}

---
ここまでに登場していない関数の説明
