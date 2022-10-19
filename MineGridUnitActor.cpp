
#include "MineGridUnit.h"
#include "MineshaftGameInstance.h"
#include "TechLabUnitActor.h"


struct FCurrencyWeight
{
	ECurrency Currency = ECurrency::Stone;
	float Weight = 0.f;
};


void AMineGridUnit::Setup(const FUnitTemplate& unitTemplate)
{
	// Determine currency in a producer cell. Favor the UnlockCurrency with configurable weights 
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;
	USessionRules* rules = sm->GetRules();
	
	TMap<ECurrency, TArray<FCurrencyWeight>> weights;
	TArray<FCurrencyWeight> cws;

	// ensure order of keys
	TArray<ECurrency> keys;
	ProducerChances.GetKeys(keys);
	
	float totalWeight = 0.f;
	for(auto& k : keys)
		totalWeight += ProducerChances[k];

	float weightSum = 0.f;
	for(auto& k : keys)
	{
		weightSum += ProducerChances[k];
		FCurrencyWeight cw;
		cw.Currency = k;
		cw.Weight = weightSum / totalWeight;
		check(cw.Weight > 0.f);
		check(cw.Weight <= 1.f);
		cws.Add(cw);
	}
	weights.Add(UnlockCurrency, cws);

	TotalProducers = 0;
	auto set_as_producer = [&](UMineshaftCell* cell)
	{
		// Producer cells have a Bank of currency to be drawn from
		TotalProducers++;
		cell->Producer = true;
		cell->Bank = FMath::RandRange(BankInitialMin, BankInitialMax);
		cell->BankMax = cell->Bank;
	};

	// Initialize cell properties
	for(int32 jdx = 0; jdx < MaxUnlockRows; ++jdx)
	{
		FMineRow row;

		// Unlock costs. First row has UnlockCost = 0
		float baseCost = UnlockBaseCost;
		int32 upgradeIndex = FMath::Max(0, jdx - 1);
		row.UnlockCost = static_cast<int32>(baseCost + (baseCost * upgradeIndex * rules->UpgradeBaseMultiplier));
		row.UnlockCurrency = UnlockCurrency;

		for(int32 idx = 0; idx < Columns; ++idx)
		{
			UMineshaftCell* cell = NewObject<UMineshaftCell>(this);
			cell->Row = jdx;
			cell->Col = idx;

			// Currency assignment
			cell->Currency = ECurrency::Stone;
			float currencyRNG = FMath::FRand();
			TArray<FCurrencyWeight>& cw = weights[UnlockCurrency];
			for(auto& c : cw)
			{
				if(currencyRNG < c.Weight)
				{
					cell->Currency = c.Currency;
					break;
				}
			}
			
			row.Cells.Add(cell);
		}
		Rows.Add(row);
	}

	// Maze assignment
	auto link = [](UMineshaftCell* a, UMineshaftCell* b)
	{
		a->Links.Add(b);
		b->Links.Add(a);
	};

	for(int32 r = Rows.Num()-1; r >= 0; --r)
	{
		auto& row = Rows[r];
		for(int32 c = 0; c < row.Cells.Num(); ++c)
		{
			UMineshaftCell* current = row.Cells[c];
			UMineshaftCell* next = nullptr;

			// Always carve right on the final row
			if(r == 0)
			{
				if(c < row.Cells.Num()-1)
				{
					next = Rows[r].Cells[c+1];
					check(next);
					link(next, current);
				}
				continue;
			}

			if( (c == 0) ||
				((c < row.Cells.Num()-1) && FMath::FRand() < 0.5f) ) 
			{
				// carve right
				next = Rows[r].Cells[c+1];
			}
			else
			{
				// carve up on final column
				next = Rows[r-1].Cells[c];
			}
			
			check(next);
			link(next, current);
		}
	}

	// REPO node - Ensure we have an output on our top row
	auto& toprow = Rows[0];
	
	// Set repo node. Random cell in top row, exclude 0 index
	UMineshaftCell* repoCell = toprow.Cells[(FMath::Rand() % (Columns - 1)) + 1];
	check(repoCell->WallVariant != WALL_NORTH && repoCell->WallVariant != WALL_EAST && repoCell->WallVariant != WALL_SOUTH && repoCell->WallVariant != WALL_WEST);
	repoCell->Repo = true;
	repoCell->WallVariant = WALL_NORTH | WALL_EAST | WALL_SOUTH | WALL_WEST;

	// Gaurentee a connection to our top producer by carving left on everythign between the repo and edge producer
	for(int32 col = repoCell->Col; col > 0; --col)
	{
		auto current = GetCell(0, col);
		auto next = GetCell(0, col-1);
		link(current, next);
	}

	// Mirror the mineshaft 50% of the time for visual variation.
	// Need to do this before Neighbor assignments or orientations will be backwards
	if(FMath::FRand() < 0.5f)
	{
		for(auto& row : Rows)
		{
			// Only need to swap half of the array
			for(int32 idx = 0; idx < (row.Cells.Num()/2); ++idx)
			{
				int32 swapIdx = row.Cells.Num()-1 - idx;
				int32 c1 = row.Cells[swapIdx]->Col;
				int32 c2 = row.Cells[idx]->Col;
				UMineshaftCell* temp = row.Cells[swapIdx];
				row.Cells[swapIdx] = row.Cells[idx];
				row.Cells[swapIdx]->Col = c1;
				row.Cells[idx] = temp;
				row.Cells[idx]->Col = c2;
			}
		}
	}
	
	// Neighbor assignment
	for (int32 r = 0; r < Rows.Num(); r++)
	{
		for (int32 c = 0; c < Rows[r].Cells.Num(); c++)
		{
			UMineshaftCell* cell = GetCell(r, c);
			
			if (!cell) continue;

			cell->Neighbors.Add(ECellOrientation::North, GetCell(r-1, c));
			cell->Neighbors.Add(ECellOrientation::East,  GetCell(r, c+1));
			cell->Neighbors.Add(ECellOrientation::South, GetCell(r+1, c));
			cell->Neighbors.Add(ECellOrientation::West,  GetCell(r, c-1));
		}
	}

	// Cell wall setup
	for(auto& row : Rows)
	{
		for(auto& c : row.Cells)
		{
			if (c->Links.Contains(c->Neighbors[ECellOrientation::North]))  c->WallVariant |= WALL_NORTH;
			if (c->Links.Contains(c->Neighbors[ECellOrientation::East]))   c->WallVariant |= WALL_EAST;
			if (c->Links.Contains(c->Neighbors[ECellOrientation::South]))  c->WallVariant |= WALL_SOUTH;
			if (c->Links.Contains(c->Neighbors[ECellOrientation::West]))   c->WallVariant |= WALL_WEST;
		}
	}

	//@TECH Perfection Skill
	for(UTechNode* tech : sm->ActiveTech)
	{
		if(tech->Trait != ETechTrait::Perfection) continue;
		if(tech->UnitKey != unitTemplate.UnitKey) continue;

		PickupTracksOnRowUnlock = false;
		RotateTracksOnSetup = false;
		SwapTracksOnSetup = false;
	}
	
	// Set properties after maze has been generated
	for(auto& row : Rows)
	{
		for(auto& c : row.Cells)
		{
			//@OPTIMIZE break this track type assignment to a funciton
			c->TrackType = UMineshaftCell::S_TrackTypes[c->WallVariant];
			c->WallOrientation = c->WallVariant;
			c->Orientation = ECellOrientation::North;

			if(RotateTracksOnSetup)
			{
				int32 rand = FMath::Rand() % 4;
				for(int32 n = 0; n < rand; ++n)
					RotateCellCW(c, false);
			}
			
			if (c->WallVariant == WALL_NORTH
				|| c->WallVariant == WALL_EAST
				|| c->WallVariant == WALL_SOUTH
				|| c->WallVariant == WALL_WEST)
			{
				set_as_producer(c);
			}

			// Binary tree carve cannot generate a cross
			check(c->Repo || (c->WallVariant != 15));
		}
	}

	//@TECH Guilded tech, auto rank up
	int32 rowUnlocks = InitialRowUnlocks;
	for(UTechNode* tech : sm->ActiveTech)
	{
		if(tech->Trait != ETechTrait::Guilded) continue;
		if(tech->UnitKey != unitTemplate.UnitKey) continue;

		rowUnlocks++;
	}

	// Unlock initial row, calculates our Yield
	UnlockedProducers = 0;
	for(int32 n = 0; n < rowUnlocks && n < MaxUnlockRows; ++n)
	{
		check(!Rows[n].Unlocked);
		Rows[n].UnlockCost = 0;
		UnlockMineRow();
	}

	Super::Setup(unitTemplate);
}

void AMineGridUnit::DoYield()
{
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;

	// Calculate CurrentYield during CalculateYield(). We need to factor in YieldMultiplier there.
	TMap<ECurrency, float> total;
	SumYieldAmount(total);
	sm->AddToWallet(total);

	Super::DoYield();
}

void AMineGridUnit::Refresh()
{
	CalculateYield();
	Super::Refresh();
}

// Rotate CW
ECellOrientation AMineGridUnit::RotateCellCW(UMineshaftCell* cell, bool calcYield /*= true*/)
{
	cell->WallOrientation = cell->WallOrientation << 1;
	int32 mask = 15; //all walls
	int32 carry = cell->WallOrientation & (~mask);
	if (carry > 0)
	{
		cell->WallOrientation -= carry; //remove carry
		cell->WallOrientation += 1; // move to the right most bit
	}
	
	int32 raw = static_cast<int32>(cell->Orientation);
	raw = raw == 4 ? 1 : raw + 1;
	cell->Orientation = static_cast<ECellOrientation>(raw);

	if(calcYield)
		CalculateYield();
	
	return cell->Orientation;
}

// Rotate CCW
ECellOrientation AMineGridUnit::RotateCellCCW(UMineshaftCell* cell, bool calcYield /*= true*/)
{
	int32 carry = cell->WallOrientation & 1;
	cell->WallOrientation = cell->WallOrientation >> 1;
	if (carry > 0)
		cell->WallOrientation += 8; // add left most bit
	
	int32 raw = static_cast<int32>(cell->Orientation);
	raw = raw == 1 ? 4 : raw - 1;
	cell->Orientation = static_cast<ECellOrientation>(raw);

	if(calcYield)
		CalculateYield();
	
	return cell->Orientation;
}

ECellOrientation AMineGridUnit::RotateCellCW(int32 row, int32 col)
{
	auto& r = Rows[row];
	auto& cell = r.Cells[col];
	return RotateCellCW(cell);
}

ECellOrientation AMineGridUnit::RotateCellCCW(int32 row, int32 col)
{
	auto& r = Rows[row];
	auto& cell = r.Cells[col];
	return RotateCellCCW(cell);
}

int32 AMineGridUnit::GetUnlockLevel()
{
	int32 ret = -1; // new mine, no rows unlocked yet
	for(auto& row : Rows)
	{
		if(row.Unlocked)
			ret++;
	}
	return ret;
}

bool AMineGridUnit::CanUnlock()
{
	int32 levelToUnlock = GetUnlockLevel() + 1;
	if(levelToUnlock == MaxUnlockRows)
		return false;
	
	auto& row = Rows[levelToUnlock];
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	auto& wallet = gi->SessionManager->Wallet.Amounts;
	return wallet.Contains(row.UnlockCurrency) ? wallet[row.UnlockCurrency] >= row.UnlockCost : false;
}

bool AMineGridUnit::UnlockMineRow()
{
	// insufficient funds
	if(!CanUnlock()) return false;
	
	int32 unlockLevel = GetUnlockLevel();
	int32 levelToUnlock = unlockLevel + 1;
	auto& row = Rows[levelToUnlock];
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	gi->SessionManager->UpdateWallet(row.UnlockCurrency, -row.UnlockCost);
	row.Unlocked = true;

	// Pickup our tracks and place in inventory
	if(PickupTracksOnRowUnlock)
	{
		for(auto& cell : row.Cells) 
			PickupTrack(cell->Row, cell->Col, false);
	}

	for(auto& cell : row.Cells)
	{
		if(cell->Producer)
			UnlockedProducers++;
	}
	
	CalculateYield();
	return true;	
}


void AMineGridUnit::RevealUnlockedMineRow(int32 row)
{
	check(row >= 0 && row < Rows.Num());

	auto& minerow = Rows[row];
	if(minerow.Unlocked)
		minerow.Revealed = true;
}

void AMineGridUnit::RevealUnlockedMineRows()
{
	for(int32 r = 0; r < Rows.Num(); ++r)
		RevealUnlockedMineRow(r);
}

// Only consider first reveal if no rows have been unlocked yet
bool AMineGridUnit::IsFirstReveal()
{
	for(auto& row : Rows)
	{
		if(row.Revealed)
			return false;
	}
	return true;
}

bool AMineGridUnit::IsFullyUnlocked()
{
	int32 unlockLevel = GetUnlockLevel();
	check(unlockLevel <= (MaxUnlockRows-1));
	bool fullyUnlocked = unlockLevel == (MaxUnlockRows-1);
	check(!fullyUnlocked || (TotalProducers == UnlockedProducers));
	return fullyUnlocked; 	
}

UMineshaftCell* AMineGridUnit::GetCell(int32 row, int32 col)
{
	if(row < 0 || col >= Columns || row >= Rows.Num()) 
		return nullptr;
	
	if(col < Columns && row >= 0 && col >= 0)
		return Rows[row].Cells[col];

	return nullptr;
}

void AMineGridUnit::ClearCellWalls(int32 row, int32 col)
{
	UMineshaftCell* cell = GetCell(row, col);

	if(!cell) return;

	// Design: Never clear producers
	check(!cell->Producer);

	cell->TrackType = EMineCellTrack::None;
	cell->WallVariant = 0;
	cell->WallOrientation = 0;
	cell->Orientation = ECellOrientation::North;
}

void AMineGridUnit::SwapCellProperties(UMineshaftCell* a, UMineshaftCell* b, UMineshaftCell* buffer)
{
	auto copy = [](UMineshaftCell* cl, UMineshaftCell* cr)
	{
		cl->Currency = cr->Currency;
		cl->Bank = cr->Bank;
		cl->BankMax = cr->BankMax;
		cl->TrackType = cr->TrackType;
		cl->WallVariant = cr->WallVariant;
		cl->WallOrientation = cr->WallOrientation;
		cl->Orientation = cr->Orientation;
		cl->Repo = cr->Repo;
		cl->Producer = cr->Producer;
		cl->Links = cr->Links;
		cl->Neighbors = cr->Neighbors;
	};

	copy(buffer, a);
	copy(a, b);
	copy(b, buffer);
}

ECellOrientation AMineGridUnit::GetOrientation(UMineshaftCell* from, UMineshaftCell* to)
{
	// we're assuming these cells are adjacent
	if(from->Row > to->Row) return ECellOrientation::North;
	if(from->Row < to->Row) return ECellOrientation::South;
	if(from->Col < to->Col) return ECellOrientation::East;
	if(from->Col > to->Col) return ECellOrientation::West;
	return ECellOrientation::None;
}

void AMineGridUnit::CalculateDistanceToExit(UMineshaftCell* cell, TSet<UMineshaftCell*>& vst, int32 distance)
{
	if(vst.Contains(cell)) return;

	vst.Add(cell);
	cell->DistanceToExit = distance;
	for(auto& link : cell->YieldLinks)
		CalculateDistanceToExit(link.Value, vst, distance+1);
}

void AMineGridUnit::SetShortestPathToExit(UMineshaftCell* current, UMineshaftCell* prev, ECurrency currency /*=ECurrency::Money*/)
{
	check(current);
	current->InProductiveChain = true;

	// Determine origin and exit of producer
	FProductionChain chain;
	if(!prev)
	{
		check(current->Producer);
		chain.Origin = ECellOrientation::None;
		chain.Currency = current->Currency;
	}
	else
	{
		chain.Origin = GetOrientation(current, prev);
		chain.Currency = currency;
	}

	if(current->Repo)
		return;

	UMineshaftCell* next = nullptr;
	if(current->DistanceToExit > 0)
	{
		int32 shortest = TNumericLimits<int32>::Max();
		for(auto& link : current->YieldLinks)
		{
			if(link.Value->DistanceToExit < shortest)
			{
				shortest = link.Value->DistanceToExit;
				next = link.Value;
			}
		}
	}
	chain.Exit = next ? GetOrientation(current, next) : ECellOrientation::North;
	current->ProductionChains.Add(chain);

	if(next)
		SetShortestPathToExit(next, current, chain.Currency);
}

void AMineGridUnit::CalculateYield()
{
	check(Rows.Num() > 0);
	
	// reset all cells
	for (int32 r = 0; r < Rows.Num(); r++)
	{
		if (!Rows[r].Unlocked) continue;

		for (int32 c = 0; c < Rows[r].Cells.Num(); c++)
		{
			Rows[r].Cells[c]->YieldLinks.Empty();
			Rows[r].Cells[c]->ProductionChains.Empty();
		}
	}

	// cell neighbors/links
	// Find our active repo nodes
	TArray<UMineshaftCell*> repos;
	for (int32 r = 0; r < Rows.Num(); r++)
	{
		if(!Rows[r].Unlocked) continue;
		
		for (int32 c = 0; c < Rows[r].Cells.Num(); c++)
		{
			auto& cell = Rows[r].Cells[c];

			if(cell->Repo)
				repos.Add(cell);
			
			cell->InProductiveChain = false;

			// link NORTH
			if((cell->WallOrientation & WALL_NORTH) > 0 && !cell->YieldLinks.Contains(ECellOrientation::North))
			{
				UMineshaftCell* neighbor = GetCell(r-1, c);
				if(neighbor && (neighbor->WallOrientation & WALL_SOUTH) > 0)
				{
					cell->YieldLinks.Add(ECellOrientation::North, neighbor);
					neighbor->YieldLinks.Add(ECellOrientation::South, cell);
				}
			}
			
			// link EAST
			if((cell->WallOrientation & WALL_EAST) > 0 && !cell->YieldLinks.Contains(ECellOrientation::East))
			{
				UMineshaftCell* neighbor = GetCell(r, c+1);
				if(neighbor && (neighbor->WallOrientation & WALL_WEST) > 0)
				{
					cell->YieldLinks.Add(ECellOrientation::East, neighbor);
					neighbor->YieldLinks.Add(ECellOrientation::West, cell);
				}
			}
			
			// link SOUTH
			if((cell->WallOrientation & WALL_SOUTH) > 0 && !cell->YieldLinks.Contains(ECellOrientation::South))
			{
				UMineshaftCell* neighbor = GetCell(r+1, c);
				if(neighbor && (neighbor->WallOrientation & WALL_NORTH) > 0)
				{
					cell->YieldLinks.Add(ECellOrientation::South, neighbor);
					neighbor->YieldLinks.Add(ECellOrientation::North, cell);
				}
			}
			
			// link WEST
			if((cell->WallOrientation & WALL_WEST) > 0 && !cell->YieldLinks.Contains(ECellOrientation::West))
			{
				UMineshaftCell* neighbor = GetCell(r, c-1);
				if(neighbor && (neighbor->WallOrientation & WALL_EAST) > 0)
				{
					cell->YieldLinks.Add(ECellOrientation::West, neighbor);
					neighbor->YieldLinks.Add(ECellOrientation::East, cell);
				}
			}
		}
	}

	// Walk each cell, breath first search for total count
	// Calc Mineshsaft Yields
	// Using REPO nodes, calucluate productive chains of PRODUCER nodes

	ActiveProducers.Empty();
	TSet<UMineshaftCell*> visitedCells;
	for(auto& repo : repos)
	{
		if(visitedCells.Contains(repo)) continue;

		TArray<UMineshaftCell*> producers;
		TArray<UMineshaftCell*> toVisit = { repo };
		TArray<UMineshaftCell*> exitRepos = { repo };

		while (toVisit.Num() > 0)
		{
			auto c = toVisit[0];
			toVisit.RemoveAt(0);

			if (visitedCells.Contains(c)) continue;

			auto& row = Rows[c->Row];
			if(!row.Unlocked) continue;

			visitedCells.Add(c);

			// Producer cell
			if (c->Producer && c->Bank > 0.f)
				producers.Add(c);

			// Check if c is a REPO cell
			if(c->Repo)
				exitRepos.Add(c);

			for (auto& link : c->YieldLinks)
			{
				if (visitedCells.Contains(link.Value)) continue;

				toVisit.Add(link.Value);
			}
		}

		// Flag this chain as productive. Sum our total exits
		if(producers.Num() > 0)
		{
			ActiveProducers.Append(producers);

			for(auto& exitRepo : exitRepos)
			{
				TSet<UMineshaftCell*> visited;
				CalculateDistanceToExit(exitRepo, visited, 0);

				for(auto& prod : producers)
					SetShortestPathToExit(prod, nullptr);
			}
		}
	}

	check(ActiveProducers.Num() <= TotalProducers);
	for(auto& prod : ActiveProducers)
	{
		check(prod->Bank > 0.f);
		prod->CurrentYield = FMath::Min(YieldBase, prod->Bank); 
	}

	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	gi->SessionManager->YieldUpdated();
}

void AMineGridUnit::SumYieldAmount(TMap<ECurrency, float>& totals)
{
	GetTotalYieldByRef(totals);
}

// yield buffs
const TMap<FName, void(AMineGridUnit::*)(TMap<ECurrency, float>&)> AMineGridUnit::S_Buffs =
{
	{ "buff_stone", 	&AMineGridUnit::Buff_StoneOutput },
	{ "buff_copper", 	&AMineGridUnit::Buff_CopperOutput },
	{ "buff_iron", 		&AMineGridUnit::Buff_IronOutput },
	{ "buff_gold", 		&AMineGridUnit::Buff_GoldOutput },
	{ "buff_gold2", 	&AMineGridUnit::Buff_GoldMultiplier },
};


void AMineGridUnit::GetTotalYieldByRef(TMap<ECurrency, float>& total)
{
	for(auto& prod : ActiveProducers)
	{
		if(!total.Contains(prod->Currency))
			total.Add(prod->Currency, 0.f);

		total[prod->Currency] += prod->CurrentYield;		
	}

	// Apply yield buffs
	for(FName& key : Buffs)
	{
		if(S_Buffs.Contains(key))
			(this->*S_Buffs[key])(total);
	}

	//@TECH Efficient: Apply tech traits
	float techBonus = 0.f;
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;
	for(auto& tech : sm->ActiveTech)
	{
		if(tech->Trait != ETechTrait::Efficient) continue;
		if(tech->UnitKey != UnitKey) continue;

		techBonus += tech->TraitValue;
	}

	if(techBonus > 0.f)
	{
		for(auto& amt : total)
			amt.Value = amt.Value * (1 + techBonus);
	}
}

TMap<ECurrency, float> AMineGridUnit::GetTotalYield()
{
	TMap<ECurrency, float> ret;
	GetTotalYieldByRef(ret);
	return ret;
}

bool AMineGridUnit::HasYield()
{
	return ActiveProducers.Num() > 0;
}

float AMineGridUnit::GetActiveProducerPercent()
{
	float pct = static_cast<float>(ActiveProducers.Num()) / static_cast<float>(UnlockedProducers);
	return pct >= 0.999f ? 1.0f : pct;
}

void AMineGridUnit::ApplyBuffs()
{
	UMineshaftGameInstance* gi = GetWorld()->GetGameInstance<UMineshaftGameInstance>();
	USessionManager* sm = gi->SessionManager;

	for(auto& coord : BuffCoords)
	{
		AGridCellActor* cell = sm->GetGridCell(coord.Row, coord.Col);
		if(cell && cell->UnitActor)
			cell->UnitActor->AddBuff(BuffKey);
	}
}

void AMineGridUnit::Buff_StoneOutput(TMap<ECurrency, float>& total)
{
	ECurrency c = ECurrency::Stone;
	if(total.Contains(c))
		total[c] *= 2.f;
}

void AMineGridUnit::Buff_CopperOutput(TMap<ECurrency, float>& total)
{
	ECurrency c = ECurrency::Copper;
	if(!total.Contains(c))
		total.Add(c, 0.f);
	
	total[c] += 25.f;
}

void AMineGridUnit::Buff_IronOutput(TMap<ECurrency, float>& total)
{
	ECurrency c = ECurrency::Iron;
	if(total.Contains(c))
		total[c] *= 2.f;
}

void AMineGridUnit::Buff_GoldOutput(TMap<ECurrency, float>& total)
{
	ECurrency c = ECurrency::Gold;
	if(!total.Contains(c))
		total.Add(c, 0.f);
	
	total[c] += 5.f;
}

void AMineGridUnit::Buff_GoldMultiplier(TMap<ECurrency, float>& total)
{
	ECurrency c = ECurrency::Gold;
	if(total.Contains(c))
		total[c] *= 3.f;
}

