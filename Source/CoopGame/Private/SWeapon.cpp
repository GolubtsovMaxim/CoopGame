// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "EnemyActor.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

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
		QueryParams.AddIgnoredActor(MyOwner); //ignore actor for debug tracing
		QueryParams.AddIgnoredActor(this); //ignore a weapon for debug tracing
		QueryParams.bTraceComplex = true;


		FHitResult Hit; //struct for hit data
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			// hit! process damage
			AActor* HitActor = Hit.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit,
								MyOwner->GetInstigatorController(), this, DamageType);

			if (ImpactEffect && Cast<AEnemyActor>(HitActor))//TO-DO. Check if I am hitting an actor-enemy
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect,
					Hit.ImpactPoint, Hit.ImpactNormal.Rotation());

				auto name = Hit.GetActor()->GetName();
			}
		}

		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.f);

		//Attachin VFX's for shooting
		if (MuzzleEffect)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
		}
	}
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

