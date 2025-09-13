#pragma once
#include "pch.h"
#include "Utils.h"

class Quests
{
public:
    static void QueueStatEvent(UFortQuestManager* _this, EFortQuestObjectiveStatEvent InType, const UObject* InTargetObject, FGameplayTagContainer* InTargetTags, FGameplayTagContainer* InSourceTags, FGameplayTagContainer* InContextTags, FFortQuestObjectiveStatTableRow* InObjectiveStat, FName InObjectiveBackendName, int InCount);

	InitHooks;
};