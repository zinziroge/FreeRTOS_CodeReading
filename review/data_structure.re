#@# 構造を書く。関数、APIは極力書かない。絵で説明する。

= データ構造

コードを読み進めるあたっていくつかのデータ構造の定義と使い方は読んで理解しておいた方が
全体の理解が進むと思うので、 この章でリスト、キュー、セマフォ、タスクについて説明します。

この章は最初に詳細に読んで理解できなくてもよいと思います。
コードリーディングを進めていって、分からなくなったら適時にこの章に戻ってきてください。

== リスト

まずはFreeRTOSのリスト構造体List_tのコードを見てみましょう。

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

コードだけでは分かりにくいので、リスト構造を@<img>{List_t-struct}に示します。

//image[List_t-struct][List_t]{
//}

FreeRTOSのリストの特徴は以下のとおりです。

 * 双方向リストです。つまり、リスト内のある要素を選択したとき、その前後の要素を1回の走査でたどることができます
 * 1個以上の要素を含みます。リストがカラの状態はxMINI_LIST_ITEMを1つ含んだ状態です。
 * リストを初期化すると(vListInitialise())、要素が1つ含まれる状態になります
 * 各要素は値を持ち、値の降順でリストの要素が並びます

#@# リストの初期化関数であるvListInitialise()も見てみましょう。
#@# 
#@# //listnum[vListInitialise][list.c::vListInitialise()]{
#@# void vListInitialise( List_t * const pxList )
#@# {
#@#     /* The list structure contains a list item which is used to mark the
#@#     end of the list.  To initialise the list the list end is inserted
#@#     as the only list entry. */
#@#     pxList->pxIndex = ( ListItem_t * ) &( pxList->xListEnd );            @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e826 !e740 !e9087 The mini list structure is used as the list end to @<embed>$|latex|\linebreak\hspace*{5ex}$save RAM.  This is checked and valid. */
#@# 
#@#     /* The list end value is the highest possible value in the list to
#@#     ensure it remains at the end of the list. */
#@#     pxList->xListEnd.xItemValue = portMAX_DELAY;
#@# 
#@#     /* The list end next and previous pointers point to itself so we know
#@#     when the list is empty. */
#@#     pxList->xListEnd.pxNext = ( ListItem_t * ) &( pxList->xListEnd );    @<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e826 !e740 !e9087 The mini list structure is used as the list end to @<embed>$|latex|\linebreak\hspace*{5ex}$save RAM.  This is checked and valid. */
#@#     pxList->xListEnd.pxPrevious = ( ListItem_t * ) &( pxList->xListEnd );@<embed>$|latex|\linebreak\hspace*{5ex}$/*lint !e826 !e740 !e9087 The mini list structure is used as the list end to @<embed>$|latex|\linebreak\hspace*{5ex}$save RAM.  This is checked and valid. */
#@# 
#@#     pxList->uxNumberOfItems = ( UBaseType_t ) 0U;
#@# 
#@#     /* Write known values into the list if
#@#     configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
#@#     listSET_LIST_INTEGRITY_CHECK_1_VALUE( pxList );
#@#     listSET_LIST_INTEGRITY_CHECK_2_VALUE( pxList );
#@# }
#@# //}
#@# 
#@# //image[List_t-vListInitialise][list.c::vListInitialise()]{
#@# //}
#@# 
#@# リストの最後の要素を示すpxIndexは、現在のリストの最後の要素であるxListEndを指すように設定します。
#@# リストの最後の要素であるxListEndの値は、割り当て可能な最大値portMAX_DELAY(Arduino UnoのFreeRTOの場合は0xffff)を設定しています。
#@# リストの終端であることを示すために設定しているようです。
#@# リストの最後の要素の次の要素(pxNext)と1つ前の要素(pxPrevious)が最後の要素自身(xListEnd)を指すことで、リストがカラであることが分かるようになっています。
#@# また、リストの要素数(uxNumberOfItems)もゼロにします。
#@# listSET_LIST_INTEGRITY_CHECK_1_VALUE(), listSET_LIST_INTEGRITY_CHECK_2_VALUE() マクロはArduino UnoのFreeRTOSでは特に意味はありません。
#@# 
#@# もう１つ、リストの終端に要素を追加する関数であるvListInsertEnd()も見ていきましょう。
#@# 
#@# //listnum[vListInsertEnd][list.c::vListInsertEnd()]{
#@# void vListInsertEnd( List_t * const pxList, ListItem_t * const pxNewListItem )
#@# {
#@# ListItem_t * const pxIndex = pxList->pxIndex;
#@# 
#@#     /* Only effective when configASSERT() is also defined, these tests may catch
#@#     the list data structures being overwritten in memory.  They will not catch
#@#     data errors caused by incorrect configuration or use of FreeRTOS. */
#@#     listTEST_LIST_INTEGRITY( pxList );
#@#     listTEST_LIST_ITEM_INTEGRITY( pxNewListItem );
#@# 
#@#     /* Insert a new list item into pxList, but rather than sort the list,
#@#     makes the new list item the last item to be removed by a call to
#@#     listGET_OWNER_OF_NEXT_ENTRY(). */
#@#     pxNewListItem->pxNext = pxIndex;
#@#     pxNewListItem->pxPrevious = pxIndex->pxPrevious;
#@# 
#@#     /* Only used during decision coverage testing. */
#@#     mtCOVERAGE_TEST_DELAY();
#@# 
#@#     pxIndex->pxPrevious->pxNext = pxNewListItem;
#@#     pxIndex->pxPrevious = pxNewListItem;
#@# 
#@#     /* Remember which list the item is in. */
#@#     pxNewListItem->pxContainer = pxList;
#@# 
#@#     ( pxList->uxNumberOfItems )++;
#@# }
#@# //}
#@# 
#@# //image[List_t-vListInsert][list.c::vListInsert()]{
#@# //}
#@# 
#@# pxList が要素を追加する作成済みのリスト、pxNewListItemが追加する要素です。
#@# listTEST_LIST_INTEGRITY()、listTEST_LIST_ITEM_INTEGRITY()は何もしません(list.hでマクロ定義されています)。
#@# pxIndex は pxList の最後の要素を指します。
#@# つまり最後の要素にpxNewListItemを挿入しています。
#@# pxNewListItem->pxContainer = pxList はこの要素がどのリストに含まれるかを記憶しておきます。


== キュー

キュー構造体Queue_tのコードを見てみましょう。

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

FreeRTOSのキューの特徴は以下のとおりです。

 * FIFO(First In First Out)であり、要素はその前後へのポインタをもつ
 ** FIFO は、最初にキューに入れたデータが最初に取り出される。ところてん形式です。
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

//listnum[xQueueCreate][queue.h]{
#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
    #define xQueueCreate( uxQueueLength, uxItemSize ) @<embed>$|latex|\linebreak\hspace*{5ex}$xQueueGenericCreate( ( uxQueueLength ), ( uxItemSize ), ( queueQUEUE_TYPE_BASE ) )
#endif
//}



== セマフォ

セマフォの説明をwikipediaから引用します。

https://ja.wikipedia.org/wiki/セマフォ
//quote{
セマフォ（英: semaphore）とは、計算機科学において、並列プログラミング環境での複数の実行単位（主にプロセス）が共有する資源にアクセスするのを制御する際の、単純だが便利な抽象化を提供する変数または抽象データ型である。
ある資源が何個使用可能かを示す記録と考えればわかりやすく、それにその資源を使用する際や解放する際にその記録を「安全に」（すなわち競合状態となることなく）書き換え、必要に応じて資源が使用可能になるまで待つ操作が結びついている。
//}

FreeRTOSのセマフォの特徴は以下のとおりです。

 * 1つの要素を持つキューとして実装されている
 * バイナリセマフォ、カウンティングセマフォの2種類がある
 ** バイナリセマフォは相互排他と同期に用いられる。
 ** カウンティングセマフォは、イベントのカウントやリソース管理に用いられる。
 * 周辺機器サービスで使うバイナリセマフォ
 ** バイナリセマフォでは、タスクはセマフォを取る(Take)だけで、与える(Give)ことは行わない。
 ** バイナリセマフォでは、ペリフェラルはセマフォを与える(Give)だけで、取る(Take)ことは行わない。


== タスク

最後にタスクを見ていきましょう。
https://www.freertos.org/RTOS-task-states.html にならってまずタスクの状態遷移図を描いてみます。

//image[task-task_flow][タスクの状態遷移]{
//}

タスクは次の4つの状態のいずれかに属します。

 * 実行中(Running)
 ** CPUで実行中のタスク
 * 実行待ち(Ready)
 ** 実行権限が与えられれば実行できる実行待ちのタスク
 * サスペンド(Suspended)
 ** vTaskSuspend()が呼ばれた時にこの状態に遷移する。またvTaskResume()が呼ばれたときにこの状態から遷移する。
 * ブロック(Block)
 ** この状態にいるタスクは、通常、タイムアウト時間をもち、タイムアウト時間が経つとブロック解除される。
 ** キューやセマフォを待つためにブロックされる。
 ** カラのキューからデータを読もうとすると発生する。次のデータが来るか、あるいはブロッキングタイムが経過すると解除される。
 ** fullyキューにデータを書こうとすると発生。データが来るか、ブロッキングタイムが経過すると解除される
 ** 1つのキューに対して複数のブロックタスクがあるときは優先度の高いタスクがブロック解除される


タスクを管理する構造体の@<code>{TCB_t} の宣言を見ていきましょう。

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

pxTopOfStackはタスクのスタックの一番上のアドレスを指します。
そのため、構造体の最初のメンバー変数として記述されている必要があります。
pxStackはタスクのスタックの開始アドレスです。pxStackは、タスクが使用するメモリ領域と考えてください。
スタックがアドレスの小さい方に伸びる場合、pxTopOfStack < pxStack になります。
スタックは小さいアドレスの方に伸びることが多いようです
実行待ち、ブロック、サスペンドから参照されているステートのリスト、スタックの先頭番地、優先度、タスクのスタックの先頭番地、タスク名



タスクを管理するリストがいくつかあります。

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

//listnum[prvInitialiseTaskLists][tasks.c()::prvInitialiseTaskLists()]{
static void prvInitialiseTaskLists( void )
{
UBaseType_t uxPriority;

    for( uxPriority = ( UBaseType_t ) 0U; uxPriority < ( UBaseType_t ) configMAX_PRIORITIES; uxPriority++ )
    {
        vListInitialise( &( pxReadyTasksLists[ uxPriority ] ) );
    }

    vListInitialise( &xDelayedTaskList1 );
    vListInitialise( &xDelayedTaskList2 );
    vListInitialise( &xPendingReadyList );

    #if ( INCLUDE_vTaskDelete == 1 )
    {
        vListInitialise( &xTasksWaitingTermination );
    }
    #endif /* INCLUDE_vTaskDelete */

    #if ( INCLUDE_vTaskSuspend == 1 )
    {
        vListInitialise( &xSuspendedTaskList );
    }
    #endif /* INCLUDE_vTaskSuspend */

    /* Start with pxDelayedTaskList using list1 and the pxOverflowDelayedTaskList
    using list2. */
    pxDelayedTaskList = &xDelayedTaskList1;
    pxOverflowDelayedTaskList = &xDelayedTaskList2;
}
//}

ポインタpxDelayedTaskListが指す先(実体)がxDelayedTaskList1、
pxOverflowDelayedTaskListが指す先(実体)がxDelayedTaskList2になります。
結局、タスクを管理するリストは4つあります。

 * pxReadyTasksLists[configMAX_PRIORITIES]
 ** 優先度毎にタスクリストを保持します。Arduino Uno版のFreeRTOSでは優先度は4段階(configMAX_PRIORITIESは4)です。
 * pxDelayedTaskList
 * pxOverflowDelayedTaskList
 * xPendingReadyList
 ** タスクスケジューラが一時停止(ペンディング)状態のときに実行待ち(レディ)状態に追加されたタスクが含まれます。
    タスクスケジューラが一時停止状態から抜けたときにpxReadyTasksLists[]に追加されます。
    @<img>{task-task_flow}のサスペンドからvTaskResume()が呼ばれてレディ状態になるときにチェックされます。


===[column] コラム：データ構造"が"大事
どこで聞いたかあるいは何で読んだか思い出せないのですが、
何かを実装するときには、まず最初に考えるべきはデータ構造、というお話がありました。
データ構造をうまく設計できていない場合には、機能実装が複雑になりがちでバグも生みやすくなると思います。
===[/column]