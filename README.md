write by lzc

Email : 544840897@qq.com

# 项目简介

这是一个C++编写的加密货币交易系统，连接okx交易所。

此项目**并非工业级**的量化交易系统，可以运行，但是有不严谨的地方。

虽然可以在实盘运行，但是作者**不建议在实盘运行**。

# 项目部署

```Bash
git clone https://github.com/58561039/crypto-quant-system.git
```



# 运行结果

运行时：

![](https://secure2.wostatic.cn/static/itJsxJLz9XhoV28mo9D8Up/image.png?auth_key=1763204551-833sztiY4kHx4V1EJRYAxb-0-84dd40b32f1e26f6fe2ddc324f15715f)

---

运行结束时：

![](https://secure2.wostatic.cn/static/8M5zNvCt9FfK33g4esrQWc/image.png?auth_key=1763204600-mDNmTfFWsPXKqsWkB9Q1C-0-371a6450d64fc473811322ee0d9bb14c)

---



运行结束时，项目内会多出.csv文件，每个品种放到各自的文件中

![](https://secure2.wostatic.cn/static/mpSDG59EFzAKnY4JdadNi9/image.png?auth_key=1763204675-hb6Xvqj2tNx9W981sbKYWp-0-8a111a8b84bebe12b9f260738ae46438)

---

下面是针对.csv文件的统计，是作者自行将数据处理之后得到的结果



![](https://secure2.wostatic.cn/static/eufFzZha7zdnysrWBRZ9SR/btc.png?auth_key=1763206546-nqjNbErUJVSFYjTtvtnoRB-0-ee99595a81745a66a4154f125b8b3ac8)

---

![](https://secure2.wostatic.cn/static/9tpXZ9tax3pcGiTmaDonFs/eth.png?auth_key=1763206551-wMQGVvx6vXc6qYKZ5gGGJA-0-80212333400f43b0fae9a6138ab848e4)

---

![](https://secure2.wostatic.cn/static/3NzoqJJ55ZuZ7WSJHxChcf/sol.png?auth_key=1763206555-7iaenfgTXfABKJB9a3i4vU-0-940b4759a304a885ebcf4d60344e9697)

不出意外的全部处于亏损状态



# 项目环境

### **安装库文件**

在`linux`环境下运行，使用`C++17`标准

下面是项目用到的库，以及安装库的指令

1. **Boost.Beast** 和 **Boost.Asio** - 用于 WebSocket 通信和异步网络 I/O

```Bash
sudo apt install libboost-all-dev
```
2. **OpenSSL** - 用于 SSL/TLS 加密、HMAC 签名和 Base64 编码

```Bash
sudo apt install libssl-dev
```
3. **libcurl** - 用于 HTTP REST API 请求

```Bash
sudo apt install libcurl4-openssl-dev
```
4. **nlohmann/json** - 用于 JSON 数据解析和处理

```Bash
sudo apt install nlohmann-json3-dev
```
5. **C++17 编译器** - 需要支持 C++17 标准的编译器（g++ 7.0+ 或 clang++ 5.0+）



### **编译前**

编译前请打开`main.cpp`，填写您的API

```C++
//使用之前，先填写apiKey，secretKey，passphrase
//实盘就写实盘的API，模拟盘就写模拟盘的API
std::string apiKey     = "Your apiKey";
std::string secretKey  = "Your secretKey";
std::string passphrase = "Your passphrase";
bool isdemo = true;//isdemo=true时连接okx的模拟盘，isdemo=false时连接okx的实盘
```



—如图所示，在模拟交易中，点击`申请模拟交易 V5 API`，创建API

—在创建的API中，点击`查看`

![](https://secure2.wostatic.cn/static/r1Zo4rrPk9cgwJA8F6xoBi/image.png?auth_key=1763207133-kqeStF7aK6TqjbmbT4WJHw-0-8d4d31e430a421a1104aa7ae633e2e57)



—复制您在okx模拟交易中创建的API到`main,cpp`中

`apiKey`=API key

`secretKey`=密钥


`passphrase`=您创建API时设置的密码



![](https://secure2.wostatic.cn/static/jAA5B3oa8aVd666orsFdrn/image.png?auth_key=1763207942-fnC7KeSPLjEMfQiRC63ukj-0-17fdf99c3d7cc18e6d738b9b38490444)




### 网络VPN

确保linux环境下可以访问到国外的服务器，可以在终端中使用以下命令查看能否访问到okx服务器

```Bash
curl -s "https://www.okx.com/api/v5/market/ticker?instId=BTC-USDT-SWAP" | grep -oP '"last":"\K[0-9.]+'

```

若输出BTC-USDT-SWAP价格，则证明可以访问到okx服务器

```Bash
95812.4
```

### 编译

在终端中

```C++
make
```

### 运行

```C++
./quant
```





# 项目讲解

### 项目架构

![](https://secure2.wostatic.cn/static/cSRaik1Fce7H49mthzmN2/drawio.png?auth_key=1763207133-2SKmRb67AEEJCNDq9qeJxg-0-63e3d451db6d3417103e62cf7805340c)




`websocket`与`order_manager`的数据更新采用**观察者模式**

`websocket` ，`okxrestclient`，采用**单例模式**





### websocket讲解

1.okx交易所提供了多个websocket频道可供订阅，每个websocket都有各自的**服务地址**

例如**账户频道**的**服务地址**为：`/ws/v5/private (需要登录)`

本项目共订阅了三个频道

|频道|服务地址|
|-|-|
|candle1m|/ws/v5/business|
|orders|/ws/v5/private (需要登录)|
|orders-algo|/ws/v5/business (需要登录)|




`candle1m`：订阅的是一分钟K线数据

`orders`：订阅的是成交数据，开仓成交时会推送，止盈止损成功时会推送

`orders-algo`：订阅的是止盈止损信息，止盈止损时会推送



`orders`：在止盈止损时返回的是成交价格，手续费，收益等数据

`orders-algo`：在止盈止损时会返回当前的订单是止盈还是止损。本频道主要是用于策略委托订单。



每个派生的websocket都要重写`set_path`函数，用于设置**服务地址**

每个派生的websocket都要重写`need_login`函数，用于判别该websocket是否需要登录使用

REST 和 WebSocket 都使用同样的签名规则：

**sign = Base64( HMAC_SHA256(secretKey, preHashString) )**

而 preHashString 的格式是：

```text
timestamp + method + request_path + body

```

**补充说明**：虽然在okx的API中，可以用websocket下单，但是websocket并不适合用来下单，并没有提供像REST那样多种参数可供选择。websocket的首选还是推送数据，



## 策略讲解

策略是很简单的策略，只看最近三根K线

本项目订阅的是`一分钟K线`数据，并非`ticker`数据或`trades`数据，只有在收K时才会判断有没有交易信号。

当收K时判断前三根K线，

如果三根K线都是阳K，则入场做多，止损放在第一根K线的最低价

如果三根K线都是阴K，则入场做空，止损放在第一根K线的最高价

根据止损的位置判断止盈的位置，默认1：1的盈亏比

如图所示：

![](https://secure2.wostatic.cn/static/gWab8XZv6EBQEvZaAKL8u8/image.png?auth_key=1763262991-gWgNcsUafexWZiZT5TPPGL-0-42a66c414d43cf167735419a78369db3)



## 仓位大小

所有的币种，仓位默认的sz=1.0

$$
\text{开仓量} = \text{sz} \times \text{合约面值} \times \text{开仓均价}
$$


$$
\text{开仓成本} = \frac{\text{开仓量}}{\text{杠杆大小}}
$$




每个币种都有一个固定的合约面值，例如BTC的合约面值是0.01

那么我发送给okx的请求中，我默认设置sz=1，开仓均价是100000 USDT，那么我的开仓量就是1000 USDT。杠杆是10倍杠杆，我的开仓成本就是100 USDT。

如果我该仓位净盈利（去掉手续费）是5 USDT，那么我这笔仓位的盈亏率就是5%



所以在添加交易币种时，需要先访问okx，确定该币种的合约面值



## 线程

本项目有**七个**线程

三个websocket中的run()函数使用三个线程

每个websocket都需要一个心跳线程，每隔10s会像交易所发送"`ping`"，提示交易所本连接未断开。

okx交易所的规则是每隔25s，客户端要发送一个"`ping`"，服务器回应一个"`pong`"，否则就会断开连接不在推送数据。

每隔10s发送，而不是每隔20s或25s发送，是为了防止因网络阻塞导致一个"`ping`"未到达服务器，进而导致连接断开。



还有一个主线程，等待用户输入"`quit`"



## okxrestclient.h（REST）

用于实现POST和GET请求

提供的接口如下：

|函数|功能|
|-|-|
|getContractValue|获取合约面值|
|placeOrderWithTPSL|下单，附带止盈止损|
|closeOrderMarket|市价全平|
|setLeverage|设置杠杆大小|
|getKlines|获取历史K线(程序刚开始时)|


# To-do-list

本项目虽然可以运行，但还有很多不完善的点。待完善的内容如下：

-仓位控制

-websocket的断线重连机制

-websocket更新数据时，不管是哪个websocket，都应该封装成一个事件结构体，通知观察者时应该用事件作为参数。

-熔断机制

-可以做的品种不应该仅限永续合约

