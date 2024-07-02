// Fill out your copyright notice in the Description page of Project Settings.


#include "Interfaces/InteractInterface.h"


FName IInteractInterface::GetSelectedFName(ETransformationObjectTypes TransformationObjectType)
{
		switch (TransformationObjectType)
	{
	case ETransformationObjectTypes::ETOT_Pandolfo_Object:
		return FName(TEXT("Pandolfo_Object"));
	case ETransformationObjectTypes::ETOT_PanWolf_Object:
		return FName(TEXT("PanWolf_Object"));
	case ETransformationObjectTypes::ETOT_PandolFlower_Object:
		return FName(TEXT("PandolFlower_Object"));
	case ETransformationObjectTypes::ETOT_PanBird_Object:
		return FName(TEXT("PanBird_Object"));
	default:
		return FName(TEXT("Pandolfo_Object"));
	}
}
