#include "pch.h"
#include "Vehicles.h"

#include "Inventory.h"


static int32 EvaluateMinMaxPercent(FScalableFloat Min, FScalableFloat Max, int32 Count)
{
    float AmmoSpawnMin = Utils::EvaluateScalableFloat(Min), AmmoSpawnMax = Utils::EvaluateScalableFloat(Max);
    auto OutVal = (int)(AmmoSpawnMax - AmmoSpawnMin) == 0 ? 0 : Count * (rand() % (int)(AmmoSpawnMax - AmmoSpawnMin));
    OutVal += Count * (100 - (int)AmmoSpawnMax) / 100;
    return OutVal;
}

void Vehicles::SpawnVehicles()
{
    auto Spawners = Utils::GetAll<AFortAthenaLivingWorldStaticPointProvider>();
    //auto Spawners = Utils::GetAll<UFortAthenaLivingWorldEventData>();
    UEAllocatedMap<FName, UClass*> VehicleSpawnerMap =
    {
        { FName(L"Athena.Vehicle.SpawnLocation.Motorcycle.Dirtbike"), Utils::FindObject<UClass>(L"/Dirtbike/Vehicle/Motorcycle_DirtBike_Vehicle.Motorcycle_DirtBike_Vehicle_C") },
        { FName(L"Athena.Vehicle.SpawnLocation.Motorcycle.Sportbike"), Utils::FindObject<UClass>(L"/Sportbike/Vehicle/Motorcycle_Sport_Vehicle.Motorcycle_Sport_Vehicle_C") },
        { FName(L"Athena.Vehicle.SpawnLocation.Valet.BasicCar.Taxi"), Utils::FindObject<UClass>(L"/Valet/TaxiCab/Valet_TaxiCab_Vehicle.Valet_TaxiCab_Vehicle_C") },
        { FName(L"Athena.Vehicle.SpawnLocation.Valet.BasicCar.Modded"), Utils::FindObject<UClass>(L"/ModdedBasicCar/Vehicle/Valet_BasicCar_Vehicle_SuperSedan.Valet_BasicCar_Vehicle_SuperSedan_C") },
        { FName(L"Athena.Vehicle.SpawnLocation.Valet.BasicTruck.Upgraded"), Utils::FindObject<UClass>(L"/Valet/BasicTruck/Valet_BasicTruck_Vehicle_Upgrade.Valet_BasicTruck_Vehicle_Upgrade_C") },
        { FName(L"Athena.Vehicle.SpawnLocation.Valet.BigRig.Upgraded"), Utils::FindObject<UClass>(L"/Valet/BigRig/Valet_BigRig_Vehicle_Upgrade.Valet_BigRig_Vehicle_Upgrade_C") },
        { FName(L"Athena.Vehicle.SpawnLocation.Valet.SportsCar.Upgraded"), Utils::FindObject<UClass>(L"/Valet/SportsCar/Valet_SportsCar_Vehicle_Upgrade.Valet_SportsCar_Vehicle_Upgrade_C") },
        { FName(L"Athena.Vehicle.SpawnLocation.Valet.BasicCar.Upgraded"), Utils::FindObject<UClass>(L"/Valet/BasicCar/Valet_BasicCar_Vehicle_Upgrade.Valet_BasicCar_Vehicle_Upgrade_C") }
    };

    for (auto& Spawner : Spawners)
    {
        UClass* VehicleClass = nullptr;
        for (auto& Tag : Spawner->FiltersTags.GameplayTags)
        {
            if (VehicleSpawnerMap.contains(Tag.TagName))
            {
                VehicleClass = VehicleSpawnerMap[Tag.TagName];
                break;
            }
        }
        if (VehicleClass)
        {
            auto Vehicle = Utils::SpawnActor<AFortAthenaVehicle>(VehicleClass, Spawner->K2_GetActorLocation(), Spawner->K2_GetActorRotation());
            //Log(L"Spawned a %s", Spawner->Name.ToWString().c_str());
        }
        else {
            for (auto& Tag : Spawner->FiltersTags.GameplayTags)
            {
                Log(L"Fix: Tag: %s", Tag.TagName.ToWString().c_str());
            }
            Log(L"");
        }
    }
    /*for (auto& Spawner : Spawners)
    {
        auto VC = Spawner->GetVehicleClass();

        if (!InoperableVehicles[VC] && Spawner->SpawnedVehicle->IsA<AFortDagwoodVehicle>())
        {
            auto VID = Spawner->CachedFortVehicleItemDef;
            InoperableVehicles[VC] = EvaluateMinMaxPercent(VID->MinPercentInoperable, VID->MaxPercentInoperable, VehicleCounts[VC]);
        }
    }
    for (auto& [VC, VehicleArray] : Vehicles)
    {
        for (int i = 0; i < InoperableVehicles[VC]; i++)
        {
            auto x = rand() % VehicleArray.Num();
            VehicleArray[x]->SpawnedVehicle->Cast<AFortDagwoodVehicle>()->bIsInoperable = true;
        }
    }*/
    Spawners.Free();

    Log(L"Spawned vehicles");
}

void Vehicles::ServerMove(UObject* Context, FFrame& Stack)
{
    FReplicatedPhysicsPawnState State;
    Stack.StepCompiledIn(&State);
    Stack.IncrementCode();
    auto Pawn = (AFortPhysicsPawn*)Context;

    UPrimitiveComponent* RootComponent = Pawn->RootComponent->Cast<UPrimitiveComponent>();

    if (RootComponent)
    {
        FRotator RealRotation = State.Rotation.Rotator();

        RealRotation.Yaw = FRotator::UnwindDegrees(RealRotation.Yaw);

        RealRotation.Pitch = 0;
        RealRotation.Roll = 0;

        RootComponent->K2_SetWorldLocationAndRotation(State.Translation, RealRotation, false, nullptr, true);
        RootComponent->SetPhysicsLinearVelocity(State.LinearVelocity, 0, FName(0));
        RootComponent->SetPhysicsAngularVelocityInDegrees(State.AngularVelocity, 0, FName(0));
    }
}

void Vehicles::EquipVehicleWeapon(APawn* Vehicle, AFortPlayerPawn* PlayerPawn, int32 SeatIdx)
{
    Log(L"Called! %d", SeatIdx);

    UFortVehicleSeatWeaponComponent* SeatWeaponComponent = (UFortVehicleSeatWeaponComponent*)Vehicle->GetComponentByClass(UFortVehicleSeatWeaponComponent::StaticClass());

    if (!SeatWeaponComponent)
        return;

    UFortWeaponItemDefinition* Weapon = nullptr;

    for (auto& WeaponDefinition : SeatWeaponComponent->WeaponSeatDefinitions)
    {
        if (WeaponDefinition.SeatIndex != SeatIdx)
            continue;

        Weapon = WeaponDefinition.VehicleWeapon;
        break;
    }

    Log(L"KYS");
    if (Weapon)
    {
        auto PlayerController = (AFortPlayerController*)PlayerPawn->Controller;

        Inventory::GiveItem(PlayerController, Weapon, 1, Inventory::GetStats(Weapon)->ClipSize);

        auto ItemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry)
            { return entry.ItemDefinition == Weapon; }
        );
        auto FortWeapon = *PlayerPawn->CurrentWeaponList.Search([&](AFortWeapon* Weapon) {
            return Weapon->ItemEntryGuid == ItemEntry->ItemGuid;
            });

        PlayerController->ServerExecuteInventoryItem(ItemEntry->ItemGuid);
        PlayerController->ClientEquipItem(ItemEntry->ItemGuid, true);
    }
    Log(L"KYS");

    /*if (!PlayerPawn)
    {
        Log(L"No PlayerPawn!");
        return;
    }

    AFortPlayerController* PlayerController = PlayerPawn->Controller->Cast<AFortPlayerController>();

    if (!PlayerController)
    {
        Log(L"No PlayerController!");
        return;
    }

    UFortVehicleSeatWeaponComponent* SeatWeaponComponent = (UFortVehicleSeatWeaponComponent*)Vehicle->GetComponentByClass(UFortVehicleSeatWeaponComponent::StaticClass());

    if (!SeatWeaponComponent)
    {
        Log(L"No SeatWeaponComponent!");
        return;
    }
    else if (!SeatWeaponComponent->WeaponSeatDefinitions.IsValidIndex(SeatIdx))
    {
        Log(L"Invalid SeatIdx!");
        return;
    }

    FWeaponSeatDefinition& WeaponSeatDefinition = SeatWeaponComponent->WeaponSeatDefinitions[SeatIdx]; // ik this might look skunked, but they get filled in order of seat idx

    UFortWeaponItemDefinition* VehicleWeapon = WeaponSeatDefinition.VehicleWeapon ? WeaponSeatDefinition.VehicleWeapon : WeaponSeatDefinition.VehicleWeaponOverride;

    if (WeaponSeatDefinition.SeatIndex != SeatIdx || !VehicleWeapon) // still make sure tho
    {
        Log(L"No VehicleWeapon or invalid SeatIdx found!");
        return;
    }

    UFortWorldItem* VehicleWeaponInstance = Inventory::GiveItem(PlayerController, VehicleWeapon, 1, 99999, 0, false, true);

    if (!VehicleWeaponInstance)
    {
        Log(L"No VehicleWeaponInstance!");
        return;
    }

    AFortInventory* WorldInventory = PlayerController->WorldInventory;

    if (WorldInventory)
    {
        for (auto& ReplicatedEntry : WorldInventory->Inventory.ReplicatedEntries)
        {
            if (ReplicatedEntry.ItemGuid == VehicleWeaponInstance->GetItemGuid())
            {
                WorldInventory->Inventory.MarkItemDirty(ReplicatedEntry);
                WorldInventory->Inventory.MarkItemDirty(VehicleWeaponInstance->ItemEntry);
                WorldInventory->HandleInventoryLocalUpdate();
            }
        }
    }

    AFortWeaponRangedForVehicle* EquippedWeapon = PlayerPawn->EquipWeaponDefinition(VehicleWeapon, VehicleWeaponInstance->GetItemGuid(), VehicleWeaponInstance->GetTrackerGuid(), false)->Cast<AFortWeaponRangedForVehicle>();

    if (!EquippedWeapon)
    {
        Log(L"No EquippedWeapon!");
        return;
    }

    // PlayerController->ClientEquipItem(VehicleWeaponInstance->GetItemGuid(), true);

    FName& MuzzleSocketName = SeatWeaponComponent->MuzzleSocketNames[SeatIdx];
    TScriptInterface<IFortVehicleInterface> VehicleUInterface = PlayerPawn->GetVehicleUInterface();

    EquippedWeapon->MuzzleSocketName = MuzzleSocketName;
    EquippedWeapon->MuzzleFalloffSocketName = MuzzleSocketName;
    EquippedWeapon->WeaponHandSocketNameOverride = MuzzleSocketName;
    EquippedWeapon->LeftHandWeaponHandSocketNameOverride = MuzzleSocketName;
    EquippedWeapon->bIsEquippingWeapon = true;
    EquippedWeapon->MountedWeaponInfo.bNeedsVehicleAttachment = true;
    EquippedWeapon->MountedWeaponInfo.bTargetSourceFromVehicleMuzzle = true;
    EquippedWeapon->MountedWeaponInfoRepped.HostVehicleCached = VehicleUInterface;
    EquippedWeapon->MountedWeaponInfoRepped.HostVehicleSeatIndexCached = SeatIdx;
    EquippedWeapon->OnRep_MountedWeaponInfoRepped();
    EquippedWeapon->OnHostVehicleSetup();
    EquippedWeapon->ServerSetMuzzleTraceNearWall(false);*/
}

void Vehicles::ServerOnExitVehicle(UObject* Context, FFrame& Stack)
{
    ETryExitVehicleBehavior ExitForceBehavior;
    bool bDestroyVehicleWhenForced;
    Stack.StepCompiledIn(&ExitForceBehavior);
    Stack.StepCompiledIn(&bDestroyVehicleWhenForced);
    Stack.IncrementCode();

    auto Pawn = (AFortPlayerPawn*)Context;
    auto PlayerController = (AFortPlayerController*)Pawn->Controller;
    auto Vehicle = Pawn->BP_GetVehicle();
    auto SeatIdx = Vehicle->FindSeatIndex(PlayerController->MyFortPawn);

    UFortVehicleSeatWeaponComponent* SeatWeaponComponent = (UFortVehicleSeatWeaponComponent*)Vehicle->GetComponentByClass(UFortVehicleSeatWeaponComponent::StaticClass());

    if (SeatWeaponComponent)
    {
        UFortWeaponItemDefinition* Weapon = nullptr;
        for (auto& WeaponDefinition : SeatWeaponComponent->WeaponSeatDefinitions)
        {
            if (WeaponDefinition.SeatIndex != SeatIdx)
                continue;

            Weapon = WeaponDefinition.VehicleWeapon;
            break;
        }

        auto ItemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([Weapon](FFortItemEntry& entry)
            { return entry.ItemDefinition == Weapon; });

        if (ItemEntry)
        {
            auto LastItem = Pawn->PreviousWeapon;
            PlayerController->ServerExecuteInventoryItem(LastItem->ItemEntryGuid);
            PlayerController->ClientEquipItem(LastItem->ItemEntryGuid, true);
            Inventory::Remove(PlayerController, ItemEntry->ItemGuid);
        }
    }

    callOG(Pawn, L"/Script/FortniteGame.FortPlayerPawn", ServerOnExitVehicle, ExitForceBehavior, bDestroyVehicleWhenForced);
}

void Vehicles::ServerUpdateTowhook(UObject* Context, FFrame& Stack)
{
    FVector_NetQuantizeNormal InNetTowhookAimDir;
    Stack.StepCompiledIn(&InNetTowhookAimDir);
    Stack.IncrementCode();

    auto Vehicle = (AFortOctopusVehicle*)Context;
    Vehicle->NetTowhookAimDir = InNetTowhookAimDir;
    Vehicle->OnRep_NetTowhookAimDir();
}

void Vehicles::OnPawnEnterVehicle(__int64 VehicleInterfaceWHY, AFortPlayerPawn* PlayerPawn, int32 SeatIdx) {
    auto Vehicle = (AFortAthenaVehicle*)(VehicleInterfaceWHY - 944);
    UFortVehicleSeatWeaponComponent* SeatWeaponComponent = (UFortVehicleSeatWeaponComponent*)Vehicle->GetComponentByClass(UFortVehicleSeatWeaponComponent::StaticClass());

    Log(L"%s\n", Vehicle->Name.ToWString().c_str());
    if (SeatWeaponComponent)
    {
        UFortWeaponItemDefinition* Weapon = nullptr;

        for (auto& WeaponDefinition : SeatWeaponComponent->WeaponSeatDefinitions)
        {
            if (WeaponDefinition.SeatIndex != SeatIdx)
                continue;

            Weapon = WeaponDefinition.VehicleWeapon;
            break;
        }

        Log(L"KYS");
        if (Weapon)
        {
            auto PlayerController = (AFortPlayerController*)PlayerPawn->Controller;

            Log(L"%s\n", Weapon->Name.ToWString().c_str());
            Inventory::GiveItem(PlayerController, Weapon, 1, Inventory::GetStats(Weapon)->ClipSize);

            auto ItemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry)
                { return entry.ItemDefinition == Weapon; }
            );

            PlayerController->ServerExecuteInventoryItem(ItemEntry->ItemGuid);
            PlayerController->ClientEquipItem(ItemEntry->ItemGuid, true);
        }
        Log(L"KYS");
    }

    OnPawnEnterVehicleOG(VehicleInterfaceWHY, PlayerPawn, SeatIdx);
}

void Vehicles::Hook()
{
    Utils::ExecHook(L"/Script/FortniteGame.FortPhysicsPawn.ServerMove", ServerMove);
    Utils::ExecHook(L"/Script/FortniteGame.FortPhysicsPawn.ServerMoveReliable", ServerMove);

    Utils::ExecHook(L"/Script/FortniteGame.FortPlayerPawn.ServerOnExitVehicle", ServerOnExitVehicle, ServerOnExitVehicleOG);
    Utils::ExecHook(L"/Script/FortniteGame.FortOctopusVehicle.ServerUpdateTowhook", ServerUpdateTowhook);
    //Utils::Hook(ImageBase + 0x143efc8, OnPawnEnterVehicle, OnPawnEnterVehicleOG);
    // Utils::ExecHook("/Script/FortniteGame.FortAthenaVehicle.OnPawnEnterVehicle", OnPawnEnterVehicle, OnPawnEnterVehicleOriginal);
    //Utils::ExecHook("/Script/FortniteGame.FortCharacterVehicle.OnPawnEnterVehicle", OnPawnEnterVehicle, OnPawnEnterVehicleOriginal);
    //Utils::ExecHookEvery<AFortCharacterVehicle>("OnPawnEnterVehicle", OnPawnEnterVehicle, OnPawnEnterVehicleOG);
}
