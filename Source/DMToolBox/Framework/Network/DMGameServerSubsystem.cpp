#include "DMGameServerSubsystem.h"

#include "Dom/JsonObject.h"
#include "DMToolBox/Framework/Common/DMMacros.h"
#include "IWebSocket.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Modules/ModuleManager.h"
#include "WebSocketsModule.h"

namespace
{
	constexpr int32 ProtocolBasePing = 1;
	constexpr int32 ProtocolBasePong = 2;
}

void UDMGameServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!FModuleManager::Get().IsModuleLoaded(TEXT("WebSockets")))
	{
		FModuleManager::LoadModuleChecked<FWebSocketsModule>(TEXT("WebSockets"));
	}

	DM_LOG(this, LogTemp, Log, TEXT("Initialize: ServerUrl=%s"), *ServerUrl);
}

void UDMGameServerSubsystem::Deinitialize()
{
	DM_LOG(this, LogTemp, Log, TEXT("Deinitialize: Connected=%s"), IsConnected() ? TEXT("true") : TEXT("false"));
	Disconnect();
	Super::Deinitialize();
}

void UDMGameServerSubsystem::SetServerUrl(const FString& InServerUrl)
{
	ServerUrl = NormalizeServerUrl(InServerUrl);

	DM_LOG(this, LogTemp, Log, TEXT("SetServerUrl: ServerUrl=%s"), *ServerUrl);
}

void UDMGameServerSubsystem::Connect()
{
	if (IsConnected())
	{
		DM_LOG(this, LogTemp, Log, TEXT("Connect skipped: already connected. ServerUrl=%s"), *ServerUrl);
		return;
	}

	if (bIsConnecting)
	{
		DM_LOG(this, LogTemp, Log, TEXT("Connect skipped: connection is already pending. ServerUrl=%s"), *ServerUrl);
		return;
	}

	if (WebSocket.IsValid())
	{
		DM_LOG(this, LogTemp, Warning, TEXT("Connect found a stale socket. ServerUrl=%s"), *ServerUrl);
		ClearSocket();
	}

	bIsConnecting = true;
	DM_LOG(this, LogTemp, Log, TEXT("Connect begin: ServerUrl=%s, Protocols=<none>, UpgradeHeaders=<none>"), *ServerUrl);
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerUrl);
	BindSocketEvents();
	WebSocket->Connect();
}

void UDMGameServerSubsystem::Disconnect()
{
	StopHeartbeat();

	if (!WebSocket.IsValid())
	{
		bIsConnecting = false;
		FailPendingRequests(0, TEXT("GameServer socket is not connected."));
		DM_LOG(this, LogTemp, Log, TEXT("Disconnect skipped: socket is invalid. ServerUrl=%s"), *ServerUrl);
		return;
	}

	if (!WebSocket->IsConnected() && !bIsConnecting)
	{
		FailPendingRequests(0, TEXT("GameServer socket is not connected."));
		DM_LOG(this, LogTemp, Log, TEXT("Disconnect skipped: socket is already disconnected. ServerUrl=%s"), *ServerUrl);
		ClearSocket();
		return;
	}

	DM_LOG(this, LogTemp, Log, TEXT("Disconnect: ServerUrl=%s, Connected=%s"),
		*ServerUrl,
		WebSocket->IsConnected() ? TEXT("true") : TEXT("false"));

	bIsConnecting = false;
	FailPendingRequests(0, TEXT("GameServer disconnected."));
	WebSocket->Close();
	ClearSocket();
}

void UDMGameServerSubsystem::SendTextMessage(const FString& Message)
{
	if (!IsConnected())
	{
		DM_LOG(this, LogTemp, Warning, TEXT("SendTextMessage failed: socket is not connected. Message=%s"), *Message);
		return;
	}

	WebSocket->Send(Message);
	DM_LOG(this, LogTemp, Log, TEXT("SendTextMessage: Message=%s"), *Message);
}

void UDMGameServerSubsystem::SendPing()
{
	if (!IsConnected())
	{
		DM_LOG(this, LogTemp, Warning, TEXT("SendPing skipped: socket is not connected."));
		return;
	}

	// ProtocolBase.PING = 1. Keep the payload empty for GameServer heartbeat refresh.
	WebSocket->Send(FString::Printf(TEXT("{\"type\":%d,\"payload\":{}}"), ProtocolBasePing));
}

void UDMGameServerSubsystem::SendProtocolRequest(
	const FName RequestProtocolName,
	const int32 RequestType,
	const FString& PayloadJson,
	const FName ResponseProtocolName,
	const int32 ResponseType,
	FDMGameServerResponseDelegate Callback)
{
	if (PendingResponseCallbacks.Contains(ResponseType))
	{
		Callback.ExecuteIfBound(false, ResponseType, FString::Printf(TEXT("Protocol request already pending. response=%s"), *ResponseProtocolName.ToString()));
		return;
	}

	FDMGameServerPendingRequest Request;
	Request.RequestProtocolName = RequestProtocolName;
	Request.RequestType = RequestType;
	Request.PayloadJson = PayloadJson;
	Request.ResponseProtocolName = ResponseProtocolName;
	Request.ResponseType = ResponseType;
	Request.Callback = Callback;

	if (IsConnected())
	{
		SendPendingRequest(Request);
		return;
	}

	PendingRequests.Add(Request);
	DM_LOG(this, LogTemp, Log, TEXT("Protocol request waits for GameServer connection. request=%s, type=%d, response=%s"),
		*RequestProtocolName.ToString(),
		RequestType,
		*ResponseProtocolName.ToString());
	Connect();
}

bool UDMGameServerSubsystem::IsConnected() const
{
	return WebSocket.IsValid() && WebSocket->IsConnected();
}

FString UDMGameServerSubsystem::NormalizeServerUrl(const FString& InServerUrl)
{
	FString Url = InServerUrl.TrimStartAndEnd();
	if (Url.IsEmpty())
	{
		return TEXT("ws://127.0.0.1:7788/ws");
	}

	if (Url.StartsWith(TEXT("http://")))
	{
		Url.RemoveFromStart(TEXT("http://"));
		Url = FString::Printf(TEXT("ws://%s"), *Url);
	}
	else if (Url.StartsWith(TEXT("https://")))
	{
		Url.RemoveFromStart(TEXT("https://"));
		Url = FString::Printf(TEXT("wss://%s"), *Url);
	}

	Url.RemoveFromEnd(TEXT("/"));
	if (!Url.EndsWith(TEXT("/ws")))
	{
		Url += TEXT("/ws");
	}

	return Url;
}

void UDMGameServerSubsystem::SendProtocolMessage(const FName ProtocolName, const int32 Type, const FString& PayloadJson)
{
	const FString Message = FString::Printf(TEXT("{\"type\":%d,\"payload\":%s}"), Type, *PayloadJson);
	if (!IsConnected())
	{
		FDMGameServerPendingRequest Request;
		Request.RequestProtocolName = ProtocolName;
		Request.RequestType = Type;
		Request.PayloadJson = PayloadJson;
		PendingRequests.Add(Request);
		DM_LOG(this, LogTemp, Log, TEXT("SendProtocolMessage queued: protocol=%s, type=%d, pending=%d"),
			*ProtocolName.ToString(),
			Type,
			PendingRequests.Num());
		Connect();
		return;
	}

	SendTextMessage(Message);
}

void UDMGameServerSubsystem::BindSocketEvents()
{
	if (!WebSocket.IsValid())
	{
		DM_LOG(this, LogTemp, Warning, TEXT("BindSocketEvents skipped: socket is invalid."));
		return;
	}

	WebSocket->OnConnected().AddLambda([this]()
	{
		bIsConnecting = false;
		DM_LOG(this, LogTemp, Log, TEXT("Connected: ServerUrl=%s"), *ServerUrl);
		StartHeartbeat();
		OnGameServerConnected.Broadcast();
		FlushPendingRequests();
	});

	WebSocket->OnConnectionError().AddLambda([this](const FString& Error)
	{
		bIsConnecting = false;
		StopHeartbeat();
		const FString ErrorText = Error.IsEmpty() ? TEXT("GameServer WebSocket connection failed.") : Error;
		DM_LOG(this, LogTemp, Error, TEXT("Connection error: ServerUrl=%s, Error=%s"), *ServerUrl, *ErrorText);
		FailPendingRequests(0, ErrorText);
		ClearSocket();
		OnGameServerError.Broadcast(ErrorText);
	});

	WebSocket->OnClosed().AddLambda([this](const int32 StatusCode, const FString& Reason, const bool bWasClean)
	{
		bIsConnecting = false;
		StopHeartbeat();
		DM_LOG(this, LogTemp, Log, TEXT("Closed: StatusCode=%d, Reason=%s, WasClean=%s"),
			StatusCode,
			*Reason,
			bWasClean ? TEXT("true") : TEXT("false"));

		FailPendingRequests(StatusCode, Reason);

		OnGameServerDisconnected.Broadcast(Reason);
		ClearSocket();
	});

	WebSocket->OnMessage().AddLambda([this](const FString& Message)
	{
		DM_LOG(this, LogTemp, Log, TEXT("Message received: %s"), *Message);
		HandleServerMessage(Message);
		OnGameServerMessage.Broadcast(Message);
	});
}

void UDMGameServerSubsystem::ClearSocket()
{
	StopHeartbeat();

	if (!WebSocket.IsValid())
	{
		return;
	}

	WebSocket->OnConnected().Clear();
	WebSocket->OnConnectionError().Clear();
	WebSocket->OnClosed().Clear();
	WebSocket->OnMessage().Clear();
	WebSocket.Reset();
}

void UDMGameServerSubsystem::SendPendingRequest(FDMGameServerPendingRequest& Request)
{
	if (!IsConnected())
	{
		return;
	}

	if (Request.ResponseType > 0 && Request.Callback.IsBound())
	{
		PendingResponseCallbacks.Add(Request.ResponseType, Request);
	}

	SendProtocolMessage(Request.RequestProtocolName, Request.RequestType, Request.PayloadJson);
	DM_LOG(this, LogTemp, Log, TEXT("Protocol request sent. request=%s, type=%d, response=%s"),
		*Request.RequestProtocolName.ToString(),
		Request.RequestType,
		*Request.ResponseProtocolName.ToString());
}

void UDMGameServerSubsystem::FlushPendingRequests()
{
	if (!IsConnected() || PendingRequests.IsEmpty())
	{
		return;
	}

	TArray<FDMGameServerPendingRequest> RequestsToSend = MoveTemp(PendingRequests);
	PendingRequests.Reset();
	for (FDMGameServerPendingRequest& Request : RequestsToSend)
	{
		SendPendingRequest(Request);
	}

	DM_LOG(this, LogTemp, Log, TEXT("Pending protocol requests flushed: Count=%d"), RequestsToSend.Num());
}

void UDMGameServerSubsystem::FailPendingRequests(const int32 ResponseCode, const FString& Reason)
{
	for (FDMGameServerPendingRequest& Request : PendingRequests)
	{
		Request.Callback.ExecuteIfBound(false, ResponseCode, Reason);
	}
	PendingRequests.Reset();

	for (TPair<int32, FDMGameServerPendingRequest>& Pair : PendingResponseCallbacks)
	{
		Pair.Value.Callback.ExecuteIfBound(false, ResponseCode, Reason);
	}
	PendingResponseCallbacks.Reset();
}

void UDMGameServerSubsystem::HandleServerMessage(const FString& Message)
{
	TSharedPtr<FJsonObject> RootObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		DM_LOG(this, LogTemp, Warning, TEXT("HandleServerMessage failed: invalid JSON."));
		return;
	}

	double TypeNumber = 0.0;
	if (!RootObject->TryGetNumberField(TEXT("type"), TypeNumber))
	{
		DM_LOG(this, LogTemp, Warning, TEXT("HandleServerMessage failed: missing protocol type."));
		return;
	}

	const int32 Type = static_cast<int32>(TypeNumber);
	if (Type == ProtocolBasePong)
	{
		DM_LOG(this, LogTemp, Log, TEXT("Heartbeat pong received."));
		return;
	}

	TSharedPtr<FJsonObject> PayloadObject;
	const TSharedPtr<FJsonObject>* PayloadObjectPtr = nullptr;
	if (RootObject->TryGetObjectField(TEXT("payload"), PayloadObjectPtr) && PayloadObjectPtr)
	{
		PayloadObject = *PayloadObjectPtr;
	}
	if (!TryExecuteProtocolCallback(Type, Message, PayloadObject))
	{
		DM_LOG(this, LogTemp, Log, TEXT("HandleServerMessage: no callback for Type=%d"), Type);
	}
}

bool UDMGameServerSubsystem::TryExecuteProtocolCallback(const int32 Type, const FString& Message, const TSharedPtr<FJsonObject>& PayloadObject)
{
	FDMGameServerPendingRequest* Request = PendingResponseCallbacks.Find(Type);
	if (!Request)
	{
		return false;
	}

	bool bSucceeded = true;
	int32 ResponseCode = Type;
	if (PayloadObject.IsValid())
	{
		PayloadObject->TryGetBoolField(TEXT("success"), bSucceeded);
		if (!bSucceeded)
		{
			double ErrorCodeNumber = ResponseCode;
			if (PayloadObject->TryGetNumberField(TEXT("errorCode"), ErrorCodeNumber))
			{
				ResponseCode = static_cast<int32>(ErrorCodeNumber);
			}
		}
	}

	FDMGameServerResponseDelegate Callback = Request->Callback;
	PendingResponseCallbacks.Remove(Type);
	Callback.ExecuteIfBound(bSucceeded, ResponseCode, Message);
	return true;
}

void UDMGameServerSubsystem::StartHeartbeat()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		DM_LOG(this, LogTemp, Warning, TEXT("StartHeartbeat skipped: World is invalid."));
		return;
	}

	const float ResolvedIntervalSeconds = FMath::Max(HeartbeatIntervalSeconds, 1.0f);
	World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
	World->GetTimerManager().SetTimer(HeartbeatTimerHandle, this, &UDMGameServerSubsystem::HandleHeartbeatTick, ResolvedIntervalSeconds, true);
	DM_LOG(this, LogTemp, Log, TEXT("Heartbeat started. interval=%.2fs"), ResolvedIntervalSeconds);
}

void UDMGameServerSubsystem::StopHeartbeat()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (World->GetTimerManager().IsTimerActive(HeartbeatTimerHandle))
	{
		World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
		DM_LOG(this, LogTemp, Log, TEXT("Heartbeat stopped."));
	}
}

void UDMGameServerSubsystem::HandleHeartbeatTick()
{
	if (!IsConnected())
	{
		StopHeartbeat();
		DM_LOG(this, LogTemp, Warning, TEXT("Heartbeat stopped: socket is not connected."));
		return;
	}

	SendPing();
	DM_LOG(this, LogTemp, Log, TEXT("Heartbeat ping sent."));
}
