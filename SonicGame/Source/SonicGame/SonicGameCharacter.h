// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SplineComponent.h"
#include "Components/AudioComponent.h"
#include "SonicGameCharacter.generated.h"

UCLASS(config=Game)
class ASonicGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ASonicGameCharacter(const FObjectInitializer& ObjectInitializer);

	void UpdatePhysics(float DeltaTime);

	void CheckGround(float DeltaTime);

	void UpdateRotation(float DeltaTime);

	void DoHomingAttack();

	void AddVelocity(FVector Force);

	void SetVelocity(FVector Velocity, bool bHorizontalOverride, bool bVerticalOverride, bool bUseAsMultiplier = false);

	FVector GetRailVelocityInDirection(FVector Velocity, bool bIsBackwards);

	void Jump();

	void StopJump();

	/**
	 * Finds the closest enemy to the player.
	 * @param radius	Search radius
	 */
	AActor* GetNearestHomingTarget(float radius);

	UFUNCTION(BlueprintCallable)
	void DetectGrindRail();

	UFUNCTION(BlueprintCallable)
	void DetectSideRail();

	void GrindOnRail(float StartDistance, USplineComponent* Rail);

	float GetClosestDistanceToLocation(USplineComponent* Spline, FVector Location, float ErrorTolerance);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHomingIcon(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent)
	void HideHomingIcon();

public:

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* CollisionPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAudioComponent* SoundEffectComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* JumpSound;

	UPROPERTY(Category = "Sonic Character Movement", EditAnywhere, BlueprintReadWrite)
	uint32 bUseCharacterVectors : 1;

	// --- Homing Attack --------------------------------------------------
	UPROPERTY(Category = "Homing Attack", EditAnywhere, BlueprintReadWrite)
	float HomingRadius = 500.0f;

	UPROPERTY(Category = "Homing Attack", EditAnywhere, BlueprintReadWrite)
	float HomingSpeed = 10.0f;

	UPROPERTY(Category = "Homing Attack", EditAnywhere, BlueprintReadWrite)
	float MinHomingThreshold = 100.0f;

	UPROPERTY(Category = "Homing Attack", EditAnywhere, BlueprintReadWrite)
	float HomingUpForce = 700.0f;

	UPROPERTY(Category = "Homing Attack", EditAnywhere, BlueprintReadWrite)
	float MinHomingViewAngle = 95.0f;

	UPROPERTY(Category = "Homing Attack", EditAnywhere, BlueprintReadWrite)
	bool bIsHoming = false;

	UPROPERTY(Category = "Homing Attack", EditAnywhere, BlueprintReadWrite)
	AActor* HomingTarget;

	UPROPERTY(Category = "Homing Attack", EditAnywhere, BlueprintReadWrite)
	USoundBase* HomingSound;

	UPROPERTY(Category = "Homing Attack", EditAnywhere, BlueprintReadWrite)
	USoundBase* LockOnSound;

	bool bCanDoHomingAttack = true;

	float HomingViewAngle;

	//--- Rail Grinding --------------------------------------------------
	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	bool bIsGrinding;

	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	bool bGrindJump;

	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	bool bLeftRailSwitch;

	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	bool bRightRailSwitch;

	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	bool bCanSwitchRails = true;

	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	FVector RailCollisionPoint;

	UPROPERTY(Category = "Rail Grinding", BlueprintReadWrite)
	FVector LeftRailCollisionPoint;

	UPROPERTY(Category = "Rail Grinding", BlueprintReadWrite)
	FVector LeftRailTargetPoint;

	UPROPERTY(Category = "Rail Grinding", BlueprintReadWrite)
	FVector RightRailCollisionPoint;

	UPROPERTY(Category = "Rail Grinding", BlueprintReadWrite)
	FVector RightRailTargetPoint;

	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	float GrindLeanDirection;

	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	float ClosestRailPointDistance;

	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	float RailOffset = 70.0f;

	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	float RailStartDistance;

	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	bool bBackwardsGrind;

	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	float RailJumpHeight = 300.0f;

	UPROPERTY(Category = "Rail Grinding", EditAnywhere, BlueprintReadWrite)
	float MaxRailSpeed = 2000.0f;

	UPROPERTY(Category = "Rail Grinding", BlueprintReadWrite)
	USplineComponent* CurrentRail;

	UPROPERTY(Category = "Rail Grinding", BlueprintReadWrite)
	USplineComponent* LeftRail;

	UPROPERTY(Category = "Rail Grinding", BlueprintReadWrite)
	USplineComponent* RightRail;

	UPROPERTY(Category = "Rail Grinding", BlueprintReadWrite)
	USceneComponent* SparkEffectPoint;

	//////////////////////////////////////////////////////////////////////

	FVector MoveInput = FVector::ZeroVector;

	FVector GroundNormal;

	bool bIsGrounded;

	bool bWasInAir;

	UPROPERTY(BlueprintReadWrite)
	bool bCanMove = true;

	float LandingConversionFactor = 2.0f;

	float GroundStickingFactor = 1.0f;

	float GroundStickingDistance = 5.0f;

	float SlopeSpeedLimit = 500.0f;

	float SlopeRunAngleLimit = 0.5f;

	float MoveAccelleration = 500.0f;

	float MoveDecelleration = 1.3f;

protected:
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaTime);
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

