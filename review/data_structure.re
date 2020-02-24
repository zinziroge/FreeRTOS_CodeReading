= データ構造

コードを読み進めるあたっていくつかのデータ構造は先に読んでおいた方が理解が進むと思うので、
この章でリスト、キュー、セマフォ、タスクについて説明します。

== リスト
FreeRTOSのリストの特徴は以下のとおりです。
 * 双方向リスト
 * uxNumberOfItems + 1 個の要素を含む。ただし、+1は初期化時に設定されるリストの終端を表す要素である。
 * リストを初期化すると(vListInitialise())、要素が1つ含まれる状態になる
 * 各要素は値を持ち、値の降順でリストが並ぶ

//listnum[List_t][list.c()::List_t]{
/*
 * Definition of the only type of object that a list can contain.
 */
struct xLIST;
struct xLIST_ITEM
{
    listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE           @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
    configLIST_VOLATILE TickType_t xItemValue;          @<embed>$|latex|\linebreak\hspace*{5ex}$/*< The value being listed.  In most cases this is used to sort the list in @<embed>$|latex|\linebreak\hspace*{5ex}$descending order. */
    struct xLIST_ITEM * configLIST_VOLATILE pxNext;     @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Pointer to the next ListItem_t in the list. */
    struct xLIST_ITEM * configLIST_VOLATILE pxPrevious; @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Pointer to the previous ListItem_t in the list. */
    void * pvOwner;                                     @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Pointer to the object (normally a TCB) that contains the list item.  There is @<embed>$|latex|\linebreak\hspace*{5ex}$therefore a two way link between the object containing the list item and the list @<embed>$|latex|\linebreak\hspace*{5ex}$item itself. */
    struct xLIST * configLIST_VOLATILE pxContainer;     @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Pointer to the list in which this list item is placed (if any). */
    listSECOND_LIST_ITEM_INTEGRITY_CHECK_VALUE          @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
};
typedef struct xLIST_ITEM ListItem_t;                   @<embed>$|latex|\linebreak\hspace*{5ex}$/* For some reason lint wants this as two separate definitions. */

struct xMINI_LIST_ITEM
{
    listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE           @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
    configLIST_VOLATILE TickType_t xItemValue;
    struct xLIST_ITEM * configLIST_VOLATILE pxNext;
    struct xLIST_ITEM * configLIST_VOLATILE pxPrevious;
};
typedef struct xMINI_LIST_ITEM MiniListItem_t;

/*
 * Definition of the type of queue used by the scheduler.
 */
typedef struct xLIST
{
    listFIRST_LIST_INTEGRITY_CHECK_VALUE                @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
    volatile UBaseType_t uxNumberOfItems;
    ListItem_t * configLIST_VOLATILE pxIndex;           @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Used to walk through the list.  Points to the last item returned by a call to @<embed>$|latex|\linebreak\hspace*{5ex}$listGET_OWNER_OF_NEXT_ENTRY (). */
    MiniListItem_t xListEnd;                            @<embed>$|latex|\linebreak\hspace*{5ex}$/*< List item that contains the maximum possible item value meaning it is always at @<embed>$|latex|\linebreak\hspace*{5ex}$the end of the list and is therefore used as a marker. */
    listSECOND_LIST_INTEGRITY_CHECK_VALUE               @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
} List_t;
//}


== キュー
FreeRTOSのキューの特徴は以下のとおりです。

 * FIFOであり、要素その前後へのポインタをもつ。
 ** FIFO(First In First Out) は、最初にキューに入れたデータが最初に取り出される。ところてん形式である。
 * タスク間、割り込みとタスク間の通信に使えるスレッドセーフなFIFOである。
 * データはリファレンス(ポインタ)ではなく、コピーである。メモリは消費するが取り扱いは簡単になる。
 * 種類はキュートセマフォの2種類ある。
 * キューの長さ、キューのアイテムごとのサイズをもつ
 * キューへの送信待ち、キューからの受信待ちのタスクリストがそれぞれ別にある
 * キューがロック中にキューから受信した(つまり、削除された)要素の数と、キューに送信した（つまり、追加した)要素の数
 ** キューのロックは..
 * ブロックタイム
 ** 空のキューからタスクがリードすると、そのタスクはブロック状態に遷移する。
    そのタスクは、ブロックタイムが過ぎるか、キューにデータが来るまでブロック状態にいる。
 ** フルのキューにタスクがライトすると、そのタスクはブロック状態に遷移する。
    そのタスクは、ブロックタイムが過ぎるか、キューからデータが読まれるまでブロック状態にいる。
 * 割り込みは "FromISR" で終わらないAPIを使ってはいけない

//listnum[Queue_t][queue.c()::Queue_t]{
/*
 * Definition of the queue used by the scheduler.
 * Items are queued by copy, not reference.  See the following link for the
 * rationale: https://www.freertos.org/Embedded-RTOS-Queues.html
 */
typedef struct QueueDef_t
{
    int8_t *pcHead;                 @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Points to the beginning of the queue storage area. */
    int8_t *pcWriteTo;              @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Points to the free next place in the storage area. */

    union
    {
        QueuePointers_t xQueue;     @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Data required exclusively when this structure is used as a queue. */
        SemaphoreData_t xSemaphore; @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Data required exclusively when this structure is used as a semaphore. */
    } u;

    List_t xTasksWaitingToSend;     @<embed>$|latex|\linebreak\hspace*{5ex}$/*< List of tasks that are blocked waiting to post onto this queue. @<embed>$|latex|\linebreak\hspace*{5ex}$ Stored in priority order. */
    List_t xTasksWaitingToReceive;  @<embed>$|latex|\linebreak\hspace*{5ex}$/*< List of tasks that are blocked waiting to read from this queue. @<embed>$|latex|\linebreak\hspace*{5ex}$ Stored in priority order. */

    volatile UBaseType_t uxMessagesWaiting;@<embed>$|latex|\linebreak\hspace*{5ex}$/*< The number of items currently in the queue. */
    UBaseType_t uxLength;           @<embed>$|latex|\linebreak\hspace*{5ex}$/*< The length of the queue defined as the number of items it will @<embed>$|latex|\linebreak\hspace*{5ex}$hold, not the number of bytes. */
    UBaseType_t uxItemSize;         @<embed>$|latex|\linebreak\hspace*{5ex}$/*< The size of each items that the queue will hold. */

    volatile int8_t cRxLock;        @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Stores the number of items received from the queue (removed from the queue) @<embed>$|latex|\linebreak\hspace*{5ex}$while the queue was locked. @<embed>$|latex|\linebreak\hspace*{5ex}$ Set to queueUNLOCKED when the queue is not locked. */
    volatile int8_t cTxLock;        @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Stores the number of items transmitted to the queue (added to the queue) while @<embed>$|latex|\linebreak\hspace*{5ex}$the queue was locked.  @<embed>$|latex|\linebreak\hspace*{5ex}$Set to queueUNLOCKED when the queue is not locked. */

    #if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) @<embed>$|latex|\linebreak\hspace*{5ex}$&& ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
        uint8_t ucStaticallyAllocated;    @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Set to pdTRUE if the memory used by the queue was statically allocated to @<embed>$|latex|\linebreak\hspace*{5ex}$ensure no attempt is made to free the memory. */
    #endif

    #if ( configUSE_QUEUE_SETS == 1 )
        struct QueueDef_t *pxQueueSetContainer;
    #endif

    #if ( configUSE_TRACE_FACILITY == 1 )
        UBaseType_t uxQueueNumber;
        uint8_t ucQueueType;
    #endif

} xQUEUE;

/* The old xQUEUE name is maintained above then typedefed to the new Queue_t
name below to enable the use of older kernel aware debuggers. */
typedef xQUEUE Queue_t;
//}

== セマフォ

FreeRTOSのセマフォの特徴は以下のとおりです。

 * 1つの要素を持つキューとして実装されている
 * バイナリセマフォ、カウンティングセマフォの2種類がある
 ** バイナリセマフォは相互排他と同期に用いられる。
 ** カウンティングセマフォは、イベントのカウントやリソース管理に用いられる。
 * 周辺機器サービスで使うバイナリセマフォ
 ** バイナリセマフォでは、タスクはセマフォを取るだけで、与えることは行わない。
 ** バイナリセマフォでは、ペリフェラルはセマフォを与えるだけで、取ることは行わない。

== タスク
 * https://www.freertos.org/RTOS-task-states.html
 * 4つの状態のいずれかに属する
 ** Running
 *** CPUで実行中のタスク
 ** Ready
 *** 実行待ちのタスク
 ** Suspended
 *** この状態"へ"遷移するのはvTaskSuspend()が呼ばれた時だけであり、またこの状態"から"遷移するのはvTaskResume()が呼ばれたときだけである。
 ** Blockステート
 *** この状態にいるタスクは、通常、タイムアウト時間をもち、タイムアウト時間が経つとブロック解除される。
 *** キューやセマフォを待つためにブロックされる。
 *** emptyキューからデータを読もうとすると発生。データが来るか、ブロッキングタイムが経過すると解除される
 *** fullyキューにデータを書こうとすると発生。データが来るか、ブロッキングタイムが経過すると解除される
 *** 1つのキューに対して複数のブロックタスクがあるときは優先度の高いタスクがブロック解除される

//listnum[TaskList][tasks.c()::TaskList]{
/*lint -save -e956 A manual analysis and inspection has been used to determine
which static variables must be declared volatile. */
PRIVILEGED_DATA TCB_t * volatile pxCurrentTCB __attribute__((used)) = NULL;

/* Lists for ready and blocked tasks. --------------------
xDelayedTaskList1 and xDelayedTaskList2 could be move to function scople but
doing so breaks some kernel aware debuggers and debuggers that rely on removing
the static qualifier. */
PRIVILEGED_DATA static List_t pxReadyTasksLists[ configMAX_PRIORITIES ];    @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Prioritised ready tasks. */
PRIVILEGED_DATA static List_t xDelayedTaskList1;                            @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Delayed tasks. */
PRIVILEGED_DATA static List_t xDelayedTaskList2;                            @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Delayed tasks (two lists are used - one for delays that have overflowed the @<embed>$|latex|\linebreak\hspace*{5ex}$current tick count. */
PRIVILEGED_DATA static List_t * volatile pxDelayedTaskList;                 @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Points to the delayed task list currently being used. */
PRIVILEGED_DATA static List_t * volatile pxOverflowDelayedTaskList;         @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Points to the delayed task list currently being used to hold tasks that have @<embed>$|latex|\linebreak\hspace*{5ex}$overflowed the current tick count. */
PRIVILEGED_DATA static List_t xPendingReadyList;                            @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Tasks that have been readied while the scheduler was suspended.  They will be @<embed>$|latex|\linebreak\hspace*{5ex}$moved to the ready list when the scheduler is resumed. */
//}

pxReadyTasksListsは、優先度毎にタスクリストを保持する

//listnum[TCB_t][task.c()::TCB_t]{
/*
 * Task control block.  A task control block (TCB) is allocated for each task,
 * and stores task state information, including a pointer to the task's context
 * (the task's run time environment, including register values)
 */
typedef struct TaskControlBlock_t
{
    volatile StackType_t    *pxTopOfStack;  @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Points to the location of the last item placed on the tasks stack.  @<embed>$|latex|\linebreak\hspace*{5ex}$THIS MUST BE THE FIRST MEMBER OF THE TCB STRUCT. */

    ...

    ListItem_t          xStateListItem;     @<embed>$|latex|\linebreak\hspace*{5ex}$/*< The list that the state list item of a task is reference from denotes the @<embed>$|latex|\linebreak\hspace*{5ex}$state of that task (Ready, Blocked, Suspended ). */
    ListItem_t          xEventListItem;     @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Used to reference a task from an event list. */
    UBaseType_t         uxPriority;         @<embed>$|latex|\linebreak\hspace*{5ex}$/*< The priority of the task.  0 is the lowest priority. */
    StackType_t         *pxStack;           @<embed>$|latex|\linebreak\hspace*{5ex}$/*< Points to the start of the stack. */
    char                pcTaskName[ configMAX_TASK_NAME_LEN ];@<embed>$|latex|\linebreak\hspace*{5ex}$/*< Descriptive name given to the task when created.  Facilitates debugging only. */ @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e971 Unqualified char types are allowed for strings and single characters @<embed>$|latex|\linebreak\hspace*{5ex}$only. */

    ...

    #if ( configUSE_MUTEXES == 1 )
        UBaseType_t     uxBasePriority;     @<embed>$|latex|\linebreak\hspace*{5ex}$/*< The priority last assigned to the task - used by the priority inheritance @<embed>$|latex|\linebreak\hspace*{5ex}$mechanism. */
        UBaseType_t     uxMutexesHeld;
    #endif

    ...

    #if( configUSE_TASK_NOTIFICATIONS == 1 )
        volatile uint32_t ulNotifiedValue;
        volatile uint8_t ucNotifyState;
    #endif

    ...
} tskTCB;

/* The old tskTCB name is maintained above then typedefed to the new TCB_t name
below to enable the use of older kernel aware debuggers. */
typedef tskTCB TCB_t;
//}

pxTopOfStackはタスクのスタックの一番上のアドレス。
pxStackはタスクのスタックの開始アドレス。
スタックがアドレスの小さい方に伸びる場合、pxTopOfStack < pxStack である。
Ready,Block,Suspendから参照されているステートのリスト、スタックの先頭番地、優先度、タスクのスタックの先頭番地、タスク名
