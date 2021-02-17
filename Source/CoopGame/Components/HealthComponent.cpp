// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	mDefaultHealth = 100.f;
	
	SetIsReplicated(true);
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	//Should work only on server
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* PlayerOwner = GetOwner();

		if (PlayerOwner)
		{
			PlayerOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
		}
	}
	
	mHealthPoints = mDefaultHealth;
}

void UHealthComponent::OnRep_Health(float OldHealth)
{
	float ActualDamage = mHealthPoints - OldHealth;

	OnHealthChanged.Broadcast(this, mHealthPoints, ActualDamage, nullptr, nullptr, nullptr);
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

float UHealthComponent::GetHealth() const
{
	return mHealthPoints;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, mHealthPoints);
}

void UHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f || mHealthPoints <= 0.0f)
	{
		return;
	}

	mHealthPoints = FMath::Clamp(mHealthPoints + HealAmount, 0.0f, mDefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s + (%s)"), *FString::SanitizeFloat(mHealthPoints), *FString::SanitizeFloat(HealAmount));

	OnHealthChanged.Broadcast(this, mHealthPoints, -HealAmount, nullptr, nullptr, nullptr);
}