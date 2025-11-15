/*
write by lzc
Email: 544840897@qq.com
*/


#include "strategy.h"

Order ThreeKlineStrategy::check(std::deque<Kline>& dq_kline) const {
    Order order;
    order.orderStatus = Status::NotCreate; // 默认不交易

    if (dq_kline.size() < 3) return order;

    const Kline& k1 = dq_kline[dq_kline.size() - 3];
    const Kline& k2 = dq_kline[dq_kline.size() - 2];
    const Kline& k3 = dq_kline[dq_kline.size() - 1];

    bool up1 = k1.close > k1.open;
    bool up2 = k2.close > k2.open;
    bool up3 = k3.close > k3.open;

    bool down1 = k1.close < k1.open;
    bool down2 = k2.close < k2.open;
    bool down3 = k3.close < k3.open;

    double entry_price = k3.close;

    // 连续三根上涨 → 做多
    if (up1 && up2 && up3) {
        order.symbol = k3.symbol;
        order.side = OrderSide::BUY;
        order.type = OrderType::MARKET;
        order.orderStatus = Status::NotOpen;
        order.quantity = 1.0;
        order.SL = k1.low;
        double diff = entry_price - order.SL;
        order.TP = entry_price + diff;
        return order;
    }

    // 连续三根下跌 → 做空
    if (down1 && down2 && down3) {
        order.symbol = k3.symbol;
        order.side = OrderSide::SELL;
        order.type = OrderType::MARKET;
        order.orderStatus = Status::NotOpen;
        order.quantity = 1.0;
        order.SL = k1.high;
        double diff = order.SL - entry_price;
        order.TP = entry_price - diff;
        return order;
    }

    return order; // 不开仓
}
