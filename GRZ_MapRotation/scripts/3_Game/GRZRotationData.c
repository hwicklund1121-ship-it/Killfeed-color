class GRZRotationRPC
{
 static const int REQUEST = 763500;
 static const int VOTE = 763501;
 static const int SNAPSHOT = 763502;
 static const int NOTICE = 763503;
 static const int MENU = 763504;
};
class GRZRotationPoint
{
 float X;
 float Z;
};
class GRZRotationArena
{
 string Id;
 string Name;
 string Blueprint;
 ref array<ref GRZRotationPoint> Boundary = new array<ref GRZRotationPoint>;
 ref array<ref GRZRotationPoint> SpawnCandidates = new array<ref GRZRotationPoint>;
 ref array<vector> SafeSpawns = new array<vector>;
};
class GRZRotationSettings
{
 int RoundSeconds = 1800;
 int VoteSeconds = 120;
 int PopupSeconds = 30;
 int OutsideSeconds = 5;
 string StartArena = "nwaf";
 ref array<ref GRZRotationArena> Arenas = new array<ref GRZRotationArena>;
};
class GRZRotationChoice
{
 string Id;
 string Name;
 string Blueprint;
 int Votes;
};
class GRZRotationState
{
 string ActiveId;
};
class GRZRotationSnapshot
{
 bool Enabled;
 string Status;
 bool Voting;
 int RoundId;
 int Remaining;
 int PopupSeconds = 30;
 string ActiveId;
 string ActiveName;
 string OwnVote;
 ref array<ref GRZRotationChoice> Choices = new array<ref GRZRotationChoice>;
};
class GRZRotationBus
{
 static ref ScriptInvoker Snapshot = new ScriptInvoker;
 static ref ScriptInvoker Notice = new ScriptInvoker;
};
