#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrapplePoint.generated.h"

UCLASS()
class PANWOLFWAR_API AGrapplePoint : public AActor
{
	GENERATED_BODY()
	
public:	
	AGrapplePoint();

	void Activate(class UPandolFlowerComponent* Player);
	void Deactivate();
	void Use();
	const FVector GetLandingZone();

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


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GrapplePoint Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Detection_Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GrapplePoint Components", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* DeactivateZone_Box;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GrapplePoint Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* LandingZone_Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GrapplePoint Components", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* GrappleWidget;

	bool bActive = false;
	bool bUsed = false;
	class UPandolFlowerComponent* PlayerRef;
	FTimerHandle GrapplePoint_TimerHandle;
	class UGrapplePointWidget* WidgetRef;
};
