#pragma once

#ifndef __TIMER_H__
#define __TIMER_H__

#include <ctime>
#include <fstream>
#include <sstream>

namespace tmr
{
	typedef unsigned long ulong;

	class Timer
	{
	private:
		ulong sTime;
		bool m_paused;

	public:
		Timer() : m_paused(false), sTime(0)
		{
		}
		Timer(int s) : m_paused(0), sTime(s)
		{
		}

		void Start()
		{
			sTime = clock();
			m_paused = false;
		}

		void Pause()
		{
			m_paused = true;
		}

		inline ulong ElapsedTime() const
		{
			return ((ulong)clock() - sTime) / CLOCKS_PER_SEC;
		}

		inline ulong ElapsedTimeMS() const
		{
			return ((ulong)clock() - sTime);
		}

		bool TimePassed(ulong seconds) const
		{
			if (m_paused) return false;
			return ElapsedTime() >= seconds;
		}

		bool TimePassedMS(ulong milliseconds) const
		{
			if (m_paused) return false;
			return ElapsedTimeMS() >= milliseconds;
		}

		Timer& operator=(const int num)
		{
			sTime = num;
		}
		bool operator>=(const ulong t)
		{
			return ElapsedTimeMS() >= t;
		}
		bool operator<=(const ulong t)
		{
			return ElapsedTimeMS() <= t;
		}
	};
}

/*namespace killer
{
	enum DayTypes {
		Week,
		Month,
		Year
	};

	class Killer
	{
	private:
		std::time_t tm;
		std::tm* ts;

		void Update()
		{
			tm = std::time(NULL);
			ts = localtime(&tm);
		}

	public:
		int Sec()
		{
			Update();
			return ts->tm_sec;
		}

		int Min()
		{
			Update();
			return ts->tm_min;
		}

		int Hour()
		{
			Update();
			return ts->tm_hour;
		}

		int Day(int dayType)
		{
			Update();
			switch (dayType)
			{
			case killer::Week: // 1 = Mon, 2 = Tue ...
				return ts->tm_wday + 1;
				break;
			case killer::Month:
				return ts->tm_mday;
				break;
			case killer::Year:
				return ts->tm_yday;
				break;
			default:
				break;
			}
		}

		int Month() // 1 = Jan, 2 = Feb ...
		{
			Update();
			return ts->tm_mon + 1;
		}

		int Year()
		{
			Update();
			return ts->tm_year + 1900;
		}
	};
}*/


#endif // !__TIMER_H__
