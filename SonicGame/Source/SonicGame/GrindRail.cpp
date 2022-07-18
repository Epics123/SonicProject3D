// Fill out your copyright notice in the Description page of Project Settings.


#include "GrindRail.h"

// Sets default values
AGrindRail::AGrindRail()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RailSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RailSpline"));
	RailSpline->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AGrindRail::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGrindRail::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

