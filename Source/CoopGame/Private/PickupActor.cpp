// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupActor.h"
#include "PowerUpActor.h"

#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"


// Sets default values
APickupActor::APickupActor()
{
	PickupSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphereComp"));
	PickupSphereComp->SetSphereRadius(75.0f);
	RootComponent = PickupSphereComp;

	PickupDecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("PickupDecalComp"));
	PickupDecalComp->SetRelativeRotation(FRotator(90, 0.0f, 0.0f));
	PickupDecalComp->DecalSize = FVector(64, 75, 75);
	PickupDecalComp->SetupAttachment(RootComponent);

	SetReplicates(true);
}

// Called when the game starts or when spawned
void APickupActor::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		Respawn();
	}
}

void APickupActor::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (Role == ROLE_Authority && PowerUpInstance)
	{
		PowerUpInstance->ActivatePowerUp(OtherActor);
		PowerUpInstance = nullptr;

		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &APickupActor::Respawn, CooldownDuration);
	}
}

void APickupActor::Respawn()
{
	if (PowerUpClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerUpClass is nullptr in %s."), *GetName());
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PowerUpInstance = GetWorld()->SpawnActor<APowerUpActor>(PowerUpClass, GetTransform(), SpawnParams);

	
}
