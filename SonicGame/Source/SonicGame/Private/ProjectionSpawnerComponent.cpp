// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectionSpawnerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectionActorBase.h"

// Sets default values for this component's properties
UProjectionSpawnerComponent::UProjectionSpawnerComponent()
{
	
}


FVector UProjectionSpawnerComponent::CalculateProjectionTargetLocation(const FVector Direction, float Distance)
{
	return GetOwner()->GetActorLocation() + (Direction * Distance);
}

void UProjectionSpawnerComponent::SpawnProjectionInCircle(TSubclassOf<AActor> ActorToSpawn, int32 NumProjections, float Distance, const float RotationAngle)
{
	SpawnTimerDelegate.BindUFunction(this, FName("SpawnProjection"), NumProjections, ActorToSpawn, Distance, RotationAngle);
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, SpawnTimerDelegate, 0.01f, true);
}

void UProjectionSpawnerComponent::SpawnProjection(int32 NumProjections, TSubclassOf<AActor> ActorToSpawn, float Distance, const float AdditionalRotationAngle)
{
	if(NumProjectionsSpawned > NumProjections)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		NumProjectionsSpawned = 0;
		return;
	}
	else
	{
		if(NumProjectionsSpawned >= NumProjections / 2)
		{
			Distance *= 2;
		}

		FTransform Transform = FTransform(GetComponentRotation(), GetComponentLocation(), GetOwner()->GetActorScale3D());
		FVector TargetLocation = CalculateProjectionTargetLocation(StartingDirection, Distance);

		SpawnDeferredProjection(ActorToSpawn, Transform, TargetLocation);
		NumProjectionsSpawned++;
		StartingDirection = StartingDirection.RotateAngleAxis(AdditionalRotationAngle, GetOwner()->GetActorForwardVector());
	}
}

void UProjectionSpawnerComponent::SpawnDeferredProjection(TSubclassOf<AActor> ActorToSpawn, const FTransform& Transform, const FVector TargetLocation)
{
	AProjectionActorBase* SpawnedProjection = Cast<AProjectionActorBase>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ActorToSpawn, Transform));
	SpawnedProjection->TargetLocation = TargetLocation;
	UGameplayStatics::FinishSpawningActor(SpawnedProjection, Transform);
}

// Called when the game starts
void UProjectionSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

