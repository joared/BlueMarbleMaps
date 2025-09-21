#ifndef BLUEMARBLE_SIGNAL
#define BLUEMARBLE_SIGNAL

#include "BlueMarbleMaps/Logging/Logging.h"
#include <functional>
#include <cassert>
#include <vector>

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

template<typename... Args>
class Signal : public ISignal
{
    //using Handler = void(*)(Args...);
    using Handler = std::function<void(Args...)>;
    using FunctionHandler = void(*)(Args...);

    public:
        using SubscriptionID = uintptr_t;
        class Subscription
        {
            public:
                // static Subscription emptySubscription()
                // {

                // }

                
                Subscription(const Subscription& other) = delete;
                Subscription(Subscription&& other) noexcept
                    : m_isValid(other.m_isValid)
                    , m_signal(other.m_signal)
                    , m_id(other.m_id)
                {
                    BMM_DEBUG() << "Subscription(Subscription&& other)\n";
                    other.m_isValid = false;
                }

                Subscription& operator=(Subscription&& other) noexcept
                {
                    BMM_DEBUG() << "Subscription::operator==\n";
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
                        //BMM_DEBUG() << "Subcription canceled\n";
                        m_isValid = false;
                        m_signal.unsubscribe(m_id);
                    }
                    else
                    {
                        BMM_DEBUG() << "Invalid subscription\n";
                    }
                }

                friend Subscription Signal::subscribe(Handler);
                friend Subscription Signal::createSubscription(const Handler& handler);

            private:
                Subscription(Signal& signal, SubscriptionID id) 
                    : m_isValid(true)
                    , m_signal(signal)
                    , m_id(id)
                {
                    //BMM_DEBUG() << "Subscription() id: " << id << "\n";
                }
                bool m_isValid;
                Signal& m_signal;
                SubscriptionID m_id;
        };

        Subscription createSubscription(const Handler& handler)
        {
            BMM_DEBUG() << "(subscription)";
            SubscriptionID id = reinterpret_cast<SubscriptionID>(&handler);
            subscribeInternal(id, std::move(handler));

            return Subscription(*this, id);
        }

        Subscription subscribe(Handler handler)
        {
            BMM_DEBUG() << "(anonymous)";
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
            BMM_DEBUG() << "(instance)";
            if (std::is_base_of_v<ISignalHandler, T>)
            {
                BMM_DEBUG() << "SIGNALHANDLER";
                ((ISignalHandler*)instance)->addSignal(this);
            }
            Handler handler = [instance, method](Args... args)
            {
                (instance->*method)(args...);
            };
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
            // static_assert(std::is_lvalue_reference<T>::value, 
            //     "You must unsubscribe with an lvalue, not a temporary!");

            unsubscribe(reinterpret_cast<SubscriptionID>(&handler));
        }

        template<typename T>
        void unsubscribe(T* instance)
        {
            BMM_DEBUG() << "(instance)";
            if (std::is_base_of_v<ISignalHandler, T>)
            {
                BMM_DEBUG() << "SIGNALHANDLER";
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
            BMM_DEBUG() << "Unsubscribe: "  << id << "\n";
            
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

        void notify(Args... args)
        {
            for (auto& [id, handler] : m_listeners)
            {
                handler(args...);
            }
        }

        template <typename Action>
        void notify(Args... args, Action&& preNotifyAction)
        {
            for (auto& [id, handler] : m_listeners)
            {
                preNotifyAction();
                handler(args...);
            }
        }

        void operator+=(Handler handler)
        {
            SubscriptionID id = generateAnonymousUniqueId();
            subscribeInternal(id, std::move(handler));
            BMM_DEBUG() << "(Permanent)";
        }

        //bool isSubcribed()

    private:
        void subscribeInternal(SubscriptionID id, Handler handler)
        {
            for (auto it=m_listeners.begin(); it!=m_listeners.end(); it++)
            {
                if (it->first == id)
                {
                    BMM_DEBUG() << "Double subscription! (id: " << id << ")\n";
                    throw std::exception();
                }
            }

            BMM_DEBUG() << "Subscription: " << id << "\n";
            m_listeners.emplace_back(id, std::move(handler));
        }

        SubscriptionID generateAnonymousUniqueId() 
        {
            return m_nextId++ | 0x8000000000000000; // Set high bit to avoid pointer conflict
        }

        SubscriptionID m_nextId = 0;
        std::vector<std::pair<SubscriptionID, Handler>> m_listeners; // Possibility to unsubscribe
};

} // BlueMarble

#endif /* BLUEMARBLE_SIGNAL */
