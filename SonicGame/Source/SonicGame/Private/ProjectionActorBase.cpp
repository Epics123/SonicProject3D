// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectionActorBase.h"

// Sets default values
AProjectionActorBase::AProjectionActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AProjectionActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectionActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

