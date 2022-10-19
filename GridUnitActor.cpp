

#include "GridUnitActor.h"
#include "MineshaftGameInstance.h"


void AGridUnitActor::Init(const FUnitTemplate& unitTemplate)
{
	UnitKey = unitTemplate.UnitKey;
	check(!UnitKey.IsNone());
	BuffKey = unitTemplate.BuffKey;
	AttackPatterns = unitTemplate.AttackPatterns;
	AttackPatternsToUse = unitTemplate.AttackPatternsToUse;
	BuffPatterns = unitTemplate.BuffPattern;
	RefreshBuffCoords();

	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;
	UnitExplosionDelay = sm->GetRules()->ExplosionDelay;
	
	SetupCompleteBP();
}

// Called the first time this unit is created
void AGridUnitActor::Setup(const FUnitTemplate& unitTemplate)
{
	static int32 id = 0;
	UnitID = ++id;
	Init(unitTemplate);
	FirstSetupCompleteBP();
};

void AGridUnitActor::PostCreate()
{
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;

	// Alert the units this dude is buffing
	for(auto& coord : BuffCoords)
	{
		auto cell = sm->GetGridCell(coord.Row, coord.Col);
		if(cell && cell->UnitActor)
			cell->UnitActor->AddBuffAlertBP();
	}
}

void AGridUnitActor::Refresh()
{
	if(IsAttackSet())
		SetAttackBP();
}

void AGridUnitActor::DoYield()
{
	YieldTickBP();
};

// Unit has been interacted with on the grid (clicked)
void AGridUnitActor::Activate()
{
	if(!CanBeActivated) return;

	OwningGridCell->EndHighlight();
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	gi->SessionManager->SetViewState(EViewState::UnitInteractable);
	ActivateBP();
};

// Unit has been exited. Returning to the wider macro view.
void AGridUnitActor::Deactivate()
{
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;
	sm->SetViewState(EViewState::MacroInteractable);
}

void AGridUnitActor::Clear(int32 delayCount)
{
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;

	if(sm && sm->IsActive())
	{
		TMap<ECurrency, float> refund = sm->GetUnitRefund(UnitKey);
		sm->AddToWallet(refund);
	}

	ClearBP(UnitExplosionDelay * delayCount);
}

void AGridUnitActor::RefreshIntent()
{
	if(IsAttackSet())
	{
		UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
		USessionManager* sm = gi->SessionManager;

		if(!sm) return;

		TArray<FCoordinates> coords;
		for(auto& atk : CurrentAttacks)
			sm->GetPatternCoordinates(OwningGridCell->Row, OwningGridCell->Col, coords, atk);	
		
		for(auto& coord : coords)
		{
			AGridCellActor* cell = sm->GetGridCell(coord.Row, coord.Col);
			cell->AddIntent(EUnitIntent::Attack);	
		}
	}
}

void AGridUnitActor::RefreshBuffCoords()
{
	// cache any potential coords up front
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;
	BuffCoords.Empty();
	TArray<FCoordinates> coords;
	sm->GetPatternCoordinates(OwningGridCell->Row, OwningGridCell->Col, coords, BuffPatterns);
	BuffCoords = TSet<FCoordinates>(coords);
}

void AGridUnitActor::ClearBuffs()
{
	Buffs.Empty();
}

void AGridUnitActor::AddBuff(const FName& key)
{
	Buffs.Add(key);
}

bool AGridUnitActor::IsAttackSet()
{
	return CurrentAttacks.Num() > 0;
}

void AGridUnitActor::ClearAttack()
{
	CurrentAttacks.Empty();
	ClearAttackBP();
}

// Attack intents should always be set one day before the attack
void AGridUnitActor::SetAttack()
{
	if(AttackPatterns.Num() == 0) return;
	
	ClearAttack();

	// Get all attack pattern indices
	TArray<int32> attackIdices;
	for(int32 idx = 0; idx < AttackPatterns.Num(); ++idx)
		attackIdices.Add(idx);

	// shuffle
	for(int32 idx = 0; idx < attackIdices.Num(); ++idx)
	{
		int32 randIdx = attackIdices[FMath::Rand() % attackIdices.Num()];
		int32 temp = attackIdices[idx];
		attackIdices[idx] = attackIdices[randIdx];
		attackIdices[randIdx] = temp;
	}

	// set attack pattern when we find a valid one
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	int32 row = OwningGridCell->Row;
	int32 col = OwningGridCell->Col;
	for(int32 n = 0; n < attackIdices.Num() && CurrentAttacks.Num() < AttackPatternsToUse; ++n)
	{
		int32 idx = attackIdices[n];
		auto& pattern = AttackPatterns[idx];
		int32 dr = row + pattern.DirectionRow;
		int32 dc = col + pattern.DirectionCol;

		// If this is a blank cell, get out of here
		if(gi->Grid->GetCell(dr, dc) == nullptr) continue;

		CurrentAttacks.Add(AttackPatterns[idx]);
	}
	
	Refresh();
}

void AGridUnitActor::Attack()
{
	if(CurrentAttacks.Num() == 0)
	{
		// Trying to prevent any last minute crashes
		// This shouldn't happen but we can hit this if we just lost on the closing sequence
		check(false);
		return;
	}

	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;
	TArray<FCoordinates> coords;
	for(auto& atk : CurrentAttacks)
		sm->GetPatternCoordinates(OwningGridCell->Row, OwningGridCell->Col, coords, atk);

	AttackBP(coords);
	ClearAttack();
}

void AGridUnitActor::MoveTo(int32 row, int32 col)
{
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;
	if(sm->MoveUnitTo(OwningGridCell->Row, OwningGridCell->Col, row, col))
		RefreshBuffCoords();
}
