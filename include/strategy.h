/*
write by lzc
Email: 544840897@qq.com


策略类

ThreeKlineStrategy三根K线策略
当出现三根同方向的K线，就入场，止损放在第一根K线的起始位置

三根阳线，做多
三根阴线，做空
*/


#pragma once

#include <deque>
#include <string>
#include "order.h"
#include "kline.h"

//策略抽象基类
class Strategy {
public:
    virtual ~Strategy() = default;

    //检查是否有开仓信号
    virtual Order check(std::deque<Kline>& dq_kline) const = 0;
};

//声明，源文件中实现具体策略
class ThreeKlineStrategy : public Strategy {
public:
    Order check(std::deque<Kline>& dq_kline) const override;
};
