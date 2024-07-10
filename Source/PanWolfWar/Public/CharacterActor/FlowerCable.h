#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlowerCable.generated.h"

class UCableComponent;

UCLASS()
class PANWOLFWAR_API AFlowerCable : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlowerCable();

	void SetCableAttachment(USceneComponent* Component, FName SocketName = NAME_None);

	void SetCableLength(float Length);
	void SetCableVisibility(bool NewVisibility);
	void SetEndCableLocation(FVector NewLocation);

	//void SwingCable(FVector Force);

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Components", meta = (AllowPrivateAccess = "true"))
	UCableComponent* CableComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hooking Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* EndCable;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hooking Components", meta = (AllowPrivateAccess = "true"))
	//class UPhysicsConstraintComponent* PhysicsConstraintComponent;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hooking Components", meta = (AllowPrivateAccess = "true"))
	//UStaticMeshComponent* StartCable;

};
