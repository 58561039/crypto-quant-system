/*
write by lzc
Email: 544840897@qq.com


使用REST向okx交易所发送请求
*/


#include "../include/okxrestclient.h"

// ---------------- Singleton ----------------
OkxRestClient& OkxRestClient::GetInstance() {
    static OkxRestClient instance;
    return instance;
}

// ---------------- Constructor ----------------
OkxRestClient::OkxRestClient() : curl_(nullptr) {}

OkxRestClient::~OkxRestClient() {
    if (curl_) curl_easy_cleanup(curl_);
    curl_global_cleanup();
}

// ---------------- API Settings ----------------
void OkxRestClient::SetSettings(const std::string& apiKey,
                                const std::string& secretKey,
                                const std::string& passphrase,
                                bool isDemo)
{
    apiKey_ = apiKey;
    secretKey_ = secretKey;
    passphrase_ = passphrase;
    isDemo_ = isDemo;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
    if (!curl_) throw std::runtime_error("curl init failed");

    baseUrl_ = "https://www.okx.com";
}

// ---------------- Helper: Get Timestamp ----------------
long long OkxRestClient::getLocalTimestampMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    ).count();
}

std::string OkxRestClient::getTimestampStr() {
    double ts_s = getLocalTimestampMs() / 1000.0;
    char buf[64];
    sprintf(buf, "%.3f", ts_s);
    return std::string(buf);
}

// ---------------- Sign ----------------
std::string OkxRestClient::sign(const std::string& msg) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int len;

    HMAC(EVP_sha256(),
         secretKey_.c_str(), secretKey_.size(),
         (unsigned char*)msg.c_str(), msg.size(),
         hash, &len);

    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, hash, len);
    BIO_flush(b64);

    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);
    std::string encoded(bptr->data, bptr->length);
    BIO_free_all(b64);

    return encoded;
}

// ---------------- GET ----------------
json OkxRestClient::httpGet(const std::string& fullPath)
{
    if (!curl_) {
        json err;
        err["code"] = -1;
        err["msg"] = "curl not initialized";
        return err;
    }

    std::string timestamp = getTimestampStr();
    std::string prehash = timestamp + "GET" + fullPath;
    std::string signStr = sign(prehash);

    std::string url = baseUrl_ + fullPath;
    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("OK-ACCESS-KEY: " + apiKey_).c_str());
    headers = curl_slist_append(headers, ("OK-ACCESS-SIGN: " + signStr).c_str());
    headers = curl_slist_append(headers, ("OK-ACCESS-TIMESTAMP: " + timestamp).c_str());
    headers = curl_slist_append(headers, ("OK-ACCESS-PASSPHRASE: " + passphrase_).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    if (isDemo_)
        headers = curl_slist_append(headers, "x-simulated-trading: 1");

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        json err;
        err["code"] = -1;
        err["msg"] = curl_easy_strerror(res);
        return err;
    }

    return response;
}

// ---------------- POST ----------------
json OkxRestClient::httpPost(const std::string& path, const std::string& body)
{
    if (!curl_) {
        json err;
        err["code"] = -1;
        err["msg"] = "curl not initialized";
        return err;
    }

    std::string timestamp = getTimestampStr();
    std::string prehash = timestamp + "POST" + path + body;
    std::string signStr = sign(prehash);

    std::string url = baseUrl_ + path;
    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("OK-ACCESS-KEY: " + apiKey_).c_str());
    headers = curl_slist_append(headers, ("OK-ACCESS-SIGN: " + signStr).c_str());
    headers = curl_slist_append(headers, ("OK-ACCESS-TIMESTAMP: " + timestamp).c_str());
    headers = curl_slist_append(headers, ("OK-ACCESS-PASSPHRASE: " + passphrase_).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    if (isDemo_)
        headers = curl_slist_append(headers, "x-simulated-trading: 1");

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        json err;
        err["code"] = -1;
        err["msg"] = curl_easy_strerror(res);
        return err;
    }

    try {
        return json::parse(response);
    } catch (const std::exception& e) {
        json err;
        err["code"] = -1;
        err["msg"] = "json parse error: " + std::string(e.what());
        err["raw"] = response;
        return err;
    }
}

// ---------------- Write Callback ----------------
size_t OkxRestClient::writeCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* s = (std::string*)userdata;
    s->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

// ---------------- Contract Value ----------------
double OkxRestClient::getContractValue(const std::string& instId)
{
    json j;
    try {
        j = json::parse(httpGet("/api/v5/public/instruments?instType=SWAP").get<std::string>());
    } catch (...) {
        return 0.0;
    }

    if (!j.contains("data")) return 0.0;

    for (auto& item : j["data"]) {
        if (item["instId"] == instId)
            return std::stod(item["ctVal"].get<std::string>());
    }

    return 0.0;
}

// ---------------- Place Order With TP/SL ----------------
json OkxRestClient::placeOrderWithTPSL(const Order& order)
{
    json body;

    body["instId"] = order.symbol;
    body["tdMode"] = order.tdMode;
    body["side"] = (order.side == OrderSide::BUY) ? "buy" : "sell";
    body["posSide"] = (order.side == OrderSide::BUY) ? "long" : "short";
    body["ordType"] = (order.type == OrderType::LIMIT) ? "limit" : "market";
    body["sz"] = std::to_string(order.quantity);

    if (order.type == OrderType::LIMIT)
        body["px"] = std::to_string(order.opening_price);

    body["clOrdId"] = order.clOrdId;

    bool hasTP = order.TP > 0;
    bool hasSL = order.SL > 0;

    if (hasTP || hasSL) {
        json algoItem;
        algoItem["attachAlgoClOrdId"] = order.clOrdId + "tpsl";

        if (hasTP) {
            algoItem["tpTriggerPx"] = std::to_string(order.TP);
            algoItem["tpOrdPx"] = "-1";
            algoItem["tpTriggerPxType"] = "last";
        }

        if (hasSL) {
            algoItem["slTriggerPx"] = std::to_string(order.SL);
            algoItem["slOrdPx"] = "-1";
            algoItem["slTriggerPxType"] = "last";
        }

        body["attachAlgoOrds"] = json::array({algoItem});
    }

    return httpPost("/api/v5/trade/order", body.dump());
}

// ---------------- Close Position ----------------
json OkxRestClient::closeOrderMarket(const std::string& instId,
                                     const std::string& posSide,
                                     double size)
{
    json body;
    body["instId"] = instId;
    body["mgnMode"] = "cross";
    return httpPost("/api/v5/trade/close-position", body.dump());
}

// ---------------- Set Leverage ----------------
json OkxRestClient::setLeverage(const std::string& instId,
                                double leverage,
                                const std::string& mgnMode,
                                const std::string& posSide)
{
    json body;
    body["instId"] = instId;
    body["mgnMode"] = mgnMode;
    body["lever"] = std::to_string(leverage);

    if (!posSide.empty())
        body["posSide"] = posSide;

    return httpPost("/api/v5/account/set-leverage", body.dump());
}

// ---------------- Klines ----------------
std::string OkxRestClient::getKlines(const std::string& instId,
                                     const std::string& bar,
                                     int limit)
{
    std::string path = "/api/v5/market/candles";
    std::string query = "?instId=" + instId +
                        "&bar=" + bar +
                        "&limit=" + std::to_string(limit);

    return httpGet(path + query);
}
