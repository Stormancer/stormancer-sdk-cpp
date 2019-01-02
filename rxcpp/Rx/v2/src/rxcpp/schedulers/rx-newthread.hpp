// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved. See License.txt in the project root for license information.

#pragma once

#if !defined(RXCPP_RX_SCHEDULER_NEW_THREAD_HPP)
#define RXCPP_RX_SCHEDULER_NEW_THREAD_HPP
#include <mutex>
#include "../rx-includes.hpp"
#include "stormancer/TimerThread.h"

namespace rxcpp {

	namespace schedulers {

		typedef std::function<std::thread(std::function<void()>)> thread_factory;

		struct new_thread : public scheduler_interface
		{
		private:
			typedef new_thread this_type;
			new_thread(const this_type&);

			static Stormancer::TimerThread& timer()
			{
				static Stormancer::TimerThread s_timer;
				return s_timer;
			}

			struct new_worker : public worker_interface
			{
			private:
				typedef new_worker this_type;

				typedef detail::action_queue queue_type;

				new_worker(const this_type&);

				struct new_worker_state : public std::enable_shared_from_this<new_worker_state>
				{
					virtual ~new_worker_state()
					{
						lifetime.unsubscribe();
					}

					explicit new_worker_state(composite_subscription cs)
						: lifetime(cs)
					{
					}

					composite_subscription lifetime;
					recursion r;
				};

				std::shared_ptr<new_worker_state> state;

			public:
				virtual ~new_worker()
				{
				}

				explicit new_worker(std::shared_ptr<new_worker_state> ws)
					: state(ws)
				{
				}

				new_worker(composite_subscription cs)
					: state(std::make_shared<new_worker_state>(cs))
				{

				}

				virtual clock_type::time_point now() const {
					return clock_type::now();
				}

				virtual void schedule(const schedulable& scbl) const {
					schedule(now(), scbl);
				}

				virtual void schedule(clock_type::time_point when, const schedulable& scbl) const {
					if (scbl.is_subscribed()) {
						state->r.reset(false);
						auto statePtr = state; // needed to pass state to the task

						timer().schedule([statePtr, scbl]()
						{
							if (scbl.is_subscribed())
							{
								statePtr->r.reset(true);
								scbl(statePtr->r.get_recurse());
							}
						}, when);
					}
				}
			};

		public:
			new_thread()
			{
			}
			explicit new_thread(thread_factory) {}

			virtual ~new_thread()
			{
			}

			virtual clock_type::time_point now() const {
				return clock_type::now();
			}

			virtual worker create_worker(composite_subscription cs) const {
				return worker(cs, std::make_shared<new_worker>(cs));
			}
		};

		inline scheduler make_new_thread() {
			static scheduler instance = make_scheduler<new_thread>();
			return instance;
		}
		inline scheduler make_new_thread(thread_factory) {
			static scheduler instance = make_scheduler<new_thread>();
			return instance;
		}
	}

}

#endif
