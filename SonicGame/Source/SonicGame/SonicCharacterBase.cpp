// Fill out your copyright notice in the Description page of Project Settings.


#include "SonicCharacterBase.h"
#include "NinjaCharacterMovementComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"

#include "Kismet/KismetMathLibrary.h"

ASonicCharacterBase::ASonicCharacterBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UNinjaCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void ASonicCharacterBase::ResetCapsuleRotation(float DeltaTime)
{
	FVector forward = GetActorForwardVector();// * moveForward;
	FVector right = GetActorRightVector(); //* moveRight;

	if (GetNinjaCharacterMovement()->IsFalling())
	{
		UCapsuleComponent* capsule = GetCapsuleComponent();
		capsule->SetWorldRotation(FMath::RInterpTo(capsule->GetComponentRotation(), FRotator(0.0f, capsule->GetComponentRotation().Yaw, 0.0f), DeltaTime, 10.0f));
	}
}

void ASonicCharacterBase::BoostStart()
{
	GetNinjaCharacterMovement()->MaxWalkSpeed = MaxBoostSpeed;
	FVector LaunchDirection = GetActorForwardVector() * MaxBoostSpeed;
	LaunchCharacter(LaunchDirection, true, true);
}

void ASonicCharacterBase::BoostEnd()
{
	GetNinjaCharacterMovement()->MaxWalkSpeed = MaxRunSpeed;
}

void ASonicCharacterBase::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && bCanMove && !bIsGrinding)
	{
		FVector Direction = FVector(0.0f);
		if(GetCapsuleComponent())
		{
			FVector CapsuleDirection = FRotationMatrix(GetCapsuleComponent()->GetComponentRotation()).GetScaledAxis(EAxis::Z);
			FVector ForwardDirection = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);


			Direction = FVector::CrossProduct(ForwardDirection, CapsuleDirection);
			Direction.Normalize();
		}
		
		AddMovementInput(Direction, Value);
	}
}

void ASonicCharacterBase::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && bCanMove && !bIsGrinding)
	{
		FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ASonicCharacterBase::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASonicCharacterBase::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASonicCharacterBase::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASonicCharacterBase::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASonicCharacterBase::StopJumping);
	PlayerInputComponent->BindAction("Boost", IE_Pressed, this, &ASonicCharacterBase::BoostStart);
	PlayerInputComponent->BindAction("Boost", IE_Released, this, &ASonicCharacterBase::BoostEnd);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASonicCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASonicCharacterBase::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASonicCharacterBase::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASonicCharacterBase::LookUpAtRate);
}

void ASonicCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ResetCapsuleRotation(DeltaTime);
}
