// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "GrindRail.generated.h"

UCLASS()
class SONICGAME_API AGrindRail : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrindRail();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void EnterRail(ASonicGameCharacter* Player);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ExitRail();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void RailJump();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(BlueprintReadWrite)
	USplineComponent* RailSpline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinRailSpeed = 500.0f;
};
