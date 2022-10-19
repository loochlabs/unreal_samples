

#include "ConverterUnitActor.h"
#include "MineshaftGameInstance.h"


void AConverterUnitActor::Setup(const FUnitTemplate& unitTemplate) 
{
	// Setup rows, X to Y, establish conversion types
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;
	USessionRules* rules = sm->GetRules();

	// Override EditAnywhere values
	Columns = 2;
	
	// Initialize cell properties
	for(int32 rowIndex = 0; rowIndex < MaxUnlockRows; ++rowIndex)
	{
		FMineRow minerow;

		// Unlock costs. First row has UnlockCost = 0
		float baseCost = UnlockBaseCost;
		int32 upgradeIndex = FMath::Max(0, rowIndex+1);
		minerow.UnlockCost = static_cast<int32>(baseCost + (baseCost * upgradeIndex * rules->UpgradeBaseMultiplier));
		minerow.UnlockCurrency = UnlockCurrency;

		// Two cells per row, cell_0 is the input, cell_1 is the output
		// Input cell
		UMineshaftCell* input = NewObject<UMineshaftCell>(this);
		input->Row = rowIndex;
		input->Col = 0;
		minerow.Cells.Add(input);
		input->Currency = ECurrency::Iron;
		input->Bank = YieldBase * (1 + (upgradeIndex * rules->UpgradeBaseMultiplier));
		
		// Output cell
		UMineshaftCell* output = NewObject<UMineshaftCell>(this);
		output->Row = rowIndex;
		output->Col = 1;
		minerow.Cells.Add(output);
		output->Bank = input->Bank * YieldMultiplierPadding;
		
		float currencyRNG = FMath::FRand();
		if(currencyRNG < 0.33f)
			output->Currency = ECurrency::Stone;
		else if(currencyRNG < 0.67f)
			output->Currency = ECurrency::Copper;
		else 
			output->Currency = ECurrency::Gold;
		
		Rows.Add(minerow);
	}

	// Unlock initial row, calculates our Yield
	int32 rowUnlocks = InitialRowUnlocks;
	for(int32 n = 0; n < rowUnlocks && n < MaxUnlockRows; ++n)
	{
		check(!Rows[n].Unlocked);
		Rows[n].UnlockCost = 0;
		UnlockMineRow();
	}
	
	AGridUnitActor::Setup(unitTemplate);
}

void AConverterUnitActor::GetTotalYieldByRef(TMap<ECurrency, float>& totals)
{
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;
	
	for(auto& row : Rows)
	{
		check(row.Cells.Num() == 2)

		UMineshaftCell* input = row.Cells[0];
		UMineshaftCell* output = row.Cells[1]; 
		if(output->Producer)
		{
			// Input. Don't spend past currency that you don't have
			float availableInputAmount = sm->GetCurrency(input->Currency);
			float inputToConvert = input->Bank;
			float conversionRatio = 1.0f;
			
			if(availableInputAmount < inputToConvert)
			{
				conversionRatio = availableInputAmount / inputToConvert;
				inputToConvert = availableInputAmount;
			}

			if(!totals.Contains(input->Currency))
				totals.Add(input->Currency, 0.f);

			totals[input->Currency] -= inputToConvert;
			
			// Output
			if(!totals.Contains(output->Currency))
				totals.Add(output->Currency, 0.f);

			totals[output->Currency] += output->Bank * conversionRatio;
		}
	}
	
	Super::GetTotalYieldByRef(totals);
}

// Only flag the output cell as our producer. That's enough state to know this row is active
void AConverterUnitActor::ToggleRowAsProducer(int32 rowIndex)
{
	check(rowIndex >= 0 && rowIndex < Rows.Num());

	UMineshaftCell* cell = Rows[rowIndex].Cells[1];
	cell->Producer = !cell->Producer;
	CalculateYield();
}

bool AConverterUnitActor::IsRowProducing(int32 rowIndex)
{
	auto& row = Rows[rowIndex];
	UMineshaftCell* cell = row.Cells[1];
	return row.Unlocked && cell->Producer;
}
