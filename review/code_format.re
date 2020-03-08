= コードリーディングの前に

FreeRTOSのコードリーディングを始める前に事前に知っておいた頂いた方がよい思われる事項や前提条件をいくつか提示します。

== 対象環境
コードリーディングを進めていくFreeRTOSのバージョンや対象とするハードウェアは次のとおりです。

 * Arduino_FreeRTOS_Library
 ** version: 10.2.0-3
 ** https://github.com/feilipu/Arduino_FreeRTOS_Library.git
 * ハードウェアとMPU
 ** Arduino Uno(ATMega328p)
 * Arduino IDE
 ** version: 1.8.10

== 環境準備
=== Arduino IDEのインストール
各OS向けのインストール案内はhttps://www.arduino.cc/en/Main/Softwareに記載されています。
Windows版は以下の3種類がありますので環境に合わせてお好きなものを選べばよいと思います。
この本ではWindows Installer版でインストールしたArduino IDEを使用しています。

 * Windows Installer
 * Zip版
 * Microsort Store

=== FreeRTOSのインストール
インストールしたArduino IDEを起動し、
メニューを"スケッチ" → "ライブラリをインクルード" → "ライブラリを管理" に辿ると
ライブラリマネージャが表示されます。

//image[arduino_ide_menu][タスクの状態遷移]{
//}


ライブラリマネージャが表示されたら、"freertos" で検索し、"バージョンを選択"から"バージョン10.2.0-3"を選択して、
インストールを押してください。

//image[arduino_ide_lib_manager][タスクの状態遷移]{
//}

== FreeRTOS
=== 概要

FreeRTOS(https://www.freertos.org/)について触れる前にまずRTOSの説明をWikipedia(https://ja.wikipedia.org/wiki/リアルタイムオペレーティングシステム)から引用します。

//quote{
リアルタイムオペレーティングシステム（英: Real-time operating system;RTOS）は、
リアルタイムシステムのためのオペレーティングシステム (OS) である。組み込みオペレーティングシステムに多い。
OSの主要な機能である資源管理において、時間資源の優先度に基づく配分と実行時間の予測可能性を提供することに特化している、
ないし、そういった機能に力を入れている。
//}

Wikipediaからさらに引用します。

//quote{
 RTOSへの要求には、以下のようなものが挙げられる。

 * ユーザーアプリケーションから、待ちが発生しないサービスコールを呼出した場合に要する最悪値
 * ハードウェア割り込みが発生してから、処理ルーチンを呼出すまでの最悪値
 * 高優先度のタスクが確実に実行されることを保証するスケジューリング規則
 * RTOSは、時間制約を保証できる設計を実現するために使われる。以上の要求は、そのために必要な事項の一例である。
//}

FreeRTOSの特徴についてオフィシャウエブサイト(https://www.freertos.org/RTOS.html)から幾つか引用、要約します。

 * 多数のアーキテクチャやツール群への１つの独立した解を提供する
 * コンパクト。典型的なカーネルのバイナリサイズは6kB～12kBに収まる。。
 * シンプル。コアは3ファイルしか含まない

OSの大きな役割の1つにリソース管理があります。リソースとは、プロセス、ファイル、メモリ、入出力などが含まれます。
FreeRTOSは、少なくともこの本で取り扱うArduino UNo版では、特にプロセス管理(以降、タスク管理として記載する)が主な役割となっています。
タスク管理は、一定時間(Tick)毎にタスクを切り替え、MPUの計算リソースをタスクに割り当てます。

少なくともArduino Uno版のFreeRTOSを使用する場合、
FreeRTOSが管理するタスク以外にArduinoのスケッチファイルのloop()関数でも処理を定期的に実行することができます。
しかし、FreeRTOSを使う場合はloop()関数内はカラにしてタスクとして実装し、する方が好ましいです。

また、FreeRTOSは、AWS re:invent 2017でAmazonがReal Time Engineersから買収することを発表され、
ライセンス形態も買収後のバージョン10からMITライセンスに変更されました。
商用アプリケーションにも自由に使用できます。詳細はライセンスを確認してください。



=== ソースコードファイル一覧
Arduino Uno版のFreeRTOSに含まれるソースコードファイル名とその概要は次のとおりです。
この本で扱うソースコードファイルと、ほとんどまたは全く扱わないものもあります。

//table[file_list_summary][ファイル一覧と概要]{
ファイル名          	概要
-----------------------------------------------------
Arduino_FreeRTOS.h  	Arduino Uno固有の定義
croutine.c          	(この本では扱いません)
croutine.h          	(この本では扱いません)
event_groups.c      	(この本では扱いません)
event_groups.h      	(この本では扱いません)
FreeRTOSConfig.h    	FreeRTOSのコンフィグレーション(設定)
FreeRTOSVariant.h   	FreeRTOSConfig.hに含まれない設定
heap_3.c            	ヒープ領域を取り扱う処理
list.c              	リストの実装
list.h              	リストの宣言
message_buffer.h    	(この本では扱いません)
mpu_wrappers.h      	(この本では扱いません)
port.c              	移植時に修正が必要なアセンブラ記述やメモリのアライメントなど
portable.h          	移植時に修正が必要なアセンブラ記述やメモリのアライメントなど
portmacro.h         	移植時に修正が必要なアセンブラ記述や基本型のサイズなど
projdefs.h          	プロジェクト毎の定義など
queue.c             	キューの実装
queue.h             	キューの宣言
semphr.h            	セマフォの宣言。実装はキューが用いられている。
stack_macros.h      	(この本では扱いません)
stream_buffer.c     	(この本では扱いません)
stream_buffer.h     	(この本では扱いません)
task.h              	タスク処理の実装
tasks.c             	タスク処理の宣言
timers.c            	(この本では扱いません)
timers.h            	(この本では扱いません)
variantHooks.cpp    	何らかの要因でフックされる処理。initVariant()が記載されている。
//}


=== 変数命名規則
FreeRTOSの変数名の命名規則は次のとおりです。
 * 'c' : char
 * 's' : int16_t
 * 'x' : BaseType_t
 * 'p' : ポインタ
 * 'u' : unsigned

上記は複数同時に使用できます。
例えば、変数名@<code>{pucStackByte}はunsignedのポインタを意味します。


=== 関数命名規則
FreeRTOSの関数名の命名規則は次のとおりです。
 * 'v' で始まる関数の戻り値は void
 * 'p' で始まる関数の戻り値は pointer
 * 'x' で始まる関数の戻り値は BaseType_t
 * Task で始まる関数の実体は、tasks.c に記述されている
 ** pvTimerGetTimerID() は void pointerを返す関数でtimers.cで定義されている
 * 'prv' で始まる関数は プライベート(private)関数


== コードリーディングの方針

 * 最初にコードを引用し、そのあと解説を加えます
 * コメントは適宜削除することがあります
 * #if defined() ~ #endif やif() ~ else if() ~ else は該当する箇所のみ記述し、適宜削除することがあります
 * コンフィグは FreeRTOSConfig.h記載のデフォルト値に従います。

=== ソースコードの引用方法

次のような関数があるとき、すべてを一度に引用すると見にくいので部分的に引用して説明することがあります。
この関数は、hoge.cから引用したfunc()という関数であることを表しています。
ソースコードのリスト名では、関数の引数と戻り値の型は省略します。

//listnum[func][hoge.c::func()]{
void func(void) {
    aaa();
    if(b) {
        bbb();
    }
    ccc();
}
//}

例えば, aaa(), bbb(), ccc() を別々に引用し説明するときは下記のように ... で省略していることを明記する。

//listnum[func_1][hoge.c::func(), 1/3]{
void func(void) {
    aaa()

    ...
}
//}

ソースコードのリスト名の終端に含まれる 1/3 は、func()の説明を3つに分け、その1番目の説明であることを示します。
ここでは、aaa() についての説明をします。

//listnum[func_2][hoge.c::func(), 2/3]{
void func(void) {
    ...

    if(b) {
        bbb();
    }

    ...
}
//}
ソースコードのリスト名の終端に含まれる 2/3 は、func()の説明を3つに分け、その2番目の説明であることを示します。
if(b) { bbb(); } についての説明をします。

//listnum[func_3][hoge.c::func(), 3/3]{
void func(void) {
    ...

    ccc()
}
//}
ソースコードリストの終端に含まれる 3/3 は、func()の説明を3つに分け、その3番目の説明であることを示します。
ccc() についての説明をします。

また、ソースコードがページの横幅に収まらないときには改行を追加したり、空白文字を削除する場合があります。
