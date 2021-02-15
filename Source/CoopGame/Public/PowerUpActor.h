// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerUpActor.generated.h"

UCLASS()
class COOPGAME_API APowerUpActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APowerUpActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float mPowerUpInterval;

	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	uint32 mTotalAmountOfTicks;

	FTimerHandle TimerHandle_Powerup;

	uint32 mTickCounter;

	UFUNCTION()
	void OnPowerUpTick();


public:	

	void ActivatePowerUp();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerUpTicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();

};
