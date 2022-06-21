// Copyright Epic Games, Inc. All Rights Reserved.

#include "SonicGameCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

#include "SonicMovementComponent.h"

//////////////////////////////////////////////////////////////////////////
// ASonicGameCharacter

ASonicGameCharacter::ASonicGameCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USonicMovementComponent>(ACharacter::CharacterMovementComponentName))
{

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	bUseCharacterVectors = false;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	CollisionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CollisionPoint"));
	CollisionPoint->SetupAttachment(RootComponent);
	CollisionPoint->SetRelativeLocation(FVector(0.0f, 0.0f, -55.0f));
}

void ASonicGameCharacter::UpdatePhysics(float DeltaTime)
{
	CheckGround(DeltaTime);
	UpdateRotation(DeltaTime);
}

void ASonicGameCharacter::CheckGround(float DeltaTime)
{
	FHitResult outHit;

	FVector start = GetActorLocation();
	FVector end = start + GetActorQuat().GetAxisZ() * -60.0f; // player's current up vector

	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(this->GetOwner());

	//DrawDebugLine(GetWorld(), start, end, FColor::Blue, false, -1.0f, 0, 1);
	
	bool isHit = GetWorld()->LineTraceSingleByChannel(outHit, start, end, ECC_Visibility, collisionParams);

	if (isHit)
	{
		GroundNormal = outHit.Normal;
		bIsGrounded = true;
	}
	else
	{
		bIsGrounded = false;
		GroundNormal = FVector::ZeroVector;
	}
}

void ASonicGameCharacter::SlopeMove()
{
	// Apply landing speed
	if (bWasInAir && bIsGrounded)
	{
		FVector AddSpeed;

		AddSpeed = GroundNormal * LandingConversionFactor;
		StickToGround(GroundStickingFactor);

		AddSpeed.Y = 0.0f;
		AddVelocity(AddSpeed);
		bWasInAir = false;
	}

	// Exit slope if speed is too low
	if (GetMovementComponent()->Velocity.SquaredLength() < SlopeSpeedLimit && SlopeRunAngleLimit > GroundNormal.Z)
	{
		SetActorRotation(FQuat::Identity);
		AddVelocity(GroundNormal * 3.0f);
	}
	else
	{
		StickToGround(GroundStickingFactor);
	}
	
}

void ASonicGameCharacter::UpdateRotation(float DeltaTime)
{
	if (bIsGrounded)
	{
		FRotator newRot = FRotationMatrix::MakeFromZX(GroundNormal, GetActorForwardVector()).Rotator();
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), newRot, DeltaTime, 10.0f));
	}
}

void ASonicGameCharacter::StickToGround(float StickingPower)
{
	FHitResult outHit;

	FVector start = CollisionPoint->GetComponentLocation();
	FVector end = start + CollisionPoint->GetComponentQuat().GetAxisZ() * -GroundStickingDistance; // component's current up vector

	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(this->GetOwner());

	DrawDebugLine(GetWorld(), start, end, FColor::Green, false, -1.0f, 0, 1);

	bool isHit = GetWorld()->LineTraceSingleByChannel(outHit, start, end, ECC_Visibility, collisionParams);

	if (isHit)
	{
		FVector force = outHit.Normal * StickingPower;
		AddVelocity(force);
	}
}

void ASonicGameCharacter::AddVelocity(FVector Force)
{
	GetMovementComponent()->Velocity += Force;
}

void ASonicGameCharacter::Jump()
{
	//GetMesh()->SetVisibility(false);
	Super::Jump();
}

void ASonicGameCharacter::StopJump()
{
	Super::StopJumping();
}

void ASonicGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASonicGameCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASonicGameCharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASonicGameCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASonicGameCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASonicGameCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASonicGameCharacter::LookUpAtRate);
}

void ASonicGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdatePhysics(DeltaTime);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("%f, %f, %f"), MoveInput.X, MoveInput.Y, MoveInput.Z));
}

void ASonicGameCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASonicGameCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASonicGameCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Add forward movement
		if (bUseCharacterVectors)
		{
			AddMovementInput(FVector::VectorPlaneProject(FRotationMatrix(GetActorRotation()).GetScaledAxis(EAxis::X), GetActorQuat().GetAxisZ()).GetSafeNormal(), Value);
		}
		else // Use camera vector
		{
			AddMovementInput(FVector::VectorPlaneProject(FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::X), GetActorQuat().GetAxisZ()).GetSafeNormal(), Value);
		}
	}
}

void ASonicGameCharacter::MoveRight(float Value)
{
	MoveRightInput = Value;
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		const FVector Up = GetActorQuat().GetAxisZ(); // player's current up vector
		FVector SideVector;

		// Add side movement
		if (bUseCharacterVectors)
		{
			// Get side vector (Up X Forward Vector)
			SideVector = Up ^ FVector::VectorPlaneProject(FRotationMatrix(GetActorRotation()).GetScaledAxis(EAxis::X), Up).GetSafeNormal(); 
			AddMovementInput(SideVector, Value);
		}
		else // Use camera vector
		{
			SideVector = Up ^ FVector::VectorPlaneProject(FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::X), Up).GetSafeNormal();
			AddMovementInput(SideVector, Value);
		}
	}
}
