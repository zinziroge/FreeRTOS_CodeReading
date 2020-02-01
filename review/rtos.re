= RTOS

https://www.freertos.org/implementation/main.html 
の副読本

RTOSは、Real-Time Operating System の略であり、特に組込み機器のOSで重視される処理のリアルアイム性に重きを置いたOSである。
ここで言うリアルタイム性とは、あるタスクが所定時間内で処理を終了すること保証することを意味する


== FreeRTOS
OSは非常に大雑把に言えば、コンピュータのリソース管理を行うプログラムであり、リソース管理としては以下のような種類がある。

 * メモリ管理
 * プロセス管理
 * ファイル管理
 * 入出力管理

FreeRTOSは、主にプロセス管理を重視している。


AWS re:invent 2017でAmazonがReal Time Engineersから買収することを発表した。
ライセンス形態も買収後のバージョン10からMITライセンスに変更された。
https://www.freertos.org/
