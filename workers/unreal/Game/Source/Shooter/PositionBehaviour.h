#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PositionBehaviour.generated.h"

class UPositionComponent;
class URotationComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_API UPositionBehaviour : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPositionBehaviour();
	virtual void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	UPositionComponent* PositionComponent;
	UFUNCTION()
	void OnPositionUpdate();

	UPROPERTY()
	URotationComponent* RotationComponent;
	UFUNCTION()
	void OnRotationUpdate();
};
