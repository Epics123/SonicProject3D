// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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


	void AddVelocity(FVector Force);

	void Jump();

	void StopJump();

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
	USoundBase* JumpSound;

	UPROPERTY(Category = "Sonic Character Movement", EditAnywhere, BlueprintReadWrite)
	uint32 bUseCharacterVectors : 1;

	FVector MoveInput = FVector::ZeroVector;

	FVector GroundNormal;

	bool bIsGrounded;

	bool bWasInAir;

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
