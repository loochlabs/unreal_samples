
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "MineEnums.h"

#include "MineshaftCell.generated.h"

USTRUCT(BlueprintType)
struct FProductionChain
{
	GENERATED_BODY();

	UPROPERTY(BlueprintReadOnly) 
	ECellOrientation Origin = ECellOrientation::None;

	UPROPERTY(BlueprintReadOnly) 
	ECellOrientation Exit = ECellOrientation::None;

	UPROPERTY(BlueprintReadOnly) 
	ECurrency Currency = ECurrency::Stone;
};


UCLASS()
class MINESHAFT3_API UMineshaftCell : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly) int32 Row = 0;
	UPROPERTY(BlueprintReadOnly) int32 Col = 0;
	
	UPROPERTY(BlueprintReadOnly) ECurrency Currency = ECurrency::Stone;
	UPROPERTY(BlueprintReadOnly) float Bank = 0.f;
	UPROPERTY(BlueprintReadOnly) float BankMax = 0.f;
	
	UPROPERTY(BlueprintReadOnly) EMineCellTrack TrackType = EMineCellTrack::None;
	UPROPERTY(BlueprintReadOnly) int32 WallVariant = 0; //0-15, initial assignment 
	UPROPERTY(BlueprintReadOnly) int32 WallOrientation = 0; //0-15
	UPROPERTY(BlueprintReadOnly) ECellOrientation Orientation = ECellOrientation::North;

	UPROPERTY(BlueprintReadOnly) bool Repo = false;
	UPROPERTY(BlueprintReadOnly) bool Producer = false;
	
	UPROPERTY(BlueprintReadOnly) bool InProductiveChain = false; 
	UPROPERTY(BlueprintReadOnly) float CurrentYield = 0.f;
	UPROPERTY(BlueprintReadOnly) TArray<FProductionChain> ProductionChains;  

	TSet<UMineshaftCell*> Links;
	TMap<ECellOrientation, UMineshaftCell*> Neighbors;
	
	TMap<ECellOrientation, UMineshaftCell*> YieldLinks; 
	int32 DistanceToExit = 0;

	inline static TMap<int32, EMineCellTrack> S_TrackTypes =
	{
		{ 0, 	EMineCellTrack::None },
		{ 1, 	EMineCellTrack::None },
		{ 2, 	EMineCellTrack::None },
		{ 3, 	EMineCellTrack::Curve },
		{ 4, 	EMineCellTrack::None },
		{ 5, 	EMineCellTrack::Straight },
		{ 6, 	EMineCellTrack::Curve },
		{ 7, 	EMineCellTrack::T },
		{ 8, 	EMineCellTrack::None },
		{ 9, 	EMineCellTrack::Curve },
		{ 10, 	EMineCellTrack::Straight },
		{ 11, 	EMineCellTrack::T },
		{ 12, 	EMineCellTrack::Curve },
		{ 13, 	EMineCellTrack::T },
		{ 14, 	EMineCellTrack::T },
		{ 15, 	EMineCellTrack::Cross },
	};

	inline static TMap<EMineCellTrack, int32> S_WallVariants =
	{
		{ EMineCellTrack::None,		0 },
		{ EMineCellTrack::Curve,	3 },
		{ EMineCellTrack::Straight, 5 },
		{ EMineCellTrack::T,		7 },
		{ EMineCellTrack::Cross,	15 },
	};
	
};