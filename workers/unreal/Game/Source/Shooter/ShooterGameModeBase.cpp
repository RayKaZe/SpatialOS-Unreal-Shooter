#include "ShooterGameModeBase.h"
#include "Shooter.h"

#include "FpsHUD.h"
#include "ShooterGameInstance.h"
#include "UObject/ConstructorHelpers.h"

AShooterGameModeBase::AShooterGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;

	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/EntityBlueprints/E_PlayerCharacter"));
	DefaultPawnClass = NULL;// PlayerPawnClassFinder.Class;
	HUDClass = AFpsHUD::StaticClass();
}

void AShooterGameModeBase::Tick(float DeltaTime)
{
	AGameModeBase::Tick(DeltaTime);
	if (!ShooterGameInstance->IsConnected())
	{
		return;
	}
	ShooterGameInstance->ProcessOps(DeltaTime);
}

void AShooterGameModeBase::BeginPlay()
{
	AGameModeBase::BeginPlay();
	ShooterGameInstance = Cast<UShooterGameInstance>(GetWorld()->GetGameInstance());
}

void AShooterGameModeBase::StartPlay()
{
	Super::StartPlay();
}
