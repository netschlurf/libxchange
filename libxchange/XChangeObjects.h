#pragma once
#include "nlohmann/json.hpp"
#include <iostream>
#include <map>
#include <vector>

class OrderData {
public:
    std::string symbol;
    std::string orderId;
    std::string side;
    std::string orderType;
    std::string cancelType;
    double price = 0.0;
    double qty = 0.0;
    std::string timeInForce;
    std::string orderStatus;
    bool reduceOnly = false;
    double cumExecQty = 0.0;
    double cumExecValue = 0.0;
    double avgPrice = 0.0;
    double cumExecFee = 0.0;
    std::string closedPnl;
    std::string createdTime;
    std::string updatedTime;
    int positionIdx = 0;
    bool closeOnTrigger = false;
    std::string category;
    std::string placeType;
    std::string smpType;
    int smpGroup = 0;
    std::string feeCurrency;
    std::string rejectReason;

    void Parse(const nlohmann::json& data);

    void PrintOrderData() const {
        std::cout << "Order Data:" << std::endl;
        std::cout << "Symbol: " << symbol << std::endl;
        std::cout << "Order ID: " << orderId << std::endl;
        std::cout << "Side: " << side << std::endl;
        std::cout << "Order Type: " << orderType << std::endl;
        std::cout << "Cancel Type: " << cancelType << std::endl;
        std::cout << "Price: " << price << std::endl;
        std::cout << "Quantity: " << qty << std::endl;
        std::cout << "Time In Force: " << timeInForce << std::endl;
        std::cout << "Order Status: " << orderStatus << std::endl;
        std::cout << "Reduce Only: " << (reduceOnly ? "true" : "false") << std::endl;
        std::cout << "Cum Exec Qty: " << cumExecQty << std::endl;
        std::cout << "Cum Exec Value: " << cumExecValue << std::endl;
        std::cout << "Avg Price: " << avgPrice << std::endl;
        std::cout << "Cum Exec Fee: " << cumExecFee << std::endl;
        std::cout << "Closed PnL: " << closedPnl << std::endl;
        std::cout << "Created Time: " << createdTime << std::endl;
        std::cout << "Updated Time: " << updatedTime << std::endl;
        std::cout << "Position Index: " << positionIdx << std::endl;
        std::cout << "Close On Trigger: " << (closeOnTrigger ? "true" : "false") << std::endl;
        std::cout << "Category: " << category << std::endl;
        std::cout << "Place Type: " << placeType << std::endl;
        std::cout << "SMP Type: " << smpType << std::endl;
        std::cout << "SMP Group: " << smpGroup << std::endl;
        std::cout << "Fee Currency: " << feeCurrency << std::endl;
    }
};


class Orderbook {
public:
    void Calculate();
    std::string symbol;
    std::map<double, double> bids;
    std::map<double, double> asks;

    double askRate = 0;
    double bidRate = 0;

    double highestVolume = 0;
    double lowestVolume = 0;

    double m_SpreadAsk = 0;
    double m_SpreadBid = 0;

    Orderbook() : symbol("") {}
    void Filter();
    void CalcBalance();
    void SetMALength(int length) { m_MALength = length; };
    void SetFilterOn(bool on) { m_bFilterOn = on; };
    void SetFilterRange(double range) { m_FilterRange = range; };

    void parseSnapshot(const nlohmann::json& parsedData);
    void parseDelta(const nlohmann::json& parsedData);
    void display() const;


    std::vector<double> m_LastBidRates;
    double m_MABuyRate = 0;

    std::vector<double> m_LastAskRates;

    std::vector<double> m_LastAskVolumes;
    std::vector<double> m_LastBidVolumes;
    std::vector<double> m_LastTotalVolumes;
    double m_MAAskVolume = 0;
    double m_MABidVolume = 0;
    double m_MATotalVolume = 0;

    long long TimeStampMS = 0;
    int m_MALength = 100;
    bool m_bFilterOn = true;
    double m_FilterRange = 100;

    double AskVolume = 0;
    double BidVolume = 0;
};



class PublicTrade {
public:

    class Trade {
    public:
        long long timestamp;
        std::string side;
        double volume;
        double price;
        std::string tickDirection;
        std::string tradeId;
        bool blockTrade;

        Trade(long long ts, const std::string& s, double v, double p, const std::string& tDir, const std::string& tId, bool bt)
            : timestamp(ts), side(s), volume(v), price(p), tickDirection(tDir), tradeId(tId), blockTrade(bt) {}
    };

    std::string symbol;
    std::vector<Trade> trades;
    std::vector<Trade>  allTrades;

    PublicTrade() : symbol("") {}

    void Calculate();
    void SetMALength(int length) { m_MALength = length; };
    void parsePublicTradeSnapshot(const nlohmann::json& parsedData);
    void parsePublicTradeDelta(const nlohmann::json& parsedData);
    void display() const;

    std::vector<double> m_LastBuyRates;
    double m_MABuyRates = 0;

    std::vector<double> m_LastSellRates;
    double m_MASellRates = 0;

    double m_sellRate;
    double m_buyRate;
    long long TimeStamp = 0;
    int m_MALength = 100;

};

class TickerData {
public:
    TickerData();
    void parseSnapshot(const nlohmann::json& parsedData);
    void parseDelta(const nlohmann::json& parsedData);
    void display() const;

public:
    std::string symbol;
    long timeStamp = 0;
    long long timeStampMS = 0;
    double lastPrice;
    double highPrice24h;
    double lowPrice24h;
    double volume24h;
    double fundingRate;
    double bid1Price;
    double ask1Price;
    double markPrice;            // New attribute for mark price
    double indexPrice;           // New attribute for index price
    double openInterestValue;    // New attribute for open interest value


};


class PositionData {
public:
    PositionData() = default;

    // Existing WebSocket parsing method (unchanged)
    void parse(const nlohmann::json& json) {
        if (json.contains("id")) id = json["id"];
        if (json.contains("topic")) topic = json["topic"];
        if (json.contains("creationTime")) creationTime = json["creationTime"];

        // Daten innerhalb des "data" Arrays parsen
        if (json.contains("data") && json["data"].is_array()) {
            for (const auto& item : json["data"]) {
                PositionItem posItem;
                posItem.parse(item);
                data.push_back(posItem);
            }
        }
    }

    // New method for REST API parsing
    void parseRestResponse(const std::string& jsonStr) {
        // Clear existing data
        data.clear();

        // Parse the entire JSON response
        nlohmann::json json = nlohmann::json::parse(jsonStr);

        // Parse top-level response fields
        retCode = json.value("retCode", 0);
        retMsg = json.value("retMsg", "");
        serverTime = json.value("time", 0);

        // Parse result object
        if (json.contains("result")) {
            auto result = json["result"];

            // Parse result-level fields
            nextPageCursor = result.value("nextPageCursor", "");
            category = result.value("category", "");

            // Parse position list
            if (result.contains("list") && result["list"].is_array()) {
                for (const auto& item : result["list"]) {
                    PositionItem posItem;
                    posItem.parseRest(item);
                    data.push_back(posItem);
                }
            }
        }
    }

    // Funktion zum Anzeigen der Daten (optional)
    void display() const {
        // Existing WebSocket display method
        std::cout << "ID: " << id << "\n";
        std::cout << "Topic: " << topic << "\n";
        std::cout << "Creation Time: " << creationTime << "\n";

        // If REST API fields are present, display them
        if (!retMsg.empty()) {
            std::cout << "Return Code: " << retCode << "\n";
            std::cout << "Return Message: " << retMsg << "\n";
            std::cout << "Server Time: " << serverTime << "\n";
            std::cout << "Next Page Cursor: " << nextPageCursor << "\n";
            std::cout << "Category: " << category << "\n";
        }

        for (const auto& item : data) {
            item.display();
        }
    }

private:
    class PositionItem {
    public:
        // Existing WebSocket fields
        int positionIdx = 0;
        int tradeMode = 0;
        int riskId = 0;
        std::string riskLimitValue;
        std::string symbol;
        std::string side;
        double size = 0.0;
        double entryPrice = 0.0;
        double leverage = 0.0;
        double positionValue = 0.0;
        double positionBalance = 0.0;
        double markPrice = 0.0;
        double positionIM = 0.0;
        double positionMM = 0.0;
        double takeProfit = 0.0;
        double stopLoss = 0.0;
        double trailingStop = 0.0;
        double unrealisedPnl = 0.0;
        double cumRealisedPnl = 0.0;
        double curRealisedPnl = 0.0;
        long long createdTime = 0;
        long long updatedTime = 0;
        std::string tpslMode;
        double liqPrice = 0.0;
        std::string category;
        std::string positionStatus;
        int adlRankIndicator = 0;
        bool isReduceOnly = false;

        // Existing WebSocket parsing method
        void parse(const nlohmann::json& item) {
            if (item.contains("positionIdx")) positionIdx = item["positionIdx"];
            if (item.contains("tradeMode")) tradeMode = item["tradeMode"];
            if (item.contains("riskId")) riskId = item["riskId"];
            if (item.contains("riskLimitValue")) riskLimitValue = item["riskLimitValue"];
            if (item.contains("symbol")) symbol = item["symbol"];
            if (item.contains("side")) side = item["side"];
            if (item.contains("size") && !item["size"].get<std::string>().empty())
                size = std::stod(item["size"].get<std::string>());

            if (item.contains("entryPrice") && !item["entryPrice"].get<std::string>().empty())
                entryPrice = std::stod(item["entryPrice"].get<std::string>());
            if (item.contains("leverage") && !item["leverage"].get<std::string>().empty())
                leverage = std::stod(item["leverage"].get<std::string>());

            if (item.contains("positionValue") && !item["positionValue"].get<std::string>().empty())
                positionValue = std::stod(item["positionValue"].get<std::string>());
            if (item.contains("positionMM") && !item["positionMM"].get<std::string>().empty())
                positionMM = std::stod(item["positionMM"].get<std::string>());
            if (item.contains("positionIM") && !item["positionIM"].get<std::string>().empty())
                positionIM = std::stod(item["positionIM"].get<std::string>());
            if (item.contains("unrealisedPnl") && !item["unrealisedPnl"].get<std::string>().empty())
                unrealisedPnl = std::stod(item["unrealisedPnl"].get<std::string>());
            if (item.contains("cumRealisedPnl") && !item["cumRealisedPnl"].get<std::string>().empty())
                cumRealisedPnl = std::stod(item["cumRealisedPnl"].get<std::string>());
            if (item.contains("curRealisedPnl") && !item["curRealisedPnl"].get<std::string>().empty())
                curRealisedPnl = std::stod(item["curRealisedPnl"].get<std::string>());
            if (item.contains("liqPrice") && !item["liqPrice"].get<std::string>().empty())
                liqPrice = std::stod(item["liqPrice"].get<std::string>());

            if (item.contains("createdTime") && !item["createdTime"].get<std::string>().empty())
                createdTime = std::stoll(item["createdTime"].get<std::string>());
            if (item.contains("updatedTime") && !item["updatedTime"].get<std::string>().empty())
                updatedTime = std::stoll(item["updatedTime"].get<std::string>());
            // ... (rest of the existing parsing code remains the same)
        }

        // New method for REST API parsing
        void parseRest(const nlohmann::json& item) {
            // Parse REST API specific fields
            symbol = item.value("symbol", "");

            // Convert string fields to appropriate types
            side = item.value("side", "");

            // Leverage (potentially as string)
            try {
                leverage = std::stod(item.value("leverage", "0"));
            }
            catch (...) {
                leverage = 0.0;
            }

            // Size
            try {
                size = std::stod(item.value("size", "0"));
            }
            catch (...) {
                size = 0.0;
            }

            // Entry Price
            try {
                entryPrice = std::stod(item.value("avgPrice", "0"));
            }
            catch (...) {
                entryPrice = 0.0;
            }

            // Position Value
            try {
                positionValue = std::stod(item.value("positionValue", "0"));
            }
            catch (...) {
                positionValue = 0.0;
            }

            // Mark Price
            try {
                markPrice = std::stod(item.value("markPrice", "0"));
            }
            catch (...) {
                markPrice = 0.0;
            }

            // Unrealized PnL
            try {
                unrealisedPnl = std::stod(item.value("unrealisedPnl", "0"));
            }
            catch (...) {
                unrealisedPnl = 0.0;
            }

            // Additional fields
            riskLimitValue = item.value("riskLimitValue", "");
            tpslMode = item.value("tpslMode", "");
            positionStatus = item.value("positionStatus", "");

            // Boolean and integer fields
            isReduceOnly = item.value("isReduceOnly", false);
            adlRankIndicator = item.value("adlRankIndicator", 0);

            // Time-related fields (as strings)
            try {
                createdTime = std::stoll(item.value("createdTime", "0"));
                updatedTime = std::stoll(item.value("updatedTime", "0"));
            }
            catch (...) {
                createdTime = 0;
                updatedTime = 0;
            }
        }

        // Existing display method
        void display() const {
            std::cout << "Symbol: " << symbol << "\n"
                << "Side: " << side << "\n"
                << "Size: " << size << "\n"
                << "Entry Price: " << entryPrice << "\n"
                << "Position Value: " << positionValue << "\n"
                << "Mark Price: " << markPrice << "\n"
                << "Unrealized PnL: " << unrealisedPnl << "\n"
                << "Position Status: " << positionStatus << "\n";
        }
    };

public:
    // Existing WebSocket fields
    std::string id;
    std::string topic;
    long long creationTime = 0;

    // New REST API fields
    int retCode = 0;
    std::string retMsg;
    long long serverTime = 0;
    std::string nextPageCursor;
    std::string category;

    std::vector<PositionItem> data;
};



class LiquidationData
{
public:
    // Eingebettete Struktur für das innere Datenobjekt
    struct Data {
        long long updatedTime;
        std::string symbol;
        std::string side;
        double size;
        double price;
    };

    std::string topic;
    std::string type;
    long long ts;
    Data data;

    // Funktion, um JSON-Daten in ein LiquidationData-Objekt zu parsen
    void parse(const nlohmann::json& j)
    {
        try
        {
            topic = j.at("topic").get<std::string>();
            type = j.at("type").get<std::string>();
            ts = j.at("ts").get<long long>();

            // Datenfeld parsen
            const auto& dataField = j.at("data");
            data.updatedTime = dataField.at("updatedTime").get<long long>();
            data.symbol = dataField.at("symbol").get<std::string>();
            data.side = dataField.at("side").get<std::string>();
            data.size = std::stod(dataField.at("size").get<std::string>());
            data.price = std::stod(dataField.at("price").get<std::string>());
        }
        catch (const std::exception e)
        {
            std::cerr << "failed parsing LiquidationData" << std::endl;
        }
    }

    // Funktion, um die Daten anzuzeigen
    void print() const {
        std::cout << "Topic: " << topic << "\n";
        std::cout << "Type: " << type << "\n";
        std::cout << "Timestamp: " << ts << "\n";
        std::cout << "Data:\n";
        std::cout << "  Updated Time: " << data.updatedTime << "\n";
        std::cout << "  Symbol: " << data.symbol << "\n";
        std::cout << "  Side: " << data.side << "\n";
        std::cout << "  Size: " << data.size << "\n";
        std::cout << "  Price: " << data.price << "\n";
    }
};


class ExecutionData {
public:
    ExecutionData() = default;

    // Funktion zum Parsen des JSON-Objekts
    void parse(const nlohmann::json& json) {
        if (json.contains("topic")) topic = json["topic"];
        if (json.contains("creationTime")) creationTime = json["creationTime"];

        // Daten innerhalb des "data" Arrays parsen
        if (json.contains("data") && json["data"].is_array()) {
            for (const auto& item : json["data"]) {
                ExecutionItem execItem;
                execItem.parse(item);
                data.push_back(execItem);
            }
        }
    }

    // Funktion zum Anzeigen der Daten (optional)
    void display() const {
        std::cout << "Topic: " << topic << "\n";
        std::cout << "Creation Time: " << creationTime << "\n";
        for (const auto& item : data) {
            item.display();
        }
    }

private:
    class ExecutionItem {
    public:
        std::string category;
        std::string symbol;
        std::string execId;
        double execPrice = 0.0;
        double execQty = 0.0;
        std::string orderId;
        bool isMaker = false;
        std::string orderLinkId;
        std::string side;
        long long execTime = 0;
        long long seq = 0;

        void parse(const nlohmann::json& item) {
            if (item.contains("category")) category = item["category"];
            if (item.contains("symbol")) symbol = item["symbol"];
            if (item.contains("execId")) execId = item["execId"];
            if (item.contains("execPrice") && !item["execPrice"].get<std::string>().empty())
                execPrice = std::stod(item["execPrice"].get<std::string>());
            if (item.contains("execQty") && !item["execQty"].get<std::string>().empty())
                execQty = std::stod(item["execQty"].get<std::string>());
            if (item.contains("orderId")) orderId = item["orderId"];
            if (item.contains("isMaker")) isMaker = item["isMaker"];
            if (item.contains("orderLinkId")) orderLinkId = item["orderLinkId"];
            if (item.contains("side")) side = item["side"];

            if (item.contains("execTime") && !item["execTime"].get<std::string>().empty())
                execTime = std::stoll(item["execTime"].get<std::string>());
            //if (item.contains("seq") && !item["seq"].get<std::string>().empty())
            //    seq = std::stoll(item["seq"].get<std::string>());
        }

        void display() const {
            std::cout << "Category: " << category << "\n"
                << "Symbol: " << symbol << "\n"
                << "Exec ID: " << execId << "\n"
                << "Exec Price: " << execPrice << "\n"
                << "Exec Qty: " << execQty << "\n"
                << "Order ID: " << orderId << "\n"
                << "Is Maker: " << (isMaker ? "true" : "false") << "\n"
                << "Order Link ID: " << orderLinkId << "\n"
                << "Side: " << side << "\n"
                << "Exec Time: " << execTime << "\n"
                << "Seq: " << seq << "\n";
        }
    };

public:
    std::string topic;
    long long creationTime = 0;
    std::vector<ExecutionItem> data;
};
