// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SonicMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class SONICGAME_API USonicMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USonicMovementComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TangentialDrag;

public:
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

private:
	FVector MoveTowards(FVector current, FVector target, float maxDistanceDelta);
};
