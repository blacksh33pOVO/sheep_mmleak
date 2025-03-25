##实现了一个内存泄露检测组件

#方式1：宏定义
#方式2：dlsym函数，hook

#共同点 将malloc出来的内存空间，用地址命名，每 malloc一次就在在block文件夹里面创建一个文件,
        文件信息包含1.[filename, funcname, line]、内存地址， 内存大小
                    2.[caller]、内存地址， 内存大小
#调用free之后，会删除block文件夹里面对应地址名字的文件

#block文件夹里剩下的文件是未被free释放的内存地址
