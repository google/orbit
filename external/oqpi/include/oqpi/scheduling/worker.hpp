#pragma once

#include "oqpi/scheduling/worker_base.hpp"


namespace oqpi {

    //----------------------------------------------------------------------------------------------
    template<typename _Thread, typename _Notifier, typename _Scheduler, typename _WorkerContext>
    class worker
        : public worker_base
        , public _WorkerContext
    {
    public:
        worker(_Scheduler &sc, int32_t id, const worker_config &config)
            : worker_base(id, config)
            , _WorkerContext(this)
            , scheduler_(sc)
            , notifier_("WorkerNotifier/" + this->getName())
            , running_(false)
        {}

        virtual ~worker()
        {
            stop();
        }

    private:
        //------------------------------------------------------------------------------------------
        virtual void start() override final
        {
            // Important to set the worker as running before starting the thread
            running_.store(true);

            // Add the id to the worker name so we can differentiate them when several workers
            // share the same config (and thus the same name)
            auto threadAttributes = worker_base::config_.threadAttributes;
            if (id_ >= 0)
            {
                threadAttributes.name_ += std::to_string(id_);
            }

            // Start the thread, running_ must be set to true beforehand
            thread_ = _Thread(threadAttributes, [this]() { run(); });
        }

        //------------------------------------------------------------------------------------------
        // Tags the worker as not running, note that this won't wake up the worker if it's asleep,
        // it's the caller's responsibility to wake it up
        virtual void stop() override final
        {
            running_.store(false);
        }

        //------------------------------------------------------------------------------------------
        // If the thread hasn't been detached, this function will block until the worker stops
        virtual void join() override final
        {
            if (thread_.joinable())
            {
                thread_.join();
            }
        }

        //------------------------------------------------------------------------------------------
        bool isRunning() const
        {
            return running_.load();
        }

        //------------------------------------------------------------------------------------------
        virtual void wait() override final
        {
            notifier_.wait();
        }

        //------------------------------------------------------------------------------------------
        virtual bool tryWait() override final
        {
            return notifier_.tryWait();
        }

        //------------------------------------------------------------------------------------------
        virtual void notify() override final
        {
            notifier_.notifyOne();
        }

        //------------------------------------------------------------------------------------------
        virtual void run() override final
        {
            // Inform the context that we're starting the worker thread
            _WorkerContext::worker_onStart();

            // This is the worker's main loop
            while (isRunning())
            {
                // Inform the context that we're potentially going idle while waiting for a task to work on
                _WorkerContext::worker_onIdle();
                // Signal to the scheduler that we want a task to work on
                scheduler_.signalAvailableWorker(*this);
                // At this point we either have a task to work on or we've been waken up to quit the thread
                oqpi_check(!worker_base::isAvailable() || !isRunning());
                // We consider ourselves active either way
                _WorkerContext::worker_onActive();

                // Check if we've waken up to work on a new task
                if (isRunning())
                {
                    if (oqpi_ensure(worker_base::hTask_.isValid()))
                    {
                        // Inform the context that we're about to start the execution of a new task
                        _WorkerContext::worker_onPreExecute(worker_base::hTask_);
                        // Actually execute the task
                        worker_base::hTask_.execute();
                        // Inform the context that we just finished the execution of a task
                        _WorkerContext::worker_onPostExecute(worker_base::hTask_);
                        // Reset the task, can potentially free the memory if there's no more reference to that task
                        worker_base::hTask_.reset();
                    }
                }
            }

            // Just to make sure the memory is released (will happen in the destructor of the worker anyway)
            worker_base::hTask_.reset();

            // Inform the context that we're stopping the worker thread
            _WorkerContext::worker_onStop();
        }

    private:
        // Reference to the parent scheduler, used to call signalAvailableWorker
        _Scheduler         &scheduler_;
        // The underlying thread
        _Thread             thread_;
        // Notifier used to signal/put to sleep the thread
        _Notifier           notifier_;
        // Whether or not the worker is up and running
        std::atomic<bool>   running_;
    };
    //----------------------------------------------------------------------------------------------

} /*oqpi*/
