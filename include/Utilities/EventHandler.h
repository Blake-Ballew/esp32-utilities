#pragma once

#include <vector>
#include <algorithm>

// Generic event handler for callbacks with no arguments
class EventHandler
{
public:
    EventHandler();

    // += operator to add a callback
    void operator+=(void (*callback)());

    // -= operator to remove callback
    void operator-=(void (*callback)());
    
    void Invoke();

protected:
    std::vector<void (*)()> _callbacks;
};

// Templated event handler for custom callback arguments
template <typename... Args>
class EventHandlerT
{
public:
    EventHandlerT()
    {
        _callbacks.clear();
    }

    // += operator to add a callback
    void operator+=(void (*callback)(Args...))
    {
        if (std::find(_callbacks.begin(), _callbacks.end(), callback) == _callbacks.end())
        {
            _callbacks.push_back(callback);
        }
    }   

    // -= operator to remove callback
    void operator-=(void (*callback)(Args...))
    {
        auto it = std::find(_callbacks.begin(), _callbacks.end(), callback);
        if (it != _callbacks.end()) 
        {
            _callbacks.erase(it);
        }
    }
    
    void Invoke(Args... args)
    {
        for (auto &callback : _callbacks) 
        {
            callback(args...);
        }
    }

protected:
    std::vector<void (*)(Args...)> _callbacks;
};