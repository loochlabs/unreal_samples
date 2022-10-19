
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "../ue_common/InteractablePawn.h"
#include "../ue_common/InteractableWidget.h"
#include "../duel/DuelEnums.h"

#include "Interactable.generated.h"

class AInteractable;
class UInteractableWidget;
class ADuelSolverActor;
class USessionProfile;
class USessionRoom;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(InteractableDelegate, AInteractable*, Interactable);


UENUM(BlueprintType)
enum class EInteractableState : uint8
{
	None = 0,	
	Active,		// Player is in the middle of an interaction
	Complete,
};

USTRUCT(BlueprintType)
struct FInteractableRecord
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly) 
	FString ProfileKey;

	UPROPERTY(BlueprintReadOnly) 
	EInteractableState State = EInteractableState::None;

	UPROPERTY(BlueprintReadOnly) 
	int32 BPID = 0;
};


UCLASS(BlueprintType)
class ALCHEMICAL_API AInteractable : public AActor
{
	GENERATED_BODY()

public:

	virtual void Setup(USessionRoom* room);

	UFUNCTION(BlueprintCallable, Category="Interactables") 
	virtual void Start();

	UFUNCTION(BlueprintCallable, Category="Interactables") 
	virtual void End(bool complete);

	virtual void Restore(const FInteractableRecord& record);
	virtual void Shutdown();

	UFUNCTION(BlueprintImplementableEvent, Category="Interactables") 
	void ShutdownBP();

	void PostSetup();

	UFUNCTION(BlueprintImplementableEvent, Category="Interactables") 
	void PostSetupBP();

	UFUNCTION(BlueprintCallable, Category="Interactables") 
	bool CanInteract()	{ return State == EInteractableState::None; };

	UFUNCTION(BlueprintCallable, Category="Interactables") 
	bool IsActive()		{ return State == EInteractableState::Active; };

	UFUNCTION(BlueprintCallable, Category="Interactables") 
	bool IsComplete()	{ return State == EInteractableState::Complete; };

	UFUNCTION(BlueprintImplementableEvent, Category="Interactables") 
	void StartBP();

	UFUNCTION(BlueprintCallable, Category="Interactables") 
	void IntroFinished();

	UFUNCTION(BlueprintImplementableEvent, Category="Interactables") 
	void RestoreBP();

	UFUNCTION(BlueprintImplementableEvent, Category="Interactables") 
	void EndBP();

	virtual TSubclassOf<AInteractablePawn> internal_GetPawnClass() { return PawnClass; }; 
	
	void internal_StartTransaction(TSubclassOf<UInteractableWidget> widgetClass, const FString& profileKey);	

	UPROPERTY(EditAnywhere) 
	TSubclassOf<USessionProfile> SessionProfileClass;

	UPROPERTY(EditAnywhere) 
	TSubclassOf<UInteractableWidget> TransactionWidgetClass;
	
	UPROPERTY(BlueprintReadWrite) 
	FString ProfileKey;

	UPROPERTY(BlueprintReadOnly) 
	USessionProfile* Profile = nullptr;

	UPROPERTY(BlueprintReadOnly) 
	EInteractableState State = EInteractableState::None;

	UPROPERTY(EditAnywhere) 
	TSubclassOf<AInteractablePawn> PawnClass;
	
	UPROPERTY(BlueprintReadOnly) 
	AInteractablePawn* Pawn = nullptr;

	UPROPERTY(BlueprintReadOnly) 
	AInteractablePawn* PreviousPawn = nullptr;

	UPROPERTY(BlueprintReadOnly) 
	UInteractableWidget* Widget = nullptr;
	
	UPROPERTY(BlueprintAssignable)
	InteractableDelegate StartDelegate;

	UPROPERTY(BlueprintAssignable)
	InteractableDelegate CompleteDelegate;

	UPROPERTY(BlueprintAssignable)
	InteractableDelegate ShutdownDelegate;

	bool bShutdown = false;
};
