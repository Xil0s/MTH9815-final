/**
 * riskservice.hpp
 * Defines the data types and Service for fixed income risk.
 *
 * @author Breman Thuraisingham
 */
#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "soa.hpp"
#include "positionservice.hpp"
#include<iostream>
using std::cout;
using std::endl;

/**
 * PV01 risk.
 * Type T is the product type.
 */
template<typename T>
class PV01
{
    
public:

  // ctor for a PV01 value
  PV01(const T &_product, double _pv01, long _quantity);

  // Get the product on this PV01 value
  const T& GetProduct() const{
    return product;
  }

  // Get the PV01 value
  double GetPV01() const{
    return pv01;
  }

  // Get the quantity that this risk value is associated with
  long GetQuantity() const{
    return quantity;
  }

private:
  T product;
  double pv01;
  long quantity;

};

/**
 * A bucket sector to bucket a group of securities.
 * We can then aggregate bucketed risk to this bucket.
 * Type T is the product type.
 */
template<typename T>
class BucketedSector
{
public:

  // ctor for a bucket sector
  BucketedSector(const vector<T> &_products, string _name);

  // Get the products associated with this bucket
  const vector<T>& GetProducts() const;

  // Get the name of the bucket
  const string& GetName() const;

private:
  vector<T> products;
  string name;

};

/**
 * Risk Service to vend out risk for a particular security and across a risk bucketed sector.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class RiskService : public Service<string,PV01 <T> >
{
private:
    map<string, PV01 <T> > data;
public:
    void AddPosition(Position<T> &position)
    {
        long aggregate = position.GetAggregatePosition();
        PV01<Bond> curr_pv01(position.GetProduct(), 0.02, aggregate);
        Service<string,PV01 <T> >::Notify(curr_pv01);
    }
    const PV01<T>& GetBucketedRisk(const BucketedSector<T> &sector) const
    {
        
    }
    virtual PV01 <T>& GetData(string key)
    {
        return data.find(key)->second;}
};

template<typename T>
PV01<T>::PV01(const T &_product, double _pv01, long _quantity) :
product(_product)
{
    pv01 = _pv01;
    quantity = _quantity;
}

template<typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) :
products(_products)
{
    name = _name;
}

template<typename T>
const vector<T>& BucketedSector<T>::GetProducts() const
{
    return products;
}

template<typename T>
const string& BucketedSector<T>::GetName() const
{
    return name;
}

#endif
