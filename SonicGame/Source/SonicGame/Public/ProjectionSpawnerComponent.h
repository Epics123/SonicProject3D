// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ProjectionSpawnerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class SONICGAME_API UProjectionSpawnerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumFinisherProjections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartingDirection;

	int32 NumProjectionsSpawned = 0;

	FTimerHandle SpawnTimerHandle;
	FTimerDelegate SpawnTimerDelegate;

public:	
	// Sets default values for this component's properties
	UProjectionSpawnerComponent();


	UFUNCTION(BlueprintCallable)
	void SpawnProjectionInCircle(TSubclassOf<AActor> ActorToSpawn, int32 NumProjections, float Distance, const float RotationAngle);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SpawnSpecialAttackFinisherProjections();

	UFUNCTION()
	void SpawnProjection(int32 NumProjections, TSubclassOf<AActor> ActorToSpawn, float Distance, const float AdditionalRotationAngle = 0.0f);

	UFUNCTION()
	void SpawnDeferredProjection(TSubclassOf<AActor> ActorToSpawn, const FTransform& Transform, const FVector TargetLocation);
	
	FVector CalculateProjectionTargetLocation(const FVector Direction, float Distance);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
