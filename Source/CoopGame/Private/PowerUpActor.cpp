// Fill out your copyright notice in the Description page of Project Settings.

#include "PowerUpActor.h"

#include "UnrealNetwork.h"


// Sets default values
APowerUpActor::APowerUpActor()
{
	mPowerUpInterval = 0.0f;
	mTotalAmountOfTicks = 0;

	bIsPowerUpActive = false;

	SetReplicates(true);
}

void APowerUpActor::OnPowerUpTick()
{
	mTickCounter++;

	OnPowerUpTicked();

	if (mTickCounter >= mTotalAmountOfTicks)
	{
		OnExpired();
		bIsPowerUpActive = false;
		OnRep_PowerupActive();

		GetWorldTimerManager().ClearTimer(TimerHandle_Powerup);
	}
}

void APowerUpActor::OnRep_PowerupActive()
{
	OnPowerUpStateChanged(bIsPowerUpActive);
}



void APowerUpActor::ActivatePowerUp(AActor* OtherActor)
{
	OnActivated(OtherActor);

	bIsPowerUpActive = true;
	OnRep_PowerupActive();
	
	if (mPowerUpInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_Powerup, this, &APowerUpActor::OnPowerUpTick, mPowerUpInterval, true);
	}
	else
	{
		OnPowerUpTick();
	}
}

void APowerUpActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APowerUpActor, bIsPowerUpActive);
}
