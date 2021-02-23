// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "SGameMode.h"


class ASGameMode;
// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	mDefaultHealth = 100.f;
	bIsDead = false;

	TeamAffiliation = 255;
	
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
	//auto test = DamageCauser->GetOwner();
	if (Damage <= 0.f || bIsDead)
	{
		return;
	}

	if ((DamageCauser != DamagedActor) && IsFriendly(DamagedActor, DamageCauser))
	{
		return;
	}

	mHealthPoints = FMath::Clamp(mHealthPoints - Damage, 0.0f, mDefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health = %s"), *FString::SanitizeFloat(mHealthPoints));

	bIsDead = mHealthPoints <= 0.0f;

	OnHealthChanged.Broadcast(this, mHealthPoints, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead)
	{
		ASGameMode* GameMode = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
	
}

float UHealthComponent::GetHealth() const
{
	return mHealthPoints;
}

uint8 UHealthComponent::GetTeamAffiliation() const
{
	return TeamAffiliation;
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

bool UHealthComponent::IsFriendly(AActor* ActorOne, AActor* ActorTwo)
{
	if (ActorOne == nullptr || ActorTwo == nullptr)
	{
		return true;
	}

	UHealthComponent* HC_A = Cast<UHealthComponent>(ActorOne->GetComponentByClass(UHealthComponent::StaticClass()));
	UHealthComponent* HC_B = Cast<UHealthComponent>(ActorTwo->GetComponentByClass(UHealthComponent::StaticClass()));

	if (HC_A == nullptr || HC_B == nullptr)
	{
		return true;
	}

	return HC_A->TeamAffiliation == HC_B->TeamAffiliation;
}
