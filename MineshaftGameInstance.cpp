
#include "MineshaftGameInstance.h"
#include "Kismet/GameplayStatics.h"



void UMineshaftGameInstance::Setup()
{
	SetupSaveGames();
	LoadSaveGames();
}

void UMineshaftGameInstance::SetupSaveGames()
{
	m_savegames.Empty();
	auto add_info = [&](ESaveGameType savetype, TSubclassOf<UMineSaveGame> classtype)
	{
		FSaveGameInfo info;
		info.Classtype = classtype;
		info.SaveIndex = 0;

		UMineSaveGame* savegame = Cast<UMineSaveGame>(classtype->GetDefaultObject());
		info.Filename = savegame->GetFilename();
		
		m_savegames.Add(savetype, info);
	};
	add_info(ESaveGameType::AppSettings, USettingsSaveGame::StaticClass());
	add_info(ESaveGameType::Career,		 UCareerSaveGame::StaticClass());
	add_info(ESaveGameType::Session,	 USessionSaveGame::StaticClass());
}

void UMineshaftGameInstance::Save(ESaveGameType savetype)
{
	TSubclassOf<UMineSaveGame> classtype = m_savegames[savetype].Classtype;
	if (UMineSaveGame* savegame = Cast<UMineSaveGame>(UGameplayStatics::CreateSaveGameObject(classtype)))
		savegame->Save(this, m_savegames[savetype]);
}

void UMineshaftGameInstance::SaveAll()
{
	for(auto& savegame : m_savegames)
		Save(savegame.Key);
}

void UMineshaftGameInstance::LoadSaveGame(ESaveGameType savetype)
{
	auto& saveinfo = m_savegames[savetype];
	saveinfo.Loaded = false;
	FString filename = saveinfo.Filename.ToString();
	int32 index = saveinfo.SaveIndex;

	UGameplayStatics::AsyncLoadGameFromSlot(
		filename, index,
		FAsyncLoadGameFromSlotDelegate::CreateLambda([&, savetype](const FString& filename, const int32, USaveGame* savegame)
		{
			UE_LOG(MineshaftLog, Warning, TEXT("[LOAD] filename=%s [%s]"), *filename, savegame ? *FString("success") : *FString("failed"));
			
			if(UMineSaveGame* mineSavegame = Cast<UMineSaveGame>(savegame))
				mineSavegame->Load(this, m_savegames[savetype]);
			
			m_savegames[savetype].Loaded = true;
			ValidateLoadComplete();
		}));
}

void UMineshaftGameInstance::LoadSaveGames()
{
	for(auto& savegame : m_savegames)
		savegame.Value.Loaded = false;
	
	for(auto& savegame : m_savegames)
		LoadSaveGame(savegame.Key);
}

void UMineshaftGameInstance::LoadSession()
{
	LoadSaveGame(ESaveGameType::Session);
}

void UMineshaftGameInstance::DeleteSave(ESaveGameType savetype)
{
	TSubclassOf<UMineSaveGame> classtype = m_savegames[savetype].Classtype;
	if (UMineSaveGame* savegame = Cast<UMineSaveGame>(UGameplayStatics::CreateSaveGameObject(classtype)))
		savegame->Delete(m_savegames[savetype]);
}

void UMineshaftGameInstance::ValidateLoadComplete()
{
	bool bComplete = true;
	for(auto& savegame : m_savegames)
		bComplete &= savegame.Value.Loaded;

	if(bComplete)
		LoadCompleteDelegate.Broadcast();
}
