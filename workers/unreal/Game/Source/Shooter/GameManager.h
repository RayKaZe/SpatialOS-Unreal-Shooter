#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameManager.generated.h"

UCLASS()
class SHOOTER_API AGameManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AGameManager();
	virtual void BeginPlay() override;
};
