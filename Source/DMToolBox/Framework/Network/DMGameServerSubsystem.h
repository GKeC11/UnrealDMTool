#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerManager.h"
#include "DMGameServerSubsystem.generated.h"

class IWebSocket;

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FDMGameServerResponseDelegate, bool, bSucceeded, int32, ResponseCode, const FString&, ResponseBody);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDMGameServerConnectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDMGameServerDisconnectedDelegate, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDMGameServerMessageDelegate, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDMGameServerErrorDelegate, const FString&, Error);

struct FDMGameServerPendingRequest
{
	FName RequestProtocolName;
	int32 RequestType = 0;
	FString PayloadJson;
	FName ResponseProtocolName;
	int32 ResponseType = 0;
	FDMGameServerResponseDelegate Callback;
};

UCLASS()
class DMTOOLBOX_API UDMGameServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "DMToolBox|GameServer")
	FDMGameServerConnectedDelegate OnGameServerConnected;

	UPROPERTY(BlueprintAssignable, Category = "DMToolBox|GameServer")
	FDMGameServerDisconnectedDelegate OnGameServerDisconnected;

	UPROPERTY(BlueprintAssignable, Category = "DMToolBox|GameServer")
	FDMGameServerMessageDelegate OnGameServerMessage;

	UPROPERTY(BlueprintAssignable, Category = "DMToolBox|GameServer")
	FDMGameServerErrorDelegate OnGameServerError;

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer")
	void SetServerUrl(const FString& InServerUrl);

	UFUNCTION(BlueprintPure, Category = "DMToolBox|GameServer")
	FString GetServerUrl() const { return ServerUrl; }

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer")
	void Connect();

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer")
	void SendTextMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer")
	void SendPing();

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer")
	void SendProtocolRequest(FName RequestProtocolName, int32 RequestType, const FString& PayloadJson, FName ResponseProtocolName, int32 ResponseType, FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintPure, Category = "DMToolBox|GameServer")
	bool IsConnected() const;

private:
	static FString NormalizeServerUrl(const FString& InServerUrl);
	void SendProtocolMessage(FName ProtocolName, int32 Type, const FString& PayloadJson);
	void BindSocketEvents();
	void ClearSocket();
	void ClearSocket(TSharedPtr<IWebSocket> SocketToClear);
	void ScheduleClearSocket(TSharedPtr<IWebSocket> SocketToClear);
	void SendPendingRequest(FDMGameServerPendingRequest& Request);
	void FlushPendingRequests();
	void FailPendingRequests(int32 ResponseCode, const FString& Reason);
	void HandleServerMessage(const FString& Message);
	bool TryExecuteProtocolCallback(int32 Type, const FString& Message, const TSharedPtr<class FJsonObject>& PayloadObject);
	void StartHeartbeat();
	void StopHeartbeat();
	void HandleHeartbeatTick();

	UPROPERTY()
	FString ServerUrl = TEXT("ws://127.0.0.1:7788/ws");

	UPROPERTY(EditDefaultsOnly, Category = "DMToolBox|GameServer")
	float HeartbeatIntervalSeconds = 30.0f;

	TSharedPtr<IWebSocket> WebSocket;
	TArray<FDMGameServerPendingRequest> PendingRequests;
	TMap<int32, FDMGameServerPendingRequest> PendingResponseCallbacks;
	FTimerHandle HeartbeatTimerHandle;
	bool bIsConnecting = false;
};
