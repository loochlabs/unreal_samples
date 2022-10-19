#pragma once

#include "CoreMinimal.h"
#include "MineGridUnit.h"
#include "ConverterUnitActor.generated.h"


UCLASS()
class MINESHAFT3_API AConverterUnitActor : public AMineGridUnit
{
	GENERATED_BODY()

public:
	virtual void Setup(const FUnitTemplate& unitTemplate) override;
	virtual void GetTotalYieldByRef(TMap<ECurrency, float>& totals) override;

	UFUNCTION(BlueprintCallable) 
	void ToggleRowAsProducer(int32 rowIndex);

	UFUNCTION(BlueprintCallable) 
	bool IsRowProducing(int32 rowIndex);
};
