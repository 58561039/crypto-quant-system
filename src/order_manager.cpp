/*
write by lzc
Email: 544840897@qq.com


订单管理类
*/


#include "../include/order_manager.h"

OrderManager::OrderManager(const std::string& symbol_) 
    : symbol(symbol_) 
{
    order_list.reserve(150);
    filename = symbol + ".csv";
    contractValue = OkxRestClient::GetInstance().getContractValue(symbol);
}

void OrderManager::setLeverage(double leverage_) {
    OkxRestClient::GetInstance().setLeverage(symbol, leverage_);
    leverage = leverage_;
}

void OrderManager::saveDataToFile() {
    std::ofstream ofs(filename, std::ios::out);
    if (!ofs.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    ofs << "symbol side type quantity opening_price closing_price tdMode TP SL "
           "total_handling_fee profit profit_ratio clOrdId tp_or_sl\n";

    for (const auto& o : order_list) {
        ofs << o.symbol << " "
            << ((o.side == OrderSide::BUY) ? "BUY" : "SELL") << " "
            << ((o.type == OrderType::LIMIT) ? "LIMIT" : "MARKET") << " "
            << o.quantity << " "
            << o.opening_price << " "
            << o.closing_price << " "
            << o.tdMode << " "
            << o.TP << " "
            << o.SL << " "
            << o.total_handling_fee << " "
            << o.profit << " "
            << o.profit_ratio << " "
            << o.clOrdId << " "
            << ((o.tp_or_sl == TP_OR_SL::TP) ? "tp" : "sl")
            << "\n";
    }

    ofs.close();
    std::cout << "Saved " << order_list.size() << " orders to file: " << filename << std::endl;
}

void OrderManager::parseHistoryData(const std::string& msg) {
    try {
        auto j = nlohmann::json::parse(msg);
        if (!j.contains("data") || !j["data"].is_array()) return;

        for (auto& item : j["data"]) {
            if (!item.is_array() || item.size() < 6) continue;

            Kline k;
            k.timestamp = stoll(item[0].get<std::string>());
            k.open = stod(item[1].get<std::string>());
            k.high = stod(item[2].get<std::string>());
            k.low = stod(item[3].get<std::string>());
            k.close = stod(item[4].get<std::string>());
            k.volume = stod(item[5].get<std::string>());

            if (dq_kline.size() >= 300)
                dq_kline.pop_back();

            dq_kline.push_front(k);
        }

        if (!dq_kline.empty())
            now_price = dq_kline.back().close;

    } catch (const std::exception& e) {
        std::cerr << "[parseHistoryData] json error: " << e.what() << std::endl;
    }
}

void OrderManager::update_public_websocket(const Kline& line) {
    if (!dq_kline.empty()) {
        if (line.timestamp < dq_kline.front().timestamp)
            return;
    }

    bool k_end = (line.confirm == 1);

    if (k_end) {
        if (dq_kline.size() > 300)
            dq_kline.pop_front();
        dq_kline.push_back(line);
    }

    now_price = line.close;

    if (k_end) {
        Order order = strategy->check(dq_kline);
        generate(order);
    }
}

void OrderManager::generate(Order& order) {
    if (order.orderStatus == Status::NotCreate) {
        return;
    }

    long long ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    order.tdMode = "cross";
    order.clOrdId = std::to_string(ts);

    OkxRestClient::GetInstance().placeOrderWithTPSL(order);

    order_map[order.clOrdId] = order;

    std::cout<<"开仓："<<symbol<<std::endl;

}

void OrderManager::Order_placed(const std::string& clOrdId,
                                const std::string& fee,
                                const std::string& avgPx) {

    order_map[clOrdId].total_handling_fee += stod(fee);
    order_map[clOrdId].opening_price = stod(avgPx);
    order_map[clOrdId].orderStatus = Status::Opened;
}

void OrderManager::TP_SL_orders(const std::string& clOrdId,
                                const std::string& fee,
                                const std::string& pnl,
                                const std::string& avgPx) {

    order_map[clOrdId].total_handling_fee += stod(fee);
    order_map[clOrdId].closing_price = stod(avgPx);
    order_map[clOrdId].orderStatus = Status::Closed;
    order_map[clOrdId].profit = stod(pnl) + order_map[clOrdId].total_handling_fee;

    order_map[clOrdId].profit_ratio =
        order_map[clOrdId].profit /
        ((order_map[clOrdId].opening_price * contractValue) / leverage);

    order_map[clOrdId].is_build_by_orders = true;

    check_finish(clOrdId);
}

void OrderManager::TP_SL_orders_algo(const std::string& clOrdId,
                                     const std::string& actualSide) {

    if (actualSide == "sl") {
        order_map[clOrdId].tp_or_sl = TP_OR_SL::SL;
        order_map[clOrdId].is_build_by_orders_algo = true;
    } else if (actualSide == "tp") {
        order_map[clOrdId].tp_or_sl = TP_OR_SL::TP;
        order_map[clOrdId].is_build_by_orders_algo = true;
    }

    check_finish(clOrdId);
}

void OrderManager::check_finish(const std::string& clOrdId) {
    if (order_map[clOrdId].is_build_by_orders &&
        order_map[clOrdId].is_build_by_orders_algo)
    {
        order_list.push_back(order_map[clOrdId]);

        std::cout<<"平仓："<<order_map[clOrdId].symbol<<' '
            <<order_map[clOrdId].opening_price<<" -> "
            <<order_map[clOrdId].closing_price<<' '
            <<order_map[clOrdId].profit<<' '
            <<order_map[clOrdId].profit_ratio<<std::endl;

        order_map.erase(clOrdId);
    }
}

void OrderManager::SetStrategy(std::shared_ptr<Strategy> s) {
    strategy = std::move(s);
}
