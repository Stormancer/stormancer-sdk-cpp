/***
* ==++==
*
* Copyright (c) Microsoft Corporation. All rights reserved.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* Parallel Patterns Library implementation (common code across platforms)
*
* For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#include "stdafx.h"

#if !defined(_WIN32) || _MSC_VER < 1800 || CPPREST_FORCE_PPLX

#include "pplx/pplx.h"

// Disable false alarm code analyze warning
#if defined(_MSC_VER)
#pragma warning (disable : 26165 26110)
#endif

namespace pplx
{
	std::function<bool(std::exception_ptr)> UnobservedExceptionHandler::unobservedExceptionHandler;
	std::mutex UnobservedExceptionHandler::unobservedExceptionHandlerMutex;


namespace details
{
    /// <summary>
    /// Spin lock to allow for locks to be used in global scope
    /// </summary>
    class _Spin_lock
    {
    public:

        _Spin_lock()
            : _M_lock(0)
        {
        }

        void lock()
        {
            if ( details::atomic_compare_exchange(_M_lock, 1l, 0l) != 0l )
            {
                do
                {
                    pplx::details::platform::YieldExecution();

                } while ( details::atomic_compare_exchange(_M_lock, 1l, 0l) != 0l );
            }
        }

        void unlock()
        {
            // fence for release semantics
            details::atomic_exchange(_M_lock, 0l);
        }

    private:
        atomic_long _M_lock;
    };

    typedef ::pplx::scoped_lock<_Spin_lock> _Scoped_spin_lock;
} // namespace details

static struct _pplx_g_sched_t
{
    typedef std::shared_ptr<pplx::scheduler_interface> sched_ptr;

    _pplx_g_sched_t()
    {
        m_state = post_ctor;
    }

    ~_pplx_g_sched_t()
    {
        m_state = post_dtor;
    }

    sched_ptr get_scheduler()
    {
        switch (m_state)
        {
        case post_ctor:
            // This is the 99.9% case.

            if (!m_scheduler)
            {
                ::pplx::details::_Scoped_spin_lock lock(m_spinlock);
                if (!m_scheduler)
                {
                    m_scheduler = std::make_shared< ::pplx::default_scheduler_t>();
                }
            }

            return m_scheduler;
        default:
            // This case means the global m_scheduler is not available.
            // We spin off an individual scheduler instead.
            return std::make_shared< ::pplx::default_scheduler_t>();
        }
    }

    void set_scheduler(sched_ptr scheduler)
    {
        if (m_state == pre_ctor || m_state == post_dtor) {
            throw invalid_operation("Scheduler cannot be initialized now");
        }

        ::pplx::details::_Scoped_spin_lock lock(m_spinlock);

        if (m_scheduler != nullptr)
        {
            throw invalid_operation("Scheduler is already initialized");
        }

        m_scheduler = std::move(scheduler);
    }

    enum
    {
        pre_ctor = 0,
        post_ctor = 1,
        post_dtor = 2
    } m_state;

private:
    pplx::details::_Spin_lock m_spinlock;
    sched_ptr m_scheduler;
} _pplx_g_sched;

_PPLXIMP std::shared_ptr<pplx::scheduler_interface> _pplx_cdecl get_ambient_scheduler()
{
    return _pplx_g_sched.get_scheduler();
}

_PPLXIMP void _pplx_cdecl set_ambient_scheduler(std::shared_ptr<pplx::scheduler_interface> _Scheduler)
{
    _pplx_g_sched.set_scheduler(std::move(_Scheduler));
}

} // namespace pplx

#endif