#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "improbable/worker.h"
#include "EntityId.h"
#include "SpatialOSCommandResult.h"
#include "ShooterGameInstance.generated.h"

class USpatialOS;
class UCommander;
class UEntityRegistry;
class USpatialOSComponentUpdater;

UCLASS()
class SHOOTER_API UShooterGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
    virtual void Init() override;
	virtual void Shutdown() override;
	
	UPROPERTY()
	USpatialOS* SpatialOS;
	UPROPERTY()
	UEntityRegistry* EntityRegistry;
	UPROPERTY()
    USpatialOSComponentUpdater* SpatialOSComponentUpdater;
	UPROPERTY()
    UCommander* Commander;
	UPROPERTY()
	FTimerHandle MetricsReporterHandle;

	FString WorkerId;

	void ProcessOps(float DeltaTime);
	void ConnectToSpatialOS();
	void LogMessage(const FString& message);
	void LogError(const FString& message);
	bool IsConnected() const;

	UFUNCTION()
	void OnSpatialOsConnected();
	UFUNCTION()
	void OnSpatialOsDisconneced();

	void InitialiseSendingMetrics();
	void InitialiseEntitySpawnerBlock();

	void SpawnPlayer();
	UFUNCTION()
	void OnReserveEntityIdResponsePlayer(const FSpatialOSCommandResult& result, FEntityId reservedEntityId);
	UFUNCTION()
	void OnCreateEntityResponse(const FSpatialOSCommandResult& result, FEntityId createdEntityId);

	void SpawnTarget();
	UFUNCTION()
	void OnReserveEntityIdResponseTarget(const FSpatialOSCommandResult& result, FEntityId reservedEntityId);

	void DestroyPlayer(const AActor* actor);
	UFUNCTION()
	void OnDeleteEntityResponse(const FSpatialOSCommandResult& result);
};
