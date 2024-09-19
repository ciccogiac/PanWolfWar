#include "Enemy/AssassinableEnemy.h"

#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"

#include "Components/PandolfoComponent.h"
#include "Interfaces/CharacterInterface.h"

#include "Kismet/KismetMathLibrary.h"
#include "Components/PandolFlowerComponent.h"

AAssassinableEnemy::AAssassinableEnemy()
{
	Niagara_DieEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara_DieEffect"));
	Niagara_DieEffect->SetupAttachment(GetMesh(),FName("pelvis"));

	Niagara_BloodEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara_BloodEffect"));
	Niagara_BloodEffect->SetupAttachment(GetMesh(), FName("neck_01"));

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box_Assassination"));
	BoxComponent->SetupAttachment(GetMesh());
	BoxComponent->bHiddenInGame = true;
	BoxComponent->SetLineThickness(2.f);

	AssassinationWidget = CreateDefaultSubobject<UWidgetComponent>(*FString::Printf(TEXT("AssassinationWidget")));
	if (AssassinationWidget)
	{
		AssassinationWidget->SetWidgetSpace(EWidgetSpace::Screen);
		AssassinationWidget->SetVisibility(false);
		AssassinationWidget->SetupAttachment(GetRootComponent());
	}

	Assassin_Preview_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Assassin_Preview_Mesh"));
	Assassin_Preview_Mesh->SetupAttachment(GetMesh());
	Assassin_Preview_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Assassin_Preview_Mesh->bHiddenInGame = true;
}

void AAssassinableEnemy::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AAssassinableEnemy::BoxCollisionEnter);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AAssassinableEnemy::BoxCollisionExit);

	GetMesh()->SetVectorParameterValueOnMaterials(FName("Emission"), UKismetMathLibrary::Conv_LinearColorToVector(FLinearColor::Red));
}

void AAssassinableEnemy::SetEnemyAware(bool NewVisibility)
{
	Super::SetEnemyAware(NewVisibility);

	if (NewVisibility && PandolfoComponent)
	{
		PandolfoComponent->SetAssassinableEnemy(nullptr);
		AssassinationWidget->SetVisibility(false);
	}


}

void AAssassinableEnemy::BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bDied) return;
	if (bSeen) return;

	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (!CharacterInterface) return;
		
		PandolfoComponent = CharacterInterface->GetPandolfoComponent();
		UPandolFlowerComponent* PandolFlowerComponent = CharacterInterface->GetPandolFlowerComponent();
		//if (!PandolfoComponent || !PandolfoComponent->IsActive()) return;

		if (!PandolfoComponent || (!PandolfoComponent->IsActive() && !PandolFlowerComponent->IsActive()) ) return;

		PandolfoComponent->SetAssassinableEnemy(this);
		AssassinationWidget->SetVisibility(true);
		MarkAsTarget(true);
	}


}

void AAssassinableEnemy::BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

	if (bDied) return;

	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (!CharacterInterface) return;

		PandolfoComponent = CharacterInterface->GetPandolfoComponent();
		UPandolFlowerComponent* PandolFlowerComponent = CharacterInterface->GetPandolFlowerComponent();
		//if (!PandolfoComponent || !PandolfoComponent->IsActive()) return;
		if (!PandolfoComponent || (!PandolfoComponent->IsActive() && !PandolFlowerComponent->IsActive())) return;

		PandolfoComponent->SetAssassinableEnemy(nullptr);
		AssassinationWidget->SetVisibility(false);
		MarkAsTarget(false);
	}
}

void AAssassinableEnemy::DisableBoxAssassination()
{
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAssassinableEnemy::Killed()
{
	Niagara_BloodEffect->SetActive(true);
	GetMesh()->SetSimulatePhysics(true);
	UGameplayStatics::PlayWorldCameraShake(this, CameraShake, GetActorLocation(), 4444.f, 4444.f);
}

void AAssassinableEnemy::Die()
{
	Super::Die();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetWorld()->GetTimerManager().SetTimer(Die_TimerHandle, [this]() {this->Destroy(); }, 5.f, false);
}

void AAssassinableEnemy::Assassinated(UAnimMontage* AssassinatedMontage, UPandolfoComponent* _PandolfoComponent, bool AirAssassination)
{
	if (!AssassinatedMontage) return;
	UAnimInstance* OwningPlayerAnimInstance =GetMesh()->GetAnimInstance();
	if (!OwningPlayerAnimInstance) return;
	//if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;
	Super::Die();
	DisableBoxAssassination();

	if (AirAssassination)
		AirAssassinated();

	OwningPlayerAnimInstance->Montage_Play(AssassinatedMontage);

	
	if(_PandolfoComponent)
		_PandolfoComponent->SetAssassinableEnemy(nullptr);
	if (AirAssassination && _PandolfoComponent)
	{
		_PandolfoComponent->SetAssassinableEnemy(nullptr);
		_PandolfoComponent->SetAssassinableAirEnemy(nullptr);
	}


	AssassinationWidget->SetVisibility(false);
	MarkAsTarget(false);
	
}

void AAssassinableEnemy::AirAssassinated()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	const FRotator NewRotation = FRotator(0.f, UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Player->GetActorLocation()).Yaw , 0.f);
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), GetActorLocation(), NewRotation , false, false, 0.05, false, EMoveComponentAction::Move, LatentInfo);
	
}

void AAssassinableEnemy::MarkAsTarget(bool IsTargeted)
{
	const float EmissiveMultiplier = UKismetMathLibrary::SelectFloat(2.0, 0.f, IsTargeted);
	GetMesh()->SetScalarParameterValueOnMaterials(FName("Emissive Multiplier"), EmissiveMultiplier);
}


