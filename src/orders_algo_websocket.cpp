#include "../include/websocket/orders_algo_websocket.h"


// ----------------------
// 单例接口
// ----------------------
OrdersAlgoWebSocket& OrdersAlgoWebSocket::GetInstance() {
    static OrdersAlgoWebSocket inst;
    return inst;
}

// ----------------------
// 设置 WebSocket 路径
// ----------------------
void OrdersAlgoWebSocket::setup_path() {
    ws_path_ = "/ws/v5/business";
    if (isDemo_) ws_path_ += "?brokerId=9999";
}

// ----------------------
// 订阅 orders-algo
// ----------------------
void OrdersAlgoWebSocket::subscribe_orders_algo() {
    json sub;
    sub["op"] = "subscribe";
    sub["args"] = {{
        {"channel", "orders-algo"},
        {"instType", "SWAP"}
    }};
    send(sub.dump());
}

// ----------------------
// 添加 OrderManager 观察者
// ----------------------
void OrdersAlgoWebSocket::add_order_manager(const std::string& symbol,
                                            std::shared_ptr<OrderManager>& ptr,
                                            std::shared_ptr<std::mutex> order_manager_mtx)
{
    order_manager_map[symbol] = ptr;
    mutex_map[symbol] = order_manager_mtx;
}

// ----------------------
// 移除 OrderManager
// ----------------------
void OrdersAlgoWebSocket::remove_order_manager(const std::string& symbol)
{
    order_manager_map.erase(symbol);
    mutex_map.erase(symbol);
}

// ----------------------
// 通知止盈止损事件
// ----------------------
void OrdersAlgoWebSocket::Notify_TP_SL(const std::string& symbol,
                                       const std::string& ClOrdId,
                                       const std::string& actualSide)
{
    auto it = mutex_map.find(symbol);
    if (it == mutex_map.end()) return;

    std::lock_guard<std::mutex> lock(*it->second);

    if (order_manager_map.count(symbol))
        order_manager_map[symbol]->TP_SL_orders_algo(ClOrdId, actualSide);
}
