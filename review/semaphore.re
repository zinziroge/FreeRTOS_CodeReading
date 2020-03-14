= ソースコードリーディング その2 ～セマフォ～
== Interrupts.ino

この章では example/Interrupts.ino で使用されているFreeRTOS API のソースコードを読んでいきましょう。
主にセマフォの実装を理解していきたいと思います。

//listnum[Interrupts][example/Interrupts.ino]{
/*
 * Example of a Arduino interruption and RTOS Binary Semaphore
 * https://www.freertos.org/Embedded-RTOS-Binary-Semaphores.html
 */


// Include Arduino FreeRTOS library
#include <Arduino_FreeRTOS.h>

// Include semaphore supoport
#include <semphr.h>

/* 
 * Declaring a global variable of type SemaphoreHandle_t 
 * 
 */
SemaphoreHandle_t interruptSemaphore;

void setup() {

  // Configure pin 2 as an input and enable the internal pull-up resistor
  pinMode(2, INPUT_PULLUP);

 // Create task for Arduino led 
  xTaskCreate(TaskLed, // Task function
              "Led", // Task name
              128, // Stack size 
              NULL, 
              0, // Priority
              NULL );

  /**
   * Create a binary semaphore.
   * https://www.freertos.org/xSemaphoreCreateBinary.html
   */
  interruptSemaphore = xSemaphoreCreateBinary();
  if (interruptSemaphore != NULL) {
    // Attach interrupt for Arduino digital pin
    attachInterrupt(digitalPinToInterrupt(2), interruptHandler, LOW);
  }

  
}

void loop() {}


void interruptHandler() {
  /**
   * Give semaphore in the interrupt handler
   * https://www.freertos.org/a00124.html
   */
  
  xSemaphoreGiveFromISR(interruptSemaphore, NULL);
}


/* 
 * Led task. 
 */
void TaskLed(void *pvParameters)
{
  (void) pvParameters;

  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) {
    
    /**
     * Take the semaphore.
     * https://www.freertos.org/a00122.html
     */
    if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
    
  }
}
//}

setup()関数のxTaskCreate()は既に説明済みなので説明から省きます。
interruptHandler()の xSemaphoreCreateBinary()を見ていきます。

== バイナリセマフォを作成する

xSemaphoreCreateBinary()はバイナリセマフォを作成し、セマフォが参照されるハンドラを返します。

#@# //quote{
#@# xSemaphoreCreateBinary()
#@# 
#@# 概要:
#@# 
#@# バイナリセマフォを作成し、セマフォが参照されるハンドラを返す。
#@# 各バイナリセマフォは少量のRAMを必要とし、セマフォの状態保持するのに使用される。
#@# バイナリセマフォが xSemaphoreCreateBinary() を使用して作成された場合、必要なRAMは自動的にFreeRTOSのヒープに割り当てられる。
#@# バイナリセマフォが xSemaphoreCreateBinaryStatic() を使用して作成された場合、必要なRAMはアプリケーション作成者によって提供され、
#@# 追加パラメータを必要とするが、コンパイル時に静的にRAMが割り当てられる。
#@# セマフォは'カラ'状態で作成され、セマフォは xSemaphoreTake()関数を使って取得される前に、まず最初に与えられる必要がある。
#@# //}

//listnum[xSemaphoreCreateBinary][semphr.h]{
#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
    #define xSemaphoreCreateBinary() xQueueGenericCreate( ( UBaseType_t ) 1, @<embed>$|latex|\linebreak\hspace*{5ex}$semSEMAPHORE_QUEUE_ITEM_LENGTH, queueQUEUE_TYPE_BINARY_SEMAPHORE )
#endif
//}

configSUPPORT_DYNAMIC_ALLOCATION は 1 です。
xSemaphoreCreateBinary() は xQueueGenericCreate()として別名定義されています。
xQueueGenericCreate() はコードを既に読んでいるので、セマフォに関係のある所をおさらいしてみます。

#@# //listnum[xQueueGenericCreate_1][queue.c, 1/2]{
#@#     QueueHandle_t xQueueGenericCreate(@<embed>$|latex|\linebreak\hspace*{5ex}$ const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, @<embed>$|latex|\linebreak\hspace*{5ex}$const uint8_t ucQueueType )
#@#     {
#@#     Queue_t *pxNewQueue;
#@#     size_t xQueueSizeInBytes;
#@#     uint8_t *pucQueueStorage;
#@# 
#@#         configASSERT( uxQueueLength > ( UBaseType_t ) 0 );
#@# 
#@#         ...
#@# //}
#@# 
#@#  * 引数の値は以下のとおりである。
#@#  * uxQueueLength = 1, uxItemSize = 0, ucQueueType = queueQUEUE_TYPE_BINARY_SEMAPHORE
#@#  * xQueueSizeInBytes = 1
#@# 
#@# //listnum[xQueueGenericCreate_2][queue.c, 2/2]{
#@#     QueueHandle_t xQueueGenericCreate(@<embed>$|latex|\linebreak\hspace*{5ex}$ const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, @<embed>$|latex|\linebreak\hspace*{5ex}$const uint8_t ucQueueType )
#@#     {
#@#         ...
#@# 
#@#         if( uxItemSize == ( UBaseType_t ) 0 )
#@#         {
#@#             /* There is not going to be a queue storage area. */
#@#             xQueueSizeInBytes = ( size_t ) 0;
#@#         }
#@#         else
#@#         {
#@#             /* Allocate enough space to hold the maximum number of items that
#@#             can be in the queue at any time. */
#@#             xQueueSizeInBytes = @<embed>$|latex|\linebreak\hspace*{5ex}$( size_t ) ( uxQueueLength * uxItemSize ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e961 MISRA exception as the casts are only redundant for some ports. */
#@#         }
#@# 
#@#         /* Allocate the queue and storage area.  Justification for MISRA
#@#         deviation as follows:  pvPortMalloc() always ensures returned memory
#@#         blocks are aligned per the requirements of the MCU stack.  In this case
#@#         pvPortMalloc() must return a pointer that is guaranteed to meet the
#@#         alignment requirements of the Queue_t structure - which in this case
#@#         is an int8_t *.  Therefore, whenever the stack alignment requirements
#@#         are greater than or equal to the pointer to char requirements the cast
#@#         is safe.  In other cases alignment requirements are not strict (one or
#@#         two bytes). */
#@#         pxNewQueue = @<embed>$|latex|\linebreak\hspace*{5ex}$( Queue_t * ) pvPortMalloc( sizeof( Queue_t ) + xQueueSizeInBytes ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9087 !e9079 see comment above. */
#@# 
#@#         if( pxNewQueue != NULL )
#@#         {
#@#             /* Jump past the queue structure to find the location of the queue
#@#             storage area. */
#@#             pucQueueStorage = ( uint8_t * ) pxNewQueue;
#@#             pucQueueStorage += sizeof( Queue_t ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9016 Pointer arithmetic allowed on char types, especially when it assists @<embed>$|latex|\linebreak\hspace*{5ex}$conveying intent. */
#@# 
#@#             #if( configSUPPORT_STATIC_ALLOCATION == 1 )
#@#             {
#@#                 /* Queues can be created either statically or dynamically, so
#@#                 note this task was created dynamically in case it is later
#@#                 deleted. */
#@#                 pxNewQueue->ucStaticallyAllocated = pdFALSE;
#@#             }
#@#             #endif /* configSUPPORT_STATIC_ALLOCATION */
#@# 
#@#             prvInitialiseNewQueue( @<embed>$|latex|\linebreak\hspace*{5ex}$uxQueueLength, uxItemSize, pucQueueStorage, ucQueueType, pxNewQueue );
#@#         }
#@#         else
#@#         {
#@#             traceQUEUE_CREATE_FAILED( ucQueueType );
#@#             mtCOVERAGE_TEST_MARKER();
#@#         }
#@# 
#@#         return pxNewQueue;
#@#     }
#@# //}
#@# 
#@# * Queueのメモリ確保
#@# ** xQueueSizeInBytesはQueueのデータを保持する部分のメモリサイズ(バイト)。
#@# ** pxNewQueue = ( Queue_t * ) pvPortMalloc( sizeof( Queue_t ) + xQueueSizeInBytes ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9087 !e9079 see comment above. */
#@# ** Queue_t構造体のアライメントを満たすことを保証する必要がある(int8_t*)
#@# ** サイズが同じか大きければ型キャストは安全、 
#@# ** Queue_t構造体のサイズ + xQueueSizeInBytes のサイズだけmallocして、Queue_t構造体のポインタを取得
#@# ** mallocに成功していれば、pucQueueStorageに今取得したQueue_tを設定してポインタを進める

#@#  * Queueの初期化
#@# //listnum[prvInitialiseNewQueue_1][queue.c, 1/2]{
#@# static void prvInitialiseNewQueue( @<embed>$|latex|\linebreak\hspace*{5ex}$const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, @<embed>$|latex|\linebreak\hspace*{5ex}$uint8_t *pucQueueStorage, const uint8_t ucQueueType, Queue_t *pxNewQueue )
#@# {
#@#     ...
#@# //}

 * 引数の値は以下のとおりである。
 * uxQueueLength = 1, uxItemSize = 0, ucQueueType = queueQUEUE_TYPE_BINARY_SEMAPHORE
 *  uint8_t *pucQueueStorage, const uint8_t ucQueueType, Queue_t *pxNewQueue )
 * xQueueSizeInBytes = 1
 * セマフォはQueueのサイズ(uxItemSize) はゼロ。
 * pcHeadは新規キューのストレージ領域の先頭のポインタ。サイズがゼロなのでNULLでもよさそうだが、NULLはmutexとして使うことを意味するのでそう設定しない。
 * Queueの長さはセマフォなので1。

== セマフォを与える


//listnum[Interrupts_2][example/Interrupts.ino:interruptHandler()]{
void interruptHandler() {
  /**
   * Give semaphore in the interrupt handler
   * https://www.freertos.org/a00124.html
   */
  
  xSemaphoreGiveFromISR(interruptSemaphore, NULL);
}
//}

interruptHandler()は割り込みハンドラーで、ピン2がLOWになったときに割り込みが発生します。
xSemaphoreGiveFromISR()で割り込みからセマフォを取得します。

//quote{
xSemaphoreGiveFromISR(interruptSemaphore, NULL);

概要:

ISR(Interrupt Service Routine)で使用できるバージョンの xSemaphoreGive()。
xSemaphoreGive() とは違い、xSemaphoreGiveFromISR() は特定されたブロック時間を許可しない。
//}

//listnum[xSemaphoreGiveFromISR][semphr.h]{
#define xSemaphoreGiveFromISR( xSemaphore, pxHigherPriorityTaskWoken )    @<embed>$|latex|\linebreak\hspace*{5ex}$xQueueGiveFromISR( ( QueueHandle_t ) ( xSemaphore ), ( pxHigherPriorityTaskWoken ) )
//}

セマフォ操作の他の関数同様にキューで実装されています。
xQueueGiveFromISR()を読んでいきましょう。
名前から想像されるとおり、割り込みからセマフォを与える関数です。

//listnum[xQueueGiveFromISR_1][queue.cx::QueueGiveFromISR, 1/2]{
BaseType_t xQueueGiveFromISR( @<embed>$|latex|\linebreak\hspace*{5ex}$QueueHandle_t xQueue, BaseType_t * const pxHigherPriorityTaskWoken )
{
BaseType_t xReturn;
UBaseType_t uxSavedInterruptStatus;
Queue_t * const pxQueue = xQueue;

...

    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    {
        const UBaseType_t uxMessagesWaiting = pxQueue->uxMessagesWaiting;

        /* When the queue is used to implement a semaphore no data is ever
        moved through the queue but it is still valid to see if the queue 'has
        space'. */
        if( uxMessagesWaiting < pxQueue->uxLength )
        {
            const int8_t cTxLock = pxQueue->cTxLock;

            traceQUEUE_SEND_FROM_ISR( pxQueue );

            /* A task can only have an inherited priority if it is a mutex
            holder - and if there is a mutex holder then the mutex cannot be
            given from an ISR.  As this is the ISR version of the function it
            can be assumed there is no mutex holder and no need to determine if
            priority disinheritance is needed.  Simply increase the count of
            messages (semaphores) available. */
            pxQueue->uxMessagesWaiting = uxMessagesWaiting + ( UBaseType_t ) 1;

            ...
//}

//listnum[xQueueGiveFromISR_2][queue.c::xQueueGiveFromISR(), 2/2]{
BaseType_t xQueueGiveFromISR( @<embed>$|latex|\linebreak\hspace*{5ex}$QueueHandle_t xQueue, BaseType_t * const pxHigherPriorityTaskWoken )
{
    ...

            /* The event list is not altered if the queue is locked.  This will
            be done when the queue is unlocked later. */
            if( cTxLock == queueUNLOCKED )
            {
                #if ( configUSE_QUEUE_SETS == 1 )
                {
                    ...
                }
                #else /* configUSE_QUEUE_SETS */
                {
                    if( listLIST_IS_EMPTY( @<embed>$|latex|\linebreak\hspace*{5ex}$&( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
                    {
                        if( xTaskRemoveFromEventList( @<embed>$|latex|\linebreak\hspace*{5ex}$&( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
                        {
                            /* The task waiting has a higher priority so record that @<embed>$|latex|\linebreak\hspace*{5ex}$a
                            context    switch is required. */
                            if( pxHigherPriorityTaskWoken != NULL )
                            {
                                *pxHigherPriorityTaskWoken = pdTRUE;
                            }
                            else
                            {

                    ...

                }
                #endif /* configUSE_QUEUE_SETS */
            }
            else
            {
                /* Increment the lock count so the task that unlocks the queue
                knows that data was posted while it was locked. */
                pxQueue->cTxLock = ( int8_t ) ( cTxLock + 1 );
            }

            xReturn = pdPASS;
        }
        else
        {
            traceQUEUE_SEND_FROM_ISR_FAILED( pxQueue );
            xReturn = errQUEUE_FULL;
        }
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

    return xReturn;
}
//}

 * メッセージ受け取り待ちのタスクが存在し、受け取り状態に遷移出来たら、コンテキストスイッチを行うように促す


== セマフォを取得する

Interrupts.ino に戻ってきました。TaskLed()を読んでいきましょう。

//listnum[TaskLed][Interrupts.ino::TaskLed()]{
void TaskLed(void *pvParameters)
{
  (void) pvParameters;

  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) {
    /**
     * Take the semaphore.
     * https://www.freertos.org/a00122.html
     */
    if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
    
  }
}
//}

xSemaphoreTake()でセマフォを取得出来たら、LED_BUILTINポートの信号値をリードして、反転させて出力させることで、LED表示をトグルさせています。
xSemaphoreTake()を見ていきましょう。

//listnum[xSemaphoreTake][semphr.c::xSemaphoreTake()]{
#define xSemaphoreTake( xSemaphore, xBlockTime )        @<embed>$|latex|\linebreak\hspace*{5ex}$xQueueSemaphoreTake( ( xSemaphore ), ( xBlockTime ) )
//}

xSemaphoreTake()はxQueueSemaphoreTake()の別名定義で、
キューで実装されていることがわかります。xQueueSemaphoreTake()を読んでいきましょう。

//listnum[xQueueSemaphoreTake_1][queue.c::xQueueSemaphoreTake(), 1/6]{
BaseType_t xQueueSemaphoreTake( QueueHandle_t xQueue, TickType_t xTicksToWait )
{
BaseType_t xEntryTimeSet = pdFALSE;
TimeOut_t xTimeOut;
Queue_t * const pxQueue = xQueue;

#if( configUSE_MUTEXES == 1 )
    BaseType_t xInheritanceOccurred = pdFALSE;
#endif

    /* Check the queue pointer is not NULL. */
    configASSERT( ( pxQueue ) );

    /* Check this really is a semaphore, in which case the item size will be
    0. */
    configASSERT( pxQueue->uxItemSize == 0 );

    /* Cannot block if the scheduler is suspended. */
    #if ( ( INCLUDE_xTaskGetSchedulerState == 1 ) || ( configUSE_TIMERS == 1 ) )
    {
        configASSERT( !( ( xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED ) @<embed>$|latex|\linebreak\hspace*{5ex}$&& ( xTicksToWait != 0 ) ) );
    }
    #endif
 
    ...

}
//}

ここまでのマクロは無効なのでここまでは特に何も実行しません。

//listnum[xQueueSemaphoreTake_2][queue.c::xQueueSemaphoreTake, 2/6]{
BaseType_t xQueueSemaphoreTake( QueueHandle_t xQueue, TickType_t xTicksToWait )
{
    ...

    /*lint -save -e904 This function relaxes the coding standard somewhat to @<embed>$|latex|\linebreak\hspace*{5ex}$allow return
    statements within the function itself.  This is done in the interest
    of execution time efficiency. */
    for( ;; )
    {
        taskENTER_CRITICAL();
        {
            /* Semaphores are queues with an item size of 0, and where the
            number of messages in the queue is the semaphore's count value. */
            const UBaseType_t uxSemaphoreCount = pxQueue->uxMessagesWaiting;

            /* Is there data in the queue now?  To be running the calling task
            must be the highest priority task wanting to access the queue. */
            if( uxSemaphoreCount > ( UBaseType_t ) 0 )
            {
                traceQUEUE_RECEIVE( pxQueue );

                /* Semaphores are queues with a data size of zero and where the
                messages waiting is the semaphore's count.  Reduce the count. */
                pxQueue->uxMessagesWaiting = uxSemaphoreCount - ( UBaseType_t ) 1;

                #if ( configUSE_MUTEXES == 1 )
                {
                    if( pxQueue->uxQueueType == queueQUEUE_IS_MUTEX )
                    {
                        /* Record the information required to implement
                        priority inheritance should it become necessary. */
                        pxQueue->u.xSemaphore.xMutexHolder = @<embed>$|latex|\linebreak\hspace*{5ex}$pvTaskIncrementMutexHeldCount();
                    }
            ...

}
//}

 * セマフォ取得待ちの他のタスクがいるときの処理
 * uxSemaphoreCount に受け取ち待ちのタスクを代入
 * セマフォ待ちタスクの数を一つ減らし、

//listnum[pvTaskIncrementMutexHeldCount][queue.c]{
//}

 * 一旦無視

//listnum[xQueueSemaphoreTake_3][queue.c::xQueueSemaphoreTake(), 3/6]{
BaseType_t xQueueSemaphoreTake( QueueHandle_t xQueue, TickType_t xTicksToWait )
{
    ...

                        pxQueue->u.xSemaphore.xMutexHolder = @<embed>$|latex|\linebreak\hspace*{5ex}$pvTaskIncrementMutexHeldCount();
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }
                }
                #endif /* configUSE_MUTEXES */

                /* Check to see if other tasks are blocked waiting to give the
                semaphore, and if so, unblock the highest priority such task. */
                if( listLIST_IS_EMPTY( @<embed>$|latex|\linebreak\hspace*{5ex}$&( pxQueue->xTasksWaitingToSend ) ) == pdFALSE )
                {
                    if( xTaskRemoveFromEventList( @<embed>$|latex|\linebreak\hspace*{5ex}$&( pxQueue->xTasksWaitingToSend ) ) != pdFALSE )
                    {
                        queueYIELD_IF_USING_PREEMPTION();
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }
                }
                else
                {
                    mtCOVERAGE_TEST_MARKER();
                }

                taskEXIT_CRITICAL();
                return pdPASS;
            }
//}

 * セマフォを送信しようとしてブロックされているタスクがあれば（キューがカラでなければ）、
   イベントリストからそのタスクを削除

 * ISR内のクリティカルセクションから呼び出しできる
 * イベントリストは優先度順に並び変えされているので、最も優先度の高いと分かり、削除される。遅延リストからTCBを削除し、実行待ちリストに追加する。
 * イベントがロックされたキューに対するものである場合は、この関数は決して呼ばれない。キューのロック・カウント がその代わりに修正される。
 * これはイベントリストへの排他アクセスを意味し、それが保証される。
 * この関数は、チェックが既に為され、pxEventListがカラでないことを確認していることを仮定している。

//listnum[xTaskRemoveFromEventList, 1/2][task.c::xTaskRemoveFromEventList()]{
BaseType_t xTaskRemoveFromEventList( const List_t * const pxEventList )
{
TCB_t *pxUnblockedTCB;
BaseType_t xReturn;

    /* THIS FUNCTION MUST BE CALLED FROM A CRITICAL SECTION.  It can also be
    called from a critical section within an ISR. */

    /* The event list is sorted in priority order, so the first in the list can
    be removed as it is known to be the highest priority.  Remove the TCB from
    the delayed list, and add it to the ready list.

    If an event is for a queue that is locked then this function will never
    get called - the lock count on the queue will get modified instead.  This
    means exclusive access to the event list is guaranteed here.

    This function assumes that a check has already been made to ensure that
    pxEventList is not empty. */

    ...
//}

//listnum[xTaskRemoveFromEventList_2][task.c::xTaskRemoveFromEventList, 2/2]{
    ...

    pxUnblockedTCB = listGET_OWNER_OF_HEAD_ENTRY( pxEventList ); @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e9079 void * is used as this macro is used with timers and co-routines too.  @<embed>$|latex|\linebreak\hspace*{5ex}$Alignment is known to be fine as the type of the pointer stored and retrieved is the same. */
    configASSERT( pxUnblockedTCB );
    ( void ) uxListRemove( &( pxUnblockedTCB->xEventListItem ) );

    if( uxSchedulerSuspended == ( UBaseType_t ) pdFALSE )
    {
        ( void ) uxListRemove( &( pxUnblockedTCB->xStateListItem ) );
        prvAddTaskToReadyList( pxUnblockedTCB );

        #if( configUSE_TICKLESS_IDLE != 0 )
        {
            /* If a task is blocked on a kernel object then xNextTaskUnblockTime
            might be set to the blocked task's time out time.  If the task is
            unblocked for a reason other than a timeout xNextTaskUnblockTime is
            normally left unchanged, because it is automatically reset to a new
            value when the tick count equals xNextTaskUnblockTime.  However if
            tickless idling is used it might be more important to enter sleep mode
            at the earliest possible time - so reset xNextTaskUnblockTime here to
            ensure it is updated at the earliest possible time. */
            prvResetNextTaskUnblockTime();
        }
        #endif
    }
    else
    {
        /* The delayed and ready lists cannot be accessed, so hold this task
        pending until the scheduler is resumed. */
        vListInsertEnd( @<embed>$|latex|\linebreak\hspace*{5ex}$&( xPendingReadyList ), &( pxUnblockedTCB->xEventListItem ) );
    }

    if( pxUnblockedTCB->uxPriority > pxCurrentTCB->uxPriority )
    {
        /* Return true if the task removed from the event list has a higher
        priority than the calling task.  This allows the calling task to know if
        it should force a context switch now. */
        xReturn = pdTRUE;

        /* Mark that a yield is pending in case the user is not using the
        "xHigherPriorityTaskWoken" parameter to an ISR safe FreeRTOS function. */
        xYieldPending = pdTRUE;
    }
    else
    {
        xReturn = pdFALSE;
    }

    return xReturn;
}
//}

 * 優先度の最も高いタスクを取得し（pxEventListの先頭のタスク）、リストから除外し、現在のステートから除外し、Readyリストに追加
 * タスクスケジューラがサスペンド状態のときは、レディーペンディングリスト追加して、タスクスケジューラがレジュームされたら追加する
 * 現在のタスクより、優先度の高いタスクがreadyに入ったら、読み出しもとにコンテキストスイッチを促す
 * ただし、xHigherPriorityTaskWokenパラメータを使ってないので実行は保留される


//listnum[xQueueSemaphoreTake_4][queue.c::xQueueSemaphoreTake, 4/6]{
BaseType_t xQueueSemaphoreTake( QueueHandle_t xQueue, TickType_t xTicksToWait )
{
    ...

        taskENTER_CRITICAL();
        {
            ...

            if( uxSemaphoreCount > ( UBaseType_t ) 0 )
            {
                ...
            }
            else
            {
                if( xTicksToWait == ( TickType_t ) 0 )
                {
                    /* For inheritance to have occurred there must have been an
                    initial timeout, and an adjusted timeout cannot become 0, as
                    if it were 0 the function would have exited. */
                    #if( configUSE_MUTEXES == 1 )
                    {
                        configASSERT( xInheritanceOccurred == pdFALSE );
                    }
                    #endif /* configUSE_MUTEXES */

                    /* The semaphore count was 0 and no block time is specified
                    (or the block time has expired) so exit now. */
                    taskEXIT_CRITICAL();
                    traceQUEUE_RECEIVE_FAILED( pxQueue );
                    return errQUEUE_EMPTY;
                }
                else if( xEntryTimeSet == pdFALSE )
                {
                    /* The semaphore count was 0 and a block time was specified
                    so configure the timeout structure ready to block. */
                    vTaskInternalSetTimeOutState( &xTimeOut );
                    xEntryTimeSet = pdTRUE;
                }
                else
                {
                    /* Entry time was already set. */
                    mtCOVERAGE_TEST_MARKER();
                }
            }
        }
        taskEXIT_CRITICAL();

        /* Interrupts and other tasks can give to and take from the semaphore
        now the critical section has been exited. */

        vTaskSuspendAll();
        prvLockQueue( pxQueue );

        ...
//}

 * セマフォ取得待ちの他のタスクがいないときの処理


//listnum[xQueueSemaphoreTake_5][queue.c::xQueueSemaphoreTake(), 5/6]{
BaseType_t xQueueSemaphoreTake( QueueHandle_t xQueue, TickType_t xTicksToWait )
{
        ...

        /* Update the timeout state to see if it has expired yet. */
        if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) == pdFALSE )
        {
            /* A block time is specified and not expired.  If the semaphore
            count is 0 then enter the Blocked state to wait for a semaphore to
            become available.  As semaphores are implemented with queues the
            queue being empty is equivalent to the semaphore count being 0. */
            if( prvIsQueueEmpty( pxQueue ) != pdFALSE )
            {
                traceBLOCKING_ON_QUEUE_RECEIVE( pxQueue );

                #if ( configUSE_MUTEXES == 1 )
                {
                    if( pxQueue->uxQueueType == queueQUEUE_IS_MUTEX )
                    {
                        taskENTER_CRITICAL();
                        {
                            xInheritanceOccurred = xTaskPriorityInherit( @<embed>$|latex|\linebreak\hspace*{5ex}$pxQueue->u.xSemaphore.xMutexHolder );
                        }
                        taskEXIT_CRITICAL();
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }
                }
                #endif

                vTaskPlaceOnEventList( @<embed>$|latex|\linebreak\hspace*{5ex}$&( pxQueue->xTasksWaitingToReceive ), xTicksToWait );
                prvUnlockQueue( pxQueue );
                if( xTaskResumeAll() == pdFALSE )
                {
                    portYIELD_WITHIN_API();
                }
                else
                {
                    mtCOVERAGE_TEST_MARKER();
                }
            }
            else
            {
                /* There was no timeout and the semaphore count was not 0, so
                attempt to take the semaphore again. */
                prvUnlockQueue( pxQueue );
                ( void ) xTaskResumeAll();
            }

    ...
//}

//listnum[xQueueSemaphoreTake_6][queue.c::xQueueSemaphoreTake(), 6/6]{
BaseType_t xQueueSemaphoreTake( QueueHandle_t xQueue, TickType_t xTicksToWait )
{
        ...
        else
        {
            /* Timed out. */
            prvUnlockQueue( pxQueue );
            ( void ) xTaskResumeAll();

            /* If the semaphore count is 0 exit now as the timeout has
            expired.  Otherwise return to attempt to take the semaphore that is
            known to be available.  As semaphores are implemented by queues the
            queue being empty is equivalent to the semaphore count being 0. */
            if( prvIsQueueEmpty( pxQueue ) != pdFALSE )
            {
                #if ( configUSE_MUTEXES == 1 )
                {
                    /* xInheritanceOccurred could only have be set if
                    pxQueue->uxQueueType == queueQUEUE_IS_MUTEX so no need to
                    test the mutex type again to check it is actually a mutex. */
                    if( xInheritanceOccurred != pdFALSE )
                    {
                        taskENTER_CRITICAL();
                        {
                            UBaseType_t uxHighestWaitingPriority;

                            /* This task blocking on the mutex caused another
                            task to inherit this task's priority.  Now this task
                            has timed out the priority should be disinherited
                            again, but only as low as the next highest priority
                            task that is waiting for the same mutex. */
                            uxHighestWaitingPriority = @<embed>$|latex|\linebreak\hspace*{5ex}$prvGetDisinheritPriorityAfterTimeout( pxQueue );
                            vTaskPriorityDisinheritAfterTimeout( @<embed>$|latex|\linebreak\hspace*{5ex}$pxQueue->u.xSemaphore.xMutexHolder, uxHighestWaitingPriority );
                        }
                        taskEXIT_CRITICAL();
                    }
                }
                #endif /* configUSE_MUTEXES */

                traceQUEUE_RECEIVE_FAILED( pxQueue );
                return errQUEUE_EMPTY;
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }
    } /*lint -restore */
}
//}