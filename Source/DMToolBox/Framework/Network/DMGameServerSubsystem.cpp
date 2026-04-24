#include "DMGameServerSubsystem.h"

#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "TimerManager.h"

void UDMGameServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ServerBaseUrl.RemoveFromEnd(TEXT("/"));
}

void UDMGameServerSubsystem::Deinitialize()
{
	StopWatchingRoom();
	Super::Deinitialize();
}

void UDMGameServerSubsystem::SetServerBaseUrl(const FString& InServerBaseUrl)
{
	ServerBaseUrl = InServerBaseUrl.TrimStartAndEnd();
	ServerBaseUrl.RemoveFromEnd(TEXT("/"));

	if (ServerBaseUrl.IsEmpty())
	{
		ServerBaseUrl = TEXT("http://127.0.0.1:7788");
	}
}

void UDMGameServerSubsystem::SendRequest(const FString& Verb, const FString& Path, const FString& Body, FDMGameServerResponseDelegate Callback)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	const FString NormalizedVerb = Verb.IsEmpty() ? TEXT("GET") : Verb.ToUpper();
	Request->SetURL(BuildUrl(Path));
	Request->SetVerb(NormalizedVerb);
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));

	if (!Body.IsEmpty())
	{
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Request->SetContentAsString(Body);
	}

	Request->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr RequestPtr, FHttpResponsePtr Response, bool bSucceeded)
		{
			const int32 ResponseCode = Response.IsValid() ? Response->GetResponseCode() : 0;
			const FString ResponseBody = Response.IsValid() ? Response->GetContentAsString() : FString();
			const bool bRequestSucceeded = bSucceeded && Response.IsValid() && ResponseCode >= 200 && ResponseCode < 300;

			Callback.ExecuteIfBound(bRequestSucceeded, ResponseCode, ResponseBody);
		});

	if (!Request->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, 0, TEXT(""));
	}
}

void UDMGameServerSubsystem::Login(const FString& AccountId, const FString& PlayerName, FDMGameServerResponseDelegate Callback)
{
	TMap<FString, FString> Fields;
	Fields.Add(TEXT("accountId"), AccountId);
	Fields.Add(TEXT("playerName"), PlayerName);
	SendRequest(TEXT("POST"), TEXT("/accounts/login"), BuildJsonString(Fields), Callback);
}

void UDMGameServerSubsystem::UpdateAccountName(const FString& AccountId, const FString& PlayerName, FDMGameServerResponseDelegate Callback)
{
	TMap<FString, FString> Fields;
	Fields.Add(TEXT("playerName"), PlayerName);
	SendRequest(TEXT("PATCH"), FString::Printf(TEXT("/accounts/%s"), *AccountId), BuildJsonString(Fields), Callback);
}

void UDMGameServerSubsystem::GetLobby(FDMGameServerResponseDelegate Callback)
{
	SendRequest(TEXT("GET"), TEXT("/lobby"), TEXT(""), Callback);
}

void UDMGameServerSubsystem::EnterLobby(const FString& AccountId, FDMGameServerResponseDelegate Callback)
{
	TMap<FString, FString> Fields;
	Fields.Add(TEXT("accountId"), AccountId);
	SendRequest(TEXT("POST"), TEXT("/lobby/enter"), BuildJsonString(Fields), Callback);
}

void UDMGameServerSubsystem::LeaveLobby(const FString& AccountId, FDMGameServerResponseDelegate Callback)
{
	TMap<FString, FString> Fields;
	Fields.Add(TEXT("accountId"), AccountId);
	SendRequest(TEXT("POST"), TEXT("/lobby/leave"), BuildJsonString(Fields), Callback);
}

void UDMGameServerSubsystem::GetRooms(FDMGameServerResponseDelegate Callback)
{
	SendRequest(TEXT("GET"), TEXT("/rooms"), TEXT(""), Callback);
}

void UDMGameServerSubsystem::GetRoom(const FString& RoomId, FDMGameServerResponseDelegate Callback)
{
	SendRequest(TEXT("GET"), FString::Printf(TEXT("/rooms/%s"), *RoomId), TEXT(""), Callback);
}

void UDMGameServerSubsystem::CreateRoom(const FString& HostAccountId, const FString& RoomName, const int32 MaxPlayers, FDMGameServerResponseDelegate Callback)
{
	SendRequest(TEXT("POST"), TEXT("/rooms"), BuildCreateRoomJsonString(HostAccountId, RoomName, MaxPlayers), Callback);
}

void UDMGameServerSubsystem::JoinRoom(const FString& RoomId, const FString& AccountId, FDMGameServerResponseDelegate Callback)
{
	TMap<FString, FString> Fields;
	Fields.Add(TEXT("accountId"), AccountId);
	SendRequest(TEXT("POST"), FString::Printf(TEXT("/rooms/%s/join"), *RoomId), BuildJsonString(Fields), Callback);
}

void UDMGameServerSubsystem::LeaveRoom(const FString& RoomId, const FString& AccountId, FDMGameServerResponseDelegate Callback)
{
	TMap<FString, FString> Fields;
	Fields.Add(TEXT("accountId"), AccountId);
	SendRequest(TEXT("POST"), FString::Printf(TEXT("/rooms/%s/leave"), *RoomId), BuildJsonString(Fields), Callback);
}

void UDMGameServerSubsystem::UpdateRoomMemberReady(const FString& RoomId, const FString& AccountId, const bool bIsReady, FDMGameServerResponseDelegate Callback)
{
	SendRequest(TEXT("PATCH"), FString::Printf(TEXT("/rooms/%s/members/%s"), *RoomId, *AccountId), BuildReadyJsonString(bIsReady), Callback);
}

void UDMGameServerSubsystem::StartGame(const FString& RoomId, FDMGameServerResponseDelegate Callback)
{
	SendRequest(TEXT("POST"), TEXT("/game/start"), BuildStartGameJsonString(RoomId), Callback);
}

void UDMGameServerSubsystem::StartWatchingRoom(const FString& RoomId, const float PollIntervalSeconds)
{
	StopWatchingRoom();

	WatchedRoomId = RoomId.TrimStartAndEnd();
	if (WatchedRoomId.IsEmpty())
	{
		return;
	}

	LastWatchedRoomResponseBody.Empty();
	bWatchRoomRequestPending = false;

	PollWatchedRoom();

	if (UWorld* World = GetWorld())
	{
		const float ResolvedPollIntervalSeconds = FMath::Max(PollIntervalSeconds, 0.2f);
		World->GetTimerManager().SetTimer(WatchRoomTimerHandle, this, &UDMGameServerSubsystem::PollWatchedRoom, ResolvedPollIntervalSeconds, true);
	}
}

void UDMGameServerSubsystem::StopWatchingRoom()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WatchRoomTimerHandle);
	}

	WatchedRoomId.Empty();
	LastWatchedRoomResponseBody.Empty();
	bWatchRoomRequestPending = false;
}

void UDMGameServerSubsystem::PollWatchedRoom()
{
	if (WatchedRoomId.IsEmpty() || bWatchRoomRequestPending)
	{
		return;
	}

	bWatchRoomRequestPending = true;

	FDMGameServerResponseDelegate Callback;
	Callback.BindDynamic(this, &UDMGameServerSubsystem::HandleWatchedRoomResponse);
	GetRoom(WatchedRoomId, Callback);
}

void UDMGameServerSubsystem::HandleWatchedRoomResponse(bool bSucceeded, int32 ResponseCode, const FString& ResponseBody)
{
	(void)ResponseCode;
	bWatchRoomRequestPending = false;

	if (WatchedRoomId.IsEmpty() || !bSucceeded)
	{
		return;
	}

	if (ResponseBody == LastWatchedRoomResponseBody)
	{
		return;
	}

	LastWatchedRoomResponseBody = ResponseBody;
	OnWatchedRoomUpdated.Broadcast(WatchedRoomId, ResponseBody);
}

FString UDMGameServerSubsystem::BuildUrl(const FString& Path) const
{
	if (Path.StartsWith(TEXT("http://")) || Path.StartsWith(TEXT("https://")))
	{
		return Path;
	}

	const FString NormalizedPath = Path.StartsWith(TEXT("/")) ? Path : FString::Printf(TEXT("/%s"), *Path);
	return ServerBaseUrl + NormalizedPath;
}

FString UDMGameServerSubsystem::BuildJsonString(const TMap<FString, FString>& StringFields)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	for (const TPair<FString, FString>& Field : StringFields)
	{
		JsonObject->SetStringField(Field.Key, Field.Value);
	}

	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(JsonObject, Writer);
	return Body;
}

FString UDMGameServerSubsystem::BuildCreateRoomJsonString(const FString& HostAccountId, const FString& RoomName, const int32 MaxPlayers)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("hostAccountId"), HostAccountId);
	JsonObject->SetStringField(TEXT("roomName"), RoomName);
	JsonObject->SetNumberField(TEXT("maxPlayers"), MaxPlayers);

	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(JsonObject, Writer);
	return Body;
}

FString UDMGameServerSubsystem::BuildReadyJsonString(const bool bIsReady)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetBoolField(TEXT("bIsReady"), bIsReady);

	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(JsonObject, Writer);
	return Body;
}

FString UDMGameServerSubsystem::BuildStartGameJsonString(const FString& RoomId)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("roomId"), RoomId);

	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(JsonObject, Writer);
	return Body;
}
