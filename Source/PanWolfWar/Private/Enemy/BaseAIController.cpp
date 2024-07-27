#include "Enemy/BaseAIController.h"
#include <PanWolfWar/PanWolfWarCharacter.h>

#include "PanWolfWar/DebugHelper.h"

ETeamAttitude::Type ABaseAIController::GetTeamAttitudeTowards(const AActor& other) const
{
	//const APanWolfWarCharacter* PanWolfWarCharacter = Cast<APanWolfWarCharacter>(&other);
	//if(PanWolfWarCharacter == nullptr)
	//{
	//	return  ETeamAttitude::Neutral;
	//}
	//else
	//{
	//	//if (PanWolfWarCharacter->IsHiding())
	//	//{
	//	//	Debug::Print("OVA");
	//	//	return ETeamAttitude::Neutral;
	//	//}
	//	//	
	//	//else
	//		return ETeamAttitude::Hostile;
	//}

	if(other.ActorHasTag("Player"))
		return ETeamAttitude::Hostile;

	return ETeamAttitude::Neutral;
}
