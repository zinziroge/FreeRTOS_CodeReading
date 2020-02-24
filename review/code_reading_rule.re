= 方針

 * 最初にコードを引用し、そのあと解説を加えます
 * コメントは適宜削除することがあります
 * #if defined() ~ #endif は該当する箇所のみ記述し、適宜削除することがあります
 * Config は FreeRTOSConfig.h のデフォルト値に従います。

== ソースコードの引用方法

 * 下記のような関数があるとき、すべてを一度に引用すると見にくいので、部分的に引用して説明する。

//listnum[func][func.c::void func()]{
void func(void) {
    aaa();
    if(b) {
        bbb();
    }
    ccc();
}
//}

例えば, aaa(), bbb(), ccc() を別々に引用し説明するときは下記のように ... で省略していることを明記する。

//listnum[func_1][func.c::void func(), 1/3]{
void func(void) {
    aaa()

    ...
}
//}
 * aaa() についての説明

//listnum[func_2][func.c::void func(), 2/3]{
void func(void) {
    ...

    if(b) {
        bbb();
    }

    ...
}
//}
if(b) { bbb(); } についての説明

//listnum[func_3][func.c::void func(), 3/3]{
void func(void) {
    ...

    ccc()
}
//}
ccc() についての説明

config~ な定義は、FreeRTOSConfig.h で定義されています。