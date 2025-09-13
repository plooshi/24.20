#pragma once
#include "pch.h"
#include "Utils.h"

class Abilities {
private:
	static void InternalServerTryActivateAbility(UFortAbilitySystemComponentAthena*, FGameplayAbilitySpecHandle, bool, FPredictionKey&, FGameplayEventData*);
public:
	static void GiveAbility(UAbilitySystemComponent*, TSubclassOf<UFortGameplayAbility>);
	static void GiveAbilitySet(UAbilitySystemComponent*, UFortAbilitySet*);

	InitHooks;
};
