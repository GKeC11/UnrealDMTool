#pragma once

class SlateLabWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SlateLabWidget){}
		SLATE_ATTRIBUTE(FText, LabText)
		SLATE_ARGUMENT(FText, LabText2)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);
};
