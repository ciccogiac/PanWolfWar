// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractInterface.h"
#include "InteractableObject.generated.h"

class UCharacterMovementComponent;
class UCapsuleComponent;
struct FInputActionValue;
class IInteractInterface;

UCLASS()
class PANWOLFWAR_API AInteractableObject : public AActor
{
	GENERATED_BODY()
	
public:	
	AInteractableObject();

	virtual bool Interact(ACharacter* _CharacterOwner = nullptr) ;
	virtual void Move(const FInputActionValue& Value);

protected:
	virtual void BeginPlay() override;

	void InitializeBoxComponents();
	FVector2D Get8DirectionVector(const FVector2D& InputVector);

	ACharacter* CharacterOwner;

	USceneComponent* InteractWidget;
	IInteractInterface* InteractInterface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact Params", meta = (AllowPrivateAccess = "true"))
	ETransformationObjectTypes TransformationObjectType = ETransformationObjectTypes::ETOT_Pandolfo_Object;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact Params", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* BoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact Params", meta = (AllowPrivateAccess = "true"))
	TArray<class UBoxComponent*> BoxComponentArray;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact Params", meta = (AllowPrivateAccess = "true"))
	TArray<class UArrowComponent*> ArrowComponentArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact Params", meta = (AllowPrivateAccess = "true"))
	TArray<class UWidgetComponent*> InteractionWidgetArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact Params", meta = (AllowPrivateAccess = "true"))
	int N_InteractBox=4;

	UFUNCTION()
	virtual void BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	FORCEINLINE void SetInteractWidget(USceneComponent* _InteractWidget) { InteractWidget = _InteractWidget; };


public:
	FORCEINLINE void SetInteractWidgetVisibility(bool bVisibility) { if(InteractWidget) InteractWidget->SetVisibility(bVisibility); };
	FORCEINLINE void ResetBox() { BoxComponent = nullptr; };
	FORCEINLINE ETransformationObjectTypes GetTransformationObjectType() const { return TransformationObjectType; };
	
};
