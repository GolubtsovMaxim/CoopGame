// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "GameFramework/Actor.h"


// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	mDefaultHealth = 100.f;
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* PlayerOwner = GetOwner();

	if (PlayerOwner)
	{
		PlayerOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
	}

	mHealthPoints = mDefaultHealth;
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0)
	{
		return;
	}

	mHealthPoints = FMath::Clamp(mHealthPoints - Damage, 0.0f, mDefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health = %s"), *FString::SanitizeFloat(mHealthPoints));

	OnHealthChanged.Broadcast(this, mHealthPoints, Damage, DamageType, InstigatedBy, DamageCauser);
}
