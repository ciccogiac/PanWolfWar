#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrapplePoint.generated.h"

UENUM(BlueprintType)
enum class EGrapplePointType : uint8
{
	EGPT_Grapple UMETA(DisplayName = "Grapple"),
	EGPT_Swing UMETA(DisplayName = "Swing")
};

UCLASS()
class PANWOLFWAR_API AGrapplePoint : public AActor
{
	GENERATED_BODY()

#pragma region Functions

public:
	AGrapplePoint();

	void Activate(class UPandolFlowerComponent* Player);
	void Deactivate();
	void Use();
	const FVector GetLandingZone();
	void ResetLandingZone();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:

	void CheckDistanceFromPlayer();
	void Reactivate();

	UFUNCTION()
	void BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

#pragma endregion

#pragma region Variables

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GrapplePoint", meta = (AllowPrivateAccess = "true"))
	EGrapplePointType GrapplePointType = EGrapplePointType::EGPT_Grapple;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GrapplePoint Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* LandingZone_Mesh;

private:

	#pragma region Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GrapplePoint Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Detection_Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GrapplePoint Components", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* DeactivateZone_Box;



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GrapplePoint Components", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* GrappleWidget;

	FVector  LandingZone_StartPos;

	class UGrapplePointWidget* WidgetRef;

	class UPandolFlowerComponent* PlayerRef;
	#pragma endregion




	bool bActive = false;
	bool bUsed = false;
	FTimerHandle GrapplePoint_TimerHandle;

#pragma endregion

};
