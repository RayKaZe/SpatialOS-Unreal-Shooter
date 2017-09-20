#include "PlayerCharacter.h"
#include "Shooter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Target.h"
#include "PositionBehaviour.h"
#include "ShooterGameInstance.h"
#include "PositionComponent.h"
#include "Shot.h"
#include "PlayerControlsComponent.h"
#include "PlayerControlsComponentUpdate.h"
#include "EntityId.h"
#include "Commander.h"
#include "ShotPoint.h"
#include "HealthComponent.h"
#include "HealthBehaviour.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PositionBehaviour = CreateDefaultSubobject<UPositionBehaviour>(TEXT("PositionBehaviour"));
	PlayerControls = CreateDefaultSubobject<UPlayerControlsComponent>(TEXT("PlayerControlsComponent"));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	GameInstance = Cast<UShooterGameInstance>(GetWorld()->GetGameInstance());
	check(GameInstance);
	PositionBehaviour->PositionComponent->OnAuthorityChange.AddDynamic(this, &APlayerCharacter::OnPositionAuthorityChange);
	PlayerControls->OnShotEventServer.AddDynamic(this, &APlayerCharacter::OnShotEventServer);
	PlayerControls->OnShotEventClient.AddDynamic(this, &APlayerCharacter::OnShotEventClient);
}

void APlayerCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	PositionBehaviour->PositionComponent->OnAuthorityChange.RemoveDynamic(this, &APlayerCharacter::OnPositionAuthorityChange);
	PlayerControls->OnShotEventServer.RemoveDynamic(this, &APlayerCharacter::OnShotEventServer);
	PlayerControls->OnShotEventClient.RemoveDynamic(this, &APlayerCharacter::OnShotEventClient);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("PlayerActionQ", IE_Pressed, this, &APlayerCharacter::DestroySelf);
	PlayerInputComponent->BindAction("PlayerActionE", IE_Pressed, this, &APlayerCharacter::SpawnTarget);
}

void APlayerCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void APlayerCharacter::DestroySelf()
{
	GameInstance->DestroyPlayer(this);
}

void APlayerCharacter::SpawnTarget()
{
	GameInstance->SpawnTarget();
}

void APlayerCharacter::OnPositionAuthorityChange(EAuthority newAuthority)
{
	if (newAuthority == EAuthority::Authoritative)
	{
		APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		Controller->UnPossess();
		Controller->Possess(this);
	}
}

void APlayerCharacter::OnShotEventServer(UShot* newEvent)
{
	DrawDebugLine(GetWorld(), newEvent->GetStart(), newEvent->GetEnd(), FColor::Red, false, 15.f);

	if (PlayerControls->GetAuthority() == EAuthority::Authoritative)
	{
		FHitResult HitResult(ForceInit);
		if (!DoLineTrace(HitResult, newEvent->GetStart(), newEvent->GetEnd()))
		{
			return;
		}
		ATarget* Target = Cast<ATarget>((HitResult.Actor).Get());
		if (!Target)
		{
			return;
		}
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Server Hit: ") + HitResult.Actor->GetName());
		
		if (Target->HealthBehaviour->HealthComponent->GetAuthority() == EAuthority::Authoritative)
		{
			Target->HealthBehaviour->TakeDamage(HitResult.ImpactPoint, newEvent->GetStart());
		}
		else
		{
			auto TargetEntityId = FEntityId(Target->HealthBehaviour->HealthComponent->GetEntityId());
			SendTakeDamageCommand(TargetEntityId, HitResult.ImpactPoint, newEvent->GetStart());
		}
	}
}

void APlayerCharacter::OnShotEventClient(UShot* newEvent)
{
	DrawDebugLine(GetWorld(), newEvent->GetStart(), newEvent->GetEnd(), FColor::Blue, false, 15.f);
}

void APlayerCharacter::FireLeft()
{
	FVector Start;
	FVector End;
	GetLineTracePoints(Start, End);

	const auto rawUpdate = improbable::PlayerControls::Update();
	const auto event = NewObject<UShot>()->Init(improbable::Shot(::improbable::Vector3f(0, 0, 0), ::improbable::Vector3f(0, 0, 0)))->SetStart(Start)->SetEnd(End);
	const auto update =	NewObject<UPlayerControlsComponentUpdate>()->Init(rawUpdate)->AddShotEventServer(event);
	PlayerControls->SendComponentUpdate(update);
}

void APlayerCharacter::FireRight()
{
	FVector Start;
	FVector End;
	GetLineTracePoints(Start, End);
	// Immediately draw a line
	DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 15.f);

	const auto rawUpdate = improbable::PlayerControls::Update();
	const auto event = NewObject<UShot>()->Init(improbable::Shot(::improbable::Vector3f(0, 0, 0), ::improbable::Vector3f(0, 0, 0)))->SetStart(Start)->SetEnd(End);
	const auto update = NewObject<UPlayerControlsComponentUpdate>()->Init(rawUpdate)->AddShotEventClient(event);
	PlayerControls->SendComponentUpdate(update);

	FHitResult HitResult(ForceInit);
	if (!DoLineTrace(HitResult, Start, End))
	{
		return;
	}
	ATarget* Target = Cast<ATarget>((HitResult.Actor).Get());
	if (!Target)
	{
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Client Hit: ") + HitResult.Actor->GetName());
	
	auto TargetEntityId = FEntityId(Target->PositionBehaviour->PositionComponent->GetEntityId());
	SendTakeDamageCommand(TargetEntityId, HitResult.ImpactPoint, Start);
}

void APlayerCharacter::SendTakeDamageCommand(const FEntityId& targetEntityId, const FVector& start, const FVector& end)
{
	auto request = NewObject<UShot>()->Init(improbable::Shot(::improbable::Vector3f(0, 0, 0), ::improbable::Vector3f(0, 0, 0)))->SetEnd(end)->SetStart(start);
	auto callback = FTakeDamageCommandCommandResultDelegate();
	callback.BindDynamic(this, &APlayerCharacter::OnTakeDamageCommandResult);
	GameInstance->Commander->TakeDamageCommand(targetEntityId, request, callback, 0);
}

void APlayerCharacter::OnTakeDamageCommandResult(const FSpatialOSCommandResult& result, UEmpty* response)
{
	if (!result.Success())
	{
		GameInstance->LogError(TEXT("OnTakeDamageCommandResult failed."));
		return;
	}
}

void APlayerCharacter::GetLineTracePoints(FVector& out_start, FVector& out_end)
{
	FVector CameraLocation;
	FRotator CameraRotation;
	GetActorEyesViewPoint(CameraLocation, CameraRotation);

	FVector GunOffset(90.f, 25.f, -12.f);
	out_start = CameraLocation + CameraRotation.RotateVector(GunOffset);
	out_end = CameraLocation + (CameraRotation.Vector() * 10000.f);
}

bool APlayerCharacter::DoLineTrace(FHitResult& out_hitResult, const FVector& Start, const FVector& End)
{
	FCollisionQueryParams TraceParams(FName(TEXT("GunShot")), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;
	return GetWorld()->LineTraceSingleByChannel(out_hitResult, Start, End, ECC_Visibility, TraceParams);
}