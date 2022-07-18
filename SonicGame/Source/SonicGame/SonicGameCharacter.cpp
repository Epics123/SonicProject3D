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
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Enemy.h"
#include "GrindRail.h"

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

	SoundEffectComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SoundEffectComponent"));
	SoundEffectComponent->SetupAttachment(RootComponent);

	SparkEffectPoint = CreateAbstractDefaultSubobject<USceneComponent>(TEXT("SparkEffectPoint"));
	SparkEffectPoint->SetupAttachment(RootComponent);
}

void ASonicGameCharacter::UpdatePhysics(float DeltaTime)
{
	//CheckGround(DeltaTime);
	UpdateRotation(DeltaTime);

	// Check for the closest enemy while player is in the air
	if (GetMovementComponent()->IsFalling())
	{
		HomingTarget = GetNearestHomingTarget(HomingRadius);
		if(HomingTarget && HomingViewAngle <= MinHomingViewAngle)
			ShowHomingIcon(HomingTarget);
	}
	else
	{
		HideHomingIcon();
		bCanDoHomingAttack = true;
	}
	
	// Are we currently performing a homing attack?
	if(bIsHoming)
		DoHomingAttack();
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

void ASonicGameCharacter::UpdateRotation(float DeltaTime)
{
	//float moveForward = GetInputAxisValue("MoveForward");
	//float moveRight = GetInputAxisValue("MoveRight");

	FVector forward = GetActorForwardVector();// * moveForward;
	FVector right = GetActorRightVector(); //* moveRight;


	if (!GetCharacterMovement()->IsFalling())
	{

		FHitResult floorHit = GetCharacterMovement()->CurrentFloor.HitResult;

		float f = floorHit.ImpactNormal.Dot(FVector(0.0f, 0.0f, 1.0f));
		float theta = FMath::RadiansToDegrees(FMath::Acos(f));

		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("%f"), theta));

		UCapsuleComponent* capsule = GetCapsuleComponent();

		FRotator r = UKismetMathLibrary::MakeRotationFromAxes(capsule->GetForwardVector(), capsule->GetRightVector(), floorHit.ImpactNormal);
		FRotator r2 = FRotationMatrix::MakeFromYZ(capsule->GetRightVector(), floorHit.ImpactNormal).Rotator();

		FRotator newRot = FRotator(r2.Pitch, capsule->GetComponentRotation().Yaw, r.Roll);

		capsule->SetWorldRotation(FMath::RInterpTo(capsule->GetComponentRotation(), newRot, DeltaTime, 10.0f));
	}
	else
	{
		UCapsuleComponent* capsule = GetCapsuleComponent();
		capsule->SetWorldRotation(FMath::RInterpTo(capsule->GetComponentRotation(), FRotator(0.0f, capsule->GetComponentRotation().Yaw, 0.0f), DeltaTime, 10.0f));
	}
}

void ASonicGameCharacter::DoHomingAttack()
{
	if (HomingTarget)
	{
		FVector targetLoc = HomingTarget->GetActorLocation();

		if (FVector::Dist(GetActorLocation(), targetLoc) <= MinHomingThreshold)
		{
			bIsHoming = false;
			bCanMove = true;
			bCanDoHomingAttack = true;
			GetCharacterMovement()->GravityScale = 1.0f;

			if (Cast<AEnemy>(HomingTarget))
			{
				HomingTarget->Destroy();
				HomingTarget = nullptr;

				LaunchCharacter(FVector(0.0f, 0.0f, 1.0f) * HomingUpForce, false, true);
			}
			
			HideHomingIcon();

			return;
		}

		FHitResult sweepHit;
		SetActorLocation(UKismetMathLibrary::VInterpTo(GetActorLocation(), targetLoc, GetWorld()->DeltaTimeSeconds, HomingSpeed), true, &sweepHit);

		// Did we hit something before making it to the target?
		if (sweepHit.bBlockingHit)
		{
			bIsHoming = false;
			bCanMove = true;
			bCanDoHomingAttack = true;
			GetCharacterMovement()->GravityScale = 1.0f;

			HomingTarget = nullptr;

			HideHomingIcon();

			return;
		}
	}
}


void ASonicGameCharacter::AddVelocity(FVector Force)
{
	GetMovementComponent()->Velocity += Force;
}

void ASonicGameCharacter::SetVelocity(FVector Velocity, bool bHorizontalOverride, bool bVerticalOverride, bool bUseAsMultiplier)
{
	if (bUseAsMultiplier)
	{
		GetCharacterMovement()->Velocity.X *= Velocity.X;
		GetCharacterMovement()->Velocity.Y *= Velocity.Y;
		GetCharacterMovement()->Velocity.Z *= Velocity.Z;
	}
	else
	{
		GetCharacterMovement()->Velocity.X = UKismetMathLibrary::SelectFloat(Velocity.X, Velocity.X + GetMovementComponent()->Velocity.X, bHorizontalOverride);
		GetCharacterMovement()->Velocity.Y = UKismetMathLibrary::SelectFloat(Velocity.Y, Velocity.Y + GetMovementComponent()->Velocity.Y, bHorizontalOverride);
		GetCharacterMovement()->Velocity.Z = UKismetMathLibrary::SelectFloat(Velocity.Z, Velocity.Z + GetMovementComponent()->Velocity.Z, bVerticalOverride);
	}
}

FVector ASonicGameCharacter::GetRailVelocityInDirection(FVector Velocity, bool bIsBackwards)
{
	return UKismetMathLibrary::SelectVector(Velocity, Velocity * -1.0f, bIsBackwards);
}

void ASonicGameCharacter::Jump()
{	
	if (!GetMovementComponent()->IsFalling() && !bIsGrinding)
	{
		if(JumpSound)
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), JumpSound, GetActorLocation());
	}

	if (bIsGrinding)
	{
		bGrindJump = true;
	}
	else if (!bIsHoming && GetMovementComponent()->IsFalling())
	{
		if (HomingTarget && HomingViewAngle <= MinHomingViewAngle)
		{
			bIsHoming = true;
			bCanMove = false;
			GetCharacterMovement()->GravityScale = 0.0f;
			GetCharacterMovement()->StopMovementImmediately();

			SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), HomingTarget->GetActorLocation()));

			if(HomingSound)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HomingSound, GetActorLocation());
		}
		else if(bCanDoHomingAttack)
		{
			bCanDoHomingAttack = false;
			GetCharacterMovement()->StopMovementImmediately();
			LaunchCharacter(GetActorForwardVector() * HomingUpForce * 6.0f, true, false);

			if (HomingSound)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HomingSound, GetActorLocation());
		}
		
	}

	Super::Jump();
}

void ASonicGameCharacter::StopJump()
{
	Super::StopJumping();
}

AActor* ASonicGameCharacter::GetNearestHomingTarget(float radius)
{
	const FVector start = GetActorLocation();
	const FVector end = GetActorLocation();
	
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(GetOwner());

	TArray<FHitResult> HitArray;

	ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Pawn);

	bool hit = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), start, end, radius, TraceChannel, false, IgnoreActors, EDrawDebugTrace::None, HitArray, true);

	float shortestDistSq = radius * radius;
	AEnemy* closestEnemy = nullptr;

	// Did sphere trace hit anything?
	if (hit)
	{
		// loop through hit results
		for (FHitResult HitResult : HitArray)
		{
			// Validate enemy
			AEnemy* enemy = Cast<AEnemy>(HitResult.GetActor());
			if (enemy != nullptr)
			{
				float distSq = FVector::DistSquared(GetActorLocation(), enemy->GetActorLocation()); // get squared distance between player and current enemy

				// update closest enemy and distance if current enemy is closer than the previous closest enemy
				if (distSq < shortestDistSq)
				{
					shortestDistSq = distSq;
					closestEnemy = enemy;
				}
			}
		}
	}

	if (closestEnemy)
	{
		//DrawDebugLine(GetWorld(), GetActorLocation(), closestEnemy->GetActorLocation(), FColor::Blue);
		FVector dir = closestEnemy->GetActorLocation() - GetActorLocation();
		HomingViewAngle = FVector::DotProduct(GetActorForwardVector(), dir.GetSafeNormal());
		//HomingViewAngle = FVector::DotProduct(FollowCamera->GetForwardVector(), dir.GetSafeNormal());
		HomingViewAngle = FMath::RadiansToDegrees(FMath::Acos(HomingViewAngle));
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("%f"), HomingViewAngle));

		if (HomingTarget && HomingTarget != closestEnemy && HomingViewAngle <= MinHomingViewAngle)
		{
			HideHomingIcon();
			ShowHomingIcon(closestEnemy);
		}
	}

	return closestEnemy;
}

void ASonicGameCharacter::DetectGrindRail()
{
	if (bIsGrinding)
	{
		GrindOnRail(RailStartDistance, CurrentRail);
	}
	else
	{
		FVector start = GetActorLocation() - FVector(0.0f, 0.0f, 60.0f);
		FVector end = GetActorLocation() - FVector(0.0f, 0.0f, 61.0f);

		FHitResult outHit;

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
		ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
		ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
		ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1));

		TArray<AActor*> IgnoreActors;

		bool traceHit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), start, end, 40.0f, ObjectTypesArray, false, IgnoreActors, EDrawDebugTrace::None, outHit, true);

		if (traceHit)
		{
			AGrindRail* hitActor = Cast<AGrindRail>(outHit.GetActor());
			if (hitActor)
			{
				RailCollisionPoint = hitActor->RailSpline->FindLocationClosestToWorldLocation(GetActorLocation(), ESplineCoordinateSpace::World);
				ClosestRailPointDistance = GetClosestDistanceToLocation(hitActor->RailSpline, RailCollisionPoint, 0.5f);

				FVector railTangent = hitActor->RailSpline->GetTangentAtDistanceAlongSpline(ClosestRailPointDistance, ESplineCoordinateSpace::World).GetSafeNormal();
				float grindDirection = FVector::DotProduct(GetActorForwardVector(), railTangent);

				bBackwardsGrind = grindDirection < 0.0f;

				FVector railLocation = hitActor->RailSpline->GetLocationAtDistanceAlongSpline(ClosestRailPointDistance, ESplineCoordinateSpace::World) + (GetActorUpVector() * RailOffset);
				FRotator railRotation = hitActor->RailSpline->GetRotationAtDistanceAlongSpline(ClosestRailPointDistance, ESplineCoordinateSpace::World);

				SetActorLocationAndRotation(railLocation, railRotation);

				RailStartDistance = ClosestRailPointDistance;
				CurrentRail = hitActor->RailSpline;
				bIsGrinding = true;

				GetCharacterMovement()->GravityScale = 0.0f;

				if (GetVelocity().Length() < hitActor->MinRailSpeed)
				{
					FVector minVelocity = hitActor->RailSpline->GetTangentAtDistanceAlongSpline(RailStartDistance, ESplineCoordinateSpace::World).GetSafeNormal() * hitActor->MinRailSpeed;
					SetVelocity(GetRailVelocityInDirection(minVelocity, bBackwardsGrind), true, true, false);
				}
				hitActor->EnterRail(this);

				GetCharacterMovement()->SetMovementMode(MOVE_Flying);
				Cast<USonicMovementComponent>(GetMovementComponent())->bIgnoreGrindingDecel = false;
			}
		}
	}
}

void ASonicGameCharacter::DetectSideRail()
{
	if (bIsGrinding)
	{
		//Right Rail Detection
		FVector rightStart = (GetActorLocation() - FVector(0.0f, 0.0f, 60.0f)) + (GetActorRightVector() * 60.0f);
		FVector rightEnd = rightStart + (GetActorRightVector() * 300.0f);

		//Left Rail Detection
		FVector leftStart = (GetActorLocation() - FVector(0.0f, 0.0f, 60.0f)) + (GetActorRightVector() * -60.0f);
		FVector leftEnd = leftStart + (GetActorRightVector() * -300.0f);

		FHitResult rightOutHit;
		FHitResult leftOutHit;

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
		ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
		ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
		ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1));

		TArray<AActor*> IgnoreActors;

		bool rightHit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), rightStart, rightEnd, 40.0f, ObjectTypesArray, false, IgnoreActors, EDrawDebugTrace::None, rightOutHit, true);
		bool leftHit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), leftStart, leftEnd, 40.0f, ObjectTypesArray, false, IgnoreActors, EDrawDebugTrace::None, leftOutHit, true);
		

		if (rightHit)
		{
			AGrindRail* rightGrindRail = Cast<AGrindRail>(rightOutHit.GetActor());
			if (rightGrindRail)
			{
				if (rightGrindRail->RailSpline != CurrentRail)
				{
					RightRailCollisionPoint = rightOutHit.ImpactPoint;
					RightRail = rightGrindRail->RailSpline;
					RightRailTargetPoint = RightRailCollisionPoint - GetVelocity();
				}
			}
		}
		else
		{
			RightRail = nullptr;
		}

		if (leftHit)
		{
			AGrindRail* leftGrindRail = Cast<AGrindRail>(leftOutHit.GetActor());
			if (leftGrindRail)
			{
				if (leftGrindRail->RailSpline != CurrentRail)
				{
					LeftRailCollisionPoint = leftOutHit.ImpactPoint;
					LeftRail = leftGrindRail->RailSpline;
					LeftRailTargetPoint = LeftRailCollisionPoint - GetVelocity();
				}
			}
		}
		else
		{
			LeftRail = nullptr;
		}
	}
	else
	{
		LeftRail = nullptr;
		RightRail = nullptr;
		bCanSwitchRails = true;
	}
}

void ASonicGameCharacter::RailBoost(FVector Direction)
{
	if (CurrentRail)
	{
		MaxRailSpeed = 3000.0f;
		FVector newVelocity = Direction * MaxRailSpeed;

		// Check if we are moving in the opposite direction of the boost vector
		if (Direction.Dot(GetActorForwardVector()) < 0.0f)
		{
			bBackwardsGrind = !bBackwardsGrind;
			SetVelocity(GetRailVelocityInDirection(newVelocity, bBackwardsGrind), true, true);
		}
		else
		{
			SetVelocity(newVelocity, true, true);
		}

		FTimerDelegate timerDelegate;
		timerDelegate.BindLambda([&]()
			{
				MaxRailSpeed = 2000.0f;
			});

		FTimerHandle delayHandle;

		GetWorld()->GetTimerManager().SetTimer(delayHandle, timerDelegate, 2.0f, false);
	}
}

void ASonicGameCharacter::GrindOnRail(float StartDistance, USplineComponent* Rail)
{
	if (Rail)
	{
		AGrindRail* grindRail = Cast<AGrindRail>(Rail->GetOwner());
		// Check if we are on the rail
		if (StartDistance < Rail->GetSplineLength() && StartDistance > 0.0f)
		{
			// Jump on the rail
			if (bGrindJump)
			{
				grindRail->RailJump();

				FVector velocity = FVector(GetVelocity().X, GetVelocity().Y, 0.0f) * -1.0f;
				FVector launchUp = Rail->GetUpVectorAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::World) * RailJumpHeight;
				FVector launchVelocity = FVector(velocity.X, velocity.Y, launchUp.Z);
				LaunchCharacter(launchVelocity, true, true);

				bIsGrinding = false;
				GetCharacterMovement()->GravityScale = 1.0f;
			}
			// Move along the rail
			else
			{
				// Calculate rail velocity
				FVector railVelocity = Rail->GetTangentAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::World).GetSafeNormal() * GetVelocity().Length();
				FVector originalRailVelocity = railVelocity;
				railVelocity = GetRailVelocityInDirection(railVelocity, bBackwardsGrind).GetClampedToSize(0.0f, MaxRailSpeed);

				// Add friction based on angle of the rail
				railVelocity += (GetActorForwardVector() * RailAccelerationMultiplier * GetWorld()->GetDeltaSeconds()) * GetActorRotation().Pitch;
				if(railVelocity.Length() < 1.0f)
					bBackwardsGrind = !bBackwardsGrind;


				SetVelocity(railVelocity, true, true);

				// Calculate player's location on the rail
				FVector locationOnRail = Rail->GetLocationAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::World) + (GetActorUpVector() * RailOffset);
				
				// Calculate player's rotation on the rail
				FVector railUp = Rail->GetUpVectorAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::World);
				float railRoll = Rail->GetRollAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::World);
				FRotator rotationOnRail = UKismetMathLibrary::MakeRotFromXZ(UKismetMathLibrary::SelectVector(originalRailVelocity * -1.0f, originalRailVelocity, bBackwardsGrind), railUp);
				rotationOnRail.Roll = railRoll;

				SetActorLocationAndRotation(locationOnRail, rotationOnRail);

				// Calculate delta to increment/decrement RailStartDistance by
				float prevVelocityDelta = (float)(GetVelocity() * GetWorld()->GetDeltaSeconds()).Length();
				float velocityDelta = (float)(railVelocity.Length() * GetWorld()->GetDeltaSeconds());
				float railDelta = prevVelocityDelta + velocityDelta;
				float zRange = UKismetMathLibrary::MapRangeClamped(GetActorForwardVector().Z, -1.0f, 1.0f, 1.15f, 0.55f);

				float forwardDelta = RailStartDistance + (railDelta * zRange);
				float backwardsDelta = RailStartDistance - (railDelta * zRange);

				// Update RailStartDistance
				RailStartDistance = UKismetMathLibrary::SelectFloat(backwardsDelta, forwardDelta, bBackwardsGrind);
			}
		}
		else
		{
			// Circle back to beginning if rail is a closed loop
			if (Rail->IsClosedLoop())
			{
				RailStartDistance = UKismetMathLibrary::SelectFloat(Rail->GetSplineLength(), 0.0f, bBackwardsGrind);
			}
			// Exit the rail if we reach either end
			else
			{
				bIsGrinding = false;
				GetCharacterMovement()->GravityScale = 1.0f;
				grindRail->SetActorEnableCollision(false);

				FVector launchVelocity = GetActorForwardVector() * GetVelocity().Length();
				LaunchCharacter(launchVelocity, true, true);

				GetCharacterMovement()->SetMovementMode(MOVE_Walking);
				Cast<USonicMovementComponent>(GetMovementComponent())->bIgnoreGrindingDecel = true;

				grindRail->ExitRail();
			}
		}
	}
}

float ASonicGameCharacter::GetClosestDistanceToLocation(USplineComponent* Spline, FVector Location, float ErrorTolerance)
{
	for (int i = 0; i < (int)Spline->GetSplineLength(); i++)
	{
		FVector splineLocation = Spline->GetLocationAtDistanceAlongSpline(i, ESplineCoordinateSpace::World);
		float distance = FVector::Distance(splineLocation, Location);
		
		if(FMath::IsNearlyEqual(distance, 0.0f, ErrorTolerance))
			return (float)i;
	}

	return -1.0f;
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

	DetectGrindRail();
	DetectSideRail();
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
	if ((Controller != nullptr) && (Value != 0.0f) && bCanMove && !bIsGrinding)
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
	if ( (Controller != nullptr) && (Value != 0.0f) && bCanMove && !bIsGrinding && (GetCharacterMovement()->MovementMode != MOVE_Flying))
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
