
/*
write by lzc
Email: 544840897@qq.com


订单管理类，
根据orders与orders-algo频道传来的消息维护订单
*/

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <mutex>
#include <map>
#include <fstream>
#include <iomanip>

#include "order.h"
#include "kline.h"
#include "strategy.h"
#include "okxrestclient.h"

class OrderManager {
public:
    OrderManager(const std::string& symbol_);

    //设置杠杆
    void setLeverage(double leverage_);

    //保存订单记录到文件中
    void saveDataToFile();

    //解析历史K线
    void parseHistoryData(const std::string& msg);

    //观察者模式中，public_websocket的通知
    void update_public_websocket(const Kline& line);

    //创建订单
    void generate(Order& order);

    //观察者模式中，orders的通知
    void Order_placed(const std::string& clOrdId,
                      const std::string& fee,
                      const std::string& avgPx);
    
    //观察者模式中，public_websocket的通知
    void TP_SL_orders(const std::string& clOrdId,
                      const std::string& fee,
                      const std::string& pnl,
                      const std::string& avgPx);
    
    //观察者模式中，public_websocket的通知
    void TP_SL_orders_algo(const std::string& clOrdId,
                           const std::string& actualSide);
    
    //每次修改订单状态后，都要检查该订单是否已经成功的止盈或止损
    void check_finish(const std::string& clOrdId);
    
    //设置策略
    void SetStrategy(std::shared_ptr<Strategy> s);

private:
    double now_price;
    std::deque<Kline> dq_kline;//用于存储K线的双端队列
    std::string symbol;//交易对名称
    std::string filename;//将订单记录保存到此文件中

    std::vector<Order> order_list;//已平仓的订单数组
    std::map<std::string, Order> order_map;//未平仓的订单数组

    std::shared_ptr<Strategy> strategy;//策略指针

    double contractValue;//合约面值
    double leverage;//杠杆大小
};
