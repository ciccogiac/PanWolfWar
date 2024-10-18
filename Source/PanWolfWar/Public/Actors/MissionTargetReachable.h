#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MissionTargetReachable.generated.h"

UCLASS()
class PANWOLFWAR_API AMissionTargetReachable : public AActor
{
	GENERATED_BODY()
	
public:	
	AMissionTargetReachable();


	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnTargetMissionActivated();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnTargetReached();

protected:
	virtual void BeginPlay() override;

private:

	UFUNCTION()
	virtual void BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact Params", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* BoxComponent;

	bool bIsCharacterInside = false;
	class AMissionManager* MissionManager;

public:
	FORCEINLINE bool IsCharacterInside() const { return bIsCharacterInside;  }

};
