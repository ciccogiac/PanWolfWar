#include "Actors/InteractableObjectBase.h"

AInteractableObjectBase::AInteractableObjectBase()
{
	N_InteractBox = 1;
	InitializeBoxComponents();

	InteractableObjectType = EInteractableObjectTypes::EIOT_DefaultInteraction;
}
