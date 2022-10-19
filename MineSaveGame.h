#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include "MineSaveGame.generated.h"

class UMineshaftGameInstance;
struct FSaveGameInfo;


struct FProfileSaveArchive : public FObjectAndNameAsStringProxyArchive
{
	FProfileSaveArchive(FArchive& InInnerArchive)
		: FObjectAndNameAsStringProxyArchive(InInnerArchive, true)
	{
		ArIsSaveGame = true;
	}
};


UCLASS()
class MINESHAFT3_API UMineSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	virtual void Save(UMineshaftGameInstance* gi, const FSaveGameInfo& saveinfo);
	virtual void Load(UMineshaftGameInstance* gi, const FSaveGameInfo& saveinfo) {};
	void Delete(const FSaveGameInfo& saveinfo);

	virtual FName GetFilename() { return TEXT(""); };
	virtual int32 GetVersion() { return 0; };
};


USTRUCT()
struct FSaveGameInfo
{
	GENERATED_BODY()
	
	TSubclassOf<UMineSaveGame> Classtype;
	int32 SaveIndex = 0;
	FName Filename;
	bool Loaded = false;
};