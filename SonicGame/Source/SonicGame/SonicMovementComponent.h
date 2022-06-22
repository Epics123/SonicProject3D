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
	FVector GetComponentAxisZ() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TangentialDrag;

public:
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

	/**
	 * Perform jump. Called by Character when a jump has been detected because Character->bPressedJump was true. Checks Character->CanJump().
	 * Note that you should usually trigger a jump through Character::Jump() instead.
	 * @param	bReplayingMoves: true if this is being done as part of replaying moves on a locally controlled client after a server correction.
	 * @return	True if the jump was triggered successfully.
	 */
	virtual bool DoJump(bool bReplayingMoves) override;

private:
	FVector MoveTowards(FVector current, FVector target, float maxDistanceDelta);
};
