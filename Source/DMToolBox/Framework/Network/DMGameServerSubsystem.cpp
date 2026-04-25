#include "DMGameServerSubsystem.h"

#include "DMToolBox/Framework/Common/DMMacros.h"
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

	// Normalize once at startup so the request builder can safely append relative API paths.
	ServerBaseUrl.RemoveFromEnd(TEXT("/"));
	DM_LOG(this, LogTemp, Log, TEXT("Initialize: ServerBaseUrl=%s"), *ServerBaseUrl);
}

void UDMGameServerSubsystem::Deinitialize()
{
	DM_LOG(this, LogTemp, Log, TEXT("Deinitialize: WatchedRoomId=%s"), *WatchedRoomId);
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

	DM_LOG(this, LogTemp, Log, TEXT("SetServerBaseUrl: ServerBaseUrl=%s"), *ServerBaseUrl);
}

void UDMGameServerSubsystem::SendRequest(const FString& Verb, const FString& Path, const FString& Body, FDMGameServerResponseDelegate Callback)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	const FString NormalizedVerb = Verb.IsEmpty() ? TEXT("GET") : Verb.ToUpper();
	const FString ResolvedUrl = BuildUrl(Path);
	Request->SetURL(ResolvedUrl);
	Request->SetVerb(NormalizedVerb);
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));

	if (!Body.IsEmpty())
	{
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Request->SetContentAsString(Body);
	}

	DM_LOG(this, LogTemp, Log, TEXT("SendRequest begin: Verb=%s, Path=%s, Url=%s, HasBody=%s"),
		*NormalizedVerb, *Path, *ResolvedUrl, Body.IsEmpty() ? TEXT("false") : TEXT("true"));

	TWeakObjectPtr<UDMGameServerSubsystem> WeakThis(this);
	Request->OnProcessRequestComplete().BindLambda(
		[WeakThis, Callback, NormalizedVerb, Path, ResolvedUrl](FHttpRequestPtr RequestPtr, FHttpResponsePtr Response, bool bSucceeded)
		{
			(void)RequestPtr;

			UDMGameServerSubsystem* Subsystem = WeakThis.Get();
			if (!Subsystem)
			{
				UE_LOG(LogTemp, Warning, TEXT("[DMGameServerSubsystem][SendRequest] Response ignored because subsystem is no longer valid. Verb=%s, Path=%s, Url=%s"),
					*NormalizedVerb, *Path, *ResolvedUrl);
				return;
			}

			// Collapse Unreal HTTP completion state into one gameplay-facing success flag for Blueprint and TS callers.
			const int32 ResponseCode = Response.IsValid() ? Response->GetResponseCode() : 0;
			const FString ResponseBody = Response.IsValid() ? Response->GetContentAsString() : FString();
			const bool bRequestSucceeded = bSucceeded && Response.IsValid() && ResponseCode >= 200 && ResponseCode < 300;

			if (bRequestSucceeded)
			{
				DM_LOG(Subsystem, LogTemp, Log, TEXT("SendRequest end: Verb=%s, Path=%s, Url=%s, Succeeded=true, ResponseCode=%d, ResponseBodyLength=%d"),
					*NormalizedVerb, *Path, *ResolvedUrl, ResponseCode, ResponseBody.Len());
			}
			else
			{
				DM_LOG(Subsystem, LogTemp, Warning, TEXT("SendRequest end: Verb=%s, Path=%s, Url=%s, Succeeded=false, ResponseCode=%d, ResponseBodyLength=%d"),
					*NormalizedVerb, *Path, *ResolvedUrl, ResponseCode, ResponseBody.Len());
			}

			Callback.ExecuteIfBound(bRequestSucceeded, ResponseCode, ResponseBody);
		});

	if (!Request->ProcessRequest())
	{
		DM_LOG(this, LogTemp, Warning, TEXT("SendRequest failed to start: Verb=%s, Path=%s, Url=%s"), *NormalizedVerb, *Path, *ResolvedUrl);
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
	// Watching is single-room state in this subsystem, so switching targets always resets the previous timer and cache.
	StopWatchingRoom();

	WatchedRoomId = RoomId.TrimStartAndEnd();
	if (WatchedRoomId.IsEmpty())
	{
		DM_LOG(this, LogTemp, Warning, TEXT("StartWatchingRoom skipped: RoomId is empty."));
		return;
	}

	LastWatchedRoomResponseBody.Empty();
	bWatchRoomRequestPending = false;
	DM_LOG(this, LogTemp, Log, TEXT("StartWatchingRoom: RoomId=%s, PollIntervalSeconds=%.2f"), *WatchedRoomId, PollIntervalSeconds);

	// Poll once immediately so UI listeners do not wait for the first timer tick.
	PollWatchedRoom();

	if (UWorld* World = GetWorld())
	{
		// Keep a small lower bound to avoid hammering the local test server with near-zero intervals.
		const float ResolvedPollIntervalSeconds = FMath::Max(PollIntervalSeconds, 0.2f);
		World->GetTimerManager().SetTimer(WatchRoomTimerHandle, this, &UDMGameServerSubsystem::PollWatchedRoom, ResolvedPollIntervalSeconds, true);
		DM_LOG(this, LogTemp, Log, TEXT("StartWatchingRoom timer started: RoomId=%s, ResolvedPollIntervalSeconds=%.2f"),
			*WatchedRoomId, ResolvedPollIntervalSeconds);
	}
	else
	{
		DM_LOG(this, LogTemp, Warning, TEXT("StartWatchingRoom timer skipped: World is invalid. RoomId=%s"), *WatchedRoomId);
	}
}

void UDMGameServerSubsystem::StopWatchingRoom()
{
	const FString PreviousWatchedRoomId = WatchedRoomId;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WatchRoomTimerHandle);
	}

	WatchedRoomId.Empty();
	LastWatchedRoomResponseBody.Empty();
	bWatchRoomRequestPending = false;
	DM_LOG(this, LogTemp, Log, TEXT("StopWatchingRoom: PreviousRoomId=%s"), *PreviousWatchedRoomId);
}

void UDMGameServerSubsystem::PollWatchedRoom()
{
	if (WatchedRoomId.IsEmpty() || bWatchRoomRequestPending)
	{
		DM_LOG(this, LogTemp, Log, TEXT("PollWatchedRoom skipped: RoomId=%s, RequestPending=%s"),
			*WatchedRoomId, bWatchRoomRequestPending ? TEXT("true") : TEXT("false"));
		return;
	}

	// Gate re-entry so a slow HTTP response cannot stack multiple overlapping room polls.
	bWatchRoomRequestPending = true;
	DM_LOG(this, LogTemp, Log, TEXT("PollWatchedRoom begin: RoomId=%s"), *WatchedRoomId);

	FDMGameServerResponseDelegate Callback;
	Callback.BindDynamic(this, &UDMGameServerSubsystem::HandleWatchedRoomResponse);
	GetRoom(WatchedRoomId, Callback);
}

void UDMGameServerSubsystem::HandleWatchedRoomResponse(bool bSucceeded, int32 ResponseCode, const FString& ResponseBody)
{
	(void)ResponseCode;
	bWatchRoomRequestPending = false;

	// Ignore stale responses after StopWatchingRoom or any failed poll; callers only react to fresh successful snapshots.
	if (WatchedRoomId.IsEmpty() || !bSucceeded)
	{
		DM_LOG(this, LogTemp, Warning, TEXT("HandleWatchedRoomResponse ignored: RoomId=%s, Succeeded=%s, ResponseCode=%d"),
			*WatchedRoomId, bSucceeded ? TEXT("true") : TEXT("false"), ResponseCode);
		return;
	}

	// Broadcast only when the payload changes so polling can drive UI refresh without duplicate work every interval.
	if (ResponseBody == LastWatchedRoomResponseBody)
	{
		DM_LOG(this, LogTemp, Log, TEXT("HandleWatchedRoomResponse skipped broadcast: RoomId=%s, ResponseUnchanged=true"), *WatchedRoomId);
		return;
	}

	LastWatchedRoomResponseBody = ResponseBody;
	DM_LOG(this, LogTemp, Log, TEXT("HandleWatchedRoomResponse broadcast: RoomId=%s, ResponseBodyLength=%d"),
		*WatchedRoomId, ResponseBody.Len());
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
