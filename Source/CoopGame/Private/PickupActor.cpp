// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupActor.h"

#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"


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
}

// Called when the game starts or when spawned
void APickupActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void APickupActor::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);


}
