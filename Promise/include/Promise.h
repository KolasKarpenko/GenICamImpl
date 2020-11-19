#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <sstream>

class IPromise
{
public:
	enum class State
	{
		Pending = 0,
		Resolved,
		Rejected,
		Canceled
	};

	virtual ~IPromise() {}
	virtual void Reset() = 0;
	virtual void Cancel() = 0;

	State GetState() const
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		return m_state;
	}

protected:
	State m_state;
	mutable std::recursive_mutex m_mutex;

	IPromise(): m_state(State::Pending)
	{
	}

};

template<typename TResult, typename TError>
class TPromise : public IPromise
{
public:
	typedef std::function<void(const TResult& result)> OnResolveFunc;
	typedef std::function<void(const TError& error)> OnRejectFunc;
	typedef std::function<void(int progress)> OnProgressFunc;
	typedef std::function<bool()> IsCanceledFunc;
	typedef std::function<void(
		const OnResolveFunc& resolve,
		const OnRejectFunc& reject,
		const OnProgressFunc& progress,
		const IsCanceledFunc& isCanceled
	)> PromiseFunc;
	typedef std::shared_ptr<TPromise<TResult, TError>> PromisePtr;

	struct Handlers
	{
		OnResolveFunc resolve;
		OnRejectFunc reject;
		OnProgressFunc progress;
	};


	void Then(const OnResolveFunc& resolve, const OnRejectFunc& reject, const OnProgressFunc& progress)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		static size_t handlerId = 0;
		handlerId++;

		m_handlers.insert(std::make_pair(handlerId, Handlers{ resolve, reject, progress }));

		switch (m_state)
		{
		case State::Resolved:
			progress(100);
			resolve(m_result);
			break;
		case State::Rejected:
			reject(m_error);
			break;
		default:
			break;
		}
	}

	void Then(const OnResolveFunc& resolve, const OnRejectFunc& reject)
	{
		Then(resolve, reject, [] (int) {});
	}

	void Then(const OnResolveFunc& resolve, const OnProgressFunc& progress)
	{
		Then(resolve, [](const TError&) {}, progress);
	}

	void Then(const OnResolveFunc& resolve)
	{
		Then(resolve, [](const TError&) {});
	}

	bool Result(TResult& result, TError& error, const OnProgressFunc& progress, uint32_t timeoutMs = 0)
	{
		std::condition_variable cv;
		std::atomic<bool> resolved(false);
		std::atomic<bool> ok(false);

		size_t resolveIndex = 0;

		{
			std::lock_guard<std::recursive_mutex> lock(m_mutex);
			m_cancelConditionPtr = &cv;

			Then(
				[&result, &resolved, &ok, &cv](const TResult& value) {
					result = value;
					resolved = true;
					ok = true;
					cv.notify_one();
				},
				[&error, &resolved, &cv](const TError& value) {
					error = value;
					resolved = true;
					cv.notify_one();
				},
				[&progress](int p) {
					progress(p);
				}
			);

			resolveIndex = m_handlers.rbegin()->first;
		}

		std::mutex m;
		std::unique_lock<std::mutex> lk(m);
		if (timeoutMs > 0) {
			cv.wait_for(lk, std::chrono::milliseconds(timeoutMs), [&resolved, this] {
				return resolved == true || GetState() == State::Canceled;
			});

			if (!ok){
				Cancel();
			}
		}
		else {
			cv.wait(lk, [&resolved, this] { 
				return resolved == true || GetState() == State::Canceled; 
			});
		}
		{
			std::lock_guard<std::recursive_mutex> lock(m_mutex);
			m_cancelConditionPtr = nullptr;
			m_handlers.erase(resolveIndex);
		}

		return ok;
	}

	bool Result(TResult& result, TError& error, uint32_t timeoutMs = 0)
	{
		return Result(result, error, [](int) {}, timeoutMs);
	}

	bool Result(TResult& result, const OnProgressFunc& progress, uint32_t timeoutMs = 0)
	{
		TError err;
		return Result(result, err, progress, timeoutMs);
	}

	bool Result(TResult& result, uint32_t timeoutMs = 0)
	{
		TError err;
		return Result(result, err, timeoutMs);
	}

	virtual void Cancel() override
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);

		if (m_state != State::Pending) {
			return;
		}

		m_state = State::Canceled;

		if (m_cancelConditionPtr != nullptr) {
			m_cancelConditionPtr->notify_one();
		}
	}

protected:
	TPromise(const PromiseFunc& impl) : IPromise(), m_impl(impl), m_cancelConditionPtr(nullptr)
	{
	}

	virtual void Resolve(const TResult& result)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);

		if (m_state != State::Pending) {
			return;
		}

		m_state = State::Resolved;
		m_result = result;

		for (const auto& cb : m_handlers) {
			cb.second.progress(100);
			cb.second.resolve(m_result);
		}
	}

	virtual void Reject(const TError& error)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);

		if (m_state != State::Pending) {
			return;
		}

		m_state = State::Rejected;
		m_error = error;
		for (const auto& cb : m_handlers) {
			cb.second.reject(m_error);
		}
	}

	void Progress(int progress)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);

		if (m_state != State::Pending) {
			return;
		}

		for (const auto& cb : m_handlers) {
			cb.second.progress(progress);
		}
	}

	void Run(
		const OnResolveFunc& resolve,
		const OnRejectFunc& reject,
		const OnProgressFunc& progress,
		const IsCanceledFunc& isCanceled
	) {
		m_impl(resolve, reject, progress, isCanceled);
	}

	bool IsCanceled() 
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		return m_state == State::Canceled;
	}

private:
	PromiseFunc m_impl;
	TResult m_result;
	TError m_error;

	std::map<size_t, Handlers> m_handlers;
	std::condition_variable* m_cancelConditionPtr;
};

class PromiseContext
{
private:

	template<typename TResult, typename TError>
	class PromiseBase : public TPromise<TResult, TError>
	{
	public:
		size_t GetId() const { return m_id; }

		virtual void Cancel() override
		{
			TPromise<TResult, TError>::Cancel();
			m_context.PopPool(m_id);
		}

	protected:
		PromiseBase(const typename TPromise<TResult, TError>::PromiseFunc& impl, PromiseContext& context) : TPromise<TResult, TError>(impl), m_context(context) {
			static size_t lastId = 0;
			m_id = lastId++;
		}

		virtual void Resolve(const TResult& result) override
		{
			TPromise<TResult, TError>::Resolve(result);
			m_context.PopPool(m_id);
		}

		virtual void Reject(const typename TError& error) override
		{
			TPromise<TResult, TError>::Reject(error);
			m_context.PopPool(m_id);
		}

	private:
		size_t m_id;
		PromiseContext& m_context;
	};

	template<typename TResult, typename TError>
	class Promise : public PromiseBase<TResult, TError>
	{
	public:
		Promise(const typename TPromise<TResult, TError>::PromiseFunc& impl, PromiseContext& context) : PromiseBase<TResult, TError>(impl, context)
		{}

		virtual void Reset() override
		{
			IPromise::m_state = IPromise::State::Pending;

			TPromise<TResult, TError>::Run(
				[this](const TResult& result) { PromiseBase<TResult, TError>::Resolve(result); },
				[this](const typename TError& error) { PromiseBase<TResult, TError>::Reject(error); },
				[this](int progress) { PromiseBase<TResult, TError>::Progress(progress); },
				[this]() { return PromiseBase<TResult, TError>::IsCanceled(); }
			);
		}
	};

	template<typename TResult, typename TError>
	class AsyncPromise : public PromiseBase<TResult, TError>
	{
	public:
		AsyncPromise(const typename TPromise<TResult, TError>::PromiseFunc& impl, PromiseContext& context) : PromiseBase<TResult, TError>(impl, context)
		{
		}

		virtual void Reset() override
		{
			if (m_threadPtr) {
				m_threadPtr->join();
			}

			IPromise::m_state = IPromise::State::Pending;

			m_threadPtr.reset(new std::thread(
				[this](
					const typename TPromise<TResult, TError>::OnResolveFunc& resolve,
					const typename TPromise<TResult, TError>::OnRejectFunc& reject,
					const typename TPromise<TResult, TError>::OnProgressFunc& progress,
					const typename TPromise<TResult, TError>::IsCanceledFunc& isCanceled
					) {
						TPromise<TResult, TError>::Run(resolve, reject, progress, isCanceled);
				},
				[this](const TResult& result) { PromiseBase<TResult, TError>::Resolve(result); },
					[this](const TError& error) { PromiseBase<TResult, TError>::Reject(error); },
					[this](int progress) { PromiseBase<TResult, TError>::Progress(progress); },
					[this]() { return PromiseBase<TResult, TError>::IsCanceled(); }
				));
		}

		~AsyncPromise()
		{
			if (m_threadPtr) {
				m_threadPtr->join();
			}
		}

	private:
		std::shared_ptr<std::thread> m_threadPtr;
	};

public:
	void Cancel()
	{
		std::map<size_t, std::shared_ptr<IPromise>> pool;
		{
			std::lock_guard<std::mutex> lock(m_poolMutex);
			pool = m_pool;
		}

		for (auto& p : pool) {
			p.second->Cancel();
		}
	}

	void Join()
	{
		std::mutex m;
		std::unique_lock<std::mutex> lk(m);
		m_exitCondition.wait(lk, [this] { return m_pool.empty(); });
	}

	template<typename TResult, typename TError>
	std::shared_ptr<TPromise<TResult, TError>> Create(const std::function<void(
		const std::function<void(const TResult & result)> & resolve,
		const std::function<void(const TError & error)> & reject,
		const std::function<void(int progress)> & progress,
		const std::function<bool()> & isCanceled
		)>& impl)
	{
		Promise<TResult, TError>* p = new Promise<TResult, TError>(impl, *this);
		std::shared_ptr<Promise<TResult, TError>> ptr(p);
		PushPool(p->GetId(), ptr);
		ptr->Reset();
		return ptr;
	}

	template<typename TResult, typename TError>
	std::shared_ptr<TPromise<TResult, TError>> CreateAsync(const std::function<void(
		const std::function<void(const TResult & result)> & resolve,
		const std::function<void(const TError & error)> & reject,
		const std::function<void(int progress)> & progress,
		const std::function<bool()>
		)> & impl)
	{
		AsyncPromise<TResult, TError>* p = new AsyncPromise<TResult, TError>(impl, *this);
		std::shared_ptr<TPromise<TResult, TError>> ptr(p);
		PushPool(p->GetId(), ptr);
		ptr->Reset();
		return ptr;
	}

	template<typename TResult, typename TError>
	std::shared_ptr<TPromise<std::vector<TResult>, TError>>  All(const std::vector<std::shared_ptr<TPromise<TResult, TError>>>& all)
	{
		return CreateAsync<std::vector<TResult>, TError>(
			[all](
				const typename Promise<std::vector<TResult>, TError>::OnResolveFunc& resolve,
				const typename Promise<std::vector<TResult>, TError>::OnRejectFunc& reject,
				const typename Promise<std::vector<TResult>, TError>::OnProgressFunc& progress,
				const typename Promise<std::vector<TResult>, TError>::IsCanceledFunc& isCanceled
				) {
					std::vector<TResult> result(all.size());
					for (size_t i = 0; i < all.size(); ++i) {

						if (isCanceled()) {
							return;
						}

						TResult res;
						TError err;
						if (all[i]->Result(res, err)) {
							result[i] = res;
							progress(int(i * 100.0 / all.size()));
						}
						else {
							if (all[i]->GetState() == IPromise::State::Rejected) {
								reject(err);
							}
							return;
						}
					}

					resolve(result);
			}
		);
	}

private:
	void PushPool(size_t id, const std::shared_ptr<IPromise>& ptr)
	{
		assert(ptr);
		std::lock_guard<std::mutex> lock(m_poolMutex);
		m_pool.insert(std::make_pair(id, ptr));
	}

	void PopPool(size_t id)
	{
		std::lock_guard<std::mutex> lock(m_poolMutex);
		m_pool.erase(id);

		if (m_pool.empty()) {
			m_exitCondition.notify_one();
		}
	}

	std::mutex m_poolMutex;
	std::map<size_t, std::shared_ptr<IPromise>> m_pool;
	std::condition_variable m_exitCondition;
};
