#include "pch.h"
#include "GameMode.h"
#include "Abilities.h"
#include "Inventory.h"
#include "Looting.h"
#include "Offsets.h"
#include "Misc.h"
#include "Replication.h"
#include "Options.h"
#include "Vehicles.h"
#include "AI.h"
#include "Lategame.h"
//#include "Player.h"

void SetPlaylist(AFortGameModeAthena* GameMode, UFortPlaylistAthena* Playlist) {
	auto GameState = (AFortGameStateAthena*)GameMode->GameState;

	GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
	GameState->CurrentPlaylistInfo.OverridePlaylist = Playlist;
	GameState->CurrentPlaylistInfo.BasePlaylist->GarbageCollectionFrequency = 9999999999999999.f;
	GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
	GameState->CurrentPlaylistInfo.MarkArrayDirty();

	GameState->CurrentPlaylistId = GameMode->CurrentPlaylistId = Playlist->PlaylistId;
	GameMode->CurrentPlaylistName = Playlist->PlaylistName;
	GameState->OnRep_CurrentPlaylistInfo();
	GameState->OnRep_CurrentPlaylistId();

	GameMode->GameSession->MaxPlayers = Playlist->MaxPlayers;

	GameState->AirCraftBehavior = Playlist->AirCraftBehavior;
	GameState->WorldLevel = Playlist->LootLevel;
	GameState->CachedSafeZoneStartUp = Playlist->SafeZoneStartUp;

	GameMode->bAlwaysDBNO = Playlist->MaxSquadSize > 1;
	GameMode->bDBNOEnabled = Playlist->MaxSquadSize > 1;
	/*GameMode->AISettings = Playlist->AISettings;
	if (GameMode->AISettings)
		GameMode->AISettings->AIServices[1] = UAthenaAIServicePlayerBots::StaticClass();*/
}

bool bReady = false;
bool GameMode::ReadyToStartMatch(UObject* Context, FFrame& Stack, bool* Ret) {
	static auto Playlist = Utils::FindObject<UFortPlaylistAthena>(L"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
	Stack.IncrementCode();
	auto GameMode = Context->Cast<AFortGameModeAthena>();
	if (!GameMode) 
		return *Ret = callOGWithRet(((AGameMode*)Context), L"/Script/Engine.GameMode", ReadyToStartMatch);
	auto GameState = ((AFortGameStateAthena*)GameMode->GameState);

	if (GameMode->CurrentPlaylistId == -1) {
		GameMode->WarmupRequiredPlayerCount = 1;
		Misc::Listen();
		SetPlaylist(GameMode, Playlist);

		GameState->bIsUsingDownloadOnDemand = false;
		for (auto& Level : Playlist->AdditionalLevels)
		{
			bool Success = false;
			std::cout << "Level: " << Level.Get()->Name.ToString() << std::endl;
			ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), Level, FVector(), FRotator(), &Success, FString(), nullptr, false);
			FAdditionalLevelStreamed level{};
			level.bIsServerOnly = false;
			level.LevelName = Level.ObjectID.AssetPath.AssetName;
			if (Success) GameState->AdditionalPlaylistLevelsStreamed.Add(level);
		}
		for (auto& Level : Playlist->AdditionalLevelsServerOnly)
		{
			bool Success = false;
			std::cout << "Level: " << Level.Get()->Name.ToString() << std::endl;
			ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), Level, FVector(), FRotator(), &Success, FString(), nullptr, false);
			FAdditionalLevelStreamed level{};
			level.bIsServerOnly = true;
			level.LevelName = Level.ObjectID.AssetPath.AssetName;
			if (Success) GameState->AdditionalPlaylistLevelsStreamed.Add(level);
		}
		GameState->OnRep_AdditionalPlaylistLevelsStreamed();


		GameMode->AIDirector = Utils::SpawnActor<AAthenaAIDirector>({});
		GameMode->AIDirector->Activate();

		GameMode->AIGoalManager = Utils::SpawnActor<AFortAIGoalManager>({});


		return *Ret = false;
	}

	if (!GameMode->bWorldIsReady) 
	{
		auto Starts = Utils::GetAll<AFortPlayerStartWarmup>();
		auto StartsNum = Starts.Num();
		Starts.Free();
		if (StartsNum == 0 || !GameState->MapInfo)
			return *Ret = false;

		void (*LoadBuiltInGameFeaturePlugins)(UFortGameFeaturePluginManager*, bool) = decltype(LoadBuiltInGameFeaturePlugins)(Sarah::Offsets::ImageBase + 0x1E8982C);
		UFortGameFeaturePluginManager* PluginManager = nullptr;
		for (int i = 0; i < UObject::GObjects->Num(); i++)
		{
			auto Object = UObject::GObjects->GetByIndex(i);
			if (Object && !Object->IsDefaultObject() && Object->Class == UFortGameFeaturePluginManager::StaticClass())
			{
				PluginManager = (UFortGameFeaturePluginManager*)Object;
				break;
			}
		}

		LoadBuiltInGameFeaturePlugins(PluginManager, true);

		GameMode->bDisableGCOnServerDuringMatch = true;
		GameMode->bPlaylistHotfixChangedGCDisabling = true;

		GameMode->DefaultPawnClass = Utils::FindObject<UClass>(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
		//Misc::Listen();

		GameState->DefaultParachuteDeployTraceForGroundDistance = 10000;

		AbilitySets.Add(Utils::FindObject<UFortAbilitySet>(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer"));

		auto AddToTierData = [&](UDataTable* Table, UEAllocatedVector<FFortLootTierData*>& TempArr) {
			Table->AddToRoot();
			if (auto CompositeTable = Table->Cast<UCompositeDataTable>())
				for (auto& ParentTable : CompositeTable->ParentTables)
					for (auto& [Key, Val] : (TMap<FName, FFortLootTierData*>) ParentTable->RowMap) {
						TempArr.push_back(Val);
					}

			for (auto& [Key, Val] : (TMap<FName, FFortLootTierData*>) Table->RowMap) {
				TempArr.push_back(Val);
			}
			};

		auto AddToPackages = [&](UDataTable* Table, UEAllocatedVector<FFortLootPackageData*>& TempArr) {
			Table->AddToRoot();
			if (auto CompositeTable = Table->Cast<UCompositeDataTable>())
				for (auto& ParentTable : CompositeTable->ParentTables)
					for (auto& [Key, Val] : (TMap<FName, FFortLootPackageData*>) ParentTable->RowMap) {
						TempArr.push_back(Val);
					}

			for (auto& [Key, Val] : (TMap<FName, FFortLootPackageData*>) Table->RowMap) {
				TempArr.push_back(Val);
			}
			};


		UEAllocatedVector<FFortLootTierData*> LootTierDataTempArr;
		auto LootTierData = Playlist->LootTierData.Get();
		if (!LootTierData)
			LootTierData = Utils::FindObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client");
		if (LootTierData)
			AddToTierData(LootTierData, LootTierDataTempArr);
		for (auto& Val : LootTierDataTempArr)
		{
			Looting::TierDataAllGroups.Add(Val);
		}

		UEAllocatedVector<FFortLootPackageData*> LootPackageTempArr;
		auto LootPackages = Playlist->LootPackages.Get();
		if (!LootPackages) LootPackages = Utils::FindObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client");
		if (LootPackages)
			AddToPackages(LootPackages, LootPackageTempArr);
		for (auto& Val : LootPackageTempArr)
		{
			Looting::LPGroupsAll.Add(Val);
		}

		for (int i = 0; i < UObject::GObjects->Num(); i++)
		{
			auto Object = UObject::GObjects->GetByIndex(i);

			if (!Object || !Object->Class || Object->IsDefaultObject())
				continue;

			if (auto GameFeatureData = Object->Cast<UFortGameFeatureData>())
			{
				auto LootTableData = GameFeatureData->DefaultLootTableData;
				auto LTDFeatureData = LootTableData.LootTierData.Get();
				auto AbilitySet = GameFeatureData->PlayerAbilitySet.Get();
				auto LootPackageData = LootTableData.LootPackageData.Get();
				if (AbilitySet) {
					Log(L"Ability set: %s\n", UKismetSystemLibrary::GetPathName(AbilitySet).ToWString().c_str());
					AbilitySet->AddToRoot();
					AbilitySets.Add(AbilitySet);
				}
				if (LTDFeatureData) {
					UEAllocatedVector<FFortLootTierData*> LTDTempData;

					AddToTierData(LTDFeatureData, LTDTempData);

					for (auto& Tag : Playlist->GameplayTagContainer.GameplayTags)
						for (auto& Override : GameFeatureData->PlaylistOverrideLootTableData)
							if (Tag.TagName == Override.First.TagName)
								AddToTierData(Override.Second.LootTierData.Get(), LTDTempData);

					for (auto& Val : LTDTempData)
					{
						Looting::TierDataAllGroups.Add(Val);
					}
				}
				if (LootPackageData) {
					UEAllocatedVector<FFortLootPackageData*> LPTempData;


					AddToPackages(LootPackageData, LPTempData);

					for (auto& Tag : Playlist->GameplayTagContainer.GameplayTags)
						for (auto& Override : GameFeatureData->PlaylistOverrideLootTableData)
							if (Tag.TagName == Override.First.TagName)
								AddToPackages(Override.Second.LootPackageData.Get(), LPTempData);

					for (auto& Val : LPTempData)
					{
						Looting::LPGroupsAll.Add(Val);
					}
				}
			}
		}

		Looting::SpawnFloorLootForContainer(Utils::FindObject<UBlueprintGeneratedClass>(L"/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C"));
		Looting::SpawnFloorLootForContainer(Utils::FindObject<UBlueprintGeneratedClass>(L"/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C"));

		Vehicles::SpawnVehicles();
		//GameMode->DefaultPawnClass = Utils::FindObject<UClass>(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");

		UCurveTable* AthenaGameDataTable = GameState->AthenaGameDataTable; // this is js playlist gamedata or default gamedata if playlist doesn't have one

		if (AthenaGameDataTable)
		{
			static FName DefaultSafeZoneDamageName = FName(L"Default.SafeZone.Damage");

			for (const auto& [RowName, RowPtr] : ((UDataTable*)AthenaGameDataTable)->RowMap) // same offset
			{
				if (RowName != DefaultSafeZoneDamageName)
					continue;

				FSimpleCurve* Row = (FSimpleCurve*)RowPtr;

				if (!Row)
					continue;

				for (auto& Key : Row->Keys)
				{
					FSimpleCurveKey* KeyPtr = &Key;

					if (KeyPtr->Time == 0.f)
					{
						KeyPtr->Value = 0.f;
					}
				}

				Row->Keys.Add(FSimpleCurveKey(1.f, 0.01f), 1);
			}
		}

		/*auto DataLayers = UWorld::GetWorld()->PersistentLevel->WorldDataLayers;
		auto SetDataLayerRuntimeState = (void (*)(AWorldDataLayers*, UDataLayerInstance*, EDataLayerRuntimeState, bool)) (Sarah::Offsets::ImageBase + 0x81ED4C0);
		auto LobbyName = FName(L"DataLayer_8DD02A1E4DB7293E83D7C297C548D1F5");
		for (auto& Instance_Uncasted : DataLayers->DataLayerInstances)
		{
			auto Instance = (UDataLayerInstanceWithAsset*)Instance_Uncasted;
			if (Instance->GetName() == "DataLayer_B69182544DFBA9C542022A8018B884F0")
			{
				SetDataLayerRuntimeState(DataLayers, Instance, EDataLayerRuntimeState::Activated, true);
			}
			printf("DLInstance: %s\n", Instance->GetName().c_str());
			printf("DLInstance_Name: %s\n", Instance->DataLayerAsset->GetName().c_str());
		}*/

		SetConsoleTitleA("Sarah 24.20: Ready");
		GameMode->bWorldIsReady = true;
		std::string playlist = "Playlist_ShowdownAlt_Solo";

		return *Ret = false;
	}

	auto VolumeManager = GameState->VolumeManager;
	TArray<FPlaylistStreamedLevelData>& AdditonalPlaylistLevels = *(TArray<FPlaylistStreamedLevelData>*) (__int64(GameState) + 0x410);

	bool bAllLevelsFinishedStreaming = true;
	for (auto& Something : AdditonalPlaylistLevels)
	{
		if (!Something.bIsFinishedStreaming)
		{
			bAllLevelsFinishedStreaming = false;
			break;
		}
	}

	bool (*AreAllAdditionalPlaylistLevelsVisible)(AFortGameStateAthena*) = decltype(AreAllAdditionalPlaylistLevelsVisible)(Sarah::Offsets::ImageBase + 0xA085EFC);
	int32(*CountReadyPlayers)(AFortGameModeAthena*, bool) = decltype(CountReadyPlayers)(Sarah::Offsets::ImageBase + 0x990F3F4);

	return *Ret = GameMode->bWorldIsReady && GameState->bPlaylistDataIsLoaded && GameMode->MatchState == FName(L"WaitingToStart") && bAllLevelsFinishedStreaming && (!VolumeManager || !VolumeManager->bInSpawningStartup) && AreAllAdditionalPlaylistLevelsVisible(GameState) && CountReadyPlayers(GameMode, false) >= GameMode->WarmupRequiredPlayerCount;

	//return *Ret = GameMode->AlivePlayers.Num() > 0;
}

APawn* GameMode::SpawnDefaultPawnFor(UObject* Context, FFrame& Stack, APawn** Ret) {
	AController* NewPlayer;
	AActor* StartSpot;
	Stack.StepCompiledIn(&NewPlayer);
	Stack.StepCompiledIn(&StartSpot);
	Stack.IncrementCode();
	auto GameMode = (AFortGameModeAthena*)Context;
	auto Transform = StartSpot->GetTransform();
	auto Pawn = GameMode->SpawnDefaultPawnAtTransform(NewPlayer, Transform);


	static bool IsFirstPlayer = false;

	if (!IsFirstPlayer)
	{
		for (ABuildingProp_ConversationCompatible* ConversationProp : Utils::GetAll<ABuildingProp_ConversationCompatible>())
		{
			if (!ConversationProp)
				continue;

			//Log(ConversationProp->GetFullWName().c_str());

			UFortNonPlayerConversationParticipantComponent* ParticipantComponent =
				(UFortNonPlayerConversationParticipantComponent*)
				ConversationProp->GetComponentByClass(UFortNonPlayerConversationParticipantComponent::StaticClass());

			if (!ParticipantComponent)
				continue;

			if (ParticipantComponent->ServiceProviderIDTag.TagName == FName(0))
			{
				ParticipantComponent->ServiceProviderIDTag.TagName = ParticipantComponent->ConversationEntryTag.TagName;
			}

			if (ParticipantComponent->GiftItemInventory)
			{
				for (const auto& [RowName, RowPtr] : ParticipantComponent->GiftItemInventory->RowMap)
				{
					FNPCGiftItemInventoryRow* Row = (FNPCGiftItemInventoryRow*)RowPtr;

					if (!RowName.IsValid() || !Row)
						continue;

					if (Row->NPC.TagName == FName(L"None") || Row->NPC.TagName == ParticipantComponent->ServiceProviderIDTag.TagName)
					{
						ParticipantComponent->SupportedGiftItems.Add(*Row);
					}
				}
			}

			if (ParticipantComponent->Gifts)
			{
				for (const auto& [RowName, RowPtr] : ParticipantComponent->Gifts->RowMap)
				{
					FNPCConversationGiftRow* Row = (FNPCConversationGiftRow*)RowPtr;

					if (!RowName.IsValid() || !Row)
						continue;

					if (Row->NPC.TagName == FName(L"None") || Row->NPC.TagName == ParticipantComponent->ServiceProviderIDTag.TagName)
					{
						ParticipantComponent->SupportedGiftTypes.Add(*Row);
					}
				}
			}

			if (ParticipantComponent->SalesInventory)
			{
				for (const auto& [RowName, RowPtr] : ParticipantComponent->SalesInventory->RowMap)
				{
					FNPCSaleInventoryRow* Row = (FNPCSaleInventoryRow*)RowPtr;

					if (!RowName.IsValid() || !Row)
						continue;

					if (Row->NPC.TagName == FName(L"None") || Row->NPC.TagName == ParticipantComponent->ServiceProviderIDTag.TagName)
					{
						ParticipantComponent->SupportedSales.Add(*Row);
					}
				}
			}

			if (ParticipantComponent->Services)
			{
				for (const auto& [RowName, RowPtr] : ParticipantComponent->Services->RowMap)
				{
					FNPCDynamicServiceRow* Row = (FNPCDynamicServiceRow*)RowPtr;

					if (!RowName.IsValid() || !Row)
						continue;

					if (Row->NPC.TagName == FName(L"None") || Row->NPC.TagName == ParticipantComponent->ServiceProviderIDTag.TagName)
					{
						ParticipantComponent->SupportedServices.GameplayTags.Add(Row->ServiceTag);
					}
				}
			}
		}
		if (GameMode->AISettings)
			AI::SpawnAIs();

		IsFirstPlayer = true;
	}

	auto PlayerController = NewPlayer->Cast<AFortPlayerController>();
	if (!PlayerController) return *Ret = Pawn;

	auto Num = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Num();
	if (Num != 0) 
	{
		PlayerController->WorldInventory->Inventory.ReplicatedEntries.ResetNum();
		PlayerController->WorldInventory->Inventory.ItemInstances.ResetNum();
		Inventory::TriggerInventoryUpdate(PlayerController, nullptr);
	}
	Inventory::GiveItem(PlayerController, PlayerController->CosmeticLoadoutPC.Pickaxe->WeaponDefinition);
	for (auto& StartingItem : ((AFortGameModeAthena*)GameMode)->StartingItems)
	{
		if (StartingItem.Count && !StartingItem.Item->IsA<UFortSmartBuildingItemDefinition>())
		{
			Inventory::GiveItem(PlayerController, StartingItem.Item, StartingItem.Count);
		}
	}

	if (Num == 0)
	{
		auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;

		for (auto& AbilitySet : AbilitySets)
			Abilities::GiveAbilitySet(PlayerState->AbilitySystemComponent, AbilitySet);

		((void (*)(APlayerState*, APawn*)) Sarah::Offsets::ApplyCharacterCustomization)(PlayerController->PlayerState, Pawn);

		auto AthenaController = (AFortPlayerControllerAthena*)PlayerController;
		PlayerState->SeasonLevelUIDisplay = AthenaController->XPComponent->CurrentLevel;
		PlayerState->OnRep_SeasonLevelUIDisplay();
		AthenaController->XPComponent->bRegisteredWithQuestManager = true;
		AthenaController->XPComponent->OnRep_bRegisteredWithQuestManager();

		AthenaController->GetQuestManager(ESubGame::Athena)->InitializeQuestAbilities(Pawn);
	}
	else
	{
		if (bLateGame)
		{
			auto Shotgun = Lategame::GetShotguns();
			auto AssaultRifle = Lategame::GetAssaultRifles();
			auto Util = Lategame::GetUtilities();
			auto Heal = Lategame::GetHeals();
			auto HealSlot2 = Lategame::GetHeals();

			int ShotgunClipSize = Inventory::GetStats((UFortWeaponItemDefinition*)Shotgun.Item)->ClipSize;
			int AssaultRifleClipSize = Inventory::GetStats((UFortWeaponItemDefinition*)AssaultRifle.Item)->ClipSize;
			int UtilClipSize = Util.Item->IsA<UFortWeaponItemDefinition>() ? Inventory::GetStats((UFortWeaponItemDefinition*)Util.Item)->ClipSize : 3;
			int HealClipSize = Heal.Item->IsA<UFortWeaponItemDefinition>() ? Inventory::GetStats((UFortWeaponItemDefinition*)Heal.Item)->ClipSize : 3;
			int HealSlot2ClipSize = HealSlot2.Item->IsA<UFortWeaponItemDefinition>() ? Inventory::GetStats((UFortWeaponItemDefinition*)HealSlot2.Item)->ClipSize : 3;

			Inventory::GiveItem(PlayerController, Lategame::GetResource(EFortResourceType::Wood), 500);
			Inventory::GiveItem(PlayerController, Lategame::GetResource(EFortResourceType::Stone), 500);
			Inventory::GiveItem(PlayerController, Lategame::GetResource(EFortResourceType::Metal), 500);

			Inventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Assault), 250);
			Inventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Shotgun), 50);
			Inventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Submachine), 400);
			Inventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Rocket), 6);
			Inventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Sniper), 20);

			Inventory::GiveItem(PlayerController, AssaultRifle.Item, 1, AssaultRifleClipSize, true);
			Inventory::GiveItem(PlayerController, Shotgun.Item, 1, ShotgunClipSize, true);
			Inventory::GiveItem(PlayerController, Util.Item, Util.Count, UtilClipSize, true);
			Inventory::GiveItem(PlayerController, Heal.Item, 3, HealClipSize, true);
			Inventory::GiveItem(PlayerController, HealSlot2.Item, 3, HealSlot2ClipSize, true);
		}
	}

	//MessageBoxA(nullptr, "tranny", "tranny", MB_OK);
	Log(L"%p\n", Pawn);
	return *Ret = Pawn;
}

EFortTeam GameMode::PickTeam(AFortGameModeAthena* GameMode, uint8_t PreferredTeam, AFortPlayerControllerAthena* Controller) {
	uint8_t ret = CurrentTeam;

	if (++PlayersOnCurTeam >= ((AFortGameStateAthena*)GameMode->GameState)->CurrentPlaylistInfo.BasePlaylist->MaxSquadSize) 
	{
		CurrentTeam++;
		PlayersOnCurTeam = 0;
	}

	return EFortTeam(ret);
}

void GameMode::HandleStartingNewPlayer(UObject* Context, FFrame& Stack) {
	AFortPlayerControllerAthena* NewPlayer;
	Stack.StepCompiledIn(&NewPlayer);
	Stack.IncrementCode();
	auto GameMode = (AFortGameModeAthena*)Context;
	auto GameState = (AFortGameStateAthena*)GameMode->GameState;
	AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)NewPlayer->PlayerState;

	PlayerState->SquadId = PlayerState->TeamIndex - 3;
	PlayerState->OnRep_SquadId();

	FGameMemberInfo Member;
	Member.MostRecentArrayReplicationKey = -1;
	Member.ReplicationID = -1;
	Member.ReplicationKey = -1;
	Member.TeamIndex = PlayerState->TeamIndex;
	Member.SquadId = PlayerState->SquadId;
	Member.MemberUniqueId = PlayerState->UniqueId;

	GameState->GameMemberInfoArray.Members.Add(Member);
	GameState->GameMemberInfoArray.MarkItemDirty(Member);


	if (!NewPlayer->MatchReport)
	{
		NewPlayer->MatchReport = reinterpret_cast<UAthenaPlayerMatchReport*>(UGameplayStatics::SpawnObject(UAthenaPlayerMatchReport::StaticClass(), NewPlayer));
	}

	return callOG(GameMode, L"/Script/Engine.GameModeBase", HandleStartingNewPlayer, NewPlayer);
}

void GameMode::OnAircraftExitedDropZone(UObject* Context, FFrame& Stack)
{
	AFortAthenaAircraft* Aircraft;
	Stack.StepCompiledIn(&Aircraft);
	Stack.IncrementCode();
	auto GameMode = (AFortGameModeAthena*)Context;
	auto GameState = (AFortGameStateAthena*)GameMode->GameState;
	for (auto& Player : GameMode->AlivePlayers)
	{
		if (Player->IsInAircraft())
		{
			Player->GetAircraftComponent()->ServerAttemptAircraftJump({});
		}
	}

	if (bLateGame)
	{
		GameState->GamePhase = EAthenaGamePhase::SafeZones;
		GameState->GamePhaseStep = EAthenaGamePhaseStep::StormHolding;
		GameState->OnRep_GamePhase(EAthenaGamePhase::Aircraft);
	}


	callOG(GameMode, L"/Script/FortniteGame.FortGameModeAthena", OnAircraftExitedDropZone, Aircraft);
}

void GameMode::Hook()
{
	Utils::ExecHook(L"/Script/Engine.GameMode.ReadyToStartMatch", ReadyToStartMatch, ReadyToStartMatchOG);
	Utils::ExecHook(L"/Script/Engine.GameModeBase.SpawnDefaultPawnFor", SpawnDefaultPawnFor);
	//Utils::Hook(Sarah::Offsets::PickTeam, PickTeam, PickTeamOG);
	Utils::ExecHook(L"/Script/Engine.GameModeBase.HandleStartingNewPlayer", HandleStartingNewPlayer, HandleStartingNewPlayerOG);
	Utils::ExecHook(L"/Script/FortniteGame.FortGameModeAthena.OnAircraftExitedDropZone", OnAircraftExitedDropZone, OnAircraftExitedDropZoneOG);
}
