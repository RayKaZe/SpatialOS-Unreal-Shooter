#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Target.generated.h"

class UPositionBehaviour;
class UHealthBehaviour;
class UShooterGameInstance;

UCLASS()
class SHOOTER_API ATarget : public AActor
{
	GENERATED_BODY()
	
public:	
	ATarget();

	virtual void BeginPlay() override;

	UShooterGameInstance* GameInstance;
	UPROPERTY()
	UPositionBehaviour* PositionBehaviour;
	UPROPERTY()
	UHealthBehaviour* HealthBehaviour;
};
   