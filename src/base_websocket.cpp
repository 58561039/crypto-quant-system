/*
write by lzc
Email: 544840897@qq.com


基类websocket
*/


#include "../include/websocket/base_websocket.h"

WSBase::WSBase() {}
WSBase::~WSBase() {}

void WSBase::settings(std::string apiKey, std::string secretKey,
                      std::string passphrase, bool isDemo)
{
    apiKey_     = std::move(apiKey);
    secretKey_  = std::move(secretKey);
    passphrase_ = std::move(passphrase);
    isDemo_     = isDemo;

    running_.store(true, std::memory_order_relaxed);
}

void WSBase::stop()
{
    running_ = false;

    if (hb_thread_.joinable())
        hb_thread_.join();

    if (ws_) {
        boost::system::error_code ec;
        ws_->next_layer().shutdown(ec);
    }

    close_ws();
}

void WSBase::set_on_message(std::function<void(const std::string&)> func)
{
    on_message_ = std::move(func);
}

bool WSBase::need_login()
{
    return true;
}

bool WSBase::connect()
{
    try {
        setup_path();
        host_ = isDemo_ ? "wspap.okx.com" : "ws.okx.com";
        port_ = "8443";

        ioc_     = std::make_unique<net::io_context>();
        ssl_ctx_ = std::make_unique<ssl::context>(ssl::context::tlsv12_client);
        ssl_ctx_->set_default_verify_paths();

        tcp::resolver resolver(*ioc_);
        auto results = resolver.resolve(host_, port_);

        beast::tcp_stream tcpStream(*ioc_);
        tcpStream.connect(results);

        ssl::stream<beast::tcp_stream> sslStream(std::move(tcpStream), *ssl_ctx_);

        if (!SSL_set_tlsext_host_name(sslStream.native_handle(), host_.c_str()))
            throw std::runtime_error("Failed to set SNI");

        sslStream.handshake(ssl::stream_base::client);

        ws_ = std::make_unique<websocket::stream<ssl::stream<beast::tcp_stream>>>(
            std::move(sslStream));

        ws_->handshake(host_, ws_path_);

        if (need_login())
            login();

        start_heartbeat();
        std::cout << "[WSBase] Connected to " << ws_path_ << std::endl;

        return true;
    }
    catch (std::exception& e) {
        std::cerr << "[WSBase] Connect failed: " << e.what() << std::endl;
        close_ws();
        return false;
    }
}

void WSBase::run()
{
    beast::flat_buffer buffer;

    while (running_) {
        try {
            beast::error_code ec;
            ws_->read(buffer, ec);

            if (ec == websocket::error::closed ||
                ec == net::error::operation_aborted) {
                break;
            }

            if (ec && running_) {
                std::cerr << "[WSBase] read error: " << ec.message() << "\n";
                break;
            }

            std::string msg = beast::buffers_to_string(buffer.data());
            buffer.consume(buffer.size());

            if (msg == "pong") continue;
            if (on_message_) on_message_(msg);
        }
        catch (std::exception& e) {
            std::cerr << "[WSBase] Read error: " << e.what() << "\n";

            if (!running_) break;

            ws_.reset();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            connect();
        }
    }
    close_ws();
}

void WSBase::send(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(write_mtx_);
    if (!running_.load() || !ws_) return;

    try {
        ws_->write(net::buffer(msg));
    } catch (...) {}
}

void WSBase::start_heartbeat()
{
    if (hb_thread_.joinable()) hb_thread_.join();
    hb_thread_ = std::thread([this]() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            if (!running_) break;
            send("ping");
        }
    });
}

bool WSBase::getRunning() {
    return running_;
}

void WSBase::login()
{
    double ts = now_ms() / 1000.0;

    char buf[64];
    sprintf(buf, "%.3f", ts);
    std::string timestamp(buf);

    std::string prehash = timestamp + "GET" + "/users/self/verify";

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int len = 0;

    HMAC(EVP_sha256(),
         secretKey_.c_str(), secretKey_.size(),
         (unsigned char*)prehash.c_str(), prehash.size(),
         hash, &len);

    std::string signature = base64_encode(hash, len);

    json j;
    j["op"] = "login";
    j["args"] = {{
        {"apiKey", apiKey_},
        {"passphrase", passphrase_},
        {"timestamp", timestamp},
        {"sign", signature}
    }};

    send(j.dump());
}

void WSBase::reconnect()
{
    std::cerr << "[WSBase] Reconnecting...\n";
    connect();
}

long long WSBase::now_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::string WSBase::base64_encode(const unsigned char* input, unsigned int length)
{
    BIO* bmem = BIO_new(BIO_s_mem());
    BIO* b64  = BIO_new(BIO_f_base64());
    b64 = BIO_push(b64, bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    BIO_write(b64, input, length);
    BIO_flush(b64);

    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);
    std::string encoded(bptr->data, bptr->length);

    BIO_free_all(b64);
    return encoded;
}

void WSBase::close_ws()
{
    std::lock_guard<std::mutex> lock(write_mtx_);
    try {
        if (ws_) {
            boost::system::error_code ec;
            ws_->close(websocket::close_code::normal, ec);
        }
    }
    catch (...) {}

    ws_.reset();
}
