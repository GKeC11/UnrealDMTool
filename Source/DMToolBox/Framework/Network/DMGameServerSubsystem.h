#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DMGameServerSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FDMGameServerResponseDelegate, bool, bSucceeded, int32, ResponseCode, const FString&, ResponseBody);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDMGameServerRoomUpdatedDelegate, const FString&, RoomId, const FString&, ResponseBody);

UCLASS()
class DMTOOLBOX_API UDMGameServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "DMToolBox|GameServer|Room")
	FDMGameServerRoomUpdatedDelegate OnWatchedRoomUpdated;

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer")
	void SetServerBaseUrl(const FString& InServerBaseUrl);

	UFUNCTION(BlueprintPure, Category = "DMToolBox|GameServer")
	FString GetServerBaseUrl() const { return ServerBaseUrl; }

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer")
	void SendRequest(const FString& Verb, const FString& Path, const FString& Body, FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Account")
	void Login(const FString& AccountId, const FString& PlayerName, FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Account")
	void UpdateAccountName(const FString& AccountId, const FString& PlayerName, FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Lobby")
	void GetLobby(FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Lobby")
	void EnterLobby(const FString& AccountId, FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Lobby")
	void LeaveLobby(const FString& AccountId, FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Room")
	void GetRooms(FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Room")
	void GetRoom(const FString& RoomId, FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Room")
	void CreateRoom(const FString& HostAccountId, const FString& RoomName, int32 MaxPlayers, FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Room")
	void JoinRoom(const FString& RoomId, const FString& AccountId, FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Room")
	void LeaveRoom(const FString& RoomId, const FString& AccountId, FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Room")
	void UpdateRoomMemberReady(const FString& RoomId, const FString& AccountId, bool bIsReady, FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Game")
	void StartGame(const FString& RoomId, FDMGameServerResponseDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Room")
	void StartWatchingRoom(const FString& RoomId, float PollIntervalSeconds = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|GameServer|Room")
	void StopWatchingRoom();

private:
	FString BuildUrl(const FString& Path) const;
	static FString BuildJsonString(const TMap<FString, FString>& StringFields);
	static FString BuildCreateRoomJsonString(const FString& HostAccountId, const FString& RoomName, int32 MaxPlayers);
	static FString BuildReadyJsonString(bool bIsReady);
	static FString BuildStartGameJsonString(const FString& RoomId);

	void PollWatchedRoom();

	UFUNCTION()
	void HandleWatchedRoomResponse(bool bSucceeded, int32 ResponseCode, const FString& ResponseBody);

	UPROPERTY()
	FString ServerBaseUrl = TEXT("http://127.0.0.1:7788");

	UPROPERTY()
	FString WatchedRoomId;

	UPROPERTY()
	FString LastWatchedRoomResponseBody;

	FTimerHandle WatchRoomTimerHandle;
	bool bWatchRoomRequestPending = false;
};
