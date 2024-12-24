// SampleXChangeClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include <thread>
#include "../libxchange/BybitWebsocketClient.h"


class MyBybitListener : public BybitListener
{
public:
    virtual void OnTicker(const TickerData& ticker) 
    {
        std::cout << "Last Price: " << ticker.lastPrice << std::endl;
    };

    virtual void OnPrivateOrder(const OrderData& order)
    {
        // IME TODO: cb wrong here, move to PrivateBybitListener
    };

    virtual void OnFastExecution(const ExecutionData& ed)
    {
        // IME TODO: cb wrong here, move to PrivateBybitListener
    };

    virtual void OnPosition(const PositionData& ed)
    {
        // IME TODO: cb wrong here, move to PrivateBybitListener
    };

};

int main()
{
    BybitWebSocketClient bybit;
    MyBybitListener myBybitListener;


    std::string symbol = "BTCUSDT";
    const std::string perpetualUrl = "wss://stream.bybit.com/v5/public/linear";
    const std::string spotUrl = "wss://stream.bybit.com/v5/public/spot";

    bybit.ConnectPublic(perpetualUrl);
    // IME: whatever u need
    // IME: public stream only here, no API key needed
    bybit.SendMessageToXChange("{\"op\": \"subscribe\", \"args\": [\"tickers." + std::string(symbol) + "\"]}");
    bybit.SendMessageToXChange("{\"op\": \"subscribe\", \"args\": [\"publicTrade." + std::string(symbol) + "\"]}");
    bybit.SendMessageToXChange("{\"op\": \"subscribe\", \"args\": [\"orderbook.50." + std::string(symbol) + "\"]}");
    bybit.SendMessageToXChange("{\"op\": \"subscribe\", \"args\": [\"liquidation." + std::string(symbol) + "\"]}");

    auto pb = bybit.GetPublicTrade();
    // IME TODO: will add code for private steam also here, have to cleanup first


    // IME TODO: will add code for placing/amending/canceling orders here, have to cleanup first

    bybit.Addlistener(&myBybitListener);

    long numTrades = 0;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bybit.OnFrame();

        if (numTrades < pb->allTrades.size())
        {
            auto pt = pb->allTrades[pb->allTrades.size() - 1];
            
            std::cout << "Trade: " << pt.side << " " << pt.volume << " " << symbol << " " << pt.price << std::endl;
            numTrades = pb->allTrades.size();
        }

    }
}


