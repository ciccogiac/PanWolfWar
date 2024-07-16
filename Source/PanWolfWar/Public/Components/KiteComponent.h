#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KiteComponent.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class APanWolfWarCharacter;
class AKiteBoard;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UKiteComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UKiteComponent();

	void KiteMove(const FInputActionValue& Value);
	void KiteJump();

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Climb")
	UInputAction* KiteMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Climb")
	UInputAction* KiteJumpAction;

private:
	ACharacter* CharacterOwner;
	APanWolfWarCharacter* PanWolfCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* KiteMappingContext;

	AKiteBoard* KiteBoard;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grappling Params", meta = (AllowPrivateAccess = "true"))
	float KiteMoveForce = 100000.f;

public:
	FORCEINLINE void SetKiteBoard(AKiteBoard* _KiteBoard) { KiteBoard = _KiteBoard; };
};
