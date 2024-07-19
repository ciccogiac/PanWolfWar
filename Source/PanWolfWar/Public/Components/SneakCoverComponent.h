#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SneakCoverComponent.generated.h"

class UInputMappingContext;
struct FInputActionValue;
class UInputAction;
class APanWolfWarCharacter;
class UPandolfoComponent;

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
	void ExitCover();
	void JumpCover();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	const FHitResult DoWalltrace(float TraceRadius = 20.f);
	void SetCharRotation(const FVector ImpactNormal);
	void SetCharLocation(const FVector HitLocation, const FVector HitNormal);
	void CheckCrouchHeight(const FVector Direction);

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Cover", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* SneakCoverMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > WorldStaticObjectTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cover State ", meta = (AllowPrivateAccess = "true"))
	bool IsCovering = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cover State ", meta = (AllowPrivateAccess = "true"))
	float CoverDirection = 0.f;
};
