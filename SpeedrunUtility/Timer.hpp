#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include "json.hpp"

namespace tmr
{
	typedef unsigned long ulong;

	class Timer
	{
	private:
		ulong sTime;

	public:
		void Start()
		{
			sTime = clock();
		}

		ulong ElapsedTime()
		{
			return ((ulong)clock() - sTime) / CLOCKS_PER_SEC;
		}

		bool TimePassed(ulong seconds)
		{
			return ElapsedTime() >= seconds;
		}
	};
}

namespace killer
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
}
