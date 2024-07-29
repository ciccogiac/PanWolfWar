#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlowerHideObject.generated.h"

class UBoxComponent;

UCLASS()
class PANWOLFWAR_API AFlowerHideObject : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlowerHideObject();

	void ChangeCollisionType(bool Enabled);

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Object_Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Collision_Box;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Border1_Box;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Border2_Box;
};
