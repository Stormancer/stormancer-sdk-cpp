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
*
* Simple Linux implementation of a static thread pool.
*
* For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
*
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
***/
#pragma once


#include <vector>
#include <functional>
#include <cstddef>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wunreachable-code"
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

namespace crossplat {
	
	class threadpool
	{
	public:

		static threadpool& shared_instance();

		virtual void schedule(std::function<void()> task) = 0;

		virtual ~threadpool() {}
	};

	class threadpool_std : public threadpool
	{
	public:

		threadpool_std(size_t n);


		void schedule(std::function<void()> task) override;

		~threadpool_std();


	private:

		std::vector<std::thread> workers;

		std::deque<std::function<void()>> tasks;
		bool stop;

		std::mutex queue_mutex;
		std::condition_variable condition;
	};

}
