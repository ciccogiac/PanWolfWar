#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/TargetInterface.h"
#include "TargetingComponent.generated.h"

class UInputAction;
class APanWolfWarCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTargetingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

	void ToggleLock();

protected:
	virtual void BeginPlay() override;

private:

	void EnableLock();
	void DisableLock();

	bool FindTarget();
	bool CanTargetActor(AActor* FindActor);
	void SetRotationMode(bool bTargetMode);

private:

	ACharacter* CharacterOwner;
	APanWolfWarCharacter* PanWolfCharacter;

	bool IsTargeting = false;
	AActor* TargetActor = nullptr;
	ITargetInterface* TargetInterface = nullptr;

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TargetLockAction;


		
};
