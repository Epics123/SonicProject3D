// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectionActorBase.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SONICGAME_API AProjectionActorBase : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanMove = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpSpeed = 5.f;
	
public:	
	// Sets default values for this actor's properties
	AProjectionActorBase();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void MoveToTargetLocation();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetTargetDirectionFromDistance(float Distance);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
