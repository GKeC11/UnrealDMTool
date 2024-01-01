// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlateLabStyle.h"


class SLATELAB_API FSlateLabCommands : public TCommands<FSlateLabCommands>
{
public:
	
	FSlateLabCommands()
		: TCommands<FSlateLabCommands>
		(
			TEXT("SlateLab"),
			NSLOCTEXT("Contexts", "SlateLab", "Slate Lab"),
			NAME_None,
			FSlateLabStyle::GetStyleSetName()
		)
	{
	}

	virtual void RegisterCommands() override;

public:
	
	TSharedPtr<FUICommandInfo> OpenSlateLab;
};
