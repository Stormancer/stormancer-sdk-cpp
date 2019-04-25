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
**/
#include "stdafx.h"
#include "pplx/threadpool-std.h"
#include <cstddef>
#include <mutex>
#include <thread>

namespace
{
	const size_t DEFAULT_NUM_THREADS(10);
}

namespace crossplat
{
	// initialize the static shared threadpool
	threadpool& threadpool::shared_instance()
	{
		// Thread-safe in C++11
		static threadpool_std instance(DEFAULT_NUM_THREADS);
		return static_cast<threadpool&>(instance);
	}

	void threadpool_std::schedule(std::function<void()> task)
	{
		{
			std::unique_lock<std::mutex> lock(this->queue_mutex);

			if (stop)
			{
				throw std::runtime_error("enqueue on stopped threadpool");
			}
			this->tasks.emplace_back(task);
		}
		condition.notify_one();
	}

	threadpool_std::threadpool_std(size_t threads)
		:stop(false)
	{
		workers.reserve(threads);
		for (size_t i = 0; i < threads; ++i)
		{
			workers.emplace_back(
				[this]
			{
				for (;;)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->condition.wait(lock,
							[this] { return this->stop || !this->tasks.empty(); });
						if (this->stop && this->tasks.empty())
							return;
						task = std::move(this->tasks.front());
						this->tasks.pop_front();
					}

					task();
				}
			});
		}
	}

	threadpool_std::~threadpool_std()
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}

		condition.notify_all();
		for (auto &worker : workers)
		{
			worker.join();
		}
	}


}
