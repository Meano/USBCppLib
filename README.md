# MCU USB CPP Lib
适用于嵌入式MCU的USB Device CPP Lib

参考了Mbed对USB Device的抽象方法，和MSD的Device Interface，写了这个将USB Device拆成HAL-Device Base-Interface大致的三层进行处理，并能动态注册载入Endpoint功能的MCU CPP USB Lib。  
借助CPP的封包特性可以更好的将USB协议进行分层描述和动态处理，奈何伟大的甲方大大们可能不会写CPP，这是一份没有需求的代码。
