= semaphore
== Interrupts.ino

次に example/Interrupts.ino で使用されているFree RTOS API のソースコードを読み解いていく。

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

setup()関数のxTaskCreate()は既に説明済みなので除外。
interruptHandler()の xSemaphoreCreateBinary()を見る。

//quote{
xSemaphoreCreateBinary()

概要:

バイナリセマフォを作成し、セマフォが参照されるハンドラを返す。
各バイナリセマフォは少量のRAMを必要とし、セマフォの状態保持するのに使用される。
バイナリセマフォが xSemaphoreCreateBinary() を使用して作成された場合、必要なRAMは自動的にFreeRTOSのヒープに割り当てられる。
バイナリセマフォが xSemaphoreCreateBinaryStatic() を使用して作成された場合、必要なRAMはアプリケーション作成者によって提供され、
追加パラメータを必要とするが、コンパイル時に静的にRAMが割り当てられる。
セマフォは'カラ'状態で作成され、セマフォは xSemaphoreTake()関数を使って取得される前に、まず最初に与えられる必要がある。
//}

//listnum[xSemaphoreCreateBinary][semphr.h]{
#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
    #define xSemaphoreCreateBinary() xQueueGenericCreate( ( UBaseType_t ) 1, semSEMAPHORE_QUEUE_ITEM_LENGTH, queueQUEUE_TYPE_BINARY_SEMAPHORE )
#endif
//}

* configSUPPORT_DYNAMIC_ALLOCATION は 1 である

//listnum[xQueueGenericCreate_1][queue.c]{
    QueueHandle_t xQueueGenericCreate( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, const uint8_t ucQueueType )
    {
    Queue_t *pxNewQueue;
    size_t xQueueSizeInBytes;
    uint8_t *pucQueueStorage;

        configASSERT( uxQueueLength > ( UBaseType_t ) 0 );

        ...
//}

 * 引数の値は以下のとおりである。
 * uxQueueLength = 1, uxItemSize = 0, ucQueueType = queueQUEUE_TYPE_BINARY_SEMAPHORE
 * xQueueSizeInBytes = 1

//listnum[xQueueGenericCreate_2][queue.c]{
    QueueHandle_t xQueueGenericCreate( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, const uint8_t ucQueueType )
    {
        ...

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

 * Queueのメモリ確保
 ** xQueueSizeInBytesはQueueのデータを保持する部分のメモリサイズ(バイト)。
 ** pxNewQueue = ( Queue_t * ) pvPortMalloc( sizeof( Queue_t ) + xQueueSizeInBytes ); /*lint !e9087 !e9079 see comment above. */
 ** Queue_t構造体のアライメントを満たすことを保証する必要がある(int8_t*)
 ** サイズが同じか大きければ型キャストは安全、 
 ** Queue_t構造体のサイズ + xQueueSizeInBytes のサイズだけmallocして、Queue_t構造体のポインタを取得
 ** mallocに成功していれば、pucQueueStorageに今取得したQueue_tを設定してポインタを進める

 * Queueの初期化
//listnum[prvInitialiseNewQueue_1][queue.c]{
static void prvInitialiseNewQueue( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, uint8_t *pucQueueStorage, const uint8_t ucQueueType, Queue_t *pxNewQueue )
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
//}

 * 引数の値は以下のとおりである。
 * uxQueueLength = 1, uxItemSize = 0, ucQueueType = queueQUEUE_TYPE_BINARY_SEMAPHORE
 *  uint8_t *pucQueueStorage, const uint8_t ucQueueType, Queue_t *pxNewQueue )
 * xQueueSizeInBytes = 1
 * セマフォはQueueのサイズ(uxItemSize) はゼロ。
 * pcHeadは新規キューのストレージ領域の先頭のポインタ。サイズがゼロなのでNULLでもよさそうだが、NULLはmutexとして使うことを意味するのでそう設定しない。
 * Queueの長さはセマフォなので1。

 * xQueueGenericReset()。Queue のリセット

//listnum[xQueueGenericReset][queue.c]{
BaseType_t xQueueGenericReset( QueueHandle_t xQueue, BaseType_t xNewQueue )
{
Queue_t * const pxQueue = xQueue;

    configASSERT( pxQueue );

    taskENTER_CRITICAL();
    {
        pxQueue->u.xQueue.pcTail = pxQueue->pcHead + ( pxQueue->uxLength * pxQueue->uxItemSize ); /*lint !e9016 Pointer arithmetic allowed on char types, especially when it assists conveying intent. */
        pxQueue->uxMessagesWaiting = ( UBaseType_t ) 0U;
        pxQueue->pcWriteTo = pxQueue->pcHead;
        pxQueue->u.xQueue.pcReadFrom = pxQueue->pcHead + ( ( pxQueue->uxLength - 1U ) * pxQueue->uxItemSize ); /*lint !e9016 Pointer arithmetic allowed on char types, especially when it assists conveying intent. */
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

 * taskENTER_CRITICAL() で割り込み禁止にして、Queue_t構造体のメンバ変数を初期化
 * taskEXIT_CRITICAL() で割り込み解除

 * listの初期化

//listnum[vListInitialise][list.c]{
void vListInitialise( List_t * const pxList )
{
    /* The list structure contains a list item which is used to mark the
    end of the list.  To initialise the list the list end is inserted
    as the only list entry. */
    pxList->pxIndex = ( ListItem_t * ) &( pxList->xListEnd );            /*lint !e826 !e740 !e9087 The mini list structure is used as the list end to save RAM.  This is checked and valid. */

    /* The list end value is the highest possible value in the list to
    ensure it remains at the end of the list. */
    pxList->xListEnd.xItemValue = portMAX_DELAY;

    /* The list end next and previous pointers point to itself so we know
    when the list is empty. */
    pxList->xListEnd.pxNext = ( ListItem_t * ) &( pxList->xListEnd );    /*lint !e826 !e740 !e9087 The mini list structure is used as the list end to save RAM.  This is checked and valid. */
    pxList->xListEnd.pxPrevious = ( ListItem_t * ) &( pxList->xListEnd );/*lint !e826 !e740 !e9087 The mini list structure is used as the list end to save RAM.  This is checked and valid. */

    pxList->uxNumberOfItems = ( UBaseType_t ) 0U;

    /* Write known values into the list if
    configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
    listSET_LIST_INTEGRITY_CHECK_1_VALUE( pxList );
    listSET_LIST_INTEGRITY_CHECK_2_VALUE( pxList );
}
//}

 * pxList->pxIndex = ( ListItem_t * ) &( pxList->xListEnd );            /*lint !e826 !e740 !e9087 The mini list structure is used as the list end to save RAM.  This is checked and valid. */
 * listの最後pxIndexはそのリストの最後xListEndを割り当てる
 * 割り当て可能な最大値0xffffを設定。なんで？
 ** pxList->xListEnd.xItemValue = portMAX_DELAY;
 * listがカラである
 * 要素数がゼロである
 * listSET_LIST_INTEGRITY_CHECK_1_VALUE(), listSET_LIST_INTEGRITY_CHECK_2_VALUE() マクロは何もしない


//listnum[prvInitialiseNewQueue_2][queue.c]{
static void prvInitialiseNewQueue( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, uint8_t *pucQueueStorage, const uint8_t ucQueueType, Queue_t *pxNewQueue )
{
    ...

    /* Initialise the queue members as described where the queue type is
    defined. */
    pxNewQueue->uxLength = uxQueueLength;
    pxNewQueue->uxItemSize = uxItemSize;
    ( void ) xQueueGenericReset( pxNewQueue, pdTRUE );

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

 * マクロはすべて何もしない

//listnum[Interrupts_2][example/Interrupts.ino]{
void interruptHandler() {
  /**
   * Give semaphore in the interrupt handler
   * https://www.freertos.org/a00124.html
   */
  
  xSemaphoreGiveFromISR(interruptSemaphore, NULL);
}
//}

//quote{
xSemaphoreGiveFromISR(interruptSemaphore, NULL);

概要:

ISR(Interrupt Service Routine)で使用できるバージョンの xSemaphoreGive()。
xSemaphoreGive() とは違い、xSemaphoreGiveFromISR() は特定されたブロック時間を許可しない。
//}

 * セマフォのリリース（割り込み処理からセマフォを与える）

//listnum[xSemaphoreGiveFromISR][semphr.h]{
#define xSemaphoreGiveFromISR( xSemaphore, pxHigherPriorityTaskWoken )    xQueueGiveFromISR( ( QueueHandle_t ) ( xSemaphore ), ( pxHigherPriorityTaskWoken ) )
//}

 * キューで実装されている

//listnum[xQueueGiveFromISR][queue.c]{
BaseType_t xQueueGiveFromISR( QueueHandle_t xQueue, BaseType_t * const pxHigherPriorityTaskWoken )
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
                    if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
                    {
                        if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
                        {
                            /* The task waiting has a higher priority so record that a
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


//listnum[TaskLed][Interrupts.ino]{
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
//}

 * セマフォを取得出来たらdigitalWrite()して、LED表示をトグルさせる。


//listnum[xSemaphoreTake][semphr.c]{
#define xSemaphoreTake( xSemaphore, xBlockTime )        xQueueSemaphoreTake( ( xSemaphore ), ( xBlockTime ) )
//}

 * キューで実装


//listnum[xQueueSemaphoreTake][queue.c]{
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
        configASSERT( !( ( xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED ) && ( xTicksToWait != 0 ) ) );
    }
    #endif
 
    ...

//}

 * マクロは何もしない

//listnum[xQueueSemaphoreTake_2][queue.c]{
    /*lint -save -e904 This function relaxes the coding standard somewhat to allow return
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
                        pxQueue->u.xSemaphore.xMutexHolder = pvTaskIncrementMutexHeldCount();
                    }
            ...
//}

 * セマフォ取得待ちの他のタスクがいるときの処理
 * uxSemaphoreCount に受け取ち待ちのタスクを代入
 * セマフォ待ちタスクの数を一つ減らし、

//listnum[pvTaskIncrementMutexHeldCount][queue.c]{
//}

 * 一旦無視

//listnum[xQueueSemaphoreTake_3][queue.c]{
                    ... 

                        pxQueue->u.xSemaphore.xMutexHolder = pvTaskIncrementMutexHeldCount();
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }
                }
                #endif /* configUSE_MUTEXES */

                /* Check to see if other tasks are blocked waiting to give the
                semaphore, and if so, unblock the highest priority such task. */
                if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) == pdFALSE )
                {
                    if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToSend ) ) != pdFALSE )
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

//listnum[xTaskRemoveFromEventList, 1/2][task.c]{
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

//listnum[xTaskRemoveFromEventList, 2/2][task.c]{
    ...

    pxUnblockedTCB = listGET_OWNER_OF_HEAD_ENTRY( pxEventList ); /*lint !e9079 void * is used as this macro is used with timers and co-routines too.  Alignment is known to be fine as the type of the pointer stored and retrieved is the same. */
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
        vListInsertEnd( &( xPendingReadyList ), &( pxUnblockedTCB->xEventListItem ) );
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


//listnum[xQueueSemaphoreTake_4][queue.c]{
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


//listnum[xQueueSemaphoreTake_5][queue.c]{
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
                            xInheritanceOccurred = xTaskPriorityInherit( pxQueue->u.xSemaphore.xMutexHolder );
                        }
                        taskEXIT_CRITICAL();
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }
                }
                #endif

                vTaskPlaceOnEventList( &( pxQueue->xTasksWaitingToReceive ), xTicksToWait );
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
        }
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
                            uxHighestWaitingPriority = prvGetDisinheritPriorityAfterTimeout( pxQueue );
                            vTaskPriorityDisinheritAfterTimeout( pxQueue->u.xSemaphore.xMutexHolder, uxHighestWaitingPriority );
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