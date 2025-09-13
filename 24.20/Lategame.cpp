#include "pch.h"
#include "Lategame.h"
#include "Utils.h"

FItemAndCount Lategame::GetShotguns()
{
    static UEAllocatedVector<FItemAndCount> Shotguns
    {

            FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/RadicalWeaponsGameplay/Weapons/RadicalShotgunPump/WID_Shotgun_RadicalPump_Athena_UR.WID_Shotgun_RadicalPump_Athena_UR")), // enhanced havoc
            FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/RadicalWeaponsGameplay/Weapons/RadicalShotgunPump/WID_Shotgun_RadicalPump_Athena_SR.WID_Shotgun_RadicalPump_Athena_SR")), // havoc
            FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/MusterCoreWeapons/Items/Weapons/Exotics/WID_Shotgun_Breach_Athena_X.WID_Shotgun_Breach_Athena_X")) // heisted breacher
    };

    return Shotguns[rand() % (Shotguns.size() - 1)];
}

FItemAndCount Lategame::GetAssaultRifles()
{
    static UEAllocatedVector<FItemAndCount> AssaultRifles
    {
            FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/RadicalWeaponsGameplay/Weapons/RadicalCoreAR/BossWID/WID_Assault_Radical_CoreAR_Athena_UR_CloneBoss.WID_Assault_Radical_CoreAR_Athena_UR_CloneBoss")), // highcard havoc
            FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/RadicalWeaponsGameplay/Weapons/RadicalCoreAR/WID_Assault_Radical_CoreAR_Athena_SR.WID_Assault_Radical_CoreAR_Athena_SR")), // havoc
            FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/RadicalWeaponsGameplay/Weapons/PulseRifleMMObj/WID_Assault_PastaRipper_Athena_MMObj.WID_Assault_PastaRipper_Athena_MMObj")), // overclocked pulse
    };

    return AssaultRifles[rand() % (AssaultRifles.size() - 1)];
}


FItemAndCount Lategame::GetUtilities()
{
    static UEAllocatedVector<FItemAndCount> Utilities
    {
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_VR_Ore_T03.WID_Sniper_Heavy_Athena_VR_Ore_T03")), 
        FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_SR_Ore_T03.WID_Sniper_Heavy_Athena_SR_Ore_T03")), 
        FItemAndCount(3, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/DryBox/Items/NyxGlass/AGID_NyxGlass.AGID_NyxGlass")), // odm gear
    };

    return Utilities[rand() % (Utilities.size() - 1)];
}


FItemAndCount Lategame::GetHeals()
{
    static UEAllocatedVector<FItemAndCount> Heals
    {
            FItemAndCount(6, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/ChillBronco/Athena_ChillBronco.Athena_ChillBronco")),
            FItemAndCount(2, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/PurpleStuff/Athena_PurpleStuff.Athena_PurpleStuff")),
            FItemAndCount(3, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/Flopper/WID_Athena_Flopper.WID_Athena_Flopper")),
            FItemAndCount(6, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall")),
            FItemAndCount(3, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/Shields/Athena_Shields.Athena_Shields")),
            FItemAndCount(2, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/Game/Athena/Items/Consumables/PurpleStuff/Athena_PurpleStuff.Athena_PurpleStuff")),
            FItemAndCount(1, {}, Utils::FindObject<UFortWeaponRangedItemDefinition>(L"/FlipperGameplay/Items/HealSpray/WID_Athena_HealSpray.WID_Athena_HealSpray"))
    };

    return Heals[rand() % (Heals.size() - 1)];
}
UFortAmmoItemDefinition* Lategame::GetAmmo(EAmmoType AmmoType)
{
    static UEAllocatedVector<UFortAmmoItemDefinition*> Ammos
    {
        Utils::FindObject<UFortAmmoItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight"),
        Utils::FindObject<UFortAmmoItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells"),
        Utils::FindObject<UFortAmmoItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium"),
        Utils::FindObject<UFortAmmoItemDefinition>(L"/Game/Athena/Items/Ammo/AmmoDataRockets.AmmoDataRockets"),
        Utils::FindObject<UFortAmmoItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy")
    };

    return Ammos[(uint8)AmmoType];
}

UFortResourceItemDefinition* Lategame::GetResource(EFortResourceType ResourceType)
{
    static UEAllocatedVector<UFortResourceItemDefinition*> Resources
    {
        Utils::FindObject<UFortResourceItemDefinition>(L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData"),
        Utils::FindObject<UFortResourceItemDefinition>(L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData"),
        Utils::FindObject<UFortResourceItemDefinition>(L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData")
    };

    return Resources[(uint8)ResourceType];
}