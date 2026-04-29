#pragma once

#include "CoreMinimal.h"
#include "DMGameServerTypes.generated.h"

USTRUCT(BlueprintType)
struct FDMGameServerPublicAccount
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString AccountId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString PlayerName;
};

USTRUCT(BlueprintType)
struct FDMGameServerAuthRequestPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString AccountId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString PlayerName;
};

USTRUCT(BlueprintType)
struct FDMGameServerTokenRefreshPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Token;
};

USTRUCT(BlueprintType)
struct FDMGameServerTokenVerifyRequestPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Token;
};

USTRUCT(BlueprintType)
struct FDMGameServerTokenVerifyPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOk = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ErrorCode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDMGameServerPublicAccount Account;
};

USTRUCT(BlueprintType)
struct FDMGameServerCurrentRoomData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString RoomId;
};

USTRUCT(BlueprintType)
struct FDMGameServerCurrentRoomPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOk = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ErrorCode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDMGameServerCurrentRoomData Data;
};

USTRUCT(BlueprintType)
struct FDMGameServerRoomMember
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsHost = false;
};

USTRUCT(BlueprintType)
struct FDMGameServerRoom
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString RoomId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString RoomName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxPlayers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDMGameServerRoomMember> Members;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ServerAddress;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Status = TEXT("waiting");

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsSynchronized = false;
};

USTRUCT(BlueprintType)
struct FDMGameServerCreateRoomPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString HostAccountId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString RoomName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxPlayers = 8;
};

USTRUCT(BlueprintType)
struct FDMGameServerRoomIdPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString RoomId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString AccountId;
};

USTRUCT(BlueprintType)
struct FDMGameServerResponsePayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOk = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ErrorCode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Message;
};

USTRUCT(BlueprintType)
struct FDMGameServerRoomPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOk = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ErrorCode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDMGameServerRoom Data;
};

USTRUCT(BlueprintType)
struct FDMGameServerLobbyRoomListPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOk = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ErrorCode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDMGameServerRoom> Data;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDMGameServerPublicAccount> OnlineAccounts;
};

USTRUCT(BlueprintType)
struct FDMGameServerStartGameData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ServerAddress;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Ticket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDMGameServerRoom Room;
};

USTRUCT(BlueprintType)
struct FDMGameServerStartGamePayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOk = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ErrorCode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDMGameServerStartGameData Data;
};
