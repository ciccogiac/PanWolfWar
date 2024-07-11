#include "Actors/GrapplePoint.h"

#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "UserWidgets/GrapplePointWidget.h"
#include "Components/Image.h"

#include "Kismet/KismetMathLibrary.h"

#include "Components/PandolFlowerComponent.h"

#include "PanWolfWar/DebugHelper.h"

#pragma region EngineFunctions

AGrapplePoint::AGrapplePoint()
{
	PrimaryActorTick.bCanEverTick = true;

	Detection_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Detection_Mesh"));
	Detection_Mesh->SetupAttachment(GetRootComponent());

	DeactivateZone_Box = CreateDefaultSubobject<UBoxComponent>(TEXT("DeactivateZone_Box"));
	DeactivateZone_Box->SetupAttachment(Detection_Mesh);

	LandingZone_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LandingZone_Mesh"));
	LandingZone_Mesh->SetupAttachment(Detection_Mesh);

	GrappleWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("GrappleWidget"));
	GrappleWidget->SetupAttachment(Detection_Mesh);
}

void AGrapplePoint::BeginPlay()
{
	Super::BeginPlay();

	DeactivateZone_Box->OnComponentBeginOverlap.AddDynamic(this, &AGrapplePoint::BoxCollisionEnter);
	DeactivateZone_Box->OnComponentEndOverlap.AddDynamic(this, &AGrapplePoint::BoxCollisionExit);

	UUserWidget* widget = GrappleWidget->GetWidget();
	if (widget)
	{
		WidgetRef = Cast<UGrapplePointWidget>(widget);
		WidgetRef->SetVisibility(ESlateVisibility::Hidden);
	}

	LandingZone_StartPos = LandingZone_Mesh->GetComponentLocation();

}

void AGrapplePoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bActive && !bUsed) CheckDistanceFromPlayer();
}

#pragma endregion



void AGrapplePoint::Activate(UPandolFlowerComponent* Player)
{
	if (bUsed) return;

	PlayerRef = Player;
	bActive = true;
	//Set widget visibility
	if(WidgetRef)
		WidgetRef->SetVisibility(ESlateVisibility::Visible);
}

void AGrapplePoint::Deactivate()
{
	if (bUsed) return;

	bActive = false;
	//Set widget visibility and stop animations
	if (WidgetRef)
	{
		WidgetRef->SetVisibility(ESlateVisibility::Hidden);
		WidgetRef->StopAllAnimations();
	}

}

void AGrapplePoint::CheckDistanceFromPlayer()
{	
	const float DistanceFromPlayer = (PlayerRef->GetOwner()->GetActorLocation() - GetActorLocation()).Length();
	const float ClimbedSize = UKismetMathLibrary::MapRangeClamped(DistanceFromPlayer,PlayerRef->GetGrappleThrowDistance(), PlayerRef->GetDetectionRadius(), 80.f, 10.f);

	if (WidgetRef)
	{
		const FVector2D NewSize = FVector2D(ClimbedSize, ClimbedSize);
		WidgetRef->Filling_Image->SetDesiredSizeOverride(NewSize);
		FLinearColor NewColor = ClimbedSize == 80.f ? FLinearColor::Green : FLinearColor::Gray;
		WidgetRef->Filling_Image->SetColorAndOpacity(NewColor);

	}
}

void AGrapplePoint::Use()
{
	bUsed = true;
	//widget animations
	if (WidgetRef)
		WidgetRef->PlayAnimation(WidgetRef->NodeUse,0.0f,1,EUMGSequencePlayMode::Forward,1.0f,true);

	GetWorld()->GetTimerManager().SetTimer(GrapplePoint_TimerHandle, this, &AGrapplePoint::Reactivate, 2.f, false);
}

void AGrapplePoint::Reactivate()
{
	bUsed = false;
}

const FVector AGrapplePoint::GetLandingZone()
{
	return LandingZone_Mesh->GetComponentLocation();
}

#pragma region BoxCollision

void AGrapplePoint::BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!PlayerRef || OtherActor != PlayerRef->GetOwner()) return;
	
	Detection_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGrapplePoint::BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!PlayerRef || OtherActor != PlayerRef->GetOwner()) return;

	Detection_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

}

#pragma endregion

void AGrapplePoint::ResetLandingZone()
{
	LandingZone_Mesh->SetSimulatePhysics(false);
	//Debug::Print(TEXT("LandingZone_HeightOffset: ") + FString::SanitizeFloat(LandingZone_HeightOffset));
	//const FVector Pos = Detection_Mesh->GetComponentLocation() + FVector(0.f, 0.f, LandingZone_HeightOffset);
	LandingZone_Mesh->SetWorldRotation(FRotator(0.f, 0.f, 0.f));
	LandingZone_Mesh->SetWorldLocation(LandingZone_StartPos);
}