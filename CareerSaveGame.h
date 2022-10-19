#pragma once

#include "CoreMinimal.h"
#include "MineSaveGame.h"
#include "CareerSaveGame.generated.h"


struct FSaveGameInfo;

USTRUCT(BlueprintType)
struct FCareerStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame) int32 HighestDifficultyCompleted = -1;
};


UCLASS()
class MINESHAFT3_API UCareerSaveGame : public UMineSaveGame
{
	GENERATED_BODY()

public:
	virtual void Save(UMineshaftGameInstance* cgi, const FSaveGameInfo& saveinfo);
	virtual void Load(UMineshaftGameInstance* cgi, const FSaveGameInfo& saveinfo);

	virtual FName GetFilename() override
	{
		return TEXT("career");
	};

	virtual int32 GetVersion() override
	{
		return VERSION;
	}

	static const uint32 VERSION = 1;
	
	void SetVersion() { Version = VERSION; };

	UPROPERTY() uint32 Version = 0;
	UPROPERTY() FCareerStats Stats;
	
};
