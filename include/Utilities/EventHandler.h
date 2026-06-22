#pragma once

#include <vector>
#include <functional>

template <typename... Args>
class EventHandler
{
public:
    using CallbackType = std::function<void(Args...)>;
    using FunctionPtr = void (*)(Args...);

    EventHandler() {}

    // += for any callable (lambdas, function pointers, bind expressions, etc.)
    void operator+=(CallbackType callback)
    {
        _callbacks.push_back(std::move(callback));
    }

    // Registers a callback that runs before all previously- and later-registered
    // callbacks (it is inserted at the front of the invocation order). Use when
    // one subscriber must act first -- e.g. playing a shutdown animation before
    // another subscriber cuts power.
    void PushFront(CallbackType callback)
    {
        _callbacks.insert(_callbacks.begin(), std::move(callback));
    }

    // -= for function pointers (finds by comparing underlying pointer)
    void operator-=(FunctionPtr callback)
    {
        for (auto it = _callbacks.begin(); it != _callbacks.end(); ++it)
        {
            auto* ptr = it->template target<FunctionPtr>();
            if (ptr && *ptr == callback)
            {
                _callbacks.erase(it);
                return;
            }
        }
    }

    void operator()(Args... args)
    {
        Invoke(args...);
    }

    void Invoke(Args... args)
    {
        ESP_LOGV("EventHandler", "Invoking event with %d subscribers", (int)_callbacks.size());
        for (auto& callback : _callbacks)
        {
            callback(args...);
        }
    }

    size_t Count()
    {
        return _callbacks.size();
    }

protected:
    std::vector<CallbackType> _callbacks;
};

// Backwards compatibility alias
template <typename... Args>
using EventHandlerT = EventHandler<Args...>;