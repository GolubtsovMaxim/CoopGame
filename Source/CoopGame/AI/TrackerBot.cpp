// Fill out your copyright notice in the Description page of Project Settings.

#include "TrackerBot.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "Components/HealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SCharacter.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

// Sets default values
ATrackerBot::ATrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	BotHealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	BotHealthComponent->OnHealthChanged.AddDynamic(this, &ATrackerBot::HandleTakeDamage);

	BotSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	BotSphereComp->SetSphereRadius(200);
	BotSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BotSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BotSphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BotSphereComp->SetupAttachment(RootComponent);

	bUseVeloctiyChange = false;
	MovementForce = 1000.f;
	RequiredDistanceToTarget = 100;

	ExplosionDamage = 100;
	ExplosionRadius = 40;

	SelfDamageInterval = 0.25f;
}

// Called when the game starts or when spawned
void ATrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint();
	}
}

FVector ATrackerBot::GetNextPathPoint()
{
	//ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0); //get pawn
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;
	
	for (TActorIterator<APawn> Itr(GetWorld()); Itr; ++Itr)
	{
		APawn* IterPawn = *Itr;
		if (IterPawn == nullptr || UHealthComponent::IsFriendly(IterPawn, this))
		{
			continue;
		}

		UHealthComponent* TestPawnHealthComp = Cast<UHealthComponent>(IterPawn->GetComponentByClass(UHealthComponent::StaticClass()));

		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.0f)
		{
			const float Distance = IterPawn->GetActorLocation().Size() - GetActorLocation().Size();
			if (NearestTargetDistance > Distance)
			{
				BestTarget = IterPawn;
				NearestTargetDistance = Distance;
			}
			break;
		}
	}

	if (BestTarget)
	{
		UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);

		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ATrackerBot::RefreshPath, 5.0f, false);

		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			//next point to move on
			return NavPath->PathPoints[1];
		}
	}
	
	return GetActorLocation();
}

void ATrackerBot::HandleTakeDamage(UHealthComponent* HealhtComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	
	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}
	
	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());

	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

void ATrackerBot::SelfDestruct()
{
	if (IsExploded)
	{
		return;
	}

	IsExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr,
			IgnoredActors, this, GetInstigatorController(), true);

		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);

		SetLifeSpan(2.0f);
		//Destroy();
	}
}

void ATrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

// Called every frame
void ATrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !IsExploded)
	{
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();
		}
		else
		{
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVeloctiyChange);
		}
	}
}

void ATrackerBot::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStartedSelfDestruction && !IsExploded)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn && !UHealthComponent::IsFriendly(OtherActor, this))
		{
			if (Role == ROLE_Authority)
			{
				GetWorldTimerManager().SetTimer(TimerHandleSelfDamage, this, &ATrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}
			bStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
			//Start self destruct as we get in radius of player
		}
	}
}

void ATrackerBot::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}
