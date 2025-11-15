/*
write by lzc
Email: 544840897@qq.com

K线数据结构
*/
#pragma once

#include<iostream>
#include<string>



//K线数据结构
struct Kline{
    std::string symbol;//交易对符号
    long long timestamp;//时间戳
    double open;//开盘价
    double high;//最高价
    double low;//最低价
    double close;//收盘价
    double volume;//成交量
    bool confirm;//是否收K
};