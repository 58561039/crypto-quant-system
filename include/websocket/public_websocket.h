
/*
write by lzc
Email: 544840897@qq.com
*/

#pragma once

#include "base_websocket.h"
#include "../kline.h"

class PublicWebSocket : public WSBase {
public:
    static PublicWebSocket& GetInstance();

    bool need_login() override;
    void setup_path() override;

    void subscribe_candle_1m(const std::string& instId);

    void Notify_Kline(const std::string& symbol, const Kline& line);

    void add_order_manager(const std::string& symbol,
                           std::shared_ptr<OrderManager>& ptr,
                           std::shared_ptr<std::mutex> order_manager_mtx);

    void remove_order_manager(const std::string& symbol);

private:
    std::unordered_map<std::string, std::shared_ptr<OrderManager>> order_manager_map;
    std::unordered_map<std::string, std::shared_ptr<std::mutex>> mutex_map;
};
