#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PanWarWeaponBase.generated.h"

class UBoxComponent;

UCLASS()
class PANWOLFWAR_API APanWarWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	APanWarWeaponBase();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
	UBoxComponent* WeaponCollisionBox;

public:
	FORCEINLINE UBoxComponent* GetWeaponCollisionBox() const { return WeaponCollisionBox; }

};
