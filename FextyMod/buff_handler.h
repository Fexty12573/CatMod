#pragma once

#ifndef INCLUDING_BUFFHANDLER
#define INCLUDING_BUFFHANDLER

#include "game_functions.h"
#include "Timer.hpp"

class BuffHandler
{
private:
	bool m_AffinityActive;

public:
	BuffHandler() : m_AffinityActive(false) { }

	enum class Mode {
		REGULAR,
		HEROICS,
		AFFINITY,
		SPEEDRUN
	};
	enum Buff {
		AttackS = 0x3A,
		AttackL = 0x3B,
		DefenseS = 0x3C,
		DefenseL = 0x3D,
		Affinity = 0x3E,
		RecSpeed = 0x3F,
		Health = 0x40,
		Stamina = 0x41
	};

	template<Mode mode>
	void NextBuff(float fAttackTimer, undefined8& _return)
	{
		switch (mode)
		{
		case BuffHandler::Mode::REGULAR:
			HandleRegularModeBuff(fAttackTimer, _return);
			break;
		case BuffHandler::Mode::HEROICS:
			HandleHeroicsModeBuff(fAttackTimer, _return);
			break;
		case BuffHandler::Mode::AFFINITY:
			HandleAffinityModeBuff(fAttackTimer, _return);
			break;
		case BuffHandler::Mode::SPEEDRUN:
			HandleSpeedrunModeBuff(fAttackTimer, _return);
			break;
		default:
			break;
		}
	}

private:
	inline void HandleRegularModeBuff(float fAttackTimer, undefined8& _return)
	{
		if (fAttackTimer <= 0.0f) _return = Buff::AttackL;
	}
	inline void HandleHeroicsModeBuff(float fAttackTimer, undefined8& _return)
	{
		if (fAttackTimer <= 0.0f) _return = Buff::AttackL;
		else _return = RandomHeroics();
	}
	inline void HandleAffinityModeBuff(float fAttackTimer, undefined8& _return)
	{
		if (fAttackTimer == 0.0f) m_AffinityActive = false;

		if (fAttackTimer <= 0)
		{
			_return = Buff::AttackL;
		}
		else if (!m_AffinityActive)
		{
			_return = Buff::Affinity;
			m_AffinityActive = true;
		}
		else
		{
			_return = Buff::AttackS;
		}
	}
	inline void HandleSpeedrunModeBuff(float fAttackTimer, undefined8& _return)
	{
		if (fAttackTimer == 0.0f) _return = Buff::AttackL;
	}

	undefined8 RandomHeroics()
	{
		srand(time(NULL));
		switch (rand() % 4 + 1)
		{
		case 1: return Buff::AttackS;     // Atk S
		case 2: return Buff::Affinity;     // Aff
		case 3: return Buff::RecSpeed;     // RecSpd
		case 4: return Buff::Stamina;     // Sta
		default: return Buff::Affinity;    // Aff
		}
	}
};

#endif // !INCLUDING_BUFFHANDLER

