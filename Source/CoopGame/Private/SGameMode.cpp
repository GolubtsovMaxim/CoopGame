// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"

#include "EngineUtils.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/HealthComponent.h"

ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 2.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
	
}

void ASGameMode::StartWave()
{
	WaveCount++;
	
	AmountOfBotsToSpawn = 2 * WaveCount;
	
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);
}

void ASGameMode::CheckAnyPlayerAlive()
{
	for (TActorIterator<APawn> Itr(GetWorld()); Itr; ++Itr)
	{
		if (Itr->GetController() != nullptr && Itr->GetController()->GetPawn())
		{
			APawn* AlivePawn = Itr->GetController()->GetPawn();
			UHealthComponent* HealthComp = Cast<UHealthComponent>(AlivePawn->GetComponentByClass(UHealthComponent::StaticClass()));
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
			{
				return;
			}
		}
	}

	GameOver();
}

void ASGameMode::GameOver()
{
	EndWave();
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();
	
	PrepareForNewWave();
}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckWaveState();
	CheckAnyPlayerAlive();
}

void ASGameMode::SpawnBotTimerElapsed()
{
	if (AmountOfBotsToSpawn > 0)
	{
		SpawnNewBot();
		AmountOfBotsToSpawn--;
	}
	else if (AmountOfBotsToSpawn == 0)
	{
		EndWave();
	}
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	//PrepareForNewWave();
}

void ASGameMode::PrepareForNewWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);
}

void ASGameMode::CheckWaveState()
{
	const bool bIsPrepairingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);
	
	if (AmountOfBotsToSpawn > 0 || bIsPrepairingForWave)
	{
		return;
	}

	bIsAnyBotAlive = false;
	
	for (TActorIterator<APawn> Itr(GetWorld()); Itr; ++Itr)
	{
		APawn* IterPawn = *Itr;
		if (IterPawn == nullptr || IterPawn->IsPlayerControlled())
		{
			continue;
		}

		UHealthComponent* HealthComp = Cast<UHealthComponent>(IterPawn->GetComponentByClass(UHealthComponent::StaticClass()));

		if (HealthComp && HealthComp->GetHealth() > 0.0f)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive)
	{
		PrepareForNewWave();
	}
	
}
