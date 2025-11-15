#include "../include/websocket/orders_websocket.h"
#include <iostream>

// ------------------------
// 单例实例
// ------------------------
OrdersWebSocket& OrdersWebSocket::GetInstance() {
    static OrdersWebSocket inst;
    return inst;
}

// ------------------------
// 设置 WS 路径
// ------------------------
void OrdersWebSocket::setup_path() {
    ws_path_ = "/ws/v5/private";
    if (isDemo_) {
        ws_path_ += "?brokerId=9999";
    }
}

// ------------------------
// 订阅订单通道
// ------------------------
void OrdersWebSocket::subscribe_orders() {
    json sub;
    sub["op"] = "subscribe";
    sub["args"] = {{
        {"channel", "orders"},
        {"instType", "SWAP"}
    }};
    send(sub.dump());
}

// ------------------------
// 通知开仓 / 加仓成交
// ------------------------
void OrdersWebSocket::Notify_Create_Orders(const std::string& symbol,
                                           const std::string& clOrdId,
                                           const std::string& fee,
                                           const std::string& avgPx)
{
    auto it = mutex_map.find(symbol);
    if (it == mutex_map.end()) return;

    std::lock_guard<std::mutex> lock(*it->second);

    if (order_manager_map.count(symbol)) {
        order_manager_map[symbol]->Order_placed(clOrdId, fee, avgPx);
    }
}

// ------------------------
// 通知平仓成交（止盈/止损）
// ------------------------
void OrdersWebSocket::Notify_Close_Orders(const std::string& symbol,
                                          const std::string& clOrdId,
                                          const std::string& fee,
                                          const std::string& pnl,
                                          const std::string& avgPx)
{
    std::cout << "开始执行 Notify_Close_Orders" << std::endl;

    auto it = mutex_map.find(symbol);
    if (it == mutex_map.end()) return;

    std::lock_guard<std::mutex> lock(*it->second);

    if (order_manager_map.count(symbol)) {
        order_manager_map[symbol]->TP_SL_orders(clOrdId, fee, pnl, avgPx);
    }
}

// ------------------------
// 添加观察者 OrderManager
// ------------------------
void OrdersWebSocket::add_order_manager(const std::string& symbol,
                                        std::shared_ptr<OrderManager>& ptr,
                                        std::shared_ptr<std::mutex> order_manager_mtx)
{
    order_manager_map[symbol] = ptr;
    mutex_map[symbol] = order_manager_mtx;
}

// ------------------------
// 移除观察者
// ------------------------
void OrdersWebSocket::remove_order_manager(const std::string& symbol)
{
    order_manager_map.erase(symbol);
    mutex_map.erase(symbol);
}
