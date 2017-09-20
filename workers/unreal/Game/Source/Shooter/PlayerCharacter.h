#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpatialOSCommandResult.h"
#include "EntityId.h"
#include "PlayerCharacter.generated.h"

class UPositionBehaviour;
class UShooterGameInstance;
class UPlayerControlsComponent;
class UEmpty;

UCLASS()
class SHOOTER_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	virtual void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);

	UShooterGameInstance* GameInstance;
	void DestroySelf();
	void SpawnTarget();

	UPROPERTY()
	UPositionBehaviour* PositionBehaviour;
	UFUNCTION()
	void OnPositionAuthorityChange(EAuthority newAuthority);

	UPROPERTY()
	UPlayerControlsComponent* PlayerControls;
	UFUNCTION()
	void OnShotEventServer(UShot* newEvent);
	UFUNCTION()
	void OnShotEventClient(UShot* newEvent);

	UFUNCTION(BlueprintCallable)
	void FireLeft();
	UFUNCTION(BlueprintCallable)
    void FireRight();
	void SendTakeDamageCommand(const FEntityId& targetEntityId, const FVector& start, const FVector& end);
	UFUNCTION()
	void OnTakeDamageCommandResult(const FSpatialOSCommandResult& result, UEmpty* response);
	
	void GetLineTracePoints(FVector& out_startPoint, FVector& out_endPoint);
	bool DoLineTrace(FHitResult& out_hitResult, const FVector& start, const FVector& end);
};
