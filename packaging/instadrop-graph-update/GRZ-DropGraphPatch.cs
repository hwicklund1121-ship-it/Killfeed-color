// GRZ patch operations and standard PBO writer. No game graph data is embedded.
using System;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Security.Cryptography;

public static class GRZDropGraphPatchV032 {
    struct Span { public int Start; public int End; }
    static Span Block(string text, string kind, string name) {
        string expression = @"\b" + Regex.Escape(kind) + @"\s+" + Regex.Escape(name) + @"\s*\{";
        var matches = Regex.Matches(text, expression);
        if (matches.Count != 1) throw new InvalidDataException("Expected one graph block: " + name);
        var m = matches[0];
        int depth = 1;
        bool quoted = false, escaped = false;
        for (int i = m.Index + m.Length; i < text.Length; ++i) {
            char c = text[i];
            if (quoted) {
                if (escaped) escaped = false;
                else if (c == '\\') escaped = true;
                else if (c == '"') quoted = false;
            } else if (c == '"') quoted = true;
            else if (c == '{') ++depth;
            else if (c == '}' && --depth == 0) return new Span { Start = m.Index, End = i + 1 };
        }
        throw new InvalidDataException("Unclosed graph block: " + name);
    }
    static string Literal(string value) {
        return "\""+value.Replace("\\","\\\\").Replace("\"","\\\"")+"\"";
    }
    static string Property(string block, string name, string before, string after) {
        string expression = @"(?m)^(\s*" + Regex.Escape(name) + @"\s+)" + Regex.Escape(Literal(before)) + @"(?=\s*$)";
        if (Regex.Matches(block, expression).Count != 1) throw new InvalidDataException("Unsupported graph property: " + name);
        return Regex.Replace(block, expression, delegate(Match m) { return m.Groups[1].Value + Literal(after); });
    }
    public static string Locomotion(string text) {
        var scope = Block(text, "AnimSrcNodeStateMachine", "ThrowIdleStanceSTM");
        string machine = text.Substring(scope.Start, scope.End - scope.Start);
        string[] ids = { "{6930A8DE26987051}", "{6930A8DE26987067}", "{6930A8DE26987073}", "{6930A8DE26987090}" };
        string[] from = { "Erc", "Cro", "Pne", "PoB" };
        string[] oldTo = { "ErcToPne", "CroToPne", "PneToErc", "PoBToErc" };
        string[] newTo = { "Pne", "Pne", "Erc", "Erc" };
        string[] conditions = { "Stance == 2", "Stance == 2", "Stance == 0", "Stance == 0" };
        string[] blends = { "0.14", "0.14", "0.16", "0.16" };
        for (int i = 0; i < ids.Length; ++i) {
            var span = Block(machine, "AnimSrcNodeTransition", "\"" + ids[i] + "\"");
            string edge = machine.Substring(span.Start, span.End - span.Start);
            edge = Property(edge, "FromState", from[i], from[i]);
            edge = Property(edge, "Condition", conditions[i], conditions[i]);
            edge = Property(edge, "ToState", oldTo[i], newTo[i]);
            edge = Property(edge, "Duration", "0.2", blends[i]);
            machine = machine.Substring(0, span.Start) + edge + machine.Substring(span.End);
        }
        string result = text.Substring(0, scope.Start) + machine + text.Substring(scope.End);
        var masterSpan = Block(result, "AnimSrcNodeStateMachine", "StanceMaster");
        string master = result.Substring(masterSpan.Start, masterSpan.End - masterSpan.Start);
        var rollAnchor = Block(master, "AnimSrcNodeTransition", "\"{6930A8DE26987785}\"");
        master = master.Substring(0, rollAnchor.Start) + RollRoutes() + master.Substring(rollAnchor.Start);
        result = Directional(result.Substring(0, masterSpan.Start) + master + result.Substring(masterSpan.End));
        var pose = Block(result,"AnimSrcNodeBlend","ThrowStrengthIdleB");
        result=result.Substring(0,pose.Start)+Property(result.Substring(pose.Start,pose.End-pose.Start),"BlendWeight","1.0","clamp(ActionProgress * 1000, 0, 1)")+result.Substring(pose.End);
        return Continuity(result);
    }
    static string Continuity(string text) {
        var scope=Block(text,"AnimSrcNodeStateMachine","StanceMaster");
        string master=text.Substring(scope.Start,scope.End-scope.Start);
        var edge=Block(master,"AnimSrcNodeTransition","\"{6930A8DE26987785}\"");
        string before="IsCommand(CMD_Roll) && (!Raised || (Raised && AimX >= -50 && AimX <= 50))";
        string after=before+" && !IsTag(\"TagMoveRolling\") && !IsTag(\"GRZBackRollActive\")";
        master=master.Substring(0,edge.Start)+Property(master.Substring(edge.Start,edge.End-edge.Start),"Condition",before,after)+master.Substring(edge.End);
        var state=Block(master,"AnimSrcNodeState","PneOnBackRoll");
        string block=master.Substring(state.Start,state.End-state.Start);
        if(block.IndexOf("\"TagProneBack\"",StringComparison.Ordinal)<0) throw new InvalidDataException("Unsupported native back-roll tag block");
        block=block.Replace("\"TagProneBack\"","\"TagProneBack\" \"GRZBackRollActive\"");
        master=master.Substring(0,state.Start)+block+master.Substring(state.End);
        return text.Substring(0,scope.Start)+master+text.Substring(scope.End);
    }
    static string Directional(string text) {
        var scope = Block(text, "AnimSrcNodeStateMachine", "ThrowIdleStanceSTM");
        string machine = text.Substring(scope.Start, scope.End - scope.Start);
        string[] states = { "Pne", "PoB" };
        string[] before = { "ThrowPneIdleG", "PoBAimB" };
        for (int i=0;i<states.Length;++i) {
            var state = Block(machine, "AnimSrcNodeState", states[i]);
            string block = Property(machine.Substring(state.Start,state.End-state.Start),"Child",before[i],"GRZDropDirectionalPose");
            machine = machine.Substring(0,state.Start)+block+machine.Substring(state.End);
        }
        string result = text.Substring(0,scope.Start)+machine+text.Substring(scope.End);
        var strength = Block(result,"AnimSrcNodeBlendT","ThrowStrenghPoBB");
        result = result.Substring(0,strength.Start)+"AnimSrcNodeBlendN ThrowStrenghPoBB {\n     BlendWeight \"AimX\"\n     BlendFn S\n     Thresholds {\n      -180 0 180\n     }\n     Children {\n      \"ThrowStrengthPoBP\" \"ThrowStrengthPneP\" \"ThrowStrengthPoBP\"\n     }\n     IsCyclic 0\n    }"+result.Substring(strength.End);
        var strengthTime = Block(result,"AnimSrcNodeBlendT","ThrowStrenghB");
        result = result.Substring(0,strengthTime.Start)+Property(result.Substring(strengthTime.Start,strengthTime.End-strengthTime.Start),"BlendTime","0.3","0.14")+result.Substring(strengthTime.End);
        var aim = Block(result,"AnimSrcNodeBlend","ThrowAimB");
        return result.Substring(0,aim.Start)+"AnimSrcNodePose2 GRZDropDirectionalPose {\n     Source \"Locomotion.PneRas.AimToBack\"\n     ValueX \"AimX\"\n     ValueY \"AimY\"\n     Table \"Y(-45): X(-180) = 0, X(-90) = 10, X(0) = 20, X(90) = 30, X(180) = 40\"\\\n     \"Y(0): X(-180) = 41, X(-90) = 51, X(0) = 61, X(90) = 71, X(180) = 81\"\\\n     \"Y(90): X(-180) = 82, X(-90) = 92, X(0) = 102, X(90) = 112, X(180) = 122\"\n    }\n    "+result.Substring(aim.Start);
    }
    static string RollRoutes() {
        string[] ids = { "{A619000000000001}", "{A619000000000002}", "{A619000000000003}", "{A619000000000004}", "{A619000000000005}", "{A619000000000006}" };
        string[] origins = { "Throwing", "Throwing", "ProneRoll", "ProneRoll", "PneOnBackRoll", "PneOnBackRoll" };
        string[] targets = { "PneOnBackRoll", "ProneRoll", "IdleLow", "IdleRas", "IdleLow", "IdleRas" };
        string[] conditions = { "IsCommand(CMD_Roll) && Stance == 2 && ThrowingOnBack && !IsTag(\"TagThrowingIn\") && !IsTag(\"TagThrowingOut\") && !IsTag(\"TagThrowingCancel\")", "IsCommand(CMD_Roll) && Stance == 2 && !ThrowingOnBack && !IsTag(\"TagThrowingIn\") && !IsTag(\"TagThrowingOut\") && !IsTag(\"TagThrowingCancel\")", "Stance == 0 && !Raised && GetLowerTime() > 0.12", "Stance == 0 && Raised && GetLowerTime() > 0.12", "Stance == 0 && !Raised && GetLowerTime() > 0.12", "Stance == 0 && Raised && GetLowerTime() > 0.12" };
        string[] durations = { "0.1", "0.1", "0.16", "0.16", "0.16", "0.16" };
        var result = new StringBuilder();
        for (int i=0; i<ids.Length; ++i) {
            result.Append("AnimSrcNodeTransition \"").Append(ids[i]).Append("\" {\n");
            result.Append("       FromState \"").Append(origins[i]).Append("\"\n");
            result.Append("       ToState \"").Append(targets[i]).Append("\"\n");
            result.Append("       Duration \"").Append(durations[i]).Append("\"\n");
            result.Append("       StartTime \"\"\n");
            result.Append("       Condition \"").Append(conditions[i].Replace("\"", "\\\"")).Append("\"\n");
            result.Append("       BlendFn S\n       MotionVecBlend 0x33 0\n      }\n      ");
        }
        return result.ToString();
    }
    public static string Root(string text) {
        const string original = "{6CA2ACDCAC1184DC}DZ/anims/workspaces/player/player_main/Locomotion.agf";
        const string replacement = "{E5500320E0494CA1}GRZ_InstaDropGraph/locomotion.agf";
        if (text.IndexOf(original, StringComparison.Ordinal) < 0 || text.IndexOf(original, StringComparison.Ordinal) != text.LastIndexOf(original, StringComparison.Ordinal)) throw new InvalidDataException("Unsupported native root graph reference.");
        return text.Replace(original, replacement);
    }
    public static string Hash(byte[] bytes) {
        using (var hash = SHA256.Create()) return BitConverter.ToString(hash.ComputeHash(bytes)).Replace("-", "").ToLowerInvariant();
    }
    static void CString(BinaryWriter w, string value) {
        w.Write(Encoding.UTF8.GetBytes(value)); w.Write((byte)0);
    }
    static void Header(BinaryWriter w, string name, uint method, uint size) {
        CString(w, name); w.Write(method); w.Write(size); w.Write((uint)0); w.Write((uint)0); w.Write(size);
    }
    public static byte[] Pbo(byte[] config, byte[] locomotion, byte[] root) {
        byte[] body;
        using (var memory = new MemoryStream()) {
            using (var w = new BinaryWriter(memory, Encoding.UTF8, true)) {
                Header(w, "", 0x56657273, 0);
                CString(w, "prefix"); CString(w, "GRZ_InstaDropGraph"); CString(w, "");
                Header(w, "config.cpp", 0, (uint)config.Length);
                Header(w, "locomotion.agf", 0, (uint)locomotion.Length);
                Header(w, "player_main.agr", 0, (uint)root.Length);
                Header(w, "", 0, 0);
                w.Write(config); w.Write(locomotion); w.Write(root);
            }
            body = memory.ToArray();
        }
        byte[] digest;
        using (var sha = SHA1.Create()) digest = sha.ComputeHash(body);
        byte[] result = new byte[body.Length + 21];
        Buffer.BlockCopy(body, 0, result, 0, body.Length);
        Buffer.BlockCopy(digest, 0, result, body.Length + 1, 20);
        return result;
    }
}
