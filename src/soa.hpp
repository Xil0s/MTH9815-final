/**
 * soa.hpp
 * Definition of our Service Oriented Architecture (SOA) Service base class
 *
 * @author Breman Thuraisingham
 */

#ifndef SOA_HPP
#define SOA_HPP

#include <vector>
#include <tuple>
using namespace std;

/**
 * Definition of a generic base class ServiceListener to listen to add, update, and remve
 * events on a Service. This listener should be registered on a Service for the Service
 * to notify all listeners for these events.
 */
template<typename V>
class ServiceListener
{

public:

  // Listener callback to process an add event to the Service
  virtual void ProcessAdd(V &data) = 0;

  // Listener callback to process a remove event to the Service
  virtual void ProcessRemove(V &data) = 0;

  // Listener callback to process an update event to the Service
  virtual void ProcessUpdate(V &data) = 0;

};

/**
 * Definition of a generic base class Service.
 * Uses key generic type K and value generic type V.
 */
template<typename K, typename V>
class Service
{
protected:
    vector<ServiceListener<V>* > listeners;
public:

  // Get data on our service given a key


  // The callback that a Connector should invoke for any new or updated data
  virtual void OnMessage(V &data) {}
  virtual void AddListener(ServiceListener<V> *listener) {
    listeners.push_back(listener);
  }

  // Get all listeners on the Service.
  virtual const vector< ServiceListener<V>* >& GetListeners() const{
    return listeners;
  }
  
  virtual void Notify(V &data){
    for (auto ele:listeners) ele->ProcessAdd(data);
  }

};  

/**
 * Definition of a Connector class.
 * This will invoke the Service.OnMessage() method for subscriber Connectors
 * to push data to the Service.
 * Services can invoke the Publish() method on this Service to publish data to the Connector
 * for a publisher Connector.
 * Note that a Connector can be publisher-only, subscriber-only, or both publisher and susbcriber.
 */
template<typename V>
class Connector{
public:
  // Publish data to the Connector
    virtual void Publish(V &data) {}
};

#endif
