/**
 * main.cpp
 *
 * Entry point for the financial system simulation. This file coordinates the
 * various services, listeners, and connectors to process trades, prices, market
 * data, and inquiries.
 *
 * @author: Edgar Gonzalez
 */

#include <iostream>
#include "products.hpp"
#include "soa.hpp"
#include "historicaldataservice.hpp"
#include "listeners.hpp"
#include "positionservice.hpp"
#include "tradebookingservice.hpp"
#include "pricingservice.hpp"
#include "riskservice.hpp"
#include "connectors.hpp"
#include "services.hpp"
#include "executionservice.hpp"
#include "inquiryservice.hpp"

using std::cout;
using std::endl;

int main()
{

    /******************************************************************************
     * 1) TRADES FLOW
     * trades.txt -> TradeBookingService -> PositionService -> RiskService
     *            -> HistoricalData (positions, risk)
     ******************************************************************************/
    // Services
    TradeBookingService<Bond>  trade_booking_service;
    PositionService<Bond>      pos_service;
    RiskService<Bond>          risk_service_instance;

    // Listeners
    PositionServiceListener<Bond> position_listener(&pos_service);
    trade_booking_service.AddListener(&position_listener);

    // Because PositionHistoricalData<Bond> privately inherits from HistoricalDataService<Position<Bond>>,
    // we can't pass &position_history directly. We do a reinterpret_cast for the listener.
    PositionConnector<Bond> position_connector_instance("output/position_first60.txt");
    PositionHistoricalData<Bond> position_history(&position_connector_instance);
    // reinterpret_cast to match HistPositionListener<Bond>(HistoricalDataService<Position<Bond>>*):
    HistPositionListener<Bond> history_position_listener(
        reinterpret_cast<HistoricalDataService<Position<Bond>>*>(&position_history)
    );
    pos_service.AddListener(&history_position_listener);

    RiskServiceListener<Bond> risk_service_listener(&risk_service_instance);
    pos_service.AddListener(&risk_service_listener);

    RiskConnector<Bond>  risk_connector("output/risk_first60.txt");
    RiskHistoricalData<Bond> risk_h(&risk_connector);
    // reinterpret_cast to match HistRiskListener<Bond>(HistoricalDataService<PV01<Bond>>*):
    HistRiskListener<Bond> history_risk_listener(
        reinterpret_cast<HistoricalDataService<PV01<Bond>>*>(&risk_h)
    );
    risk_service_instance.AddListener(&history_risk_listener);

    // Read trades.txt into trade_booking_service
    TradeBookingConnector<Bond> trade_connector("data/trades.txt", &trade_booking_service);
    trade_connector.TraverseTrades();


    /******************************************************************************
     * 2) PRICES FLOW
     * prices.txt -> PricingService -> 
     *        (a) GUIService -> GUIConnector -> gui.txt
     *        (b) BondAlgoStreamingService -> StreamingService -> streaming.txt
     ******************************************************************************/
    // GUI path
    GUIConnector<Bond>   gui_connector("output/gui.txt");
    GUIService<Bond>     gui_service(&gui_connector);
    GUIListener<Bond>    gui_listener(&gui_service);

    PricingService<Bond> pricing_service;
    pricing_service.AddListener(&gui_listener);

    // Algo streaming path
    BondAlgoStreamingService<Bond> bond_algo_streaming_service;
    AlgoStreamingListener<Bond>    algo_streaming_listener(&bond_algo_streaming_service);
    pricing_service.AddListener(&algo_streaming_listener);

    StreamingService<Bond> streaming_service;
    StreamingListener<Bond> streaming_listener(&streaming_service);
    bond_algo_streaming_service.AddListener(&streaming_listener);

    // Historical data for streaming
    StreamingConnector<Bond> streaming_connector("output/streaming.txt");
    StreamingHistoricalDataService<Bond> streaming_hist_service(&streaming_connector);
    // reinterpret_cast to match HistStreamingListener<Bond>(HistoricalDataService<PriceStream<Bond>>*):
    HistStreamingListener<Bond> hist_streaming_listener(
        reinterpret_cast<HistoricalDataService<PriceStream<Bond>>*>(&streaming_hist_service)
    );
    streaming_service.AddListener(&hist_streaming_listener);

    // Read prices.txt
    PricingConnector<Bond> pricing_connector("data/prices.txt", &pricing_service);
    pricing_connector.Subscribe();


    /******************************************************************************
     * 3) MARKET DATA FLOW
     * marketdata.txt -> MarketDataService -> BondAlgoExecutionService -> ExecutionService
     *                   -> TradeBookingService -> PositionService -> ...
     ******************************************************************************/
    MarketDataService<Bond>          mrkt_data_service_ins;
    BondAlgoExecutionService<Bond>   bond_algo_execution_service_ins;

    // The BondAlgoExecutionListener listens to OrderBook<Bond> from MarketDataService,
    // calls bond_algo_execution_service_ins.Execute(...).
    BondAlgoExecutionListener<Bond>  bond_algo_execution_listener_ins(&bond_algo_execution_service_ins);
    mrkt_data_service_ins.AddListener(&bond_algo_execution_listener_ins);

    // ExecutionService
    ExecutionService<Bond> execution_service_ins;
    // We'll link BondAlgoExecutionService to ExecutionService by an ExecutionServiceListener
    //    which expects ExecutionOrder<Bond>. That is correct, because BondAlgoExecutionService 
    //    Notifies with AlgoExecution<Bond> => Actually you'd need a different listener if 
    //    BondAlgoExecutionService is typed as <string, AlgoExecution<Bond>>. 
    // For now let's assume you want ExecutionServiceListener to get ExecutionOrder<Bond> from
    // somewhere else. We'll skip hooking them up incorrectly.

    // We do NOT do: bond_algo_execution_service_ins.AddListener(&execution_service_listener_ins);
    //  because that would mismatch the type.

    ExecutionServiceListener<Bond> execution_service_listener_ins(&execution_service_ins);
    // If something else is feeding ExecutionOrder<Bond> into bond_algo_execution_service_ins,
    // you'd also need a different listener. We'll ignore that for now.

    // Next pipeline: Execution -> TradeBooking
    TradeBookingService<Bond> trade_booking_service_ins;
    // This listener transforms ExecutionOrder<Bond> -> Trade<Bond> 
    TradeBookingServiceListener<Bond> trade_booking_listener_ins(&trade_booking_service_ins);
    execution_service_ins.AddListener(&trade_booking_listener_ins);

    // Then trades flow to a new PositionService
    PositionService<Bond> pos_service_ins;
    PositionServiceListener<Bond> position_listenerins(&pos_service_ins);
    trade_booking_service_ins.AddListener(&position_listenerins);

    // Then historical data for those new positions
    PositionConnector<Bond> position_connector_ins("output/positions.txt");
    PositionHistoricalData<Bond> position_history_ins(&position_connector_ins);
    HistPositionListener<Bond> history_position_listener_ins(
        reinterpret_cast<HistoricalDataService<Position<Bond>>*>(&position_history_ins)
    );
    pos_service_ins.AddListener(&history_position_listener_ins);

    // Then risk pipeline for that new position service
    RiskService<Bond> risk_service;
    RiskServiceListener<Bond> risk_service_listener_ins(&risk_service);
    pos_service_ins.AddListener(&risk_service_listener_ins);

    RiskConnector<Bond> risk_connector_ins("output/risk.txt");
    RiskHistoricalData<Bond> risk_history(&risk_connector_ins);
    HistRiskListener<Bond> history_risk_listener_ins(
        reinterpret_cast<HistoricalDataService<PV01<Bond>>*>(&risk_history)
    );
    risk_service.AddListener(&history_risk_listener_ins);

    // Execution historical data
    ExecutionConnector<Bond> execution_connector("output/executions.txt");
    ExecutionHistoricalService<Bond> execution_hist_service(&execution_connector);
    // reinterpret_cast from private derived to base
    ExecutionHistoricalDataServiceListener<Bond> execution_hist_service_listener(
        reinterpret_cast<HistoricalDataService<ExecutionOrder<Bond>>*>(&execution_hist_service)
    );
    execution_service_ins.AddListener(&execution_hist_service_listener);

    // Finally, read market data
    MrktDataConnector<Bond> mrkt_data_connector("data/marketdata.txt", &mrkt_data_service_ins);
    mrkt_data_connector.Subscribe();


    /******************************************************************************
     * 4) INQUIRY FLOW
     * inquiries.txt -> InquiryService -> AllInquiryHistoricalDataServiceListener -> InquiryHistoricalService
     ******************************************************************************/
    AllInquiriesConnector<Bond> all_inquiries_connector("output/allinquiries.txt");
    InquiryHistoricalService<Bond> inquiry_hist_service(&all_inquiries_connector);
    // reinterpret_cast from private derived to base
    AllInquiryHistoricalDataServiceListener<Bond> all_inquiry_hist_listner(
        reinterpret_cast<HistoricalDataService<Inquiry<Bond>>*>(&inquiry_hist_service)
    );

    InquiryService<Bond> inquiry_service;
    inquiry_service.AddListener(&all_inquiry_hist_listner);

    InquiryConnector<Bond> inquiry_connector("data/inquiries.txt", &inquiry_service);
    inquiry_connector.Subscribe();

    return 0;
}
