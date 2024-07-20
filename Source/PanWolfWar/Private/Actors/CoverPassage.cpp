#include "Actors/CoverPassage.h"

#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

#include "Interfaces/CharacterInterface.h"
#include "Components/PandolfoComponent.h"
#include "Components/SneakCoverComponent.h"

#include "PanWolfWar/DebugHelper.h"

ACoverPassage::ACoverPassage()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(GetRootComponent());
	BoxComponent->bHiddenInGame = true;
	BoxComponent->SetLineThickness(4.f);
	BoxComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(BoxComponent);
	ArrowComponent->ArrowLength = BoxComponent->GetScaledBoxExtent().X;
	
}

void ACoverPassage::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ACoverPassage::BoxCollisionEnter);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ACoverPassage::BoxCollisionExit);
}

void ACoverPassage::BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (CharacterInterface)
		{
			/*bIsCharacterInside = true;
			bRightState = CharacterInterface->GetTransformationComponent()->AddItemStamina(TransformationItemType, StaminaToAdd);*/
			UPandolfoComponent* PandolfoComponent = CharacterInterface->GetPandolfoComponent();
			if (PandolfoComponent && PandolfoComponent->IsActive())
			{
				USneakCoverComponent* SneakCoverComponent = PandolfoComponent->GetSneakCoverComponent();
				if (!SneakCoverComponent) return;

				switch (CoverPassageType)
				{
				case ECoverPassageType::ECPT_Wall:
					Debug::Print(TEXT("EnterWall"));
					if (PandolfoComponent->PandolfoState == EPandolfoState::EPS_Covering)
					{
						SneakCoverComponent->StopCover();
					}
					else
					{
						SneakCoverComponent->ActivateWallSearch();
					}
					
					break;
				case ECoverPassageType::ECPT_Narrow:
					//Debug::Print(TEXT("EnterNarrow"));
					if (PandolfoComponent->PandolfoState == EPandolfoState::EPS_Covering && SneakCoverComponent->IsNarrowing() && !bNarrowEntering)
					{
						SneakCoverComponent->StopNarrow(GetActorLocation() + GetActorForwardVector()*35.f);
						bNarrowEntering = false;
					}
					else if (PandolfoComponent->PandolfoState == EPandolfoState::EPS_Covering && !SneakCoverComponent->IsNarrowing() && !bNarrowEntering)
					{
						bNarrowEntering = true;
						SneakCoverComponent->StartNarrow();
						
					}
					else if(!bNarrowEntering)
					{
						bNarrowEntering = true;
						SneakCoverComponent->StartNarrow();
						
					}
					break;
				default:
					break;
				}
			}
			
		}
	}
}

void ACoverPassage::BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (CharacterInterface)
		{
			UPandolfoComponent* PandolfoComponent = CharacterInterface->GetPandolfoComponent();
			if (PandolfoComponent && PandolfoComponent->IsActive())
			{
				USneakCoverComponent* SneakCoverComponent = PandolfoComponent->GetSneakCoverComponent();
				if (!SneakCoverComponent) return;

				switch (CoverPassageType)
				{
				case ECoverPassageType::ECPT_Wall:
					if (PandolfoComponent->PandolfoState == EPandolfoState::EPS_Covering)
					{
						SneakCoverComponent->StopCover();
					}
					else
					{
						SneakCoverComponent->DeactivateWallSearch();
					}

					break;
				case ECoverPassageType::ECPT_Narrow:
					
					if (PandolfoComponent->PandolfoState == EPandolfoState::EPS_Covering && SneakCoverComponent->IsNarrowing() && bNarrowEntering )
					{
						//SneakCoverComponent->StopNarrow();
						bNarrowEntering = false;
					}
					//else
					//{
					//	//SneakCoverComponent->StartNarrow();
					//}
					break;
				default:
					break;
				}
			}

		}
	}
}

