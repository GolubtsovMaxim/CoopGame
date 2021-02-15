// Fill out your copyright notice in the Description page of Project Settings.

#include "PowerUpActor.h"


// Sets default values
APowerUpActor::APowerUpActor()
{
	mPowerUpInterval = 0.0f;
	mTotalAmountOfTicks = 0;
}

// Called when the game starts or when spawned
void APowerUpActor::BeginPlay()
{
	Super::BeginPlay();
}

void APowerUpActor::OnPowerUpTick()
{
	mTickCounter++;

	OnPowerUpTicked();

	if (mTickCounter >= mTotalAmountOfTicks)
	{
		OnExpired();

		GetWorldTimerManager().ClearTimer(TimerHandle_Powerup);
	}
}

void APowerUpActor::ActivatePowerUp()
{
	OnActivated();
	if (mPowerUpInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_Powerup, this, &APowerUpActor::OnPowerUpTick, mPowerUpInterval, true);
	}
	else
	{
		OnPowerUpTick();
	}
}
