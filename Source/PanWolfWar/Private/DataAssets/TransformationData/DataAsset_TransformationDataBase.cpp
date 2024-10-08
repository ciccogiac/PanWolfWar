#include "DataAssets/TransformationData/DataAsset_TransformationDataBase.h"

FTransformationCharacterData UDataAsset_TransformationDataBase::GetTransformationCharacterData(ETransformationState TransformationState)
{
    if (TransformationsCharactersData.Contains(TransformationState))
    {
        return *TransformationsCharactersData.Find(TransformationState);
    }

    UE_LOG(LogTemp, Warning, TEXT("TransformationState non trovato!"));

    return FTransformationCharacterData();
}
