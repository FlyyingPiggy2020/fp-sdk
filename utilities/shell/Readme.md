shell的最主要的功能就是命令行解析器(command line interpreter)，一般来说由两部分组成：parser和executor。

`parser`负责扫描输入，把输入按照某个分割符(比如说空格)分割成token。复杂的还会去构建一个抽象语法树(abstract syntax tree)来执行一些逻辑。

`executor`执行器，根据parser分割后的结果去执行对应代码的。实现命令注册等功能。


目前单片机的shell无法避免的几个问题：

1.换行。由于不清楚终端的宽度，无法处理换行的问题。如果遇到键入的长度要换行。那么遇到退格键，方向键就会现实出错。

目前没有太好的办法。
