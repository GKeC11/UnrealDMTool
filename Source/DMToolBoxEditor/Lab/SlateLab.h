#pragma once
#include "SlateLabWidget.h"

class SlateLab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SlateLab){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

private:
	FText GetLabText() const;
	
private:
	TSharedPtr<SlateLabWidget> LabWidget = nullptr;
	
	FText LabText;
};
