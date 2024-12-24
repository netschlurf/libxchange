#include "BybitWebsocketClient.h"

#include <curl/curl.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <sstream>
#include <iomanip>
#include <chrono>
#include "XUtil.h"
#include "private.h"

CURL* curl_ = nullptr;


size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append((char*)contents, size * nmemb);
    return size * nmemb;
}

long long getBybitServerTime() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    long long serverTime = 0;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.bybit.com/v5/market/time");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {

            nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);
            if (jsonResponse.contains("result")) {
                long long timeNow = std::stoll(jsonResponse["result"]["timeSecond"].get<std::string>());
                serverTime = static_cast<long long>(timeNow*1000); // Ganze Sekunden
                
            }
        }
        else {
            std::cerr << "curl_easy_perform() fehlgeschlagen: " << curl_easy_strerror(res) << std::endl;
        }
    }
    return serverTime;
}

BybitWebSocketClient::~BybitWebSocketClient() 
{
    curl_global_cleanup();
}



bool BybitWebSocketClient::ConnectPublic(const std::string& url) 
{
    url_ = url;
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Initialize curl handle
    curl_ = curl_easy_init();
    if (!curl_) {
        std::cerr << "Failed to initialize curl!" << std::endl;
        return false;
    }

    // Set the URL for the WebSocket
    curl_easy_setopt(curl_, CURLOPT_CAINFO, "curl-ca-bundle.crt");
    curl_easy_setopt(curl_, CURLOPT_URL, url_.c_str());
    curl_easy_setopt(curl_, CURLOPT_CONNECT_ONLY, 2L);  // Only connect, no data transfer
    //curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1L);


    // Perform the connection
    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK) {
        std::cerr << "Connection failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    return true;
}

bool BybitWebSocketClient::SendMessageToXChange(const std::string& message) {
    size_t sent;
    CURLcode res = curl_ws_send(curl_, message.c_str(), message.size(), &sent, 0, CURLWS_TEXT);
    if (res != CURLE_OK) {
        std::cerr << "Failed to send message: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    return true;
}

void BybitWebSocketClient::Addlistener(BybitListener* listener)
{
    m_BybitListeners.push_back(listener);
};


//void BybitWebSocketClient::PlaceLimitOrder(long long timeStamp, int reqId, const std::string& symbol, const std::string& side, const std::string& qty, const std::string& price)
//{
//    nlohmann::json message;
//    static int rId = 1;
//    rId++;
//
//    message["reqId"] = rId;
//    message["header"] = {
//        { "X-BAPI-TIMESTAMP", std::to_string(getBybitServerTime())},
//        { "X-BAPI-RECV-WINDOW", "80000" }
//    };
//    message["op"] = "order.create";
//    message["args"] = nlohmann::json::array({
//        {
//            { "symbol", symbol },
//            { "side", side },
//            { "qty", qty },
//            { "price", price },
//            { "category", "linear" },
//            { "orderType", "Limit" },
//            { "timeInForce", "PostOnly" },
//            //{ "reduceOnly", false }
//        }
//        });
//    auto dbg = message.dump(4);
//    SendMessageToXChange(dbg);
//    ReceiveMessage();
//}

void BybitWebSocketClient::OnFrame()
{
    long long now = XUtil::Now();
    ReceiveMessage();

    if ((now - m_LastPing) > 20)
    {
        auto strNow = std::to_string(now);
        SendMessageToXChange("{\"op\": \"ping\", \"req_id\": \"" + strNow + "\"}");
        m_LastPing = now;
    }

    if ((now - m_LastConnectionCheck) > 10)
    {
        bool connected = IsConnected();
        if (!connected)
            ConnectPublic(url_);
        m_LastConnectionCheck = now;
    }
}

bool BybitWebSocketClient::IsConnected() {
    if (!curl_) return false;

    long httpCode = 0;
    CURLcode res = curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &httpCode);
    if (res != CURLE_OK || httpCode != 101) { // 101: Switching Protocols (WebSocket handshake)
        std::cerr << "WebSocket connection not active. HTTP Code: " << httpCode << std::endl;
        return false;
    }
    return true;
}

bool BybitWebSocketClient::ConnectPrivate() {
    url_ = "wss://stream.bybit.com/v5/private";
    // Initialize curl globally
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Initialize curl handle
    curl_ = curl_easy_init();
    if (!curl_) {
        std::cerr << "Failed to initialize curl!" << std::endl;
        return false;
    }

    // Set the URL for the WebSocket
    curl_easy_setopt(curl_, CURLOPT_CAINFO, "curl-ca-bundle.crt");
    curl_easy_setopt(curl_, CURLOPT_URL, url_.c_str());
    curl_easy_setopt(curl_, CURLOPT_CONNECT_ONLY, 2L);  // Only connect, no data transfer
    //curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1L);


    // Perform the connection
    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK) {
        std::cerr << "Connection failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }


    SubscribeToPositionsAndOrders();
    ReceiveMessage();
    return true;
}
#pragma comment(lib, "Crypt32.lib")
std::string createSignature(const std::string& expires, const std::string& secret) {

    std::string signString = "GET/realtime" + expires;

    unsigned char* digest;
    digest = HMAC(EVP_sha256(), secret.c_str(), secret.length(),
        reinterpret_cast<const unsigned char*>(signString.c_str()),
        signString.length(), nullptr, nullptr);

    std::ostringstream hexStream;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }

    return hexStream.str();
}


void BybitWebSocketClient::SubscribeToPositionsAndOrders() {
    auto now = std::chrono::system_clock::now();

    auto expires = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() + 50000;
    std::string apiKey = API_KEY;
    std::string secret = SECRET;

    std::string signature = createSignature(std::to_string(expires), secret);

    nlohmann::json auth_message = {
        {"op", "auth"},
        {"args", {apiKey, std::to_string(expires), signature}}
    };

    std::string auth_message_str = auth_message.dump();
    SendMessageToXChange(auth_message_str);
    ReceiveMessageSync();

    nlohmann::json subscribe_message = {
        {"op", "subscribe"},
        {"args", { "execution.fast", "order.linear", "position.linear"}}
    };
    
    std::string subscribe_message_str = subscribe_message.dump();
    SendMessageToXChange(subscribe_message_str);
    ReceiveMessageSync();
}


bool BybitWebSocketClient::ReceiveMessage() {
    size_t rlen;
    const struct curl_ws_frame* meta;
    static char buffer[10000000];
    CURLcode res = curl_ws_recv(curl_, buffer, sizeof(buffer), &rlen, &meta);

    if (res != CURLE_OK && res != CURLE_AGAIN) {
        return false;
    }

    if (rlen > 0) {
        std::string message(buffer, rlen);
        //std::cout << "Received: " << message << std::endl;
        DispatchBybitMessage(message);
    }

    return true;
}

bool BybitWebSocketClient::ReceiveMessageSync() 
{
    size_t rlen;
    const struct curl_ws_frame* meta;
    static char buffer[1000000];

    long long timeout = 500;
    long long start = XUtil::NowMs();
    long long elapsed = 0;

    while (elapsed < timeout)
    {
        CURLcode res = curl_ws_recv(curl_, buffer, sizeof(buffer), &rlen, &meta);

        if (res != CURLE_OK && res != CURLE_AGAIN) {
            return false;
        }

        if (rlen > 0) {
            std::string message(buffer, rlen);
            nlohmann::json jMsg = nlohmann::json::parse(message);
            std::cout << message;
        }
        long long now = XUtil::NowMs();
        elapsed = now - start;
    }


    return false;
}

std::string BybitWebSocketClient::ReceiveMessageRest() {
    size_t rlen;
    const struct curl_ws_frame* meta;
    static char buffer[10000000];
    CURLcode res = curl_ws_recv(curl_, buffer, sizeof(buffer), &rlen, &meta);

    if (res != CURLE_OK && res != CURLE_AGAIN) {
        return "";
    }

    if (rlen > 0) {
        std::string message(buffer, rlen);
        //std::cout << "Received: " << message << std::endl;
        return message;
    }

    return "";
}

void BybitWebSocketClient::DispatchBybitMessage(const std::string& message)
{
    nlohmann::json jMsg;
    try
    {
        jMsg = nlohmann::json::parse(message);

        if (jMsg.contains("op"))
        {
            std::string op = jMsg["op"];

            if (op == "auth")
            {
                std::cerr << message << std::endl;
            }
        }

        if (jMsg.contains("topic")) 
        {
            std::string topic = jMsg["topic"];

            
            if (topic.find("tickers") == 0)
            {
                std::string type = jMsg["type"];
                if (type == "snapshot")
                {
                    m_TicketData.parseSnapshot(jMsg);
                }
                else
                {
                    m_TicketData.parseDelta(jMsg);
                }
                
                for (size_t i = 0; i < m_BybitListeners.size(); i++)
                {
                    m_BybitListeners[i]->OnTicker(m_TicketData);
                }
            }
            else
            {
                if (topic.find("orderbook") != 0)
                    if (topic.find("publicTrade") != 0)
                        std::cout << topic << std::endl;
            }
            if (topic== "order.linear")
            {
                if (jMsg.contains("data")) {
                    for (const auto& item : jMsg["data"]) {
                        OrderData od;
                        od.Parse(item);
                        for (size_t i = 0; i < m_BybitListeners.size(); i++)
                        {
                            m_BybitListeners[i]->OnPrivateOrder(od);
                        }
                    }
                }
            }
            if (topic == "greeks")
            {
                std::cout << message << std::endl;
            }
            if (topic == "position.linear")
            {
                PositionData pos;
                try
                {
                    pos.parse(jMsg);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "position.linear: " << e.what() << std::endl;
                }

                for (size_t i = 0; i < m_BybitListeners.size(); i++)
                {
                    m_BybitListeners[i]->OnPosition(pos);
                }
            }
            if (topic == "execution.fast")
            {
                ExecutionData ed;
                try
                {
                    ed.parse(jMsg);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "execution.fast: " << e.what() << std::endl;
                }
                
                for (size_t i = 0; i < m_BybitListeners.size(); i++)
                {
                    m_BybitListeners[i]->OnFastExecution(ed);
                    std::string msg = "FE:" + ed.data[0].symbol;
                    msg += "," + ed.data[0].side;
                    msg += "," + std::to_string(ed.data[0].execQty);
                    msg += "," + std::to_string(ed.data[0].execPrice);
                    msg += "," + std::to_string(ed.data[0].isMaker);
                }
            }
            if (topic.find("publicTrade") == 0)
            {
                //std::cout << message << std::endl;
                std::string type = jMsg["type"];
                if (type == "snapshot")
                {
                    m_PublicTrade.parsePublicTradeSnapshot(jMsg);
                }
                else
                {
                    m_PublicTrade.parsePublicTradeDelta(jMsg);
                }
                m_PublicTrade.Calculate();
            }
            else if (topic.find("orderbook") == 0)
            {
                std::string type = jMsg["type"];
                if (type == "snapshot")
                {
                    m_OrderBookData.parseSnapshot(jMsg);
                }
                else
                {
                    m_OrderBookData.parseDelta(jMsg);
                }
                m_OrderBookData.Calculate();
            }
            else if (topic.find("kline") == 0)
            {
                std::string type = jMsg["type"];
                if (type == "snapshot")
                {
                    //m_TicketData.parseSnapshot(jMsg);
                }
                else
                {
                    //m_TicketData.parseDelta(jMsg);
                }
            }
            else if (topic.find("liquidation") == 0)
            {
                std::cout << message << std::endl;
                LiquidationData ld;
                ld.parse(jMsg);
                std::cout << "Liquidated: " << ld.data.size << std::endl;
                std::string msg = ld.data.side + " Liquidated: " + std::to_string(ld.data.size) + " at " + std::to_string(m_TicketData.lastPrice);
            }
            else
            {
                
                volatile int depp = 0;
            }
        }
    }
    catch (const std::exception& e)
    {
        e;
        std::string rest = message + ReceiveMessageRest();
        if(nlohmann::json::accept(rest))
            DispatchBybitMessage(rest);
        // otherwise i drop it
    }
}
