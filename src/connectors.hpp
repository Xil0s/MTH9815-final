/**
 * connectors.hpp
 *
 * This file defines connector classes for interfacing with external data sources
 * and writing data to output files. Connectors are used to facilitate the
 * ingestion and export of financial data such as trades, prices, and market data.
 *
 * @author: Edgar Gonzalez
 */

#ifndef CONNECTORS_HPP
#define CONNECTORS_HPP

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <chrono>
#include "soa.hpp"
#include "utils.hpp"
#include "products.hpp"
#include "pricingservice.hpp"
#include "marketdataservice.hpp"
#include "tradebookingservice.hpp"
#include "positionservice.hpp"
#include "riskservice.hpp"
#include "executionservice.hpp"
#include "streamingservice.hpp"
#include "inquiryservice.hpp"
#include "historicaldataservice.hpp"

/******************************************************************************
 * TradeBookingConnector
 * Subscribe-only: no Publish; OnMessage() -> tradebookingservice->OnMessage().
 *****************************************************************************/
template<typename V>
class TradeBookingConnector : public Connector<Trade<V>>
{
private:
    std::string file_name;
    TradeBookingService<V>* tradebookingservice;

public:
    TradeBookingConnector(std::string _file_name, TradeBookingService<V>* _tradebookingservice)
        : file_name(_file_name), tradebookingservice(_tradebookingservice) {}

    void Publish(const Trade<V> &data) {}

    void OnMessage(const Trade<V> &data)
    {
        tradebookingservice->OnMessage(data);
    }

    void TraverseTrades(){
    std::ifstream f(file_name);
    if (!f.is_open())
    {
        std::cout << "Error: could not open trades file: " << file_name << std::endl;
        return;
    }

    std::string line;
    while (std::getline(f, line))
    {
        std::cout << "[TradeBookingConnector] Reading trade line: " 
                  << line << std::endl;

        // Parse the line to create a Trade<Bond>.
        auto tokens = FormatParser::ParseCommaSepLine(line);
        if (tokens.size() < 6) continue;

        Bond bond = ProductMap::GetProductMap()[tokens[0]];
        Trade<Bond> trade(bond, tokens[1], std::stod(tokens[4]), tokens[2],
                          std::stol(tokens[3]), (tokens[5] == "BUY" ? BUY : SELL));

        // Forward to the service
        tradebookingservice->OnMessage(trade);
    }
    std::cout << "[TradeBookingConnector] Done reading trades.\n";
    f.close();
    }
};

/******************************************************************************
 * PositionConnector
 * Publish-only: writes positions to a file, no inbound OnMessage.
 *****************************************************************************/
template<typename V>
class PositionConnector : public Connector<Position<V>>
{
private:
    std::string file_name;

public:
    explicit PositionConnector(std::string file_name_)
        : file_name(file_name_)
    {
        std::ofstream out(file_name, std::ios::trunc);
        out.close();
    }

    void Publish(const Position<V> &data)
    {
        std::ofstream out(file_name, std::ios::app);

        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        // Local mutable strings
        std::string b1 = "TRSY1";
        std::string b2 = "TRSY2";
        std::string b3 = "TRSY3";

        long p1 = data.GetPosition(b1);
        long p2 = data.GetPosition(b2);
        long p3 = data.GetPosition(b3);
        long aggPos = p1 + p2 + p3;

        out << now_ms << ","
            << data.GetProduct().GetTicker() << ","
            << p1 << ","
            << p2 << ","
            << p3 << ","
            << aggPos << "\n";

        out.close();
    }

    void OnMessage(Position<V> &data) {}
};


/******************************************************************************
 * RiskConnector
 * Publish-only: writes PV01 data to a file, no inbound OnMessage.
 *****************************************************************************/
template<typename V>
class RiskConnector : public Connector<PV01<V>>
{
private:
    std::string file_name;

public:
    explicit RiskConnector(std::string file_name_)
        : file_name(file_name_)
    {
        std::ofstream out(file_name, std::ios::trunc);
        out.close();
    }

    void Publish(const PV01<V> &data)
    {
        std::ofstream out(file_name, std::ios::app);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

        double totalRisk = data.GetPV01() * data.GetQuantity();
        out << now_ms << ","
            << data.GetProduct().GetTicker() << ","
            << totalRisk << "\n";
        out.close();
    }

    void OnMessage(PV01<V> &data) {}
};

/******************************************************************************
 * PricingConnector
 * Subscribe-only: reads from file, calls pricing_service->OnMessage(). No Publish.
 *****************************************************************************/
template<typename V>
class PricingConnector : public Connector<Price<V>>
{
private:
    std::string file_name;
    PricingService<V>* pricing_service;

public:
    // Constructor
    PricingConnector(const std::string &file_name_, PricingService<V>* pricingService_)
        : file_name(file_name_), pricing_service(pricingService_) {}

    // Publish is a no-op in this design (subscribe-only)
    void Publish(const Price<V> &data)
    {
        // no-op
    }

    // OnMessage is also no-op here if we only push data into the service from Subscribe()
    void OnMessage(Price<V> &data)
    {
        // no-op
    }

    // Subscribe() reads from prices.txt, logs progress, parses, and calls pricing_service->OnMessage(...)
    void Subscribe()
    {
        std::ifstream f(file_name);
        if (!f.is_open())
        {
            std::cout << "[PricingConnector] ERROR: cannot open " << file_name << std::endl;
            return;
        }

        std::cout << "[PricingConnector] Reading prices from " << file_name << " ...\n";

        std::string line;
        int lineCount = 0;

        while (std::getline(f, line))
        {
            lineCount++;
            // Log progress every 100,000 lines to avoid spamming the console
            if (lineCount % 100000 == 0)
            {
                std::cout << "[PricingConnector] Processed " << lineCount << " price lines.\n";
            }

            auto tokens = FormatParser::ParseCommaSepLine(line);
            if (tokens.size() < 3) continue;  // need at least productID, bid, ask

            // Map product ID to a Bond object
            auto product_map = ProductMap::GetProductMap();
            if (product_map.find(tokens[0]) == product_map.end()) continue;
            V bond = product_map[tokens[0]];

            // Convert fractional strings (like 99-16) to decimal double
            double bid = FormatParser::ParsePriceFormat(tokens[1]);
            double ask = FormatParser::ParsePriceFormat(tokens[2]);

            double mid    = (bid + ask) / 2.0;
            double spread = ask - bid;

            // Create a Price object
            Price<V> price(bond, mid, spread);

            // Send to PricingService
            pricing_service->OnMessage(price);
        }

        std::cout << "[PricingConnector] Finished reading " << lineCount << " price lines.\n";
        f.close();
    }
};

/******************************************************************************
 * GUIConnector
 * Publish-only: writes Price<V> data to a file (throttled at service level).
 *****************************************************************************/
template<typename V>
class GUIConnector : public Connector<Price<V>>
{
private:
    std::string file_name;

public:
    explicit GUIConnector(std::string file_name_)
        : file_name(file_name_)
    {
        std::ofstream out(file_name, std::ios::trunc);
        out.close();
    }

    void Publish(const Price<V> &data)
    {
        std::ofstream out(file_name, std::ios::app);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

        out << now_ms << ","
            << data.GetProduct().GetTicker() << ","
            << data.GetMid() << ","
            << data.GetBidOfferSpread() << "\n";
        out.close();
    }

    void OnMessage(Price<V> &data) {}
};

/******************************************************************************
 * StreamingConnector
 * Publish-only: writes PriceStream<V> to a file, no inbound OnMessage.
 *****************************************************************************/
template<typename V>
class StreamingConnector : public Connector<PriceStream<V>>
{
private:
    std::string file_name;

public:
    explicit StreamingConnector(const std::string &file_name_)
        : file_name(file_name_)
    {
        std::ofstream out(file_name, std::ios::trunc);
        out.close();
    }

    void Publish(const PriceStream<V> &data)
    {
        std::ofstream out(file_name, std::ios::app);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

        out << now_ms << ","
            << data.GetProduct().GetTicker() << ","
            << data.GetBidOrder().GetPrice() << ","
            << data.GetOfferOrder().GetPrice() << "\n";
        out.close();
    }

    void OnMessage(PriceStream<V> &data) {}
};

/******************************************************************************
 * MrktDataConnector
 * Subscribe-only: reads order book data, calls marketdataservice->ProcessOrderBook().
 *****************************************************************************/
template<typename T>
class MrktDataConnector : public Connector<OrderBook<T>>{
private:
    std::string file_name;
    MarketDataService<T>* marketdataservice;

public:
    // Constructor
    MrktDataConnector(const std::string &file_name_, MarketDataService<T>* mds)
        : file_name(file_name_), marketdataservice(mds) {}

    // No outbound publish in this design
    void Publish(const  OrderBook<T> &data)
    {
        // no-op
    }

    // No inbound OnMessage needed because we pull from file in Subscribe()
    void OnMessage(OrderBook<T> &data)
    {
        // no-op
    }

    // Subscribe reads from marketdata.txt, logs progress, parses lines, calls MarketDataService->ProcessOrderBook(...)
    void Subscribe()
    {
        std::ifstream f(file_name);
        if (!f.is_open())
        {
            std::cout << "[MrktDataConnector] ERROR: cannot open " << file_name << std::endl;
            return;
        }

        std::cout << "[MrktDataConnector] Reading order books from " << file_name << " ...\n";

        std::string line;
        int lineCount = 0;
        auto product_map = ProductMap::GetProductMap();

        while (std::getline(f, line))
        {
            lineCount++;
            // Log progress periodically
            if (lineCount % 100000 == 0)
            {
                std::cout << "[MrktDataConnector] Processed " << lineCount 
                          << " orderbook lines.\n";
            }

            auto parsed = FormatParser::ParseOrderBook(line);
            std::string ticker = std::get<0>(parsed);
            auto px = std::get<1>(parsed); // vector<double>

            if (product_map.find(ticker) == product_map.end()) continue;
            T product = product_map[ticker];

            std::vector<Order> bidStack, askStack;
            for (int i = 0; i < 5; i++)
            {
                double bidP = px[2*i];
                double askP = px[2*i + 1];
                long size   = 1000000 * (i+1);

                bidStack.emplace_back(bidP, size, BID);
                askStack.emplace_back(askP, size, OFFER);
            }

            // Build an OrderBook
            OrderBook<T> obook(product, bidStack, askStack);

            // Forward to MarketDataService
            marketdataservice->ProcessOrderBook(obook);
        }

        std::cout << "[MrktDataConnector] Finished reading " << lineCount 
                  << " orderbook lines.\n";
        f.close();
    }
};

/******************************************************************************
 * ExecutionConnector
 * Publish-only: writes ExecutionOrder<V> to a file, no inbound OnMessage.
 *****************************************************************************/
template<typename V>
class ExecutionConnector : public Connector<ExecutionOrder<V>>
{
private:
    std::string file_name;

public:
    explicit ExecutionConnector(std::string file_name_)
        : file_name(file_name_)
    {
        std::ofstream out(file_name, std::ios::trunc);
        out.close();
    }

    void Publish(const ExecutionOrder<V> &data)
    {
        std::ofstream out(file_name, std::ios::app);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

        std::string side = (data.GetPricingSide() == BID ? "BUY" : "SELL");
        out << now_ms << ","
            << data.GetProduct().GetTicker() << ","
            << "TID_" << data.GetOrderId() << ","
            << "MarketOrder" << ","
            << side << ","
            << data.GetPrice() << ","
            << data.GetVisibleQuantity() << ","
            << data.GetHiddenQuantity() << "\n";
        out.close();
    }

    void OnMessage(ExecutionOrder<V> &data) {}
};

/******************************************************************************
 * InquiryConnector
 * Bidirectional: reads inbound inquiries, publishes states back to InquiryService.
 *****************************************************************************/
template<typename V>
class InquiryConnector : public Connector<Inquiry<V>>
{
private:
    std::string file_name;
    InquiryService<V>* inquiry_service;

public:
    InquiryConnector(std::string file_name_, InquiryService<V>* inquiry_service_)
        : file_name(file_name_), inquiry_service(inquiry_service_) {}

    void Publish(const Inquiry<V>& data)
    {
        if(data.GetState() == RECEIVED)
        {
            Inquiry<V> q = data;
            q.SetState(QUOTED);
            inquiry_service->OnMessage(q);

            q.SetState(DONE);
            inquiry_service->OnMessage(q);
        }
    }

    void OnMessage(Inquiry<V> &data)
    {
        inquiry_service->OnMessage(data);
    }

    void Subscribe(){
        std::ifstream f(file_name);
        if (!f.is_open())
        {
            std::cout << "Error: cannot open inquiries file: " << file_name << std::endl;
            return;
        }

        std::string line;
        while (std::getline(f, line))
        {
            std::cout << "[InquiryConnector] Reading inquiry line: " 
                      << line << std::endl;

            auto tokens = FormatParser::ParseCommaSepLine(line);
            if (tokens.size() < 3) continue;

            std::string inqId = tokens[0];
            Bond product = ProductMap::GetProductMap()[tokens[1]];
            Side side = (tokens[2] == "BUY" ? BUY : SELL);
            Inquiry<Bond> inquiry(inqId, product, side, 1000000, -1, RECEIVED);

            // Pass to the service
            inquiry_service->OnMessage(inquiry);
        }

        std::cout << "[InquiryConnector] Done reading inquiries.\n";
        f.close();
    }

};

/******************************************************************************
 * AllInquiriesConnector
 * Publish-only: writes inquiry data to a file, no inbound OnMessage.
 *****************************************************************************/
template<typename V>
class AllInquiriesConnector : public Connector<Inquiry<V>>
{
private:
    std::string file_name;

public:
    explicit AllInquiriesConnector(std::string file_name_)
        : file_name(file_name_)
    {
        std::ofstream out(file_name, std::ios::trunc);
        out.close();
    }

    void Publish(const Inquiry<V> &data)
    {
        std::ofstream out(file_name, std::ios::app);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

        std::string s = (data.GetSide() == BUY ? "BUY" : "SELL");
        InquiryState st = data.GetState();
        std::string state;
        if(st == RECEIVED) state = "RECEIVED";
        else if(st == QUOTED) state = "QUOTED";
        else if(st == DONE) state = "DONE";
        else if(st == REJECTED) state = "REJECTED";

        out << now_ms << ","
            << "TID_" << data.GetInquiryId() << ","
            << data.GetProduct().GetTicker() << ","
            << s << ","
            << data.GetPrice() << ","
            << state << "\n";
        out.close();
    }

    void OnMessage(Inquiry<V> &data) {}
};

#endif // CONNECTORS_HPP
