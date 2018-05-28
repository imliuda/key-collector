# 设计结构

对于监控领域，已经存在许多数据采集的软件，如zabbix agent，osquery，telegraf，collectd，nagios等等，还有阿里等大公司开源的类似产品。这些软件都是为服务器监控的设计的。而随着iot的发展，物联网设备必然也会有运维的需求。而上述这些软件或多或少都存在一些缺点，例如：

* 采用顺序调度，执行时间会依次推迟（zabbix和osquery）。
* 结构复杂，难于扩展（osquery和telegraf）。
* 库依赖多，难于部署（因为客户环境复杂，如osquery，collectd）。
* 体积大，难于应用到嵌入式设备上（osquery和telegraf）。

针对于这些软件目前存在的问题，以及物联网运维潜在的市场，我设计了osclt，它并不是完全的从零开始，而是吸纳现有软件的优点，规避缺点，设计出一个我期望的尽可能完美的方案。

**osclt的设计目标是做到简单,可靠，易于扩展。能够适用于不同的监控领域。**

* 简单：功能简单，只负责数据采集和数据发送，不做过多的处理。同时体积小巧，以便能够部署到各种嵌入式设备上。
* 可靠：程序能够稳定运行，数据准确，不丢数据。
* 易于扩展：增加新功能简单，方便用户添加自定义采集功能。

## 程序结构

osclt的结构图如下：

![osclt arch](images/osclt_arch.png)

osclt主要由config，collect，process，cache和output五个核心功能模块组成，collect，process和output模块可由插件扩展。核心模块的作用分别如下：

### config模块

负责解析osclt.conf和osclt.conf.d目录下的配置文件，用于初始化程序全局配置和各个模块的配置。collect模块根据配置启动不同的采集功能模块，process根据配置对数据进行加工处理，output根据配置输出到不同的目标。

### collect模块

collect模块负责数据采集，用户可扩展该模块实现自己的采集，osclt默认提供exec和listener插件。exec模块负责调用外部的程序或命令，将命令的输出作为采集结果传递给process模块。listener模块监听unix socket连接，oscltput程序通过这个连接向osclt异步写入数据，如systemd服务的`ExecStop=/usr/lib/osclt/bin/ocput service,service=nginx stop=true`。

### process模块

process模块负责对采集到的数据进行加工处理。如果配置文件中未配置process选项，则由process模块不对数据进行任何处理，直接放入cache的metrics表中。若配置了process函数，则根据情况选择是否放入到history和metrics表中。

### cache模块

cache负责数据的缓存，cache采用sqlite实现，其优点是接口简单，并且支持内存数据库和文件数据库方式。cache中包含两个表：history表和metrics表。history表用来存储原始数据，为process模块提供临时缓存。metrics表负责存储等待发送的数据。metrics表中的数据不可丢失，如果采用内存数据库方式，程序退出时，会将metrics表中的数据保存到文件中，再次启动时加载到内存中。cache模块为osclt内部使用，不可扩展。

### output模块

output模块负责数据的输出，可将数据保存到本地文件，InfluxDB，zabbix server等。数据成功保存后，会在metrics表中删除这些数据，如果超过一定时间数据都未发送成功，可以自动删除。output模块可配置为有数据就执行发送操作，也可配置按固定周期发送数据，以便一次请求能够传输多条数据。

## 数据格式

数据格式采用[InfluxDB Line Protocol](https://docs.influxdata.com/influxdb/v1.4/write_protocols/line_protocol_reference/)。该数据格式的优点是每条数据都包含了完整的可自描述的信息。每条数据都包含名称，标签，键值对和时间戳信息。名称表示的测量指标的名称，如cpu\_usage；标签用于描述该条数据的属性信息，如设备标识，位置；键值对表示采集的该指标项的相关值；时间戳表示该数据的生成时间。

osclt的exec插件要求调用的外部命令都是该数据格式。如获取物理cpu信息的命令cpu\_basic输出：

```
cpu_basic manufaturer="intel",freq=2400,l2_c=2048,l3_c=10240,core=2
```

命令中输出的数据中，必须包含名称和指标项的键值对，标签和时间戳是可选的。如果输出的同一个类型数据的多条数据，如cpu使用率，多个处理器的信息分别用cpu=0，cpu=1等标签代表不同的处理器，如：

```
cpu_usage,cpu=0 user=366,system=127,idle=6576,nice=0,irq=23,iowait=356
cpu_usage,cpu=1 user=366,system=7,idle=6876,nice=0,irq=3,iowait=456
cpu_usage,cpu=2 user=66,system=27,idle=6856,nice=0,irq=23,iowait=346
cpu_usage,cpu=3 user=366,system=17,idle=657,nice=0,irq=3,iowait=356
```

如果时间戳信息省略，则osclt会自动添加程序结束时的时间信息。除exec模块，其他的collect模块也可省略标签和时间戳信息。

## 可扩展性

osclt的collect模块，process模块和output模块可通过插件进行扩展，插件可静态编译或动态链接方式运行。除了核心模块扩展的扩展，最重要的是采集功能的扩展，用户可编写独立的程序进行采集功能扩展，通过简单的增加可执行程序和配置文件的方式实现数据采集的扩展。同时，由于涉及硬件的程序可能会出现异常，通过独立各个采集程序，一定成都上增加了osclt的问题性。
