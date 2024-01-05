# 简介

本文主要对[FTE.com](https://fte.com/default.aspx)的Virtual Sniff机制进行说明和实现。

## PTS自带Bluetooth Protocol Viewer

最开始是用PTS（[PTS Release (bluetooth.com)](https://pts.bluetooth.com/download)）时，会看到PTS运行过程中有实时打印蓝牙的HCI日志，所以就想去了解怎么实现的。直接看PTS自带的Protocol View下资料很少。

![image-20240104161118995](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104161118995.png)

## FTE提供的Virtual Sniffing

还好公司有购买FTE的产品，所以这边有其他版本的安装包。东西太大了，就不传到Github上了，需要的私聊。

![image-20240104161437029](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104161437029.png)

**注意**：这里要吐槽下，蓝牙的两个方案商提供安装包都必须要联系他们厂商，而不是直接提供下载，理论上都是买设备的产商，软件有什么好限制下载的，不如好好学学Jlink这些。

安装好该软件后，看其UserManual，当然也可以打开软件后直接通过菜单栏**Help**跳转打开。

![image-20240104161644550](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104161644550.png)

会看到有一个描述**Virtual Sniffing**，这个很像我们所需的东西。

![image-20240104161749264](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104161749264.png)

进一步看，这里有些介绍，大体就是这个东西的一些历史和作用什么的，确认就是我们所需的东西，但是如何使用并没有说。

![image-20240104162015520](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104162015520.png)



## FTE Virtual Sniffing测试运行

大概知道东西是什么以后（有关键词Virtual Sniff），继续到官网上去找资料，就可以找到这个网址[Welcome to Live Import for Virtual Sniffing (fte.com)](https://fte.com/WebHelpII/LiveImport/Content/Documentation/LiveImport/WelcometoLiveImport.htm)。

![image-20240104162345794](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104162345794.png)



安装官网资料一步步看，就能看到这个页面[Installing Live Import Development Kit (fte.com)](https://fte.com/WebHelpII/LiveImport/Content/Documentation/LiveImport/InstallingLiveImport/InstallingLiveImport.htm?Highlight=Live Import Developer Kit)，

![image-20240104162612033](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104162612033.png)

直接到本地，运行这个路径下的**Live Import Developer Kit.exe**。

![image-20240104161549301](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104161549301.png)

而后就看到目录下有这些Example了，源码这些也有，dll也有了。

![image-20240104162753298](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104162753298.png)

然后这里的例程并不能直接运行，因为一些关联配置是有问题的（liveimport.ini中的ConnectionString不匹配）。所以直接Executables目录下运行他自带的Example。

![image-20240104162919155](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104162919155.png)

启动Example之前需要先打开PTS，选这个，

![image-20240104163032480](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104163032480.png)

启动Record，然后再打开Example。

![image-20240104163130025](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104163130025.png)

依次执行Example中的几个测试项目，可以看到PTS上也正确显示了。

![image-20240104163233347](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104163233347.png)



# 原理说明

在上一个章节已经实现了FTE的Virtual Sniffing功能，后续需要集成到自己的项目中，必须要理解其原理。还是看[Welcome to Live Import for Virtual Sniffing (fte.com)](https://fte.com/WebHelpII/LiveImport/Content/Documentation/LiveImport/WelcometoLiveImport.htm)提供的说明资料。

## 框架说明

这个框架图很好的说明了Virtual Sniffing功能是如何实现的[Overview - Creating Live Import Data Source (fte.com)](https://fte.com/WebHelpII/LiveImport/Content/Documentation/LiveImport/CreatingDataSource/CreatingDataSource.htm)。

Datasource，就是自己所需开发的项目，需要在这里写自己的代码；

Liveimport.ini，配置文件，这个文件是Datasource和Frontline Software所共享的配置文件，千万别自己写一套配置（之前通过exe导出的sample不能使用也是因为这里配置不同导致）。

LiveImport.dll，动态链接库，Datasource通过这个和Frontline Software进行通信。

![image-20240104163658133](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104163658133.png)

## dll接口说明

通过上述的说明就很清楚了，再看看dll提供的API如何使用[Adding Live Import to an Application (fte.com)](https://fte.com/WebHelpII/LiveImport/Content/Documentation/LiveImport/Introduction/AddingLiveImportToAnApplication.htm)，其实看完可以知道只要用到几个简单接口就行。

![image-20240104164240046](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104164240046.png)

### 路径说明

在生成的这个路径下有x64的版本，当然到各个sample也有32位的版本。

![image-20240104165005817](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104165005817.png)

![image-20240104165056805](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104165056805.png)

此外软件执行目录下也是有dll的。

![image-20240104165128432](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104165128432.png)



### load接口

就是加载dll。

![image-20240104164410719](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104164410719.png)

### initialize接口

就是将ini的参数传给dll。

![image-20240104164442145](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104164442145.png)

### send接口

最主要的接口，需要将数据包发送给FTE软件，每调用一次页面就会新增一条数据。

sample有byte发送和frame发送2种方式，平时用frame发送即可。

![image-20240104164527765](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104164527765.png)

## ini配置文件说明

再看看这个配置文件，配置了一些东西，也看不懂。

![image-20240104165218493](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104165218493.png)

其实这里有一个这样的文件，里面详细讲了各个参数的意义。

![image-20240104165250854](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104165250854.png)

仓库种贴一下这个文件内容，避免不同版本不提供之类。

当然也可以到官网去看说明：[The Configuration String (fte.com)](https://fte.com/WebHelpII/LiveImport/Content/Documentation/LiveImport/CreatingDataSource/ConfigurationString.htm)。

![image-20240104165619098](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104165619098.png)

### 路径说明

这里一定要使用FTE软件的ini配置，不能使用sample的配置，不然无法使用。因为**ConnectionString**配置不同。

![image-20240104165727890](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104165727890.png)



### ConnectionString

这个配置最重要，一定要保证FTE软件和你的Datasource配置是相同的。

![image-20240104170402672](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104170402672.png)

### Sides

这个配置也很重要，但是只要用他默认配置就行。

![image-20240104165959385](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104165959385.png)

### Stack

这个配置好像直接决定你发包格式，关联DecoderScript（[Frontline DecoderScript | Overview (fte.com)](https://fte.com/webhelpii/hsu/Content/Documentation/DecoderScript/Overview/Overview.htm)）。

试着使用了PTS的Stack，发现解析就乱码了，所以不要乱用。

![image-20240104170115359](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104170115359.png)



# 本项目实现说明

本项目基本直接抄的**Straight C Sample**的代码。

## 编译

直接运行**make all**即可。

![image-20240104171100661](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104171100661.png)

## 运行

切换到output目录下，运行**main.exe**，而后等出现提示后，按照提示输入command **send**。

![image-20240104172500178](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104172500178.png)

就可以看到这4笔包了。

![image-20240104172423859](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104172423859.png)



## 协议说明

### DecoderScript

FTE的数据包格式说明都是通过DecoderScript实现的，关于DecoderScript可以看安装包这个路径的资料。

![image-20240104174546435](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104174546435.png)

在ini文件中会定义stack信息，程序运行起来会去找匹配的解析包格式。

简单看了下，.dec是原始文件，.dex是加密后的文件，蓝牙相关都是加密的文件。

所以不能直接通过stack ID找到解析文件，进而分析协议。

所以还是从sample入手，将示例代码抄过来就行。

![image-20240104180429044](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104180429044.png)





一般蓝牙只需要常用的几种类型的包格式即可。sco目前还没试出来，后续可以试试。

示例HCI数据包。

```c
// HCI Read_BD_ADDR
// Simulate the sending of an HCI Read_BD_ADDR command to a Bluetooth Device.
BYTE sampleFrame_cmd[]= {0x09,0x010,0x00};
// Command Complete Event
// Simulate the response from a Bluetooth Device to an HCI Read_BD_ADDR command.
BYTE sampleFrame_event[]= {0x0e,0x0a,0x01,0x09,0x10,0x00,0x82,0x14,0x01,0x5b,0x02,0x00};
// Connection Complete Event
// Simulate a Connection Complete reponse from a Bluetooth Device for an ACL connection. (This establishes the connection handle for the next two buttons.)
BYTE sampleFrame_event1[]= {0x03,0x0b,0x00,0x28,0x00,0x2d,0x18,0x00,0x5b,0x08,0x00,0x01,0x00};
// Send ACL Data
// Simulate sending ACL data from a Host to a remote Bluetooth Device.
BYTE sampleFrame_acl_send[]= {0x28,0x20,0x1c,0x00,0x18,0x00,0x40,0x00,0x4d,0x65,0x73,0x73,0x61,0x67,0x65,0x20,0x74,0x6f,0x20,0x72,0x65,0x6d,0x6f,0x74,0x65,0x20,0x64,0x65,0x76,0x69,0x63,0x65};
// Receive ACL Data
// Simulate receiving ACL data from a remote Bluetooth Device.
BYTE sampleFrame_acl_recv[]= {0x28,0x20,0x1e,0x00,0x1a,0x00,0x40,0x00,0x4d,0x65,0x73,0x73,0x61,0x67,0x65,0x20,0x66,0x72,0x6f,0x6d,0x20,0x72,0x65,0x6d,0x6f,0x74,0x65,0x20,0x64,0x65,0x76,0x69,0x63,0x65};
```



### command

![image-20240104173132063](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104173132063.png)

### event

![image-20240104173227358](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104173227358.png)

### acl

![image-20240104173253500](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104173253500.png)



# PTS Bluetooth Protocol Viewer尝试

其实从上述逻辑可以看出只要拿到dll和ini就可以和FTE软件进行通信了。

ini在这个路径下。

![image-20240105094035427](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240105094035427.png)

dll在这个路径下。

![image-20240105094112672](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240105094112672.png)



## 配置差异

从ini可以看到，也就是ConnectionString和Stack不同。

ConnectionString不同直接复制过来就行。

## 尝试1-直接复制ini

Stack不同就不好处理了。前面说了，这些蓝牙的DecoderScript都是加密的dex格式，不知道里面的协议是什么。

![image-20240105094246272](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240105094246272.png)

FTE是能连接的，但是刚启动程序，还没发送数据，就会提示下面信息。

调用send，也没任何响应。

![image-20240104160355342](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104160355342.png)



## 尝试2-用示例的ini替代pts的ini

这样就不用考虑stack的问题。

FTE是能连接的，但是刚启动程序，还没发送数据，就会提示下面信息。

调用send，也没任何响应。

**看来别人就不想你用，所以还是放弃了。**

![image-20240104160355342](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240104160355342.png)





