// GRZ patch operations and standard PBO writer. No game graph data is embedded.
using System;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Security.Cryptography;

public static class GRZDropGraphPatchV016 {
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
    static string Property(string block, string name, string before, string after) {
        string expression = @"(?m)^(\s*" + Regex.Escape(name) + @"\s+)" + Regex.Escape("\"" + before + "\"") + @"(?=\s*$)";
        if (Regex.Matches(block, expression).Count != 1) throw new InvalidDataException("Unsupported graph property: " + name);
        return Regex.Replace(block, expression, delegate(Match m) { return m.Groups[1].Value + "\"" + after + "\""; });
    }
    public static string Locomotion(string text) {
        var scope = Block(text, "AnimSrcNodeStateMachine", "ThrowIdleStanceSTM");
        string machine = text.Substring(scope.Start, scope.End - scope.Start);
        string[] ids = { "{6930A8DE26987051}", "{6930A8DE26987067}" };
        string[] from = { "Erc", "Cro" };
        string[] oldTo = { "ErcToPne", "CroToPne" };
        for (int i = 0; i < ids.Length; ++i) {
            var span = Block(machine, "AnimSrcNodeTransition", "\"" + ids[i] + "\"");
            string edge = machine.Substring(span.Start, span.End - span.Start);
            edge = Property(edge, "FromState", from[i], from[i]);
            edge = Property(edge, "Condition", "Stance == 2", "Stance == 2");
            edge = Property(edge, "ToState", oldTo[i], "Pne");
            edge = Property(edge, "Duration", "0.2", "0.03");
            machine = machine.Substring(0, span.Start) + edge + machine.Substring(span.End);
        }
        return text.Substring(0, scope.Start) + machine + text.Substring(scope.End);
    }
    public static string Root(string text) {
        const string original = "{6CA2ACDCAC1184DC}DZ/anims/workspaces/player/player_main/Locomotion.agf";
        const string replacement = "{4E5A645A1B134796}GRZ_InstaDropGraph/locomotion.agf";
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
