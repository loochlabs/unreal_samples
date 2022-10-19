
#include "MineSaveGame.h"
#include "MineshaftGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UMineSaveGame::Save(UMineshaftGameInstance* gi, const FSaveGameInfo& saveinfo)
{
	FString filename = GetFilename().ToString();
	int32 ver = GetVersion();
	UGameplayStatics::AsyncSaveGameToSlot(this, filename, saveinfo.SaveIndex,
		FAsyncSaveGameToSlotDelegate::CreateLambda([=](const FString&, const int32, bool success)
			{
				UE_LOG(MineshaftLog, Warning, TEXT("[SAVE] %s [v%d]: %s"),
					*filename, ver, success ? *FString("success") : *FString("fail"));
			})
	);
}

void UMineSaveGame::Delete(const FSaveGameInfo& saveinfo)
{
	FString filename = GetFilename().ToString();
	UGameplayStatics::DeleteGameInSlot(filename, saveinfo.SaveIndex);
}