#include "Target.h"
#include "Shooter.h"

#include "PositionBehaviour.h"
#include "HealthBehaviour.h"
#include "PositionComponent.h"
#include "ShooterGameInstance.h"

ATarget::ATarget()
{
	PrimaryActorTick.bCanEverTick = true;
	PositionBehaviour = CreateDefaultSubobject<UPositionBehaviour>(TEXT("PositionBehaviour"));
	HealthBehaviour = CreateDefaultSubobject<UHealthBehaviour>(TEXT("HealthBehaviour"));
}

void ATarget::BeginPlay()
{
	Super::BeginPlay();
	GameInstance = Cast<UShooterGameInstance>(GetWorld()->GetGameInstance());
	check(GameInstance);
}