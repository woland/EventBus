#include "dexode/eventbus/strategy/Protected.hpp"

namespace dexode::eventbus::strategy
{

std::size_t Protected::processLimit(const std::size_t maxCountOfEvents)
{
	std::size_t processed = 0;
	std::function<void()> postponeEventCallback;

	while(processed < maxCountOfEvents)
	{
		{
			std::unique_lock writeLock{_mutex};
			if(_eventQueue.empty())
			{
				break;
			}
			postponeEventCallback = std::move(_eventQueue.front());
			_eventQueue.pop_front();
		}
		postponeEventCallback();
		++processed;
	}
	return processed;
}

void Protected::unlistenAll(const std::uint32_t listenerID)
{
	std::unique_lock writeLock{_mutex}; // TODO locking already locked mutex
	for(auto& element : _callbacks)
	{
		element.second->remove(listenerID);
	}
}

bool Protected::wait()
{
	using namespace std::chrono_literals;
	std::unique_lock<std::mutex> lock(_waitMutex);
	_eventWaiting.wait(lock);

	std::shared_lock readLock{_mutex};
	return not _eventQueue.empty();
}
bool Protected::waitFor(std::chrono::milliseconds timeout)
{
	using namespace std::chrono_literals;
	std::unique_lock<std::mutex> lock(_waitMutex);
	_eventWaiting.wait_for(lock, timeout);

	std::shared_lock readLock{_mutex};
	return not _eventQueue.empty();
}

} // namespace dexode::eventbus::strategy
