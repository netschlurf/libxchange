#pragma once
#include <iostream>
#include <map>
#include <vector>
#include "XChangeObjects.h"



class Scale;
class TickerData;
class OrderData;
class ExecutionData;
class PositionData;

class BybitListener
{
public:
    virtual void OnTicker(const TickerData& ticker) = 0;
    virtual void OnPrivateOrder(const OrderData& order) = 0;
    virtual void OnFastExecution(const ExecutionData& ed) = 0;
    virtual void OnPosition(const PositionData& ed) = 0;
};



class BybitWebSocketClient {
public:
    BybitWebSocketClient() {};
    ~BybitWebSocketClient();

    bool ConnectPublic(const std::string& url);
    bool ConnectPrivate();
    bool IsConnected();

    // if connected to public, used for sending subscriptions to xchange
    bool SendMessageToXChange(const std::string& message);

    void OnFrame();

    

    // do not use, i another file will provide better impl
    // void PlaceLimitOrder(long long timeStamp, int reqId, const std::string& symbol, const std::string& side, const std::string& qty, const std::string& price);

    void            Addlistener(BybitListener* listener);

    Orderbook*      GetOrderbook() { return &m_OrderBookData; };
    PublicTrade*    GetPublicTrade() { return &m_PublicTrade; };
    TickerData*     GetTicker() { return &m_TicketData; };

private:
    void SubscribeToPositionsAndOrders();
    bool ReceiveMessage();
    bool ReceiveMessageSync();
    std::string ReceiveMessageRest();
    void DispatchBybitMessage(const std::string& message);

protected:
    std::string url_;
    

    TickerData m_TicketData;
    Orderbook m_OrderBookData;
    PublicTrade m_PublicTrade;

    std::vector<BybitListener*> m_BybitListeners;
    long long m_LastPing;
    std::vector<PublicTrade::Trade> m_TradeLog;
    long long m_LastConnectionCheck;

    std::vector<double> m_ObCenters;
    std::vector<double> m_ObCentersAvg;
};

