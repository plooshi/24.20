#pragma once
#include "pch.h"
#include "Utils.h"

class AI
{
private:
	static void SpawnAI(const wchar_t*, const wchar_t*);
	static FFortItemEntry& GiveItem(AFortAthenaAIBotController*, FItemAndCount);
	DefHookOg(void, AIBotBehaviorOnSpawned, UFortAthenaAISpawnerDataComponent_AIBotBehavior*, AFortPlayerPawn*);
	DefHookOg(void, InventoryOnSpawned, UFortAthenaAISpawnerDataComponent_InventoryBase*, AFortPlayerPawn*);
	DefHookOg(void, AIBotConversationOnSpawned, UFortAthenaAISpawnerDataComponent_AIBotConversation*, AFortPlayerPawn*);
	DefHookOg(void, AIBotGameplayAbilityBaseOnSpawned, UFortAthenaAISpawnerDataComponent_AIBotGameplayAbilityBase*, AFortPlayerPawn*);
	DefHookOg(void, AIBotSkillsetOnSpawned, UFortAthenaAISpawnerDataComponent_AIBotSkillset*, AFortPlayerPawn*);
	DefHookOg(void, CreateAndConfigureNavigationSystem, UAthenaNavSystemConfig*, UWorld*);
	DefHookOg(void, OnPawnAISpawned, __int64, AFortPlayerPawn*);
	DefHookOg(__int64, WaitForGamePhaseToSpawn, UAthenaAIServicePlayerBots*, __int64*, __int64);
	static void GetLoadout(UFortAthenaAISpawnerDataComponent_CosmeticLibrary*, FFortAthenaLoadout*);
	static void GetDances(UFortAthenaAISpawnerDataComponent_CosmeticLibrary*, TArray<UAthenaDanceItemDefinition*>*, AFortAthenaAIBotController*);
	static void InitializeMMRInfos();
	static void SpawnAIHook(UAthenaAIServicePlayerBots* AIServicePlayerBots, FFrame& Stack, APawn** Ret);
	static UFortAthenaAIBotAimingDigestedSkillSet* GetOrCreateDigestedAimingSkillSet(AFortAthenaAIBotController*);
	DefHookOg(FDigestedWeaponAccuracy*, GetWeaponAccuracy, UFortAthenaAIBotAimingDigestedSkillSet*, UFortWeaponItemDefinition*);

	template <typename T>
	static T PickWeighted(TArray<T>& Map, float (*RandFunc)(float), bool bCheckZero = true) // pls put this into utils...
	{
		float TotalWeight = std::accumulate(Map.begin(), Map.end(), 0.0f, [&](float acc, T& p) { return acc + Utils::EvaluateScalableFloat(p.Weight); });
		float RandomNumber = RandFunc(TotalWeight);

		for (auto& Element : Map)
		{
			float Weight = Utils::EvaluateScalableFloat(Element.Weight);
			if (bCheckZero && Weight == 0)
				continue;

			if (RandomNumber <= Weight) return Element;

			RandomNumber -= Weight;
		}

		return T();
	}

public:
	static void SpawnAIs();

	InitHooks;
};