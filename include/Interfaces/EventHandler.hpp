// #pragma once

// #include <functional>
// #include <vector>

// namespace DisplayModule
// {
//     // -------------------------------------------------------------------------
//     // EventHandler<Args...>
//     // -------------------------------------------------------------------------
//     // Lightweight multicast delegate — holds a list of std::function callbacks
//     // with the same signature and invokes them all on demand.
//     //
//     // Usage:
//     //   EventHandler<> onSomething;         // void()
//     //   EventHandler<int, float> onUpdate;  // void(int, float)
//     //
//     //   onSomething += []() { doWork(); };
//     //   onSomething();                       // invokes all registered handlers
//     //
//     // The += operator returns *this so subscriptions can be chained if needed.
//     // Handlers are invoked in registration order.  A null std::function is
//     // silently skipped.

//     template<typename... Args>
//     class EventHandler
//     {
//     public:
//         using Fn = std::function<void(Args...)>;

//         // Register a handler
//         EventHandler &operator+=(Fn fn)
//         {
//             _handlers.push_back(std::move(fn));
//             return *this;
//         }

//         // Invoke all registered handlers.
//         // Takes a snapshot of the list first so handlers that add or remove
//         // entries during dispatch see a consistent view and don't invalidate
//         // the iterator.
//         void operator()(Args... args) const
//         {
//             const auto snapshot = _handlers;
//             for (const auto &fn : snapshot)
//                 if (fn) fn(args...);
//         }

//         // Remove all registered handlers
//         void clear() { _handlers.clear(); }

//         bool empty() const { return _handlers.empty(); }

//     private:
//         std::vector<Fn> _handlers;
//     };

// } // namespace DisplayModule
