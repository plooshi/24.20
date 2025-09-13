#pragma once
#include "pch.h"

enum class EAmmoType : uint8
{
    Assault = 0,
    Shotgun = 1,
    Submachine = 2,
    Rocket = 3,
    Sniper = 4
};

class Lategame
{
public:
    static FItemAndCount GetShotguns();
    static FItemAndCount GetAssaultRifles();
    static FItemAndCount GetUtilities();
    static FItemAndCount GetHeals();

    static UFortAmmoItemDefinition* GetAmmo(EAmmoType);
    static UFortResourceItemDefinition* GetResource(EFortResourceType);
};