/*
write by lzc
Email: 544840897@qq.com
*/


#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "../order_manager.h"

#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace websocket = boost::beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

using json = nlohmann::json;

class WSBase {
public:
    WSBase();
    virtual ~WSBase();

    virtual void setup_path() = 0;//设置服务地址
    virtual bool need_login();//是否需要登录

    void settings(std::string apiKey, std::string secretKey,
                  std::string passphrase, bool isDemo);//初始化

    void stop();//优雅退出
    void set_on_message(std::function<void(const std::string&)> func);//设置回调函数


    bool connect();
    void run();

    void send(const std::string& msg);//发送ping和签名

    void start_heartbeat();//心跳函数，用于每隔10秒向okx交易所发送 “ping”
    bool getRunning();//获取running_，判断该websocket是否处于运行的状态中

protected:
    // lifecycle managers
    std::unique_ptr<net::io_context> ioc_;
    std::unique_ptr<ssl::context>    ssl_ctx_;

    std::string apiKey_, secretKey_, passphrase_;
    bool isDemo_ = true;

    std::string host_, port_;
    std::string ws_path_;

    std::unique_ptr<websocket::stream<ssl::stream<beast::tcp_stream>>> ws_;
    std::function<void(const std::string&)> on_message_;

    std::thread hb_thread_;//心跳线程
    std::mutex  write_mtx_;
    std::atomic<bool> running_{false};

    void login();
    void reconnect();
    long long now_ms();
    std::string base64_encode(const unsigned char* input, unsigned int length);
    void close_ws();
};
