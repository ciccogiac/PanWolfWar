// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/PickUps/PanWarStoneBase.h"

void APanWarStoneBase::OnPickUpCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Implements<UInteractInterface>())
	{
		IInteractInterface* InteractInterface = Cast<IInteractInterface>(OtherActor);
		if (InteractInterface)
		{
			InteractInterface->ConsumeStone(StoneValue, StoneType);
			BP_OnStoneConsumed();
		}
	}

}