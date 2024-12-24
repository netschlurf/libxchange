#include "pch.h"
#include "XChangeObjects.h"


void Orderbook::parseSnapshot(const nlohmann::json& parsedData) {

    highestVolume = 0;
    lowestVolume = 10000000000000;
    long long srvTs = parsedData["ts"].get<long long>();
    TimeStampMS = srvTs;
    // Check if data is in the expected format
    if (parsedData.contains("data")) {
        const auto& data = parsedData["data"];
        symbol = parsedData["topic"].get<std::string>();

        // Parse bids and asks
        for (const auto& bid : data["b"]) {
            float units = std::stof(bid[1].get<std::string>());
            bids[std::stof(bid[0].get<std::string>())] = units;
            if (units > highestVolume)
                highestVolume = units;
            if (units < lowestVolume)
                lowestVolume = units;
        }
        for (const auto& ask : data["a"]) {
            float units = std::stof(ask[1].get<std::string>());
            asks[std::stof(ask[0].get<std::string>())] = units;
            if (units > highestVolume)
                highestVolume = units;
            if (units < lowestVolume)
                lowestVolume = units;
        }
    }
}

void Orderbook::parseDelta(const nlohmann::json& parsedData) {
    // Check if data is in the expected format
    long long srvTs = parsedData["ts"].get<long long>();
    TimeStampMS = srvTs;
    if (parsedData.contains("data")) {
        const auto& data = parsedData["data"];

        // Parse bids and asks (if present in the delta)
        if (data.contains("b")) {
            for (const auto& bid : data["b"]) {
                float units = std::stof(bid[1].get<std::string>());
                if (units <= 0.0000001)
                    bids.erase(std::stof(bid[0].get<std::string>()));
                else
                    bids[std::stof(bid[0].get<std::string>())] = units;
                if (units > highestVolume)
                    highestVolume = units;
                if (units < lowestVolume)
                    lowestVolume = units;
            }
        }
        if (data.contains("a")) {
            for (const auto& ask : data["a"]) {
                float units = std::stof(ask[1].get<std::string>());
                if (units <= 0.0000001)
                    asks.erase(std::stof(ask[0].get<std::string>()));
                else
                    asks[std::stof(ask[0].get<std::string>())] = units;
                if (units > highestVolume)
                    highestVolume = units;
                if (units < lowestVolume)
                    lowestVolume = units;
            }
        }
    }
}

void Orderbook::Calculate()
{
    double lowestAsk = 10000000;
    double highestBid = 0;

    for (auto& ask : asks)
    {
        if (ask.first < lowestAsk)
            lowestAsk = ask.first;
    }
    for (auto& bid : bids)
    {
        if (bid.first > highestBid)
            highestBid = bid.first;
    }
    m_SpreadAsk = lowestAsk;
    m_SpreadBid = highestBid;

    double rateSum = 0;
    for (size_t i = 0; i < m_LastBidRates.size(); i++)
    {
        rateSum += m_LastBidRates[i];
    }
    rateSum /= m_MALength;
    m_MABuyRate = rateSum;

    double askVolSum = 0;
    for (size_t i = 0; i < m_LastAskVolumes.size(); i++)
    {
        askVolSum += m_LastAskVolumes[i];
    }
    askVolSum /= m_MALength;
    m_MAAskVolume = askVolSum;

    double bidVolSum = 0;
    for (size_t i = 0; i < m_LastBidVolumes.size(); i++)
    {
        bidVolSum += m_LastBidVolumes[i];
    }
    bidVolSum /= m_MALength;
    m_MABidVolume = bidVolSum;

    double totalVolSum = 0;
    for (size_t i = 0; i < m_LastTotalVolumes.size(); i++)
    {
        totalVolSum += m_LastTotalVolumes[i];
    }
    totalVolSum /= m_MALength;
    m_MATotalVolume = totalVolSum;


    double rateAskSum = 0;
    for (size_t i = 0; i < m_LastAskRates.size(); i++)
    {
        rateAskSum += m_LastAskRates[i];
    }
    rateAskSum /= m_MALength;

    if (m_bFilterOn)
        Filter();
    CalcBalance();
}

void Orderbook::CalcBalance()
{
    double aVolume = 0;
    double bVolume = 0;
    for (auto& ask : asks)
    {
        aVolume += ask.second;
    }
    for (auto& bid : bids)
    {
        bVolume += bid.second;
    }

    double volume = aVolume + bVolume;

    askRate = aVolume / volume;
    bidRate = bVolume / volume;
    AskVolume = aVolume;
    BidVolume = bVolume;

    m_LastBidRates.push_back(bidRate);
    while (m_LastBidRates.size() > m_MALength)
    {
        m_LastBidRates.erase(m_LastBidRates.begin(), m_LastBidRates.begin() + 1);
    }

    m_LastAskRates.push_back(askRate);
    while (m_LastAskRates.size() > m_MALength)
    {
        m_LastAskRates.erase(m_LastAskRates.begin(), m_LastAskRates.begin() + 1);
    }

    m_LastAskVolumes.push_back(AskVolume);
    while (m_LastAskVolumes.size() > m_MALength)
    {
        m_LastAskVolumes.erase(m_LastAskVolumes.begin(), m_LastAskVolumes.begin() + 1);
    }
    m_LastBidVolumes.push_back(BidVolume);
    while (m_LastBidVolumes.size() > m_MALength)
    {
        m_LastBidVolumes.erase(m_LastBidVolumes.begin(), m_LastBidVolumes.begin() + 1);
    }

    m_LastTotalVolumes.push_back(BidVolume + AskVolume);
    while (m_LastTotalVolumes.size() > m_MALength)
    {
        m_LastTotalVolumes.erase(m_LastTotalVolumes.begin(), m_LastTotalVolumes.begin() + 1);
    }
}

void Orderbook::Filter()
{
    std::vector<double> askKill;
    std::vector<double> bidKill;
    double spreadFilterAsk = m_SpreadAsk * (1.0 + m_FilterRange / 100);
    double spreadFilterBid = m_SpreadBid * (1.0 - m_FilterRange / 100);
    for (auto& ask : asks)
    {
        if (ask.first > spreadFilterAsk)
            askKill.push_back(ask.first);
        //if(ask.second > 8)
        //    askKill.push_back(ask.first);
    }
    for (auto& bid : bids)
    {
        if (bid.first < spreadFilterBid)
            bidKill.push_back(bid.first);
        //if (bid.second > 8)
        //    bidKill.push_back(bid.first);
    }

    for (size_t i = 0; i < askKill.size(); i++)
    {
        asks.erase(askKill[i]);
    }
    for (size_t i = 0; i < bidKill.size(); i++)
    {
        bids.erase(bidKill[i]);
    }




    highestVolume = 0;
    lowestVolume = 10000000000000;
    for (auto& ask : asks)
    {
        if (ask.second > highestVolume)
            highestVolume = ask.second;
        if (ask.second < lowestVolume)
            lowestVolume = ask.second;
    }
    for (auto& bid : bids)
    {
        if (bid.second > highestVolume)
            highestVolume = bid.second;
        if (bid.second < lowestVolume)
            lowestVolume = bid.second;
    }
}

void Orderbook::display() const {
    std::cout << "Orderbook for " << symbol << ":\n"
        << "Bids:\n";
    for (const auto& bid : bids) {
        std::cout << "  " << bid.first << " x " << bid.second << "\n";
    }
    std::cout << "Asks:\n";
    for (const auto& ask : asks) {
        std::cout << "  " << ask.first << " x " << ask.second << "\n";
    }
}

void PublicTrade::parsePublicTradeSnapshot(const nlohmann::json& parsedData) {
    trades.clear();

    if (parsedData.contains("data")) {
        const auto& data = parsedData["data"];
        symbol = data[0]["s"].get<std::string>();  // Beispiel: "SOLUSDT"

        for (const auto& trade : data) {
            long long timestamp = trade["T"].get<long long>();
            std::string side = trade["S"].get<std::string>();
            float volume = std::stof(trade["v"].get<std::string>());
            float price = std::stof(trade["p"].get<std::string>());
            std::string tickDirection;
            if (trade.contains("L"))
                tickDirection = trade["L"].get<std::string>();
            bool blockTrade = trade["BT"].get<bool>();

            auto isZero = tickDirection.find("Zero") == 0;
            //if (tickDirection.length()  && isZero)
            {
                trades.emplace_back(timestamp, side, volume, price, "tickDirection", "tradeId", blockTrade);
                allTrades.emplace_back(timestamp, side, volume, price, "tickDirection", "tradeId", blockTrade);
            }
        }
    }
#define PUBLIC_TRADE_BUFFER 1000
    if (allTrades.size() > PUBLIC_TRADE_BUFFER)
    {
        auto numDel = allTrades.size() - PUBLIC_TRADE_BUFFER;
        allTrades.erase(allTrades.begin(), allTrades.begin() + numDel);
    }
}

void PublicTrade::parsePublicTradeDelta(const nlohmann::json& parsedData) {
    if (parsedData.contains("data")) {
        const auto& data = parsedData["data"];

        for (const auto& trade : data) {
            long long timestamp = trade["T"].get<long long>();
            std::string side = trade["S"].get<std::string>();
            float volume = std::stof(trade["v"].get<std::string>());
            float price = std::stof(trade["p"].get<std::string>());
            std::string tickDirection = trade["L"].get<std::string>();
            std::string tradeId = trade["i"].get<std::string>();
            bool blockTrade = trade["BT"].get<bool>();

            trades.emplace_back(timestamp, side, volume, price, tickDirection, tradeId, blockTrade);
        }
    }
#define PUBLIC_TRADE_BUFFER 1000
    if (allTrades.size() > PUBLIC_TRADE_BUFFER)
    {
        auto numDel = allTrades.size() - PUBLIC_TRADE_BUFFER;
        allTrades.erase(allTrades.begin(), allTrades.begin() + numDel);
    }
}

void PublicTrade::display() const {
    std::cout << "Public Trades for " << symbol << ":\n";
    for (const auto& trade : trades) {
        std::cout << "Timestamp: " << trade.timestamp
            << ", Side: " << trade.side
            << ", Volume: " << trade.volume
            << ", Price: " << trade.price
            << ", Tick Direction: " << trade.tickDirection
            << ", Trade ID: " << trade.tradeId
            << ", Block Trade: " << (trade.blockTrade ? "Yes" : "No")
            << "\n";
    }
}

void PublicTrade::Calculate() {
    double totalBuyVolume = 0.0f;
    double totalSellVolume = 0.0f;
    double totalBuyPriceVolume = 0.0f;  // Summe von Preis * Volumen für Käufe
    double totalSellPriceVolume = 0.0f; // Summe von Preis * Volumen für Verkäufe
    int buyCount = 0;
    int sellCount = 0;

    for (const auto& trade : trades) {
        if (trade.side == "Buy") {
            totalBuyVolume += trade.volume;
            totalBuyPriceVolume += trade.price * trade.volume;
            buyCount++;
        }
        else if (trade.side == "Sell") {
            totalSellVolume += trade.volume;
            totalSellPriceVolume += trade.price * trade.volume;
            sellCount++;
        }
    }

    double avgBuyPrice = (buyCount > 0) ? totalBuyPriceVolume / totalBuyVolume : 0.0f;
    double avgSellPrice = (sellCount > 0) ? totalSellPriceVolume / totalSellVolume : 0.0f;
    double avgTradeSize = (buyCount + sellCount > 0) ? (totalBuyVolume + totalSellVolume) / (buyCount + sellCount) : 0.0f;

    m_LastBuyRates.push_back(totalBuyVolume);
    if (m_LastBuyRates.size() > m_MALength)
    {
        m_LastBuyRates.erase(m_LastBuyRates.begin(), m_LastBuyRates.begin() + 1);
    }

    m_LastSellRates.push_back(totalSellVolume);
    if (m_LastSellRates.size() > m_MALength)
    {
        m_LastSellRates.erase(m_LastSellRates.begin(), m_LastSellRates.begin() + 1);
    }

    double sVolume = 0;
    double bVolume = 0;
    for (auto& sell : m_LastSellRates)
    {
        sVolume += sell;
    }
    for (auto& buy : m_LastBuyRates)
    {
        bVolume += buy;
    }

    double volume = sVolume + bVolume;

    m_sellRate = sVolume / volume;
    m_buyRate = bVolume / volume;
    if (trades.size())
        TimeStamp = trades[trades.size() - 1].timestamp;
}

void OrderData::Parse(const nlohmann::json& data)
{
    if (data.contains("symbol")) symbol = data["symbol"];
    if (data.contains("orderId")) orderId = data["orderId"];
    if (data.contains("side")) side = data["side"];
    if (data.contains("orderType")) orderType = data["orderType"];
    if (data.contains("cancelType")) cancelType = data["cancelType"];
    if (data.contains("price") && !data["price"].get<std::string>().empty())
        price = std::stod(data["price"].get<std::string>());
    if (data.contains("qty") && !data["qty"].get<std::string>().empty())
        qty = std::stod(data["qty"].get<std::string>());
    if (data.contains("timeInForce")) timeInForce = data["timeInForce"];
    if (data.contains("orderStatus")) orderStatus = data["orderStatus"];
    if (data.contains("reduceOnly")) reduceOnly = data["reduceOnly"];
    if (data.contains("cumExecQty") && !data["cumExecQty"].get<std::string>().empty())
        cumExecQty = std::stod(data["cumExecQty"].get<std::string>());
    if (data.contains("cumExecValue") && !data["cumExecValue"].get<std::string>().empty())
        cumExecValue = std::stod(data["cumExecValue"].get<std::string>());
    if (data.contains("avgPrice") && !data["avgPrice"].get<std::string>().empty())
        avgPrice = std::stod(data["avgPrice"].get<std::string>());
    if (data.contains("cumExecFee") && !data["cumExecFee"].get<std::string>().empty())
        cumExecFee = std::stod(data["cumExecFee"].get<std::string>());
    if (data.contains("closedPnl")) closedPnl = data["closedPnl"];
    if (data.contains("createdTime")) createdTime = data["createdTime"];
    if (data.contains("updatedTime")) updatedTime = data["updatedTime"];
    if (data.contains("positionIdx")) positionIdx = data["positionIdx"];
    if (data.contains("closeOnTrigger")) closeOnTrigger = data["closeOnTrigger"];
    if (data.contains("category")) category = data["category"];
    if (data.contains("placeType")) placeType = data["placeType"];
    if (data.contains("smpType")) smpType = data["smpType"];
    if (data.contains("smpGroup")) smpGroup = data["smpGroup"];
    if (data.contains("feeCurrency")) feeCurrency = data["feeCurrency"];
    if (data.contains("rejectReason") && !data["rejectReason"].get<std::string>().empty())
        rejectReason = data["rejectReason"].get<std::string>();
}



TickerData::TickerData()
    : symbol(""), lastPrice(0.0), highPrice24h(0.0), lowPrice24h(0.0),
    volume24h(0.0), fundingRate(0.0), bid1Price(0.0), ask1Price(0.0),
    markPrice(0.0), indexPrice(0.0), openInterestValue(0.0) {}

void TickerData::parseSnapshot(const nlohmann::json& parsedData)
{
    long long ts = 0;
    if (parsedData.contains("ts"))
        ts = parsedData["ts"].get<long long>();

    // std::stof(

    timeStamp = ts / 1000;
    timeStampMS = ts;
    // Check if data is in the expected format
    if (parsedData.contains("data")) {
        const auto& data = parsedData["data"];
        symbol = data["symbol"].get<std::string>();
        lastPrice = std::stof(data["lastPrice"].get<std::string>());
        highPrice24h = std::stof(data["highPrice24h"].get<std::string>());
        lowPrice24h = std::stof(data["lowPrice24h"].get<std::string>());
        volume24h = std::stof(data["volume24h"].get<std::string>());
        //fundingRate = std::stof(data["fundingRate"].get<std::string>());
        //bid1Price = std::stof(data["bid1Price"].get<std::string>());
        //ask1Price = std::stof(data["ask1Price"].get<std::string>());
    }
}

void TickerData::parseDelta(const nlohmann::json& parsedData)
{

    long long ts = 0;
    if (parsedData.contains("ts"))
        ts = parsedData["ts"].get<long long>();

    // std::stof(

    timeStamp = ts / 1000;
    timeStampMS = ts;

    // Check if data is in the expected format
    if (parsedData.contains("data")) {
        const auto& data = parsedData["data"];
        // Update only the fields that are present in the delta
        if (data.contains("lastPrice")) {
            lastPrice = std::stof(data["lastPrice"].get<std::string>());
        }
        if (data.contains("markPrice")) {
            markPrice = std::stof(data["markPrice"].get<std::string>());
        }
        if (data.contains("indexPrice")) {
            indexPrice = std::stof(data["indexPrice"].get<std::string>());
        }
        if (data.contains("openInterestValue")) {
            openInterestValue = std::stof(data["openInterestValue"].get<std::string>());
        }
    }

    //display();
}

void TickerData::display() const {
    std::cout << "Ticker Data for " << symbol << ":\n"
        << "Last Price: " << lastPrice << "\n";
    //<< "High Price (24h): " << highPrice24h << "\n"
    //<< "Low Price (24h): " << lowPrice24h << "\n"
    //<< "Volume (24h): " << volume24h << "\n"
    //<< "Funding Rate: " << fundingRate << "\n"
    //<< "Bid Price: " << bid1Price << "\n"
    //<< "Ask Price: " << ask1Price << "\n"
    //<< "Mark Price: " << markPrice << "\n"
    //<< "Index Price: " << indexPrice << "\n"
    //<< "Open Interest Value: " << openInterestValue << "\n";
}


