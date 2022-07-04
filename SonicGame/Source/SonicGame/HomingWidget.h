// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HomingWidget.generated.h"

/**
 * 
 */
UCLASS()
class SONICGAME_API UHomingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHomingIcon();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHomingIcon();
};
