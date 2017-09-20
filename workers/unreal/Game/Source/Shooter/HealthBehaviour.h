#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthBehaviour.generated.h"

class UHealthComponent;
class UTakeDamageCommandCommandResponder;
class UShotPoint;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_API UHealthBehaviour : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthBehaviour();
	virtual void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	TSubclassOf<AActor> ExplosionEffect;
	UPROPERTY()
	UHealthComponent* HealthComponent;

	void TakeDamage(const FVector& position, const FVector& start);
	UFUNCTION()
	void OnTakeDamageCommandRequest(UTakeDamageCommandCommandResponder* responder);
	UFUNCTION()
	void OnTakeDamageEvent(UShotPoint* newEvent);
};
