
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "RulesConfig.h"
#include "SessionManager.h"
#include "SettingsSaveGame.h"
#include "CareerSaveGame.h"
#include "SessionSaveGame.h"

#include "MineshaftGameInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(MineshaftLog, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLoadCompleteDelegate);


// Global Z Order for all widgets in the game. Higher order layers are rendered on the top. (defined by UE)
// Always call Gi->AddWidgetToViewport() to ensure one of these global UI layers is assigned. 
UENUM(BlueprintType)
enum class EUIZOrderLayer : uint8
{
	None = 0,
	Game,
	GameOverlay,
	Menu,
};


UCLASS()
class MINESHAFT3_API UMineshaftGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable) 
	void Setup(URulesConfig* cfg);

	void SetupSaveGames();
	
	UFUNCTION(BlueprintCallable) 
	void Save(ESaveGameType saveType);
	
	UFUNCTION(BlueprintCallable) 
	void SaveAll();
	
	void LoadSaveGame(ESaveGameType saveType);
	void LoadSaveGames();
	
	UFUNCTION(BlueprintCallable) 
	void LoadSession();
	
	void ValidateLoadComplete();
	
	UFUNCTION(BlueprintCallable) 
	void DeleteSave(ESaveGameType savetype);
	
	UFUNCTION(BlueprintCallable) 
	void AddWidgetToViewport(UUserWidget* widget, EUIZOrderLayer layer = EUIZOrderLayer::None);

	UPROPERTY(BlueprintReadOnly) 
	bool SetupComplete = false;
	
	UPROPERTY(BlueprintReadOnly) 
	FAppSettings AppSettings;
	
	UPROPERTY(BlueprintReadOnly) 
	FCareerStats CareerStats;

	UPROPERTY(BlueprintAssignable)
	FLoadCompleteDelegate LoadCompleteDelegate;
	
private:
	TMap<ESaveGameType, FSaveGameInfo> m_savegames;
};
