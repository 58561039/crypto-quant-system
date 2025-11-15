/*
write by lzc
Email: 544840897@qq.com
*/


#pragma once

#include "base_websocket.h"
#include <unordered_map>
#include <memory>
#include <mutex>

class OrdersWebSocket : public WSBase {
public:
    // 单例实例获取
    static OrdersWebSocket& GetInstance();

    // 覆盖设置路径
    void setup_path() override;

    // 订阅订单通道
    void subscribe_orders();

    // 通知委托成交（开仓或加仓）
    void Notify_Create_Orders(const std::string& symbol,
                              const std::string& clOrdId,
                              const std::string& fee,
                              const std::string& avgPx);

    // 通知平仓（止盈/止损/手动平仓）
    void Notify_Close_Orders(const std::string& symbol,
                             const std::string& clOrdId,
                             const std::string& fee,
                             const std::string& pnl,
                             const std::string& avgPx);

    // 添加某个 symbol 对应的 OrderManager
    void add_order_manager(const std::string& symbol,
                           std::shared_ptr<OrderManager>& ptr,
                           std::shared_ptr<std::mutex> order_manager_mtx);

    // 移除 symbol
    void remove_order_manager(const std::string& symbol);

private:
    OrdersWebSocket() = default; // 单例构造

    std::unordered_map<std::string, std::shared_ptr<OrderManager>> order_manager_map;
    std::unordered_map<std::string, std::shared_ptr<std::mutex>> mutex_map;
};
