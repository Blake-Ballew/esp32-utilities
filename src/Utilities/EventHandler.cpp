#include "EventHandler.h"

EventHandler::EventHandler() 
{
    _callbacks.clear();
}

// += operator to add a callback
void EventHandler::operator+=(void (*callback)()) 
{
    if (std::find(_callbacks.begin(), _callbacks.end(), callback) == _callbacks.end())
    {
        _callbacks.push_back(callback);
    }
}

// -= operator to remove callback
void EventHandler::operator-=(void (*callback)()) 
{
    auto it = std::find(_callbacks.begin(), _callbacks.end(), callback);
    if (it != _callbacks.end()) 
    {
        _callbacks.erase(it);
    }
}

void EventHandler::Invoke() 
{
    for (auto &callback : _callbacks) {
        callback();
    }

}

// Templated event handler for custom callback arguments

template <typename... Args>
EventHandlerT<Args...>::EventHandlerT() 
{
    _callbacks.clear();
}

// += operator to add a callback
template <typename... Args>
void EventHandlerT<Args...>::operator+=(void (*callback)(Args...)) 
{
    if (std::find(_callbacks.begin(), _callbacks.end(), callback) == _callbacks.end())
    {
        _callbacks.push_back(callback);
    }
}

// -= operator to remove callback
template <typename... Args>
void EventHandlerT<Args...>::operator-=(void (*callback)(Args...)) 
{
    auto it = std::find(_callbacks.begin(), _callbacks.end(), callback);
    if (it != _callbacks.end()) 
    {
        _callbacks.erase(it);
    }
}

template <typename... Args>
void EventHandlerT<Args...>::Invoke(Args... args) 
{
    for (auto &callback : _callbacks) {
        callback(args...);
    }

}

