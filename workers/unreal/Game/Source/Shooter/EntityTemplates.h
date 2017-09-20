#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EntityTemplates.generated.h"

class UEntityTemplate;

UCLASS()
class SHOOTER_API UEntityTemplates : public UObject
{
	GENERATED_BODY()
	
public:
	static UEntityTemplate* CreatePlayerCharacterTemplate(const FString& workerId, const FVector& position);
	static UEntityTemplate* CreateTargetTemplate(const FString& workerId, const FVector& position);
};
