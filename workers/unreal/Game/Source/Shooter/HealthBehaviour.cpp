#include "HealthBehaviour.h"
#include "Shooter.h"

#include "HealthComponent.h"
#include "ShooterGameInstance.h"
#include "TakeDamageCommandCommandResponder.h"
#include "Shot.h"

UHealthBehaviour::UHealthBehaviour()
{
	PrimaryComponentTick.bCanEverTick = true;
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	static ConstructorHelpers::FObjectFinder<UBlueprint> ExplosionEffectFinder(TEXT("/Game/Assets/Blueprint_Effect_Explosion"));
	if (ExplosionEffectFinder.Object)
	{
		ExplosionEffect = (UClass*)ExplosionEffectFinder.Object->GeneratedClass;
	}
}

void UHealthBehaviour::BeginPlay()
{
	Super::BeginPlay();
	GameInstance = Cast<UShooterGameInstance>(GetWorld()->GetGameInstance());
	check(GameInstance);
	HealthComponent->OnTakeDamageCommandCommandRequest.AddDynamic(this, &UHealthBehaviour::OnTakeDamageCommandRequest);
	HealthComponent->OnTakeDamageEvent.AddDynamic(this, &UHealthBehaviour::OnTakeDamageEvent);
}

void UHealthBehaviour::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!IsPendingKill())
	{
		HealthComponent->OnTakeDamageCommandCommandRequest.RemoveDynamic(this, &UHealthBehaviour::OnTakeDamageCommandRequest);
		HealthComponent->OnTakeDamageEvent.RemoveDynamic(this, &UHealthBehaviour::OnTakeDamageEvent);
	}
}

void UHealthBehaviour::TakeDamage(const FVector& position, const FVector& start)
{
	if (HealthComponent->GetAuthority() != EAuthority::Authoritative)
	{
		GameInstance->LogError(TEXT("UHealthBehaviour::TakeDamage: No authority."));
		return;
	}

	const auto rawUpdate = improbable::Health::Update();
	const auto event = NewObject<UShotPoint>()->Init(improbable::ShotPoint(::improbable::Vector3f(0, 0, 0)))->SetPosition(position);
	const auto update = NewObject<UHealthComponentUpdate>()->Init(rawUpdate)->AddTakeDamageEvent(event);
	HealthComponent->SendComponentUpdate(update);

	TArray<UStaticMeshComponent*> MeshComponents;
	GetOwner()->GetComponents<UStaticMeshComponent>(MeshComponents);
	if (MeshComponents.Num() > 0)
	{
		FVector Impulse = start + position;
		Impulse.Normalize();
		MeshComponents[0]->AddImpulseAtLocation(Impulse * 50000.f, position);
	}
}

void UHealthBehaviour::OnTakeDamageCommandRequest(UTakeDamageCommandCommandResponder* responder)
{
	auto response = NewObject<UEmpty>()->Init(improbable::Empty());
	responder->SendResponse(response);

	auto request = responder->GetRequest();
	TakeDamage(request->GetEnd(), request->GetStart());
}

void UHealthBehaviour::OnTakeDamageEvent(UShotPoint* newEvent)
{
	auto ExplosionInstance = GetWorld()->SpawnActor<AActor>(*ExplosionEffect, newEvent->GetPosition(), FRotator::ZeroRotator, FActorSpawnParameters());
	ExplosionInstance->SetActorScale3D(FVector(0.3f, 0.3f, 0.3f));

	/*
	auto GameInstance = Cast<UShooterGameInstance>(GetWorld()->GetGameInstance());
	check(GameInstance);
	GameInstance->LogError(TEXT("I'm hit!"));
	*/
}