#include "SlateLab.h"

void SlateLab::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SAssignNew(LabWidget, SlateLabWidget)
		.LabText(this, &SlateLab::GetLabText)
		.LabText2(FText::FromString("Default Lab Text 2"))
	];
}

FText SlateLab::GetLabText() const
{
	return LabText;
}
