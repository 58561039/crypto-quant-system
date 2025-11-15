/*
write by lzc
Email: 544840897@qq.com
*/


#pragma once

#include "base_websocket.h"
#include <unordered_map>
#include <memory>
#include <mutex>

class OrdersAlgoWebSocket : public WSBase {
public:
    static OrdersAlgoWebSocket& GetInstance();

    void setup_path() override;

    void subscribe_orders_algo();

    void add_order_manager(const std::string& symbol,
                           std::shared_ptr<OrderManager>& ptr,
                           std::shared_ptr<std::mutex> order_manager_mtx);

    void remove_order_manager(const std::string& symbol);

    void Notify_TP_SL(const std::string& symbol,
                      const std::string& ClOrdId,
                      const std::string& actualSide);

private:
    // 私有构造：单例模式
    OrdersAlgoWebSocket() = default;

    // 存储各个 symbol 对应的 OrderManager
    std::unordered_map<std::string, std::shared_ptr<OrderManager>> order_manager_map;
    std::unordered_map<std::string, std::shared_ptr<std::mutex>> mutex_map;
};
