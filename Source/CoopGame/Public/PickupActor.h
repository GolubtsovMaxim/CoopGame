#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupActor.generated.h"

UCLASS()
class COOPGAME_API APickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* PickupSphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UDecalComponent* PickupDecalComp;

public:	

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
