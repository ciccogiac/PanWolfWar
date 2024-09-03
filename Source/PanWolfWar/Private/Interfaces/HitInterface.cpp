#include "Interfaces/HitInterface.h"
#include "Kismet/KismetMathLibrary.h"

FName IHitInterface::DirectionalHitReact(AActor* InAttacker, AActor* InVictim)
{
	check(InAttacker && InVictim);

	const FVector VictimForward = InVictim->GetActorForwardVector();
	const FVector VictimToAttackerNormalized = (InAttacker->GetActorLocation() - InVictim->GetActorLocation()).GetSafeNormal();

	const float DotResult = FVector::DotProduct(VictimForward, VictimToAttackerNormalized);
	float Theta = UKismetMathLibrary::DegAcos(DotResult);

	const FVector CrossProduct = FVector::CrossProduct(VictimForward, VictimToAttackerNormalized);

	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	if (Theta >= -45.f && Theta <= 45.f)
	{
		return FName("FromFront");
	}
	else if (Theta < -45.f && Theta >= -135.f)
	{
		return FName("FromLeft");
	}
	else if (Theta < -135.f || Theta>135.f)
	{
		return FName("FromBack");
	}
	else if (Theta > 45.f && Theta <= 135.f)
	{
		return FName("FromRight");
	}

	return FName("FromFront");

}
