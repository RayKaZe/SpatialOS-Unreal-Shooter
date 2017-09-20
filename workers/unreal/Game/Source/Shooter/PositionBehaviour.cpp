#include "PositionBehaviour.h"
#include "Shooter.h"

#include "Coordinates.h"
#include "Worker.h"
#include "Vector.h"
#include "PositionComponent.h"
#include "PositionComponentUpdate.h"
#include "ShooterGameInstance.h"
#include "SpatialOSConversionFunctionLibrary.h"

UPositionBehaviour::UPositionBehaviour()
{
	PrimaryComponentTick.bCanEverTick = true;
	PositionComponent = CreateDefaultSubobject<UPositionComponent>(TEXT("PositionComponent"));
}

void UPositionBehaviour::BeginPlay()
{
	Super::BeginPlay();
	PositionComponent->OnCoordsUpdate.AddDynamic(this, &UPositionBehaviour::OnPositionUpdate);
}

void UPositionBehaviour::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!IsPendingKill())
	{
		PositionComponent->OnCoordsUpdate.RemoveDynamic(this, &UPositionBehaviour::OnPositionUpdate);
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
}

void UPositionBehaviour::OnPositionUpdate()
{
	if (PositionComponent->GetAuthority() == EAuthority::Authoritative)
	{
		return;
	}
	GetOwner()->SetActorLocation(USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(PositionComponent->Coords));
}