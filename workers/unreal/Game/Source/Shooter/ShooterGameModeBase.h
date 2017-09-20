#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameModeBase.generated.h"

class UShooterGameInstance;

UCLASS()
class SHOOTER_API AShooterGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AShooterGameModeBase();
	virtual void StartPlay() override;
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	UPROPERTY()
    UShooterGameInstance* ShooterGameInstance;
};
