#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FpsHUD.generated.h"

UCLASS()
class SHOOTER_API AFpsHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	AFpsHUD();
	virtual void DrawHUD() override;
	UTexture2D* CrosshairTexture;
};
