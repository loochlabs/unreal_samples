
#pragma once

#include "CoreMinimal.h"

#include "GridUnitActor.h"
#include "MineEnums.h"
#include "MineshaftCell.h"

#include "MineGridUnit.generated.h"


USTRUCT(BlueprintType)
struct FMineRow
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite) bool Unlocked = false;
	UPROPERTY(SaveGame, BlueprintReadWrite) bool Revealed = false;
	UPROPERTY(SaveGame, BlueprintReadWrite) float UnlockCost = 0.f;
	UPROPERTY(SaveGame, BlueprintReadWrite) ECurrency UnlockCurrency = ECurrency::Stone;
	UPROPERTY(BlueprintReadWrite) TArray<UMineshaftCell*> Cells;
};


UCLASS()
class MINESHAFT3_API AMineGridUnit : public AGridUnitActor
{
	GENERATED_BODY()

public:
	virtual void Setup(const FUnitTemplate& unitTemplate) override;
	virtual void DoYield() override;
	virtual void Refresh() override;

	UFUNCTION(BlueprintCallable) 
	UMineshaftCell* GetCell(int32 row, int32 col);

	UFUNCTION(BlueprintCallable) 
	void ClearCellWalls(int32 row, int32 col);

	void SwapCellProperties(UMineshaftCell* a, UMineshaftCell* b, UMineshaftCell* buffer);
	
	UFUNCTION(BlueprintCallable) 
	ECellOrientation GetOrientation(UMineshaftCell* from, UMineshaftCell* to);
	
	void CalculateDistanceToExit(UMineshaftCell* c, TSet<UMineshaftCell*>& vst, int32 distance);
	void SetShortestPathToExit(UMineshaftCell* cell, UMineshaftCell* prev, ECurrency currency = ECurrency::Stone);

	ECellOrientation RotateCellCW(UMineshaftCell* cell, bool calcYield = true);
	ECellOrientation RotateCellCCW(UMineshaftCell* cell, bool calcYield = true);
	UFUNCTION(BlueprintCallable) ECellOrientation RotateCellCW(int32 row, int32 col);
	UFUNCTION(BlueprintCallable) ECellOrientation RotateCellCCW(int32 row, int32 col);
	
	UFUNCTION(BlueprintCallable) int32 GetUnlockLevel();
	UFUNCTION(BlueprintCallable) bool CanUnlock();
	UFUNCTION(BlueprintCallable) bool UnlockMineRow();
	UFUNCTION(BlueprintCallable) void RevealUnlockedMineRow(int32 row);
	UFUNCTION(BlueprintCallable) void RevealUnlockedMineRows();
	UFUNCTION(BlueprintCallable) bool IsFirstReveal();
	UFUNCTION(BlueprintCallable) bool IsFullyUnlocked();
	UFUNCTION(BlueprintCallable) void CalculateYield();

	virtual void SumYieldAmount(TMap<ECurrency, float>& totals) override;

	UFUNCTION(BlueprintCallable) virtual void GetTotalYieldByRef(TMap<ECurrency, float>& total);
	UFUNCTION(BlueprintCallable) TMap<ECurrency, float> GetTotalYield();
	UFUNCTION(BlueprintCallable) bool HasYield();
	UFUNCTION(BlueprintCallable) float GetActiveProducerPercent();

	virtual void ApplyBuffs() override;

	static const TMap<FName, void(AMineGridUnit::*)(TMap<ECurrency, float>&)> S_Buffs;
	
	void Buff_StoneOutput(TMap<ECurrency, float>& total);
	void Buff_CopperOutput(TMap<ECurrency, float>& total);
	void Buff_IronOutput(TMap<ECurrency, float>& total);
	void Buff_GoldOutput(TMap<ECurrency, float>& total);
	void Buff_GoldMultiplier(TMap<ECurrency, float>& total);

	UPROPERTY(SaveGame, EditAnywhere) 
	float BankInitialMin = 200.f;

	UPROPERTY(SaveGame, EditAnywhere) 
	float BankInitialMax = 300.f;

	UPROPERTY(SaveGame, BlueprintReadWrite) 
	float YieldAmount = 50.f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite) 
	float YieldBase = 5.0f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite) 
	float YieldMultiplierPadding = 1.f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite) 
	int32 Columns = 8;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite) 
	int32 MaxUnlockRows = 2;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite) 
	int32 InitialRowUnlocks = 1;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite) 
	ECurrency UnlockCurrency = ECurrency::Stone;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite) 
	float UnlockBaseCost = 20.f;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite) 
	TMap<ECurrency, float> ProducerChances;

	UPROPERTY(SaveGame, EditAnywhere) 
	bool PickupTracksOnRowUnlock = true;

	UPROPERTY(SaveGame, EditAnywhere) 
	bool RotateTracksOnSetup = true;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite) 
	bool EnableTransactions = false;

	UPROPERTY(SaveGame, BlueprintReadWrite) 
	int32 TotalProducers = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite) 
	int32 UnlockedProducers = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite) 
	TArray<FMineRow> Rows;
	
	UPROPERTY(BlueprintReadWrite) 
	TSet<UMineshaftCell*> ActiveProducers; 
};