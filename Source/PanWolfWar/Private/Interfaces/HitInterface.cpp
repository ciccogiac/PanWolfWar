#include "Interfaces/HitInterface.h"

FName IHitInterface::DirectionalHitReact(const AActor* ActorReacting, const FVector& ImpactPoint)
{
	const FVector Forward = ActorReacting->GetActorForwardVector();
	//Lower impact point to the enemy actor location z
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, ActorReacting->GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - ActorReacting->GetActorLocation()).GetSafeNormal();


	//Forward * ToHit = |Forward| |ToHit| * cos(theta)
	// |Forward| = 1 , |ToHit| = 1 , so Forward * ToHit = cos(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	//Take the inverse cosine (Arc-Cosine) of cos (theta) to get theta
	double Theta = FMath::Acos(CosTheta);
	//convert from radians to degrees
	Theta = FMath::RadiansToDegrees(Theta);

	//if CrossProduct points down , Theta should be negative
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	FName Section("FromBack");

	if (Theta >= -45.f && Theta < 45.f) { Section = FName("FromFront"); }
	else if (Theta >= -135.f && Theta < -45.f) { Section = FName("FromLeft"); }
	else if (Theta >= 45.f && Theta < 135.f) { Section = FName("FromRight"); }

	return Section;
	//Debug::Print(TEXT("Hit From : ") + Section.ToString());
}
