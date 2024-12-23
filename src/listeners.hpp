/**
 * listeners.hpp
 *
 * This file defines the listener interfaces and base implementations for
 * observing updates to data services. Listeners are used to enable event-driven
 * processing in the financial system.
 *
 * @author: Edgar Gonzalez
 */

#ifndef LISTENERS_HPP
#define LISTENERS_HPP

#include "soa.hpp"
#include "products.hpp"
#include "tradebookingservice.hpp"
#include "positionservice.hpp"
#include "riskservice.hpp"
#include "executionservice.hpp"
#include "streamingservice.hpp"
#include "inquiryservice.hpp"
#include "historicaldataservice.hpp"
#include "services.hpp" 

/************************************************************************************************
 * PositionServiceListener
 * Listens to Trade<T> and forwards trades to PositionService<T>::AddTrade(...)
 * 
 * Referenced by: 
 *   PositionServiceListener<Bond> position_listener(&pos_service);
 ************************************************************************************************/
template<typename T>
class PositionServiceListener : public ServiceListener<Trade<T>>
{
private:
    PositionService<T> *positionService;

public:
    explicit PositionServiceListener(PositionService<T>* posService)
        : positionService(posService) {}

    void ProcessAdd(Trade<T>& data) override
    {
        positionService->AddTrade(data);
    }
    void ProcessRemove(Trade<T>& data) override {}
    void ProcessUpdate(Trade<T>& data) override {}
};

/************************************************************************************************
 * HistPositionListener
 * Listens to Position<T> from a PositionService and persists them via 
 * HistoricalDataService<Position<T>> -> PersistData(...)
 * 
 * Referenced by:
 *   HistPositionListener<Bond> history_position_listener(&position_history);
 ************************************************************************************************/
template<typename T>
class HistPositionListener : public ServiceListener<Position<T>>
{
private:
    HistoricalDataService<Position<T>> *histService;

public:
    explicit HistPositionListener(HistoricalDataService<Position<T>>* hs)
        : histService(hs) {}

    void ProcessAdd(Position<T>& data) override
    {
        histService->PersistData(data.GetProduct().GetProductId(), data);
    }
    void ProcessRemove(Position<T>& data) override {}
    void ProcessUpdate(Position<T>& data) override {}
};

/************************************************************************************************
 * RiskServiceListener
 * Listens to Position<T> from a PositionService, calls RiskService<T>::AddPosition(...)
 * 
 * Referenced by:
 *   RiskServiceListener<Bond> risk_service_listener(&risk_service_instance);
 ************************************************************************************************/
template<typename T>
class RiskServiceListener : public ServiceListener<Position<T>>
{
private:
    RiskService<T> *riskService;

public:
    explicit RiskServiceListener(RiskService<T>* rService)
        : riskService(rService) {}

    void ProcessAdd(Position<T>& data) override
    {
        riskService->AddPosition(data);
    }
    void ProcessRemove(Position<T>& data) override {}
    void ProcessUpdate(Position<T>& data) override {}
};

/************************************************************************************************
 * HistRiskListener
 * Listens to PV01<T> from a RiskService, persists them via 
 * HistoricalDataService<PV01<T>> -> PersistData(...)
 * 
 * Referenced by:
 *   HistRiskListener<Bond> history_risk_listener(&risk_h);
 ************************************************************************************************/
template<typename T>
class HistRiskListener : public ServiceListener<PV01<T>>
{
private:
    HistoricalDataService<PV01<T>> *histService;

public:
    explicit HistRiskListener(HistoricalDataService<PV01<T>>* hs)
        : histService(hs) {}

    void ProcessAdd(PV01<T>& data) override
    {
        histService->PersistData(data.GetProduct().GetProductId(), data);
    }
    void ProcessRemove(PV01<T>& data) override {}
    void ProcessUpdate(PV01<T>& data) override {}
};

/************************************************************************************************
 * GUIListener
 * Listens to Price<T> from a PricingService<T>, calls GUIService<T>::ProvideData(...) 
 * 
 * Referenced by:
 *   GUIListener<Bond> gui_listener(&gui_service);
 ************************************************************************************************/
template<typename T>
class GUIListener : public ServiceListener<Price<T>>
{
private:
    GUIService<T>* guiService;

public:
    explicit GUIListener(GUIService<T>* gService)
        : guiService(gService) {}

    void ProcessAdd(Price<T>& data) override
    {
        guiService->ProvideData(data);
    }
    void ProcessRemove(Price<T>& data) override {}
    void ProcessUpdate(Price<T>& data) override {}
};

/************************************************************************************************
 * AlgoStreamingListener
 * Listens to Price<T> from PricingService<T>, calls BondAlgoStreamingService<T>::PublishPrice(...) 
 * 
 * Referenced by:
 *   AlgoStreamingListener<Bond> algo_streaming_listener(&bond_algo_streaming_service);
 ************************************************************************************************/
template<typename T>
class AlgoStreamingListener : public ServiceListener<Price<T>>
{
private:
    BondAlgoStreamingService<T>* algoStreamingService;

public:
    explicit AlgoStreamingListener(BondAlgoStreamingService<T>* servicePtr)
        : algoStreamingService(servicePtr) {}

    void ProcessAdd(Price<T>& data) override
    {
        // Let the algo streaming service build & publish a PriceStream<T>.
        algoStreamingService->PublishPrice(data);
    }
    void ProcessRemove(Price<T>& data) override {}
    void ProcessUpdate(Price<T>& data) override {}
};

/************************************************************************************************
 * StreamingListener
 * Listens to PriceStream<T> from BondAlgoStreamingService, calls StreamingService<T>::PublishPrice(...)
 * 
 * Referenced by:
 *   StreamingListener<Bond> streaming_listener(&streaming_service);
 ************************************************************************************************/
template<typename T>
class StreamingListener : public ServiceListener<PriceStream<T>>
{
private:
    StreamingService<T>* streamingService;

public:
    explicit StreamingListener(StreamingService<T>* servicePtr)
        : streamingService(servicePtr) {}

    void ProcessAdd(PriceStream<T>& data) override
    {
        streamingService->PublishPrice(data);
    }
    void ProcessRemove(PriceStream<T>& data) override {}
    void ProcessUpdate(PriceStream<T>& data) override {}
};

/************************************************************************************************
 * HistStreamingListener
 * Listens to PriceStream<T> from StreamingService<T>, persists them via 
 * HistoricalDataService<PriceStream<T>> -> PersistData(...)
 * 
 * Referenced by:
 *   HistStreamingListener<Bond> hist_streaming_listener(&streaming_hist_service);
 ************************************************************************************************/
template<typename T>
class HistStreamingListener : public ServiceListener<PriceStream<T>>
{
private:
    HistoricalDataService<PriceStream<T>>* histService;

public:
    explicit HistStreamingListener(HistoricalDataService<PriceStream<T>>* hs)
        : histService(hs) {}

    void ProcessAdd(PriceStream<T>& data) override
    {
        histService->PersistData(data.GetProduct().GetProductId(), data);
    }
    void ProcessRemove(PriceStream<T>& data) override {}
    void ProcessUpdate(PriceStream<T>& data) override {}
};

/************************************************************************************************
 * BondAlgoExecutionListener
 * Listens to OrderBook<T> from MarketDataService<T>, calls BondAlgoExecutionService<T>::Execute(...)
 * 
 * Referenced by:
 *   BondAlgoExecutionListener<Bond> bond_algo_execution_listener_ins(&bond_algo_execution_service_ins);
 ************************************************************************************************/
template<typename T>
class BondAlgoExecutionListener : public ServiceListener<OrderBook<T>>
{
private:
    BondAlgoExecutionService<T>* algoExecService;

public:
    explicit BondAlgoExecutionListener(BondAlgoExecutionService<T>* servicePtr)
        : algoExecService(servicePtr) {}

    void ProcessAdd(OrderBook<T>& data) override
    {
        algoExecService->Execute(data);
    }
    void ProcessRemove(OrderBook<T>& data) override {}
    void ProcessUpdate(OrderBook<T>& data) override {}
};

/************************************************************************************************
 * ExecutionServiceListener
 * Listens to ExecutionOrder<T> from BondAlgoExecutionService<T> (or other service), calls 
 * ExecutionService<T>::ExecuteOrder(...) with a Market argument
 * 
 * Referenced by:
 *   ExecutionServiceListener<Bond> execution_service_listener_ins(&execution_service_ins);
 ************************************************************************************************/
template<typename T>
class ExecutionServiceListener : public ServiceListener<ExecutionOrder<T>>
{
private:
    ExecutionService<T>* execService;

public:
    explicit ExecutionServiceListener(ExecutionService<T>* svc)
        : execService(svc) {}

    void ProcessAdd(ExecutionOrder<T>& data) override
    {
        // If your service requires a Market parameter, pass it here:
        execService->ExecuteOrder(data, CME);
    }
    void ProcessRemove(ExecutionOrder<T>& data) override {}
    void ProcessUpdate(ExecutionOrder<T>& data) override {}
};

/************************************************************************************************
 * TradeBookingServiceListener
 * Listens to ExecutionOrder<T> from ExecutionService<T> and transforms them into Trade<T>, 
 * passing them to TradeBookingService<T>::OnMessage(...) or BookTrade(...).
 * 
 * Referenced by:
 *   TradeBookingServiceListener<Bond> trade_booking_listener_ins(&trade_booking_service_ins);
 ************************************************************************************************/
template<typename T>
class TradeBookingServiceListener : public ServiceListener<ExecutionOrder<T>>
{
private:
    TradeBookingService<T>* tradeBookingService;

public:
    explicit TradeBookingServiceListener(TradeBookingService<T>* svc)
        : tradeBookingService(svc) {}

    void ProcessAdd(ExecutionOrder<T>& data) override {}
    void ProcessRemove(ExecutionOrder<T>& data) override {}
    void ProcessUpdate(ExecutionOrder<T>& data) override {}
};

/************************************************************************************************
 * ExecutionHistoricalDataServiceListener
 * Listens to ExecutionOrder<T> from ExecutionService<T> and persists them via
 * ExecutionHistoricalService<T>::PersistData(...).
 * 
 * Referenced by:
 *   ExecutionHistoricalDataServiceListener<Bond> execution_hist_service_listener(&execution_hist_service);
 ************************************************************************************************/
template<typename T>
class ExecutionHistoricalDataServiceListener : public ServiceListener<ExecutionOrder<T>>
{
private:
    HistoricalDataService<ExecutionOrder<T>>* histService;

public:
    explicit ExecutionHistoricalDataServiceListener(HistoricalDataService<ExecutionOrder<T>>* svc)
        : histService(svc) {}

    void ProcessAdd(ExecutionOrder<T>& data) override
    {
        histService->PersistData(data.GetProduct().GetProductId(), data);
    }
    void ProcessRemove(ExecutionOrder<T>& data) override {}
    void ProcessUpdate(ExecutionOrder<T>& data) override {}
};

/************************************************************************************************
 * AllInquiryHistoricalDataServiceListener
 * Listens to Inquiry<T> from InquiryService<T> and persists them via 
 * InquiryHistoricalService<T>::PersistData(...).
 * 
 * Referenced by:
 *   AllInquiryHistoricalDataServiceListener<Bond> all_inquiry_hist_listner(&inquiry_hist_service);
 ************************************************************************************************/
template<typename T>
class AllInquiryHistoricalDataServiceListener : public ServiceListener<Inquiry<T>>
{
private:
    HistoricalDataService<Inquiry<T>>* histService;

public:
    explicit AllInquiryHistoricalDataServiceListener(HistoricalDataService<Inquiry<T>>* svc)
        : histService(svc) {}

    void ProcessAdd(Inquiry<T>& data) override
    {
        histService->PersistData(data.GetInquiryId(), data);
    }
    void ProcessRemove(Inquiry<T>& data) override {}
    void ProcessUpdate(Inquiry<T>& data) override {}
};

#endif // LISTENERS_HPP
