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