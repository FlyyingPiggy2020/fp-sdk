shell的最主要的功能就是命令行解析器(command line interpreter)，一般来说由两部分组成：parser和executor。

`parser`负责扫描输入，把输入按照某个分割符(比如说空格)分割成token。复杂的还会去构建一个抽象语法树(abstract syntax tree)来执行一些逻辑。

`executor`执行器，根据parser分割后的结果去执行对应代码的。实现命令注册等工鞥。
