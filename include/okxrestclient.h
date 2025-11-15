/*
write by lzc
Email: 544840897@qq.com

通过使用REST与交易所交互
功能：
1.下单附带止盈止损             httpPost       
2.设置杠杆                     httpPost
3.获取历史K线                  httpGet
4.获取合约面值                 httpGet
5.市价平仓                     httpPost


*/

#pragma once
#include <string>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <chrono>
#include <nlohmann/json.hpp>
#include <iostream>

#include "order.h"

using json = nlohmann::json;

class OkxRestClient {
public:
    static OkxRestClient& GetInstance();

    double getContractValue(const std::string& instId);

    void SetSettings(const std::string& apiKey,
                     const std::string& secretKey,
                     const std::string& passphrase,
                     bool isDemo = false);

    ~OkxRestClient();

    json placeOrderWithTPSL(const Order& order);

    json closeOrderMarket(const std::string& instId,
                          const std::string& posSide,
                          double size);

    json setLeverage(const std::string& instId,
                     double leverage,
                     const std::string& mgnMode = "cross",
                     const std::string& posSide = "");

    std::string getKlines(const std::string& instId,
                          const std::string& bar = "1m",
                          int limit = 300);

private:
    OkxRestClient();
    OkxRestClient(const OkxRestClient&) = delete;
    OkxRestClient& operator=(const OkxRestClient&) = delete;

    CURL* curl_;
    std::string apiKey_, secretKey_, passphrase_;
    std::string baseUrl_;
    bool isDemo_ = false;

    long long getLocalTimestampMs();
    std::string getTimestampStr();

    std::string sign(const std::string& msg);

    json httpGet(const std::string& fullPath);
    json httpPost(const std::string& path, const std::string& body);

    static size_t writeCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
};
