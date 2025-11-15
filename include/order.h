/*
write by lzc
Email: 544840897@qq.com

订单结构体
*/



#pragma once
#include <string>

enum class OrderType {
    MARKET,  // 市价
    LIMIT    // 限价
};

enum class OrderSide {
    BUY,     // 多
    SELL     // 空
};

enum class Status {
    NotCreate,  // 未创建
    NotOpen,    // 已创建但未发送到交易所
    Opened,     // 已开仓
    Closed      // 已平仓
};

enum class TP_OR_SL{//记录该订单是止盈还是止损
    TP,         //止盈
    UNCERTAIN,  //未触发止盈或止损
    SL          //止损
};

struct Order {
    // 基本订单信息
    std::string symbol;      // 交易对
    OrderSide side;          // BUY / SELL
    OrderType type;          // MARKET / LIMIT
    Status orderStatus;      // 订单状态
    // 止盈止损设置
    double TP = 0.0;    // 止盈触发价
    double SL = 0.0;    // 止损触发价

    double quantity = 0.0;       // 下单张数
    double opening_price = 0.0;  // 平均开仓价
    double closing_price = 0.0;  // 平均平仓价
    std::string tdMode;          // 全仓还是逐仓，全仓：cross，逐仓：isolate

    

    // 盈亏信息
    double total_handling_fee = 0.0;  // 手续费累加（开+平）
    double profit = 0.0;              // 净利润=pnl（盈亏）+fee(手续费，手续费为负，所以相加）
    double profit_ratio = 0.0;        // 盈利率

    // 订单 ID 管理
    std::string clOrdId;             // 主订单自定义 ID
    TP_OR_SL tp_or_sl=TP_OR_SL::UNCERTAIN; //这个订单收止盈还是止损

    //用于记录该订单是否已经修改完成
    //一个订单触发止盈止损后，通过orders频道发来的数据确定该订单的  盈亏信息
    //通过orders-algo频道发来的数据确定该订单触发的是止盈还是止损
    bool is_build_by_orders=false;     
    bool is_build_by_orders_algo=false;
};
