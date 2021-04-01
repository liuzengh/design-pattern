#include <mutex>
#include <atomic>
/*
class Singleton
{
    protected:
        Singleton() 
        {
            //do what you need to do 
        }
    public:
        static Singleton& getSingleton()
        {
            static Singleton instance;
            return instance;
        };
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        Singleton(Singleton&&) = delete;
        Singleton& operator=(Singleton&&) = delete;
};
*/
class Singleton
{
    protected:
        Singleton();
    private:
        static std::mutex m_mutex;
        static std::atomic<Singleton*> m_instance = nullptr;
    public:
        static Singleton* Singleton::getInstance() 
        {
            Singleton* tmp = m_instance.load(std::memory_order_acquire);
            if (tmp == nullptr) 
            {
                //std::scoped_lock(m_mutex);
                std::lock_guard<std::mutex> lock(m_mutex);
                tmp = m_instance.load(std::memory_order_relaxed);
                if (tmp == nullptr) 
                {
                    tmp = new Singleton;
                    m_instance.store(tmp, std::memory_order_release);
                }
            }
            return tmp;
        }
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        Singleton(Singleton&&) = delete;
        Singleton& operator=(Singleton&&) = delete;

};



