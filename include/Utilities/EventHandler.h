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

    void Invoke(Args... args)
    {
        for (auto& callback : _callbacks)
        {
            callback(args...);
        }
    }

protected:
    std::vector<CallbackType> _callbacks;
};

// Backwards compatibility alias
template <typename... Args>
using EventHandlerT = EventHandler<Args...>;