
#include "Interactable.h"
#include "../ue_common/InteractableWidget.h"
#include "../ue_duel/DuelSolverActor.h"


// Initial setup when this actor is spawned in the level.
void AInteractable::Setup(USessionRoom* room)
{
	static uint32 profileID = 0;
	ProfileKey = FString::Printf(TEXT("SessionProfile_%d"), ++profileID);
	Profile = NewObject<USessionProfile>(this, SessionProfileClass, *ProfileKey);
	Profile->profileKey = ProfileKey;
	Profile->Setup(room);
	UCareerGameInstance* cgi = GetWorld()->GetGameInstance<UCareerGameInstance>();
	cgi->SessionSolver->AddProfile(Profile);
}

// Player interacted with this actor. Start this interaction.
void AInteractable::Start()
{
	check(State == EInteractableState::None);
	bShutdown = false;
	State = EInteractableState::Active;

	check(PawnClass);
	check(ProfileKey.Len() > 0); 
	APlayerController* controller = GetWorld()->GetFirstPlayerController();
	PreviousPawn = Cast<AInteractablePawn>(controller->GetPawn());
	if(PreviousPawn)
		PreviousPawn->EndInteractionBP();

	static int32 pawnID;
	FActorSpawnParameters params;
	params.Owner = this;
	params.Name = FName(TEXT("InteractablePawn_"), ++pawnID);
	TSubclassOf<AInteractablePawn> pawnClass = internal_GetPawnClass();
	Pawn = GetWorld()->SpawnActor<AInteractablePawn>(pawnClass, GetTransform(), params);
	Pawn->BeginInteractionBP();

	StartDelegate.Broadcast(this);
	StartBP();
}

// Restoring from a save game
void AInteractable::Restore(const FInteractableRecord& record)
{
	bShutdown = false;
	ProfileKey = record.ProfileKey;
	UCareerGameInstance* cgi = GetWorld()->GetGameInstance<UCareerGameInstance>();
	check(!Profile);
	Profile = cgi->SessionSolver->GetProfile(ProfileKey);
	State = record.State;
	RestoreBP();
}

void AInteractable::Shutdown()
{
	if(bShutdown)
	{
		check(!Widget);
		check(!PreviousPawn);
		check(!Pawn);
		return;
	}
	
	if(Widget)
	{
		Widget->Shutdown();
		Widget = nullptr;
	}

	if(PreviousPawn)
	{
		PreviousPawn->Destroy();
		PreviousPawn = nullptr;
	}
	
	if(Pawn)
	{
		Pawn->Destroy();
		Pawn = nullptr;
	}
	
	bShutdown = true;
	ShutdownBP();
	ShutdownDelegate.Broadcast(this);
}

// Interactables typically play animations when first interacted with.
// Allow for those to finish before starting our widget transaction.
void AInteractable::IntroFinished()
{
	internal_StartTransaction(TransactionWidgetClass, ProfileKey);
}

void AInteractable::internal_StartTransaction(TSubclassOf<UInteractableWidget> widgetClass, const FString& profileKey)
{
	check(!Widget);
	Widget = CreateWidget<UInteractableWidget>(GetWorld(), widgetClass);
	Widget->TransCompleteDelegate.BindUObject(this, &AInteractable::End);
	UCareerGameInstance* cgi = GetWorld()->GetGameInstance<UCareerGameInstance>();
	cgi->AddWidgetToViewport(Widget, EUIZOrderLayer::Interactable);
	Widget->StartTransaction(profileKey);
}

void AInteractable::End(bool complete)
{
	check(PreviousPawn);
	APlayerController* controller = GetWorld()->GetFirstPlayerController();
	controller->Possess(PreviousPawn);
	PreviousPawn->BeginInteractionBP();
	Pawn->Destroy();

	State = complete ? EInteractableState::Complete : EInteractableState::None;
	EndBP();
	CompleteDelegate.Broadcast(this);
	
	Widget = nullptr;
	Pawn = nullptr;
	PreviousPawn = nullptr;
}
