#include "Enemy/AssassinableEnemy.h"

#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"

#include "Components/PandolfoComponent.h"
#include "Interfaces/CharacterInterface.h"

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
}

void AAssassinableEnemy::SetPlayerVisibility(bool NewVisibility)
{
	Super::SetPlayerVisibility(NewVisibility);

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
		if (!PandolfoComponent || !PandolfoComponent->IsActive()) return;

		PandolfoComponent->SetAssassinableEnemy(this);
		AssassinationWidget->SetVisibility(true);
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
		if (!PandolfoComponent || !PandolfoComponent->IsActive()) return;

		PandolfoComponent->SetAssassinableEnemy(nullptr);
		AssassinationWidget->SetVisibility(false);
	}
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
	//Niagara_DieEffect->SetActive(true);
	GetWorld()->GetTimerManager().SetTimer(Die_TimerHandle, [this]() {this->Destroy(); }, 5.f, true);
}

void AAssassinableEnemy::Assassinated(UAnimMontage* AssassinatedMontage)
{
	if (!AssassinatedMontage) return;
	UAnimInstance* OwningPlayerAnimInstance =GetMesh()->GetAnimInstance();
	if (!OwningPlayerAnimInstance) return;
	//if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	OwningPlayerAnimInstance->Montage_Play(AssassinatedMontage);

	Die();
	if(PandolfoComponent)
		PandolfoComponent->SetAssassinableEnemy(nullptr);
	AssassinationWidget->SetVisibility(false);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


