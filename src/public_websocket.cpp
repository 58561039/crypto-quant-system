/*
write by lzc
Email: 544840897@qq.com
*/


#include "../include/websocket/public_websocket.h"

// ---------------- Singleton ----------------
PublicWebSocket& PublicWebSocket::GetInstance() {
    static PublicWebSocket inst;
    return inst;
}

// ---------------- Override functions ----------------
bool PublicWebSocket::need_login() {
    return false;
}

void PublicWebSocket::setup_path() {
    ws_path_ = "/ws/v5/business";
}

// ---------------- API functions ----------------
void PublicWebSocket::subscribe_candle_1m(const std::string& instId) {
    json sub;
    sub["op"] = "subscribe";
    sub["args"] = {{
        {"channel", "candle1m"},
        {"instId", instId}
    }};
    send(sub.dump());
}

void PublicWebSocket::Notify_Kline(const std::string& symbol, const Kline& line) {
    std::lock_guard<std::mutex> lock(*mutex_map[symbol]);
    order_manager_map[symbol]->update_public_websocket(line);
}

void PublicWebSocket::add_order_manager(const std::string& symbol,
                                        std::shared_ptr<OrderManager>& ptr,
                                        std::shared_ptr<std::mutex> order_manager_mtx)
{
    order_manager_map[symbol] = ptr;
    mutex_map[symbol] = order_manager_mtx;
}

void PublicWebSocket::remove_order_manager(const std::string& symbol) {
    order_manager_map.erase(symbol);
    mutex_map.erase(symbol);
}
