#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SneakCoverComponent.generated.h"

class UInputMappingContext;
struct FInputActionValue;
class UInputAction;
class APanWolfWarCharacter;
class UPandolfoComponent;
class UAnimMontage;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API USneakCoverComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USneakCoverComponent();

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

	void CoverMove(const FInputActionValue& Value);
	void StartCover();
	void StopCover();
	bool CanEnterCover(const FVector StartPoint);
	void EnterCover();
	void ExitCover();
	void JumpCover();

	void StartNarrow();
	void StopNarrow(const FVector EndLocation);

	UFUNCTION(BlueprintCallable)
	void ExitNarrow();

	void ActivateWallSearch();
	void DeactivateWallSearch();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	const FHitResult DoWalltrace(float TraceRadius = 20.f,float Direction = 0.f);
	void SetCharRotation(const FVector ImpactNormal, bool Istantaneus = false);
	void SetCharLocation(const FVector HitLocation, const FVector HitNormal, bool Istantaneus = false);
	bool CheckCrouchHeight(const float Direction);
	bool CheckCanTurn(const FVector TurnPoint);

#pragma region InputActions

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Cover")
	UInputAction* StartCoverAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Cover")
	UInputAction* StopCoverAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Cover")
	UInputAction* JumpCoverAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Cover")
	UInputAction* SneakCoverMoveAction;

#pragma endregion

private:
	ACharacter* CharacterOwner;
	APanWolfWarCharacter* PanWolfCharacter;
	UPandolfoComponent* PandolfoComponent;

	FTimerHandle WallSearch_TimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Cover", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* SneakCoverMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > WorldStaticObjectTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cover State ", meta = (AllowPrivateAccess = "true"))
	bool IsCovering = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cover State ", meta = (AllowPrivateAccess = "true"))
	float CoverDirection = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Montage, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ExitNarrowMontage;

	float LastCoverDirection = 0.f;
	FVector SavedAttachPoint;
	FVector SavedAttachNormal;
	bool bIsNarrowing = false;

public:
	FORCEINLINE bool IsNarrowing() const { return bIsNarrowing; }
};
