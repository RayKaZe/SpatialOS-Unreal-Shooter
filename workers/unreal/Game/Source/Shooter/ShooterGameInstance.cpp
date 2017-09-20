#include "ShooterGameInstance.h"
#include "Shooter.h"

#include "Commander.h"
#include "EntityPipeline.h"
#include "EntityRegistry.h"
#include "EntityTemplates.h"
#include "SpatialOS.h"
#include "SimpleEntitySpawnerBlock.h"
#include "SpatialOSComponentUpdater.h"

void UShooterGameInstance::Init()
{
	UGameInstance::Init();
	SpatialOS = NewObject<USpatialOS>(this);
	SpatialOS->OnConnectedDelegate.AddDynamic(this, &UShooterGameInstance::OnSpatialOsConnected);
	SpatialOS->OnDisconnectedDelegate.AddDynamic(this, &UShooterGameInstance::OnSpatialOsDisconneced);
	EntityRegistry = NewObject<UEntityRegistry>(this);
	SpatialOSComponentUpdater = NewObject<USpatialOSComponentUpdater>(this);
	ConnectToSpatialOS();
}

void UShooterGameInstance::Shutdown()
{
	UGameInstance::Shutdown();
	SpatialOS->Disconnect();
	SpatialOS->OnConnectedDelegate.RemoveDynamic(this, &UShooterGameInstance::OnSpatialOsConnected);
	SpatialOS->OnDisconnectedDelegate.RemoveDynamic(this, &UShooterGameInstance::OnSpatialOsDisconneced);
}

void UShooterGameInstance::ProcessOps(float DeltaTime)
{
	if (SpatialOS != nullptr && SpatialOS->GetEntityPipeline() != nullptr)
	{
		SpatialOS->ProcessOps();
		SpatialOS->GetEntityPipeline()->ProcessOps(SpatialOS->GetView(), SpatialOS->GetConnection(), GetWorld());
		SpatialOSComponentUpdater->UpdateComponents(EntityRegistry, DeltaTime);
	}
}

void UShooterGameInstance::ConnectToSpatialOS()
{
	auto workerConfig = FSOSWorkerConfigurationData();
	//workerConfig.SpatialOSApplication.WorkerPlatform = "UnrealWorker";
	workerConfig.SpatialOSApplication.WorkerId = "UnrealWorkerDefaultId";
	SpatialOS->ApplyConfiguration(workerConfig);
	SpatialOS->Connect();
}

void UShooterGameInstance::LogMessage(const FString& message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *message);
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Black, message);
	SpatialOS->GetConnection().Pin()->SendLogMessage(worker::LogLevel::kInfo, std::string("WorkerLogger"), std::string(TCHAR_TO_UTF8(*message)));
}

void UShooterGameInstance::LogError(const FString& message)
{
	UE_LOG(LogTemp, Error, TEXT("%s"), *message);
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, message);
	SpatialOS->GetConnection().Pin()->SendLogMessage(worker::LogLevel::kError, std::string("WorkerLogger"), std::string(TCHAR_TO_UTF8(*message)));
}

bool UShooterGameInstance::IsConnected() const
{
	if (SpatialOS == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpatialOS is nullptr"));
		return false;
	}
	return SpatialOS->IsConnected();
}

void UShooterGameInstance::OnSpatialOsConnected()
{
	LogMessage(TEXT("SpatialOS connected."));
	WorkerId = FString(SpatialOS->GetConnection().Pin()->GetWorkerId().c_str());
	InitialiseSendingMetrics();
	Commander = NewObject<UCommander>(this, UCommander::StaticClass())->Init(nullptr, SpatialOS->GetConnection(), SpatialOS->GetView());
	InitialiseEntitySpawnerBlock();

	if (SpatialOS->GetWorkerConfiguration().GetWorkerType() == TEXT("UnrealClient"))
	{
		SpawnPlayer();
	}
}

void UShooterGameInstance::OnSpatialOsDisconneced()
{
	UE_LOG(LogTemp, Warning, TEXT("SpatialOS disconnected."));
	GetWorld()->GetTimerManager().ClearTimer(MetricsReporterHandle);
	Commander = nullptr;
}

void UShooterGameInstance::InitialiseSendingMetrics()
{
	const auto ShouldTimerLoop = true;
	const auto InitialDelay = 2.0f;
	const auto LoopDelay = 2.0f;
	auto MetricsDelegate = FTimerDelegate::CreateLambda([this]()
	{
		auto Connection = SpatialOS->GetConnection().Pin();
		if (Connection.IsValid())
		{
			Connection->SendMetrics(SpatialOS->GetMetrics());
		}
	});
	GetWorld()->GetTimerManager().SetTimer(MetricsReporterHandle, MetricsDelegate, LoopDelay, ShouldTimerLoop, InitialDelay);
}

void UShooterGameInstance::InitialiseEntitySpawnerBlock()
{
	TArray<FString> BlueprintPaths;
	BlueprintPaths.Add(TEXT("/Game/EntityBlueprints"));
	EntityRegistry->RegisterEntityBlueprints(BlueprintPaths);

	auto EntitySpawnerBlock = NewObject<USimpleEntitySpawnerBlock>();
	EntitySpawnerBlock->Init(EntityRegistry);
	SpatialOS->GetEntityPipeline()->AddBlock(EntitySpawnerBlock);
}

void UShooterGameInstance::SpawnPlayer()
{
	LogMessage(TEXT("Calling SpawnPlayer."));
	auto callback = FReserveEntityIdResultDelegate();
	callback.BindDynamic(this, &UShooterGameInstance::OnReserveEntityIdResponsePlayer);
	Commander->ReserveEntityId(callback, 0);
}

void UShooterGameInstance::OnReserveEntityIdResponsePlayer(const FSpatialOSCommandResult& result, FEntityId reservedEntityId)
{
	LogMessage(TEXT("Calling OnReserveEntityIdResponsePlayer."));
	if (!result.Success())
	{
		LogError(TEXT("ReserveEntityId Player failed."));
		return;
	}
	auto callback = FCreateEntityResultDelegate();
	callback.BindDynamic(this, &UShooterGameInstance::OnCreateEntityResponse);
	UEntityTemplate* entity = UEntityTemplates::CreatePlayerCharacterTemplate(WorkerId, FVector(0.f, 10.f, 0.f));
	Commander->CreateEntity(entity, reservedEntityId, callback, 0);
}

void UShooterGameInstance::OnCreateEntityResponse(const FSpatialOSCommandResult& result, FEntityId createdEntityId)
{
	LogMessage(TEXT("Calling OnCreateEntityResponse."));
	if (!result.Success())
	{
		LogError(TEXT("OnCreateEntityResponse failed."));
		return;
	}
}

void UShooterGameInstance::SpawnTarget()
{
	LogMessage(TEXT("Calling SpawnTarget."));
	auto callback = FReserveEntityIdResultDelegate();
	callback.BindDynamic(this, &UShooterGameInstance::OnReserveEntityIdResponseTarget);
	Commander->ReserveEntityId(callback, 0);
}

void UShooterGameInstance::OnReserveEntityIdResponseTarget(const FSpatialOSCommandResult& result, FEntityId reservedEntityId)
{
	LogMessage(TEXT("Calling OnReserveEntityIdResponseTarget."));
	if (!result.Success())
	{
		LogError(TEXT("ReserveEntityId Target failed."));
		return;
	}
	auto callback = FCreateEntityResultDelegate();
	callback.BindDynamic(this, &UShooterGameInstance::OnCreateEntityResponse);
	UEntityTemplate* entity = UEntityTemplates::CreateTargetTemplate(WorkerId, FVector(0.f, 10.f, -8.f));
	Commander->CreateEntity(entity, reservedEntityId, callback, 0);
}


void UShooterGameInstance::DestroyPlayer(const AActor* actor)
{
	LogMessage(TEXT("Calling DestroyPlayer."));
	auto EntityId = EntityRegistry->GetEntityIdFromActor(actor);
	auto callback = FDeleteEntityResultDelegate();
	callback.BindDynamic(this, &UShooterGameInstance::OnDeleteEntityResponse);
	Commander->DeleteEntity(EntityId.ToSpatialEntityId(), callback, 0);
}

void UShooterGameInstance::OnDeleteEntityResponse(const FSpatialOSCommandResult& result)
{
	LogMessage(TEXT("Calling OnDeleteEntityResponse."));
	if (!result.Success())
	{
		LogError(TEXT("OnDeleteEntityResponse failed."));
		return;
	}
}