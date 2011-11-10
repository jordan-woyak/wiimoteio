/*
Copyright (c) 2011 - wiimoteio project

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#pragma once

#include <map>
#include <functional>
#include <thread>
#include <condition_variable>
#include <chrono>

namespace wio
{

class worker_thread
{
public:
	//typedef std::chrono::high_resolution_clock clock;
	typedef std::chrono::steady_clock clock;

	typedef unsigned int job_type;

	worker_thread()
	{
		m_job_thread = std::thread(std::mem_fun(&worker_thread::job_thread_func), this);
	}

	~worker_thread()
	{
		schedule_job(nullptr);
		m_job_thread.join();
	}

	template <typename F>
	void schedule_job_at(F&& func, const clock::time_point& timept, job_type _type = 0)
	{
		{
		std::lock_guard<std::mutex> lk(m_job_lock);

		job j;
		j.func = std::forward<F>(func);
		j.type = _type;

		m_job_queue.insert(std::make_pair(timept, std::move(j)));
		}

		m_job_cond.notify_one();
	}

	template <typename F, typename Rep, typename Per>
	void schedule_job_in(F&& func, std::chrono::duration<Rep, Per> duration, job_type _type = 0)
	{
		schedule_job_at(std::forward<F>(func), clock::now() + duration, _type);
	}

	template <typename F>
	void schedule_job(F&& func, job_type _type = 0)
	{
		schedule_job_in(std::forward<F>(func), std::chrono::seconds(0), _type);
	}

	void remove_type(job_type _type)
	{
		std::lock_guard<std::mutex> lk(m_job_lock);
		
		for (auto iter = m_job_queue.begin(); iter != m_job_queue.end(); )
			if (_type == iter->second.type)
				m_job_queue.erase(iter++);
			else
				++iter;
	}

	void remove_all()
	{
		std::lock_guard<std::mutex> lk(m_job_lock);
		m_job_queue.clear();
	}

private:
	worker_thread(const worker_thread&);
	worker_thread& operator=(const worker_thread&);

	void job_thread_func()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lk(m_job_lock);

			if (m_job_queue.empty())
				m_job_cond.wait(lk);
			else
				m_job_cond.wait_until(lk, m_job_queue.begin()->first);

			auto const iter = m_job_queue.begin();
			if (iter != m_job_queue.end() && clock::now() >= iter->first)
			{
				auto job = std::move(iter->second);
				m_job_queue.erase(iter);

				lk.unlock();

				if (!job.func)
					break;

				job.func();
			}
		}
	}

	struct job
	{
		std::function<void()> func;
		job_type type;
	};

	std::multimap<clock::time_point, job> m_job_queue;
	std::thread m_job_thread;
	std::mutex m_job_lock;
	std::condition_variable m_job_cond;
};

}	// namespace

