#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Engine/TargetPoint.h>
#include "BaseEnemy.generated.h"

class UBehaviorTree;

UCLASS()
class PANWOLFWAR_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseEnemy();

	UFUNCTION(BlueprintCallable)
	virtual void SetPlayerVisibilityWidget(bool NewVisibility);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	bool bDied = false;
	bool bSeen = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination Components", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PlayerVisibleWidget;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TArray<ATargetPoint*> PatrolPoints;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsDead() const { return bDied; }
};
