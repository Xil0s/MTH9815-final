/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @author Breman Thuraisingham
 */
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"
#include "utils.hpp"

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{

public:

  // ctor for a position
  Position(const T &_product):product(_product){
        string books_[3]{"TRSY1","TRSY2","TRSY3"};
        for (int i=0;i<3;i++){
            positions.insert(make_tuple(books_[i],0));
        }
    }

  // Get the product
  const T& GetProduct() const{
    return product;
  }

  // Get the position quantity
 long GetPosition(const string &book) const
{
    auto it = positions.find(book);
    if (it == positions.end()) {
        cout << "Can't find book " << book << endl;
    }
    return it->second; // purely reading, so it's safe to be const
}

  // Get the aggregate position
  long GetAggregatePosition(){
        long result=0;
        string books_[3]{"TRSY1","TRSY2","TRSY3"};
        for (int i=0;i<3;i++) result+=positions.find(books_[i])->second;
        return result;
    }
    
  void UpdatePosition(string& book, long quantity, Side side){
      if(side == BUY){
          positions.find(book)->second += quantity;
      }
      else{
          positions.find(book)->second -= quantity;
      }
  }

private:
  T product;
  map<string,long> positions;

};

/**
 * Position Service to manage positions across multiple books and secruties.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string,Position <T> >
{
private:
  unordered_map<string, Position<T> > name_p;

public:
  // Add a trade to the service
    PositionService(){
        unordered_map<string,Bond> bond_map=ProductMap::GetProductMap();
        vector<string> tickers=ProductMap::GetTickers();
        
        for (auto ele : tickers){
            Position<Bond> temp(bond_map.find(ele)->second);
            auto pair = make_pair(ele, Position<Bond>(bond_map.find(ele)->second));
            name_p.insert(pair);
        }
    }
    virtual void AddTrade(const Trade<T> &trade___) {
        string ticker=trade___.GetProduct().GetTicker();
        string book=trade___.GetBook();
        long quantity=trade___.GetQuantity();
        Side side= trade___.GetSide();
        if (name_p.find(ticker) != name_p.end())
        {
            Position<Bond>& pos = name_p.find(ticker)->second;
            pos.UpdatePosition(book, quantity, side);

        } else{
            cout << "iAddTrade(), key = " << ticker << "doesn't exist!" << endl;
        }

        Service<string,Position <T> >::Notify(name_p.find(ticker)->second);
    }
    virtual Position <T>& GetData(string key)
    {
            return name_p.find(key)->second;
    }
};
#endif