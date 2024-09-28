#include "Controllers/PanWarAIController_Advanced.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BehaviorTree/BlackboardComponent.h"


#include "Enemy/BaseEnemy.h"
#include "Kismet/GameplayStatics.h"

#include "PanWolfWar/DebugHelper.h"

APanWarAIController_Advanced::APanWarAIController_Advanced(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer) 
{
	AISenseConfig_Sight->SightRadius = 1200.f;
	AISenseConfig_Sight->LoseSightRadius = 0.f;
	AISenseConfig_Sight->PeripheralVisionAngleDegrees = 60.f;

	AISenseConfig_Hearing = CreateDefaultSubobject<UAISenseConfig_Hearing>("EnemySenseConfig_Hearing");
	AISenseConfig_Hearing->DetectionByAffiliation.bDetectEnemies = true;
	AISenseConfig_Hearing->DetectionByAffiliation.bDetectFriendlies = false;
	AISenseConfig_Hearing->DetectionByAffiliation.bDetectNeutrals = false;
	AISenseConfig_Hearing->HearingRange = 650.f;

	AISenseConfig_Damage = CreateDefaultSubobject<UAISenseConfig_Damage>("EnemySenseConfig_Damage");

	// Assicurati che `EnemyPerceptionComponent` sia valido
	if (EnemyPerceptionComponent)
	{
		// Aggiungi i nuovi sensi al componente di percezione
		EnemyPerceptionComponent->ConfigureSense(*AISenseConfig_Hearing);
		EnemyPerceptionComponent->ConfigureSense(*AISenseConfig_Damage);

		// Imposta di nuovo il senso dominante se necessario
		EnemyPerceptionComponent->SetDominantSense(UAISenseConfig_Sight::StaticClass());
	}
	
	
}

void APanWarAIController_Advanced::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (OwnerBaseEnemy)
	{
		OwnerBaseEnemy->OnEnemyDeath.AddDynamic(this, &ThisClass::OnPossessedPawnDeath);
	}

}

void APanWarAIController_Advanced::OnPossessedPawnDeath()
{
	if (EnemyPerceptionComponent)
	{
		// Disattiva il PerceptionComponent
		EnemyPerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
		EnemyPerceptionComponent->SetSenseEnabled(UAISense_Hearing::StaticClass(), false);
		EnemyPerceptionComponent->SetSenseEnabled(UAISense_Damage::StaticClass(), false);

		if(CharacterInterface)
			CharacterInterface->RemoveEnemyAware(GetPawn());

		if (OwnerBaseEnemy)
		OwnerBaseEnemy->SetEnemyAware(false);

		GetWorld()->GetTimerManager().ClearTimer(FoundTarget_TimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(LostTarget_TimerHandle);
		Awareness = 0.f;
		OwnerBaseEnemy->UpdateCurrentEnemyAwareness(0.f);
	}
}

void APanWarAIController_Advanced::OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{	
	if (!CharacterInterface) { CharacterInterface = Cast<ICharacterInterface>(Actor); }
	if (!CharacterInterface || !OwnerBaseEnemy) return;

	if (Stimulus.WasSuccessfullySensed() && !OwnerBaseEnemy->IsEnemyAware())
	{
		if (Stimulus.Type.Name == "Default__AISense_Sight")
		{
			/*UE_LOG(LogTemp, Warning, TEXT("AI perceived sight from: %s"), *Actor->GetName());*/

			if (CharacterInterface->IsHiding())
			{
				GetWorld()->GetTimerManager().ClearTimer(FoundTarget_TimerHandle);
				EnemyPerceptionComponent->ForgetAll();
				return;
			}

			else
			{
				GetWorld()->GetTimerManager().ClearTimer(LostTarget_TimerHandle);
				GetWorld()->GetTimerManager().SetTimer(FoundTarget_TimerHandle, [this, Actor]() {this->IncrementAwareness(Actor); }, FoundTarget_TimerLoop, true);
				/*SetNewTargetActor(Actor);*/
			}

		}

		// DamageSense
		if (Stimulus.Type.Name == "Default__AISense_Damage")
		{
			/*UE_LOG(LogTemp, Warning, TEXT("AI perceived damage from: %s"), *Actor->GetName());*/
			SetNewTargetActor(Actor);
		} 

		// HearingSense
		if (Stimulus.Type.Name == "Default__AISense_Hearing")
		{
			/*UE_LOG(LogTemp, Warning, TEXT("AI perceived hear from: %s"), *Actor->GetName());*/
			SetNewTargetActor(Actor);
		}

	
		
	}

	//Lose Target View

	else if(!Stimulus.WasSuccessfullySensed())
	{
		if (Stimulus.Type.Name == "Default__AISense_Sight")
		{
			if (OwnerBaseEnemy->IsEnemyAware())
			{

				float DistanceFromLastSeen = FVector::Dist(OwnerBaseEnemy->GetActorLocation(), Stimulus.StimulusLocation);
				if (DistanceFromLastSeen < MinRangeToConvalidateLost) return;

				UE_LOG(LogTemp, Warning, TEXT("AI perceived sight Lost Actor: %s"), *Actor->GetName());
				SetBlackboardTargetActor(nullptr);
				CharacterInterface->RemoveEnemyAware(GetPawn());
				OwnerBaseEnemy->SetEnemyAware(false);
			}
			else
			{
				GetWorld()->GetTimerManager().ClearTimer(FoundTarget_TimerHandle);
				GetWorld()->GetTimerManager().SetTimer(LostTarget_TimerHandle, [this]() {this->DecrementAwareness(); }, LostTarget_TimerLoop, true);
			}
				
		}
	}

}

void APanWarAIController_Advanced::IncrementAwareness(AActor* DetectedActor)
{
	if (!DetectedActor || !OwnerBaseEnemy) return;

	float Distance = FVector::Dist(OwnerBaseEnemy->GetActorLocation(), DetectedActor->GetActorLocation());
	float DistanceFactor = FMath::Clamp(1.0f - (Distance / MaxAwarenessDistance), 0.0f, 1.0f);
	Awareness += AwarenessIncrementRate * DistanceFactor ;
	Awareness = FMath::Clamp(Awareness, 0.0f, 1.0f); // Limita il valore tra 0 e 1
	

	/*Awareness = FMath::Clamp(Awareness + AwarenessIncrementRate, 0.f, 1.f);*/
	OwnerBaseEnemy->UpdateCurrentEnemyAwareness(Awareness);
	if (Awareness >= 1)
	{
		GetWorld()->GetTimerManager().ClearTimer(FoundTarget_TimerHandle);
		SetNewTargetActor(DetectedActor);
	}
}

void APanWarAIController_Advanced::DecrementAwareness()
{
	//float Distance = FVector::Dist(OwnerBaseEnemy->GetActorLocation(), DetectedActor->GetActorLocation());
	//float DistanceFactor = FMath::Clamp(1.0f - (Distance / MaxAwarenessDistance), 0.0f, 1.0f);
	//Awareness -= AwarenessDecrementRate * (1.0f - DistanceFactor) * GetWorld()->GetDeltaSeconds();
	//Awareness = FMath::Clamp(Awareness, 0.0f, 1.0f); // Limita il valore tra 0 e 1

	Awareness = FMath::Clamp(Awareness - AwarenessDecrementRate, 0.f, 1.f);
	OwnerBaseEnemy->UpdateCurrentEnemyAwareness(Awareness);
	if (Awareness <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(LostTarget_TimerHandle);
	}
}

void APanWarAIController_Advanced::SetNewTargetActor(AActor* Actor)
{
	if (!Actor) return;
	if (!CharacterInterface) { CharacterInterface = Cast<ICharacterInterface>(Actor); }
	if (!CharacterInterface || !OwnerBaseEnemy) return;

	GetWorld()->GetTimerManager().ClearTimer(FoundTarget_TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(LostTarget_TimerHandle);
	Awareness = 1.f;
	OwnerBaseEnemy->UpdateCurrentEnemyAwareness(1.f);

	SetBlackboardTargetActor(Actor);
	CharacterInterface->AddEnemyAware(GetPawn());
	OwnerBaseEnemy->SetEnemyAware(true);

	NotifyNearbyAllies(Actor);
}

void APanWarAIController_Advanced::SetBlackboardTargetActor(AActor* Actor)
{
	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		if (!BlackboardComponent->GetValueAsObject(FName("TargetActor")) && Actor)
		{
			BlackboardComponent->SetValueAsObject(FName("TargetActor"), Actor);
		}

		else if (BlackboardComponent->GetValueAsObject(FName("TargetActor")) && !Actor)
		{
			BlackboardComponent->ClearValue(FName("TargetActor"));
		}
	}
}

void APanWarAIController_Advanced::NotifyNearbyAllies(AActor* DetectedActor)
{

	if (!OwnerBaseEnemy || !DetectedActor) return;

	const FVector Start = OwnerBaseEnemy->GetActorLocation();
	const FVector End = Start + OwnerBaseEnemy->GetActorForwardVector();
	TArray<FHitResult> Hit;
	TArray<TEnumAsByte<EObjectTypeQuery>> Objects;
	Objects.Add(EObjectTypeQuery::ObjectTypeQuery3);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(DetectedActor);
	ActorsToIgnore.Add(OwnerBaseEnemy);

	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	const bool bBlocked = UKismetSystemLibrary::BoxTraceMultiForObjects(this, Start, End , AlliesResearch_BoxExtent, OwnerBaseEnemy->GetActorRotation(), Objects, false, ActorsToIgnore, DebugTraceType, Hit, true);

	if (!bBlocked || Hit.Num() <= 0) return;

	for (size_t i = 0; i < Hit.Num(); i++)
	{
		FHitResult EnemyHit = Hit[i];
		if (EnemyHit.bBlockingHit)
		{
			APanWarAIController_Advanced* EnemyController = Cast<APanWarAIController_Advanced>(EnemyHit.GetActor()->GetInstigatorController());
			if (EnemyController)
			{
				if (EnemyController->OwnerBaseEnemy->IsEnemyAware()) continue;
				UE_LOG(LogTemp, Warning, TEXT("%s sta avvisando %s della presenza del bersaglio!"), *GetPawn()->GetName(), *EnemyController->GetName());
				EnemyController->SetNewTargetActor(DetectedActor);
			}
		}

	}
}