#include "EntityTemplates.h"
#include "Shooter.h"

#include "EntityAclData.h"
#include "EntityBuilder.h"
#include "EntityTemplate.h"
#include "ImprobableWorkerAttributeSetList.h"
#include "PositionComponent.h"
#include "PositionData.h"
#include "WorkerAttributeSet.h"
#include "WorkerRequirementSet.h"

UEntityTemplate* UEntityTemplates::CreatePlayerCharacterTemplate(const FString& workerId, const FVector& position)
{
	improbable::WorkerAttributeSet unrealWorkerAttributeSet{ { worker::List<std::string>{"UnrealWorker"} } };
	improbable::WorkerAttributeSet unrealClientAttributeSet{ { worker::List<std::string>{"UnrealClient"} } };
	improbable::WorkerAttributeSet unrealSpecificClientAttributeSet{ { worker::List<std::string>{std::string("workerId:") + TCHAR_TO_UTF8(*workerId)} } };

	improbable::WorkerRequirementSet unrealWorkerWritePermission{ { unrealWorkerAttributeSet } };
	improbable::WorkerRequirementSet unrealClientWritePermission{ { unrealClientAttributeSet } };
	improbable::WorkerRequirementSet specificClientWritePermission{	{ unrealSpecificClientAttributeSet } };
	improbable::WorkerRequirementSet anyWorkerReadPermission{ { unrealClientAttributeSet, unrealWorkerAttributeSet } };

	improbable::PositionData positionData(::improbable::Coordinates(position.X, position.Y, position.Z));
	improbable::MetadataData metadataData("E_PlayerCharacter");

	auto entity = ::improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(positionData, specificClientWritePermission)
		.AddMetadataComponent(NewObject<UMetadataData>()->Init(metadataData)->GetUnderlying())
		.SetPersistence(false)
		.SetReadAcl(anyWorkerReadPermission)
		.AddComponent<::improbable::Rotation>(::improbable::RotationData(::improbable::Quaternion(0, 0, 0, 0)), specificClientWritePermission)
		.AddComponent<::improbable::PlayerControls>(::improbable::PlayerControlsData(), specificClientWritePermission)
		.AddComponent<::improbable::Health>(::improbable::HealthData(100), specificClientWritePermission)
		.Build();

	UEntityTemplate* EntityTemplate = NewObject<UEntityTemplate>()->Init(entity);

	return EntityTemplate;
}

UEntityTemplate* UEntityTemplates::CreateTargetTemplate(const FString& workerId, const FVector& position)
{
	improbable::WorkerAttributeSet unrealWorkerAttributeSet{ { worker::List<std::string>{"UnrealWorker"} } };
	improbable::WorkerAttributeSet unrealClientAttributeSet{ { worker::List<std::string>{"UnrealClient"} } };
	improbable::WorkerAttributeSet unrealSpecificClientAttributeSet{ { worker::List<std::string>{std::string("workerId:") + TCHAR_TO_UTF8(*workerId)} } };

	improbable::WorkerRequirementSet unrealWorkerWritePermission{ { unrealWorkerAttributeSet } };
	improbable::WorkerRequirementSet unrealClientWritePermission{ { unrealClientAttributeSet } };
	improbable::WorkerRequirementSet specificClientWritePermission{ { unrealSpecificClientAttributeSet } };
	improbable::WorkerRequirementSet anyWorkerReadPermission{ { unrealClientAttributeSet, unrealWorkerAttributeSet } };

	improbable::PositionData positionData(::improbable::Coordinates(position.X, position.Y, position.Z));
	improbable::MetadataData metadataData("E_Target");

	auto entity = ::improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(positionData, specificClientWritePermission)
		.AddMetadataComponent(NewObject<UMetadataData>()->Init(metadataData)->GetUnderlying())
		.SetPersistence(false)
		.SetReadAcl(anyWorkerReadPermission)
		.AddComponent<::improbable::Rotation>(::improbable::RotationData(::improbable::Quaternion(0, 0, 0, 0)), specificClientWritePermission)
		.AddComponent<::improbable::Health>(::improbable::HealthData(100), specificClientWritePermission)
		.Build();

	UEntityTemplate* EntityTemplate = NewObject<UEntityTemplate>()->Init(entity);

	return EntityTemplate;
}