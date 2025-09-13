#pragma once
#include "pch.h"
#include "Utils.h"

class Vehicles
{
public:
	static void SpawnVehicles();
private:
	static void ServerMove(UObject*, FFrame&);
	static void EquipVehicleWeapon(APawn* Vehicle, AFortPlayerPawn* PlayerPawn, int32 SeatIdx);
	static void ServerUpdateTowhook(UObject* Context, FFrame& Stack);
	DefUHookOg(ServerOnExitVehicle);
	DefHookOg(void, OnPawnEnterVehicle, __int64, AFortPlayerPawn*, int32);

	InitHooks;
};