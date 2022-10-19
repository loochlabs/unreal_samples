
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MineEnums.h"
#include "UnitTemplateDataTable.h"

#include "GridUnitActor.generated.h"

class AGridCellActor;
struct FUnitTemplate;

typedef void(AGridUnitActor::*GridUnitActorPtr)();

UENUM()
enum class EUnitState : uint8
{
	Normal = 0,
	Warning,
	ToBeCleared
};


UCLASS()
class MINESHAFT3_API AGridUnitActor : public AActor
{
	GENERATED_BODY()

public:
	
	void Init(const FUnitTemplate& unitTemplate);
	virtual void Prepare(const FUnitTemplate& unitTemplate) {};
	virtual void Setup(const FUnitTemplate& unitTemplate);

	UFUNCTION(BlueprintImplementableEvent) 
	void FirstSetupCompleteBP();

	UFUNCTION(BlueprintImplementableEvent) 
	void SetupCompleteBP();

	virtual void PostCreate();
	virtual void Refresh();

	virtual void DoYield();

	UFUNCTION(BlueprintImplementableEvent) 
	void YieldTickBP();

	virtual void PostYield() {};

	UFUNCTION(BlueprintImplementableEvent) 
	void UpdateStatusBP();

	// Aggregate any potential yield amounts from all Grid Units
	virtual void SumYieldAmount(TMap<ECurrency, float>& total) {};
	
	UFUNCTION(BlueprintCallable) 
	void Activate();

	UFUNCTION(BlueprintImplementableEvent) 
	void ActivateBP();

	UFUNCTION(BlueprintCallable) 
	void Deactivate();

	UFUNCTION(BlueprintImplementableEvent) 
	void NeighborResetBP();

	virtual void Clear(int32 delayCount);

	UFUNCTION(BlueprintImplementableEvent) 
	void ClearBP(float delay);

	virtual void RefreshIntent();
	virtual void UpdateIntent() {};
	
	void RefreshBuffCoords();	
	void ClearBuffs();
	void AddBuff(const FName& key);
	virtual void ApplyBuffs() {};

	UFUNCTION(BlueprintImplementableEvent) 
	void AddBuffAlertBP();

	UFUNCTION(BlueprintImplementableEvent) 
	void ShowMacroTooltipBP();

	UFUNCTION(BlueprintImplementableEvent) 
	void ClearMacroTooltipBP();

	UFUNCTION(BlueprintCallable) 
	bool IsAttackSet();
	
	void ClearAttack();

	UFUNCTION(BlueprintImplementableEvent) 
	void ClearAttackBP();
	
	void SetAttack();

	UFUNCTION(BlueprintImplementableEvent) 
	void SetAttackBP(); 

	void Attack();

	UFUNCTION(BlueprintImplementableEvent) 
	void AttackBP(const TArray<FCoordinates>& coords);
	
	void MoveTo(int32 row, int32 col);
	
	UPROPERTY(BlueprintReadOnly) 
	FName UnitKey;

	UPROPERTY(SaveGame, BlueprintReadOnly) 
	int32 UnitID = -1;

	UPROPERTY(BlueprintReadOnly) 
	AGridCellActor* OwningGridCell = nullptr;

	UPROPERTY(SaveGame, BlueprintReadOnly) 
	EUnitState State = EUnitState::Normal;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly) 
	bool CanBeActivated = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly) 
	bool CanBeRecycled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly) 
	bool CanBeExploded = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly) 
	TMap<ECurrency, float> Cost;

	UPROPERTY(BlueprintReadOnly)
	float UnitExplosionDelay = 0.f;

	// buffs this unit has
	UPROPERTY(BlueprintReadOnly) 
	TArray<FName> Buffs; 

	FName BuffKey; // Buff that this unit can apply

	TArray<FBuffPattern> BuffPatterns;
	TSet<FCoordinates> BuffCoords;

	TArray<FBuffPattern> AttackPatterns;

	UPROPERTY(SaveGame, BlueprintReadOnly) 
	TArray<FBuffPattern> CurrentAttacks;

	UPROPERTY(BlueprintReadOnly) 
	int32 AttackPatternsToUse = 1;
};
