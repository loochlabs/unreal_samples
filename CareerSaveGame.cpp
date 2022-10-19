
#include "CareerSaveGame.h"
#include "MineshaftGameInstance.h"


void UCareerSaveGame::Save(UMineshaftGameInstance* gi, const FSaveGameInfo& saveinfo)
{
	Version = VERSION;
	Stats = gi->CareerStats;
	
	Super::Save(gi, saveinfo);
}

void UCareerSaveGame::Load(UMineshaftGameInstance* gi, const FSaveGameInfo& saveinfo)
{
	if(Version != VERSION)
	{
		UE_LOG(MineshaftLog, Warning, TEXT("[LOAD] CareerStats SaveGame [v%d]: Version mismatch detected. Found version=%d."), Version, VERSION);
		return;
	}

	gi->CareerStats = Stats;
}