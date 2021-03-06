[@zinziroge](https://twitter.com/zinziroge) 訳
序文、第1章のみの日本語訳。
161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf

# Mastering FreeRTOS Real Time Kernel ハンズオン・チュートリアルガイド
Richard Barry 著


Caroline, India、そして Maxへ捧ぐ。

# 序文

## 小規模組込みシステムでのマルチタスク

### FreeRTOSとは

### 価値命題

### 用語に関する注意
FreeRTOSでは、実行中の各'スレッド'は、'タスク'と呼ばれる。組込み界隈内で用語に合意事項はないが、私は'スレッド'より'タスク'を好む、というのはスレッドは、いくつかのアプリケーションの分野で、より多くの特別な意味を持つからだ。

### なぜリアルタイム・カーネルを使うか？

- タイミング情報を抽象化する
- メンテナンス性／拡張性
- モジュール性
- チーム開発
- 簡単なテスト
- コード再利用
- 改善された効率
- アイドルタイム
- パワーマネージメント
- 柔軟な割り込みハンドラ
- 混合処理の要件

### FreeRTOSの機能
FreeRTOSは以下の標準的な機能を持つ。
- 
- 非常に柔軟なタスク優先度割り当て
- 柔軟で速くそして軽いタスク通知機構
- キュー
- 2値セマフォ
- カウンティング・セマフォ
- 排他制御
- 再帰排他制御
- ソフトウェアタイマ
- イベントグループ
- Tickフック関数
- アイドルフック関数
- スタックオーバーフロー検出
- トレース記録
- タスク・ランタイム・統計量収集
- オプションの商用ライセンスとサポート
- フル階層割り込みモデル（幾つかのアーキテクチャにおいて）
- 極端な省電力アプリケーションへ対応できるtick-less能力
- 適当なケースでのソフトウェア制御割り込みスタック（これはRAM使用量を抑える。）

### xxx, ライセンス、FreeRTOS、OpenRTOS、SafeRTOSファミリ
FreeRTOS オープンソースライセンスは、
1. FreeRTOSは商用アプリケーションに使用できる
2. FreeRTOSそれ自体は、すべての人にフリーで利用できる。
3. FreeRTOSユーザーは、知的財産の所有権を保持する。
最新のオープンライセンス情報は[http://www.FreeRTOS.org/license](http://www.FreeRTOS.org/license)を参照のこと。

OpenRTOSは、FreeRTOSの商用ば版ライセンスで、サードパーティーのRealTime Engineers LTc.から供給される。

SafeRTOSは、FreeRTOSと同じ使い方モデルを共有する。しかし、FreeRTOは実践や手順に従い開発されてきた。
しかしながら、FreeRTOSは実践と手順と必要性を処理し、主張するプロセスの必要性な様々な国際的に認識された標準にかかわる安全性の順守を主張する。

## 含まれるソースファイルとプロジェクト

### この本に付随するサンプルを入手する

# 第1章 FreeRTOSの頒布形態 

# 1.1 章紹介と対象範囲
FreeRTOSは1つのzipアーカイブファイルとして配布され、全ての公式のFreeRTOSのポーツと、多くのコンフィグ済みのデモアプリケーションを含む。

## 対象範囲
この章は、ユーザーがユーザー自身をFreeRTOSのファイルとディレクトリに向かうことを助ける。
- FreeRTOS ディレクトリ構造の最上位のViewを提供する
- どのファイルが実際に任意のFreeRTOSプロジェクト必要とされるかを説明する
- デモアプリケーションを紹介する
- 新規プロジェクトがどのように作成されるかの情報を提供する

ここでの説明は公式FreeRTOS頒布形態のみに関連する。この本のサンプルは若干異なる仕組みを使う。

# 1.2x FreeRTOSの配布を理解する
### 定義: FreeRTOS ポーツ
FreeRTOS は約20の異なるコンパイラでビルドできる。また、30以上の異なるプロセッサアーキテクチャで動作する。サポートされたそれぞれのコンパイラとプロセッサの組み合わせは、別々のFreeRTOSポーツと考える。

### FreeRTOSをビルドする
### FreeRTOSConfig.h
### 公式FreeRTOS頒布形態
### FreeRTOS頒布形態の最上位ディレクトリ
### すべてのポーツに共通な FreeRTOS ソースファイル
- queue.c
- timers.c
- event_groups.c
- croutine.c

### ポーツに特有の FreeRTOS ソースファイル

### インクルードパス
FreeRTOSは3つのディレクトリを必要とし、それらはコンパイラのインクルードパスに含まれる。それらは以下の通り。
1. コアFreeRTOSヘッダーファイルへのパス。これは常にFreeRTOS/Source/includeである。 
2. FreeRTOSポーツ特有のソースファイルへのパス。上述したように、このパスはFreeRTOS/Source/portable/[compiler]/[architecture]である。
3. FreeRTOSConfig.h ヘッダーファイルへのパス

### ヘッダーファイル
FreeRTOS APIを使用するソースファイルは、FreeRTOS.hをインクルードしなければならない。そのあとに、使用されるAPI関数へのプロトタイプを含むヘッダーファイル、task.h, queue.h, semphr.h timers.h, event_groups.h のいずれか、が続く。

# 1.3x デモ・アプリケーション
各FreeRTOSポーツは少なくとも1つのデモアプリケーションを伴い、それはエラーやワーニングを生成することなくビルドすべきである。しかし、幾つかのデモは古く、時々、デモのリリース後のビルドツールの変更が問題の原因となりうる。

Linuxユーザへの注意：FreeRTOSはWindowsホストで開発、検証されている。デモプロジェクトがLinuxホストでビルドされる場合に、時折、これがビルドエラーになる。ビルドエラーはほとんど常に、ファイル名を参照するときの大文字小文字の使用や、ファイルパスで使用されるスラッシュ文字の向きに関連している。FreeRTOS のコンタクトフォーム [http://www.FreeRTOS.org/contact](http://www.FreeRTOS.org/contact)を使って、我々にそのようなエラーについて警告して欲しい。

デモアプリケーションはいくつの目的がある。
- 正しいファイル群を含み、正しいコンパイラオプション群とともに、動作例とプレコンフィギュアされたプロジェクトを提供すること。
- 最小限の設定や事前知識で枠を超える('out of the box')経験を可能にする
- FreeRTOS API がどのように使用できるかのデモンストレーション
- 実際のアプリケーションが生成されるベース



# 1.4x FreeRTOSプロジェクトを作成する

# 1.5 データタイプとコーディングスタイルガイド
## データタイプ
FreeRTOSの各ポーツは、固有のportmacro.hヘッダーファイルを持ち、そのファイルは（特に）2つのポート特有のデータタイプ: TickType_tとBaseType_tを含む。これらのデータタイプは、表2で説明される。

表2: FreeRTOSで使われるポート特有のデータタイプ

|マクロまたは使用されるtypedef|実際の型|
|---|---|
|TickType_t|FreeRTOSは、tick interruptと呼ばれる時間間隔を設定する。tick interruptはFreeRTOSアプリケーションが始まってから発生し、その数はtick periodと呼ばれる。Timesは、複数のtick periodとして指定される。TickType_tはtick count値を保持するために使用されるデータ型であり、時間を特定する。TickType_tは、符号無し16bit型または符号無し32bit型であり、FreeRTOSConfig.h内のconfigUSE_1_BIT_TICKSの設定に依存する。もし、configUSE_16_BITが1に設定されている場合、TickType_tはuinit16_tとして定義される。もし、configUSE_16_BITが0に設定されている場合、TickType_tはuint32_tとして定義される。uint16_t型を使用することは、8bitないし16bitアーキテクチャでは非常に効率を改善する。しかし、制約される最大ブロック期間を非常に制限する。32bitアーキテクチャで16bit型を使用する理由はない。|
|BaseType_t|これは常にアーキテクチャにとって最も効果的なデータ型として定義される。典型的には、32bitアーキテクチャでは32bit型、16bitアーキテクチャでは16bit型、そして8bitアーキテクチャでは8bit型である。BaseType_tは、一般的に戻り値の型として使用され、値の非常に限定された範囲のみを用いる。pdTRUE/pdFALSE型はBoolean(2値)である。
|

幾つかのコンパイラは全ての修飾されていないchar変数を符号無しにする、一方他のコンパイラは符号有りにする。このため、FreeRTOSソースコードはすべてのcharの使用を明示的に'signed'または'unsigned'で修飾する。ただし、charがアスキー文字を保持するために使用されている場合や、charへのポインターが文字列を指すために使用されている場合は除く。

int型そのものは決して使用されない。

## 変数名
変数は、型ごとにプレフィックスされる。：'c'はchar, 's'は int16_t(short), 'l'は int32_t(long), 'x'はBaseType_t と他の標準外の型（構造体、タスクハンドラ、キューハンドラなど）

もし変数が符号無しの場合、さらに'u'でプレフィックスされる。もし、変数がポインタの場合、さらに'p'でプレフィックスされる。例えば、uint8_t型の変数は'uc'でプレフィックスされる。また、charポインタ型の変数は'pc'でプレフィックスされる。

## 関数名
- vTaskPrioritySet() は、voidを返し、task.c内で定義されている。
- xQueueReceive() は、BaseType_t型の変数を返し、queue.c内で定義されている。
- pvTimerGetTimerID() はvoidポインタを返し、timers.c内で定義されている。

ファイルスコープ（プライベート）関数は、prvで始まる。
1つのタブは常に4つのスペースと同じである。

## マクロ名
多くのマクロは、大文字で記述され、小文字で始まり、それはマクロが定義されている場所を示している。表3は、プレフィックスのリストを提供する。

表3：マクロのプレフィックス
|プレフィックス|マクロ定義の場所|
|---|---|
|port(例えば、portMAX_DELAY)|portable.h または portmacrot.h|
|task(例えば、taskENTER_CRITICAL())|task.h|
|pd(例えば、pdTRUE)|projdefs.h|
|config(例えば、configUSE_PREEMPTION)|FreeRTOSConfig.h|
|err(例えば、errQUEUE_FULL)|projdefs.h|
|

備考：セマフォAPIは、ほとんど一連のマクロとして記述されているが、マクロのネーミング規則ではなく、関数のネーミング規則に従う。

表4で定義されているマクロは、FreeRTOSソースコード全体を通じて使用される。

表4:共通マクロ定義
|マグロ|値|
|---|---|
|pdTRUE|1|
|pdFALSE|0|
|pdPASS|1|
|pdFAIL|0|
|

## 過度な型変換の理由
FreeRTOSソースコードは、多くの異なるコンパイラでコンパイルできる。全てのコンパイラは、どのように、いつワーニングを出力するかは異なる。特に、異なるコンパイラは型変換(casting)を異なる方法で使用されることを求める。結果として、FrreRTOSソースコードは、通常保証される以上の型変換を含んでいる。