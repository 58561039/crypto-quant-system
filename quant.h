
/*
write by lzc
Email: 544840897@qq.com
*/

#pragma once


#include "include/websocket/public_websocket.h"
#include "include/websocket/orders_websocket.h"
#include "include/websocket/orders_algo_websocket.h"
#include"include/order.h"
#include"include/okxrestclient.h"
#include"include/callback.h"



class Quant{
public:
    static Quant& GetInstance(){
        static Quant instance;
        return instance;
    }

    //初始化，三个websocket，并设置回调函数
    void init(const std::string apiKey,const std::string secretKey,const std::string passphrase,const bool isdemo){
        apiKey_=apiKey;
        secretKey_=secretKey;
        passphrase_=passphrase;
        isdemo_=isdemo;

        auto& client = OkxRestClient::GetInstance();
        client.SetSettings(apiKey_, secretKey_, passphrase_, isdemo_);

        auto& pub=PublicWebSocket::GetInstance();
        auto& ords=OrdersWebSocket::GetInstance();
        auto& algo=OrdersAlgoWebSocket::GetInstance();


        pub.settings(apiKey_, secretKey_, passphrase_, isdemo_);
        ords.settings(apiKey_, secretKey_, passphrase_, isdemo_);
        algo.settings(apiKey_, secretKey_, passphrase_, isdemo_);

        pub.set_on_message(public_websocket_callback);
        ords.set_on_message(orders_websocket_callback);
        algo.set_on_message(orders_algo_websocket_callback);
    }


    //添加交易品种
    void add_symbol(const std::string& symbol, double leverage){
        auto manager = std::make_shared<OrderManager>(symbol);
        manager->setLeverage(leverage);

        auto s1 = std::make_shared<ThreeKlineStrategy>();
        manager->SetStrategy(s1);

        auto mtx = std::make_shared<std::mutex>();

        ordermanager_map[symbol] = manager;

        auto& pub=PublicWebSocket::GetInstance();
        auto& ords=OrdersWebSocket::GetInstance();
        auto& algo=OrdersAlgoWebSocket::GetInstance();

        pub.add_order_manager(symbol, manager, mtx);
        ords.add_order_manager(symbol, manager, mtx);
        algo.add_order_manager(symbol, manager, mtx);

        manager->parseHistoryData(
            OkxRestClient::GetInstance().getKlines(symbol)
        );
    }

    //连接交易所并订阅数据
    void connect_and_subscribe(){
        auto& pub=PublicWebSocket::GetInstance();
        auto& ords=OrdersWebSocket::GetInstance();
        auto& algo=OrdersAlgoWebSocket::GetInstance();


        pub.connect();
        ords.connect();
        algo.connect();

        for(auto &e:ordermanager_map){
            pub.subscribe_candle_1m(e.first);
        }

        ords.subscribe_orders();
        algo.subscribe_orders_algo();
    }

    //quant内部有三个线程，分别运行三个websocket
    void run(){
        auto& pub=PublicWebSocket::GetInstance();
        auto& ords=OrdersWebSocket::GetInstance();
        auto& algo=OrdersAlgoWebSocket::GetInstance();


        t1=std::thread([&]() { pub.run(); });
        t2=std::thread([&]() { ords.run(); });
        t3=std::thread([&]() { algo.run(); });
    }

    //停止运行，安全退出，把已经平仓的订单输出到文件中
    void stop(){
        auto& pub=PublicWebSocket::GetInstance();
        auto& ords=OrdersWebSocket::GetInstance();
        auto& algo=OrdersAlgoWebSocket::GetInstance();


        pub.stop();
        ords.stop();
        algo.stop();

        std::this_thread::sleep_for(std::chrono::microseconds(2000));
        for(auto& e:ordermanager_map){
            e.second->saveDataToFile();
        }

        std::this_thread::sleep_for(std::chrono::microseconds(2000));
        t1.join();
        t2.join();
        t3.join();
    }
private:
    Quant(){}
    Quant(const Quant&)=delete;
    Quant& operator=(const Quant&)=delete;

    std::string apiKey_;
    std::string secretKey_;
    std::string passphrase_;
    bool isdemo_;

    std::unordered_map<std::string,std::shared_ptr<OrderManager>> ordermanager_map;

    std::thread t1,t2,t3;
};