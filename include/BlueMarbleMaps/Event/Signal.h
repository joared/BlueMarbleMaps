#ifndef BLUEMARBLE_SIGNAL
#define BLUEMARBLE_SIGNAL

#include "BlueMarbleMaps/Logging/Logging.h"
#include <functional>
#include <cassert>
#include <vector>
#include <mutex>

namespace BlueMarble
{

class ISignal
{
    public:
        virtual void unsubscribe(void* id) = 0;
};

class ISignalHandler
{
    public:
        ISignalHandler()
            : m_subscribedSignals()
        {}
        virtual ~ISignalHandler()
        {
            for (auto s : m_subscribedSignals)
            {
                BMM_DEBUG() << "Signal handler died, unsubscribing!\n";
                s->unsubscribe(this);
            }
        }
        void unSubscribe(ISignal* signal)
        {

        }
        void addSignal(ISignal* signal) { m_subscribedSignals.push_back(signal); }
        void removeSignal(ISignal* signal) 
        { 
            for (auto it=m_subscribedSignals.begin(); it!=m_subscribedSignals.end(); it++)
            {
                if (*it == signal)
                {
                    m_subscribedSignals.erase(it);
                    return;
                }
            }
            BMM_DEBUG() << "removeSignal called for non-existing signal\n";
            throw std::exception();
        }
    private:
        std::vector<ISignal*> m_subscribedSignals;
};

struct NoLockPolicy 
{
    void lock() {}
    void unlock() {}
};

struct MutexLockPolicy 
{
    std::mutex m;
    void lock()   { m.lock(); }
    void unlock() { m.unlock(); }
};

template<typename LockPolicy = NoLockPolicy, typename... Args>
class SignalImpl : public ISignal
{
    //using Handler = void(*)(Args...);
    using Handler = std::function<void(Args...)>;
    using FunctionHandler = void(*)(Args...);

    public:
        using SubscriptionID = uintptr_t;
        class Subscription
        {
            public:
                Subscription(const Subscription& other) = delete;
                Subscription(Subscription&& other) noexcept
                    : m_isValid(other.m_isValid)
                    , m_isPermanent(other.m_isPermanent)
                    , m_signal(other.m_signal)
                    , m_id(other.m_id)
                {
                    // BMM_DEBUG() << "Subscription(Subscription&& other)\n";
                    other.m_isValid = false;
                }

                Subscription& operator=(Subscription&& other) noexcept
                {
                    // BMM_DEBUG() << "Subscription::operator==\n";
                    if (this != &other) {
                        if (m_isValid) // && m_id != other.m_id) 
                        {
                            m_signal.unsubscribe(m_id);
                        }
                        m_isValid = other.m_isValid;
                        m_id = other.m_id;
                        other.m_isValid = false;
                    }
                    return *this;
                }
                
                ~Subscription()
                {
                    //BMM_DEBUG() << "Subcription delete\n";
                    cancel();
                }

                void cancel()
                {
                    if (m_isValid)
                    {
                        if (m_isPermanent)
                        {
                            BMM_DEBUG() << "Subscription died but is marked permanent\n";
                            return;
                        }
                        //BMM_DEBUG() << "Subcription canceled\n";
                        m_isValid = false;
                        m_signal.unsubscribe(m_id);
                    }
                    else
                    {
                        BMM_DEBUG() << "Invalid subscription\n";
                    }
                }

                void permanent()
                {
                    m_isPermanent = true;
                }

                friend Subscription SignalImpl::subscribe(Handler);
                friend Subscription SignalImpl::createSubscription(const Handler& handler);

            private:
                Subscription(SignalImpl& signal, SubscriptionID id) 
                    : m_isValid(true)
                    , m_isPermanent(false)
                    , m_signal(signal)
                    , m_id(id)
                {
                    //BMM_DEBUG() << "Subscription() id: " << id << "\n";
                }
                bool m_isValid;
                bool m_isPermanent;
                SignalImpl& m_signal;
                SubscriptionID m_id;
        };

        SignalImpl()
            : m_nextId(0)
            , m_listeners()
            , m_lockPolicy()
        {

        }

        Subscription createSubscription(const Handler& handler)
        {
            // BMM_DEBUG() << "(subscription)";
            SubscriptionID id = reinterpret_cast<SubscriptionID>(&handler);
            subscribeInternal(id, std::move(handler));

            return Subscription(*this, id);
        }

        Subscription subscribe(Handler handler)
        {
            // BMM_DEBUG() << "(anonymous)";
            SubscriptionID id = generateAnonymousUniqueId();
            subscribeInternal(id, std::move(handler));
            
            return Subscription(*this, id);
        }

        //void subscribe(FunctionHandler handler)
        // template<typename F>
        // auto subscribe(F&& handler) -> std::enable_if_t<std::is_convertible_v<F, FunctionHandler>>
        // {
        //     BMM_DEBUG() << "(function)";
        //     SubscriptionID id = reinterpret_cast<SubscriptionID>(handler);
        //     subscribeInternal(id, std::move(handler));
        // }

        // Subscribe using instance method.
        template<typename T>
        void subscribe(T* instance, void (T::*method)(Args...))
        {
            // BMM_DEBUG() << "(instance)";
            if (std::is_base_of_v<ISignalHandler, T>)
            {
                BMM_DEBUG() << "SIGNALHANDLER";
                std::lock_guard<LockPolicy> guard(m_lockPolicy);
                ((ISignalHandler*)instance)->addSignal(this);
            }
            Handler handler = Handler([instance, method](Args... args)
            {
                (instance->*method)(args...);
            });
            // This limits to ONE subscription per instance of this signal 
            SubscriptionID id = reinterpret_cast<SubscriptionID>(instance);
            subscribeInternal(id, std::move(handler));
        }

        // void subscribe(void* id, Handler handler)
        // {
        //     BMM_DEBUG() << "Subscription pointer id!\n";
        //     subscribeInternal((uintptr_t)id, std::move(handler));
        // }

        template<typename T>
        void unsubscribe(const T&& handler)
        {
            static_assert(std::is_lvalue_reference<T>::value, 
                "You must unsubscribe with an lvalue, not a temporary!");

            unsubscribe(reinterpret_cast<SubscriptionID>(&handler));
        }

        template<typename T>
        void unsubscribe(const T& handler)
        {
            static_assert(std::is_lvalue_reference<T>::value, 
                "You must unsubscribe with an lvalue, not a temporary!");

            unsubscribe(reinterpret_cast<SubscriptionID>(&handler));
        }

        template<typename T>
        void unsubscribe(T* instance)
        {
            // BMM_DEBUG() << "(instance)";
            
            if (std::is_base_of_v<ISignalHandler, T>)
            {
                BMM_DEBUG() << "SIGNALHANDLER";
                std::lock_guard<LockPolicy> guard(m_lockPolicy);
                ((ISignalHandler*)instance)->removeSignal(this);
            }

            // This limits to ONE subscription per instance of this signal 
            SubscriptionID id = reinterpret_cast<SubscriptionID>(instance);
            unsubscribe(id);
        }

        void unsubscribe(void* instance)
        {
            unsubscribe(reinterpret_cast<SubscriptionID>(instance));
        }

        void unsubscribe(SubscriptionID id)
        {
            // BMM_DEBUG() << "Unsubscribe: "  << id << "\n";
            std::lock_guard<LockPolicy> guard(m_lockPolicy);
            for (auto it=m_listeners.begin(); it!=m_listeners.end(); it++)
            {
                if (it->first == id)
                {
                    m_listeners.erase(it);
                    return;
                }
            }
            BMM_DEBUG() << "Signal::unsubscribe called for a non-existing subscription id: " << id << "\n";
            throw std::exception();
        }

        void unsubscribeAll()
        {
            std::lock_guard<LockPolicy> guard(m_lockPolicy);
            // FIXME: if using ISignalHandler, this will not remove the signal from it
            m_listeners.clear();
        }

        // Notify subscribers
        void notify(Args... args)
        {
            std::lock_guard<LockPolicy> guard(m_lockPolicy);
            for (auto& [id, handler] : m_listeners)
            {
                handler(args...);
            }
        }

        // Notify subscribers with an action to be performed before each subscriber is notified
        template <typename Action>
        void notify(Args... args, Action&& preNotifyAction)
        {
            std::lock_guard<LockPolicy> guard(m_lockPolicy);
            for (auto& [id, handler] : m_listeners)
            {
                preNotifyAction();
                handler(args...);
            }
        }

        // Notify subscribers with an action to be performed before and after each subscriber is notified
        template <typename Action1, typename Action2>
        void notify(Args... args, Action1&& preNotifyAction, Action2&& postNotifyAction)
        {
            std::lock_guard<LockPolicy> guard(m_lockPolicy);
            for (auto& [id, handler] : m_listeners)
            {
                preNotifyAction();
                handler(args...);
                postNotifyAction();
            }
        }

        void operator+=(Handler handler)
        {
            SubscriptionID id = generateAnonymousUniqueId();
            subscribeInternal(id, std::move(handler));
            // BMM_DEBUG() << "(Permanent)";
        }

    private:
        void subscribeInternal(SubscriptionID id, Handler handler)
        {
            std::lock_guard<LockPolicy> guard(m_lockPolicy);
            for (auto it=m_listeners.begin(); it!=m_listeners.end(); it++)
            {
                if (it->first == id)
                {
                    throw std::runtime_error("Double subscription! (id: " + std::to_string(id) + ")\n");
                }
            }

            // BMM_DEBUG() << "Subscription: " << id << "\n";
            m_listeners.emplace_back(id, std::move(handler));
        }

        SubscriptionID generateAnonymousUniqueId() 
        {
            std::lock_guard<LockPolicy> guard(m_lockPolicy);
            return m_nextId++ | 0x8000000000000000; // Set high bit to avoid pointer conflict
        }

        SubscriptionID m_nextId;
        std::vector<std::pair<SubscriptionID, Handler>> m_listeners; // Possibility to unsubscribe
        mutable LockPolicy m_lockPolicy;
};

template <typename... Args>
using Signal = SignalImpl<NoLockPolicy, Args...>;

template <typename... Args>
using SafeSignal = SignalImpl<MutexLockPolicy, Args...>;

} // BlueMarble

#endif /* BLUEMARBLE_SIGNAL */
