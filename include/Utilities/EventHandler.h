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
    EventHandlerT();

    // += operator to add a callback
    void operator+=(void (*callback)(Args...));

    // -= operator to remove callback
    void operator-=(void (*callback)(Args...));
    
    void Invoke(Args... args);

protected:
    std::vector<void (*)(Args...)> _callbacks;
};