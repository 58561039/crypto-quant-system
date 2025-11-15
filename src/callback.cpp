/*
write by lzc
Email: 544840897@qq.com


三个websocket的回调函数的实现
*/


#include <iostream>
#include <nlohmann/json.hpp>



#include "../include/callback.h"

#include "kline.h"
#include "../include/websocket/public_websocket.h"
#include "../include/websocket/orders_websocket.h"
#include "../include/websocket/orders_algo_websocket.h"


using json = nlohmann::json;

// public_websocket的回调函数
void public_websocket_callback(const std::string& msg) {
    try {
        auto j = json::parse(msg);

        if (!j.contains("arg") || !j.contains("data")) return;

        std::string instId = j["arg"]["instId"];

        Kline line;
        const auto arr = j["data"][0];

        line.symbol = instId;
        line.timestamp = std::stoll(arr[0].get<std::string>());
        line.open = std::stod(arr[1].get<std::string>());
        line.high = std::stod(arr[2].get<std::string>());
        line.low = std::stod(arr[3].get<std::string>());
        line.close = std::stod(arr[4].get<std::string>());
        line.volume = std::stod(arr[5].get<std::string>());
        line.confirm = (arr[8].get<std::string>() == "1");

        PublicWebSocket::GetInstance().Notify_Kline(instId, line);
    } catch (std::exception& e) {
        if (!PublicWebSocket::GetInstance().getRunning()) return;
        std::cout << "public_websocket_callback失败" << std::endl;
    }
}

void orders_websocket_callback(const std::string& msg) {
    try {
        auto j = json::parse(msg);

        if (!j.contains("arg") || !j.contains("data")) return;

        auto arr = j["data"][0];

        if (arr["state"].get<std::string>() == "filled") {
            if (arr["reduceOnly"].get<std::string>() == "false") {
                std::string instId = arr["instId"].get<std::string>();
                std::string clOrdId = arr["clOrdId"].get<std::string>();
                std::string fee = arr["fee"].get<std::string>();
                std::string avgPx = arr["avgPx"].get<std::string>();

                OrdersWebSocket::GetInstance().Notify_Create_Orders(instId, clOrdId, fee, avgPx);
            }
            if (arr["reduceOnly"].get<std::string>() == "true") {
                std::string instId = arr["instId"].get<std::string>();
                std::string algoClOrdId = arr["algoClOrdId"].get<std::string>();
                std::string clOrdId = algoClOrdId.substr(0, algoClOrdId.size() - 4);

                std::string fee = arr["fee"].get<std::string>();
                std::string pnl = arr["pnl"].get<std::string>();
                std::string avgPx = arr["avgPx"].get<std::string>();

                OrdersWebSocket::GetInstance().Notify_Close_Orders(instId, clOrdId, fee, pnl, avgPx);
            }
        }
    } catch (std::exception& e) {
        if (!OrdersWebSocket::GetInstance().getRunning()) return;
        std::cout << "orders_websocket_callback失败" << std::endl;
    }
}

void orders_algo_websocket_callback(const std::string& msg) {
    try {
        auto j = json::parse(msg);

        if (!j.contains("arg") || !j.contains("data")) return;

        auto arr = j["data"][0];

        if (arr["state"] == "effective" && (!arr["actualSide"].empty())) {
            std::string instId = arr["instId"].get<std::string>();
            std::string algoClOrdId = arr["algoClOrdId"].get<std::string>();
            std::string clOrdId = algoClOrdId.substr(0, algoClOrdId.size() - 4);
            std::string actualSide = arr["actualSide"].get<std::string>();

            OrdersAlgoWebSocket::GetInstance().Notify_TP_SL(instId, clOrdId, actualSide);
        }
    } catch (std::exception& e) {
        if (!OrdersAlgoWebSocket::GetInstance().getRunning()) return;
        std::cout << "orders_algo_websocket_callback失败" << std::endl;
    }
}
