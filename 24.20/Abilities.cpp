#include "pch.h"
#include "Abilities.h"

void Abilities::InternalServerTryActivateAbility(UFortAbilitySystemComponentAthena* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey& PredictionKey, FGameplayEventData* TriggerEventData) {
    auto Spec = AbilitySystemComponent->ActivatableAbilities.Items.Search([&](FGameplayAbilitySpec& item) {
        return item.Handle.Handle == Handle.Handle;
        });
    if (!Spec)
        return AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);

    Spec->InputPressed = true;

    UGameplayAbility* InstancedAbility = nullptr;
    auto Abilites = (bool (*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle, FPredictionKey, UGameplayAbility**, void*, const FGameplayEventData*)) Sarah::Offsets::InternalTryActivateAbility;
    if (!Abilites(AbilitySystemComponent, Handle, PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
    {
        AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        Spec->InputPressed = false;
    }
    AbilitySystemComponent->ActivatableAbilities.MarkItemDirty(*Spec);
}

void Abilities::GiveAbility(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UFortGameplayAbility> Ability)
{
    if (!AbilitySystemComponent || !Ability)
        return;

    FGameplayAbilitySpec Spec{};

    ((void (*)(FGameplayAbilitySpec*, UGameplayAbility*, int, int, UObject*)) (Sarah::Offsets::ImageBase + 0x8577A3C))(&Spec, (UGameplayAbility*)Ability->DefaultObject, 1, -1, nullptr);
    ((FGameplayAbilitySpecHandle * (__fastcall*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec)) (Sarah::Offsets::ImageBase + 0x858A458))(AbilitySystemComponent, &Spec.Handle, Spec);
}

void Abilities::GiveAbilitySet(UAbilitySystemComponent* AbilitySystemComponent, UFortAbilitySet* Set)
{
    if (Set)
    {
        for (auto& GameplayAbility : Set->GameplayAbilities)
            GiveAbility(AbilitySystemComponent, GameplayAbility);
        for (auto& GameplayEffect : Set->PassiveGameplayEffects)
            AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(GameplayEffect.GameplayEffect.Get(), GameplayEffect.Level, AbilitySystemComponent->MakeEffectContext());
    }
}

void Abilities::Hook() {
    Utils::HookEvery<UAbilitySystemComponent>(Sarah::Offsets::InternalServerTryActivateAbilityVft, InternalServerTryActivateAbility);
}
