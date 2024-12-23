/**
 * services.hpp
 *
 * This file defines the Service base class and associated functionalities
 * for managing financial data processing. Services act as the core components
 * that store data and interact with listeners.
 *
 * @author: Edgar Gonzalez
 */

#ifndef SERVICES_HPP
#define SERVICES_HPP

#include "soa.hpp"
#include "streamingservice.hpp"
#include "pricingservice.hpp"
#include "marketdataservice.hpp"
#include "executionservice.hpp"
#include "inquiryservice.hpp"

// If you need them:
#include "utils.hpp"     // ProductMap, FormatParser

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>

/**********************************************
 * GUIService
 **********************************************/
template<typename T>
class GUIConnector; // Forward-declare if needed

template<typename T>
class GUIService : public Service<std::string, Price<T> >
{
private:
    long long int service_start_time;
    long long int last_quote_time;
    GUIConnector<T>* gui_connector;
    
public:
    explicit GUIService(GUIConnector<T>* gui_connector_)
        : gui_connector(gui_connector_)
    {
        using namespace std::chrono;
        service_start_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        last_quote_time = service_start_time;
    }
    void ProvideData(Price<T> data)
    {
        using namespace std::chrono;
        long long int current_epic = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        if(current_epic - last_quote_time > 300)
        {
            last_quote_time = current_epic;
            gui_connector->Publish(data);
        }
    }
    // Other required Service<> methods can be no-ops if not used
};

/**********************************************
 * BondAlgoStreamingService
 **********************************************/
template <class V>
class BondAlgoStreamingService : public Service<std::string, PriceStream<V> >
{
public:
    void PublishPrice(Price<V> & data)
    {
        double bid_price = data.GetMid() - data.GetBidOfferSpread()/2;
        double ask_price = data.GetMid() + data.GetBidOfferSpread()/2;
        PriceStreamOrder bid_order(bid_price, 1000000, 1000000, BID);
        PriceStreamOrder ask_order(ask_price, 1000000, 1000000, OFFER);
        PriceStream<V> price_stream(data.GetProduct(), bid_order, ask_order);
        Service<std::string,PriceStream<V> >::Notify(price_stream);
    }
};

/**********************************************
 * BondAlgoExecutionService
 **********************************************/
template <class T>
class AlgoExecution
{
private:
    ExecutionOrder<T>* order;
    Market market;
    static int counter;
public:
    AlgoExecution(OrderBook<T> data_): market(CME)
    {
        counter++;
        PricingSide side = PricingSide(counter % 2); 
        std::string orderID = std::to_string(counter);
        T product = data_.GetProduct();
        double price;
        long quantity;
        double hidden_ratio = 0.9;

        if(side == BID){
            price    = data_.GetBidStack()[0].GetPrice();
            quantity = data_.GetOfferStack()[0].GetQuantity();
        } else {
            price    = data_.GetOfferStack()[0].GetPrice();
            quantity = data_.GetBidStack()[0].GetQuantity();
        }

        order = new ExecutionOrder<T>(product, side, orderID, MARKET,
                                      price, quantity, 
                                      static_cast<long>(quantity * hidden_ratio),
                                      orderID, false);
    }

    ExecutionOrder<T> GetOrder()
    {
        return *order;
    }
    Market GetMarket(){
        return market;
    }
    ~AlgoExecution(){
        delete order;
    }
};

template <class T>
int AlgoExecution<T>::counter = 0;


template <class T>
class BondAlgoExecutionService : public Service<std::string, AlgoExecution<T> >
{
public:
    void Execute(OrderBook<T> data_)
    {
        double best_bid   = data_.GetBidStack()[0].GetPrice();
        double best_offer = data_.GetOfferStack()[0].GetPrice();
        double spread = best_offer - best_bid;
        double tol    = 1.0 / 127.0;

        // Only cross the spread if it’s at the tightest (1/128 ~ 0.0078)
        if(spread <= tol)
        {
            AlgoExecution<T> algo_execution(data_);
            Service<std::string, AlgoExecution<T> >::Notify(algo_execution);
        }
    }
};

#endif // SERVICES_HPP
