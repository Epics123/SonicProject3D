// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NinjaCharacter.h"
#include "Components/SplineComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "SonicCharacterBase.generated.h"

/**
 * 
 */
UCLASS()
class SONICGAME_API ASonicCharacterBase : public ANinjaCharacter
{
	GENERATED_BODY()

public:
	ASonicCharacterBase(const FObjectInitializer& ObjectInitializer);

	void ResetCapsuleRotation(float DeltaTime);

	void BoostStart();
	void BoostEnd();

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
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* JumpBallMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystemComponent* JumpBallPS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* CollisionPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAudioComponent* SoundEffectComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* JumpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* PsyloopPoint;

	UPROPERTY(BlueprintReadWrite)
	bool bCanMove = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxBoostSpeed = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxRunSpeed = 1800.0f;

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
	float RailAccelerationMultiplier = 5.0f;

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

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const
	{
		return CameraBoom;
	}
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const
	{
		return FollowCamera;
	}
};
