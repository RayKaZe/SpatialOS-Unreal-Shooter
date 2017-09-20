#include "PositionBehaviour.h"
#include "Shooter.h"

#include "Coordinates.h"
#include "Worker.h"
#include "Vector.h"
#include "PositionComponent.h"
#include "PositionComponentUpdate.h"
#include "RotationComponent.h"
#include "RotationComponentUpdate.h"
#include "ShooterGameInstance.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "Quaternion.h"

UPositionBehaviour::UPositionBehaviour()
{
	PrimaryComponentTick.bCanEverTick = true;
	PositionComponent = CreateDefaultSubobject<UPositionComponent>(TEXT("PositionComponent"));
	RotationComponent = CreateDefaultSubobject<URotationComponent>(TEXT("RotationComponent"));
}

void UPositionBehaviour::BeginPlay()
{
	Super::BeginPlay();
	PositionComponent->OnCoordsUpdate.AddDynamic(this, &UPositionBehaviour::OnPositionUpdate);
	RotationComponent->OnQuatUpdate.AddDynamic(this, &UPositionBehaviour::OnRotationUpdate);
}

void UPositionBehaviour::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!IsPendingKill())
	{
		PositionComponent->OnCoordsUpdate.RemoveDynamic(this, &UPositionBehaviour::OnPositionUpdate);
		RotationComponent->OnQuatUpdate.RemoveDynamic(this, &UPositionBehaviour::OnRotationUpdate);
	}
}

void UPositionBehaviour::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (PositionComponent->GetAuthority() == EAuthority::Authoritative)
	{
		const auto rawUpdate = improbable::Position::Update();
		const auto update = NewObject<UPositionComponentUpdate>()->Init(rawUpdate)->SetCoords(USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinates(GetOwner()->GetActorLocation()));
		PositionComponent->SendComponentUpdate(update);
	}

	if (RotationComponent->GetAuthority() == EAuthority::Authoritative)
	{
		const auto rawUpdate = improbable::Rotation::Update();
		const auto currentRotationFquat = USpatialOSConversionFunctionLibrary::UnrealRotationToSpatialOsRotation(GetOwner()->GetActorQuat());
		const auto currentRotationQuat = NewObject<UQuaternion>()->Init(improbable::Quaternion(currentRotationFquat.X, currentRotationFquat.Y, currentRotationFquat.Z, currentRotationFquat.W));
		const auto update = NewObject<URotationComponentUpdate>()->Init(rawUpdate)->SetQuat(currentRotationQuat);
		RotationComponent->SendComponentUpdate(update);
	}
}

void UPositionBehaviour::OnPositionUpdate()
{
	if (PositionComponent->GetAuthority() == EAuthority::Authoritative)
	{
		return;
	}
	GetOwner()->SetActorLocation(USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(PositionComponent->Coords));
}

void UPositionBehaviour::OnRotationUpdate()
{
	if (RotationComponent->GetAuthority() == EAuthority::Authoritative)
	{
		return;
	}
	const auto newRotationQuat = RotationComponent->Quat;
	const auto newRotationFquat = USpatialOSConversionFunctionLibrary::SpatialOsRotationToUnrealRotation(FQuat(newRotationQuat->GetX(), newRotationQuat->GetY(), newRotationQuat->GetZ(), newRotationQuat->GetW()));
	GetOwner()->SetActorRotation(newRotationFquat);
}