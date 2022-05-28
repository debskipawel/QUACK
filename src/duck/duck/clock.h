#pragma once
#include <cstdint>
#include "Windows.h"

//********************* NEW *********************

namespace mini
{
	namespace detail
	{
		inline int64_t GetInternalClockFrequency()
		{
			LARGE_INTEGER tmp;
			QueryPerformanceFrequency(&tmp);
			return tmp.QuadPart;
		}

		inline int64_t GetInternalClockTicks()
		{
			LARGE_INTEGER tmp;
			QueryPerformanceCounter(&tmp);
			return tmp.QuadPart;
		}
	}

	//Helper class to measure time between animation frames using Windows high resolution timer
	template<size_t Log2Samples = 6>
	class FPSClock
	{
	public:
		static constexpr size_t NSamples = 1 << Log2Samples;
		static constexpr size_t _IndexMask = NSamples - 1;

		FPSClock()
			: m_frequency(detail::GetInternalClockFrequency()), m_lastTicks(detail::GetInternalClockTicks()),
			m_frameTickSamples{ 0 }, m_ticksSamplesTotal(0), m_currentIndex(0)
		{ }

		//Update the clock. Returns time from last query or clock creation in seconds.
		double Query()
		{
			int64_t newTicks = detail::GetInternalClockTicks();
			m_currentIndex = (m_currentIndex + 1) & _IndexMask;
			m_ticksSamplesTotal -= m_frameTickSamples[m_currentIndex];
			m_frameTickSamples[m_currentIndex] = newTicks - m_lastTicks;
			m_ticksSamplesTotal += m_frameTickSamples[m_currentIndex];
			m_lastTicks = newTicks;
			return getFrameTime();
		}

		//Returns the time between last and second to last query in seconds;
		double getFrameTime() const { return  getFrameTicks() / static_cast<double>(m_frequency); }

		//Returns the number of internal clock ticks between the last and second to last query.
		int64_t getFrameTicks() const { return m_frameTickSamples[m_currentIndex]; }

		//Returns the average FPS over the last NSamples frames
		double getFPS() const { return (NSamples * m_frequency)/static_cast<double>(m_ticksSamplesTotal); }

	private:
		int64_t m_frequency, m_lastTicks, m_frameTickSamples[NSamples], m_ticksSamplesTotal;
		size_t m_currentIndex;
	};

	using Clock = FPSClock<>;
}
