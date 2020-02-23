// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "EnemyActor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

static float DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing (
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw Debug Lines for Weapons"), 
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	// PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
}

// Called when the game starts or when spawned
/*void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}*/ //removed during weapon polishing code

void ASWeapon::Fire()
{
	// Trace the world from pawn eyes to crosshair location

	AActor* MyOwner = GetOwner(); //the one who shoots
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector(); //shot direction for damage apply
		FVector TraceEnd = EyeLocation + (ShotDirection * 1000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner); //ignore an actor for debug tracing
		QueryParams.AddIgnoredActor(this); //ignore a weapon for debug tracing
		QueryParams.bTraceComplex = true;

		// Paricle "Target" parameter 
		FVector TracerEndPoint = TraceEnd;

		FHitResult Hit; //struct for hit data
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			// hit! process damage
			AActor* HitActor = Hit.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit,
								MyOwner->GetInstigatorController(), this, DamageType);

			if (ImpactEffect && Cast<AEnemyActor>(HitActor)->IsValidLowLevel())//TO-DO. Check if I am hitting an actor-enemy
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect,
					Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}

			TracerEndPoint = Hit.ImpactPoint;
		}

		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
		}
		//Attaching VFX's for shooting
		PlayFireVFX(TracerEndPoint);
		
	}
}

void ASWeapon::PlayFireVFX(FVector TracerEnd)
{
	APawn* pCurrentOwner = Cast<APawn>(GetOwner());
	if (pCurrentOwner != nullptr)
	{
		APlayerController* pPlayerController = Cast<APlayerController>(pCurrentOwner->GetController());
		if (pPlayerController != nullptr)
		{
			pPlayerController->ClientPlayCameraShake(FireCameraShake);
		}
	}

	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEnd);
		}
	}

}

// Called every frame
/*void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/ //nothing to tick

