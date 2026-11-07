using System;
using System.IO;
using System.Text;
using System.Linq;
using System.Collections.Generic;
using System.Security.Cryptography;

// Reads installed config tables; emits only class names, inheritance and size overrides.
public static class GRZInventoryBuilder
{
    public class Definition
    {
        public string Name;
        public string Parent;
        public string[] Slots;
        public bool AppendSlots;
        public int[] CargoSize;
        public HashSet<string> ParentChoices = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        public HashSet<string> AddonParentChoices = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        public HashSet<string> Providers = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
    }
    public class Report
    {
        public int ArchivesRead;
        public string[] Clothing;
        public string[] NormalClothing;
        public string[] Guns;
        public string[] RequiredAddons;
        public string PboPath;
        public string ConfigPath;
    }
    private class Entry { public string Name; public uint Method; public uint Original; public uint Size; }
    private class Node { public string Name; public string Parent; public uint Offset; }
    private class ConfigResource { public string Name; public byte[] Data; }
    private class SourceNode
    {
        public string Name, Parent = "";
        public Metadata Metadata = new Metadata();
        public List<SourceNode> Children = new List<SourceNode>();
    }
    // Only class/slot/cargo metadata is interpreted. Unsupported preprocessing
    // fails explicitly instead of claiming complete mod coverage.
    private class SourceConfig
    {
        private List<string> tokens = new List<string>();
        private int at;
        public SourceConfig(byte[] bytes)
        {
            string text = Encoding.UTF8.GetString(bytes).TrimStart('\uFEFF');
            text = System.Text.RegularExpressions.Regex.Replace(text, @"(?m)^\s*#define\s+_ARMA_\s*$", "");
            if (System.Text.RegularExpressions.Regex.IsMatch(text, @"(?m)^\s*#"))
                throw new InvalidDataException("Unsupported source-config preprocessor directive; supply a binarized config.");
            int pos = 0;
            while (pos < text.Length)
            {
                char c = text[pos];
                if (Char.IsWhiteSpace(c)) { pos++; continue; }
                if (c == '/' && pos + 1 < text.Length && text[pos + 1] == '/')
                { while (pos < text.Length && text[pos] != '\n') pos++; continue; }
                if (c == '/' && pos + 1 < text.Length && text[pos + 1] == '*')
                {
                    int end = text.IndexOf("*/", pos + 2, StringComparison.Ordinal);
                    if (end < 0) throw new InvalidDataException("Unterminated source-config comment.");
                    pos = end + 2; continue;
                }
                int start = pos++;
                if (c == '"')
                {
                    while (pos < text.Length && text[pos] != '"') pos++;
                    if (pos == text.Length) throw new InvalidDataException("Unterminated source-config string.");
                    pos++;
                }
                else if (Char.IsLetterOrDigit(c) || c == '_' || c == '-' || c == '+')
                {
                    while (pos < text.Length && (Char.IsLetterOrDigit(text[pos]) || text[pos] == '_' || text[pos] == '.' || text[pos] == '-' || text[pos] == '+')) pos++;
                }
                tokens.Add(text.Substring(start, pos - start));
            }
        }
        private string Take() { if (at >= tokens.Count) throw new InvalidDataException("Truncated source config."); return tokens[at++]; }
        private void Expect(string token) { if (Take() != token) throw new InvalidDataException("Expected source-config token: " + token); }
        private static string StringValue(string token)
        {
            if (token.Length < 2 || token[0] != '"' || token[token.Length - 1] != '"') throw new InvalidDataException("Expected literal inventorySlot string.");
            return token.Substring(1, token.Length - 2);
        }
        public SourceNode Read()
        {
            SourceNode root = new SourceNode { Name = "" };
            Body(root, false, 0);
            return root;
        }
        private void Body(SourceNode node, bool nested, int depth)
        {
            if (depth > 64) throw new InvalidDataException("Source config nesting is too deep.");
            while (at < tokens.Count)
            {
                string name = Take();
                if (name == "}") { if (!nested) throw new InvalidDataException("Unexpected closing config brace."); return; }
                if (name == ";") continue;
                if (name == "class")
                {
                    SourceNode child = new SourceNode { Name = Take() };
                    string next = Take();
                    if (next == ":") { child.Parent = Take(); next = Take(); }
                    if (next == ";") continue; // forward declaration
                    if (next != "{") throw new InvalidDataException("Expected source class body.");
                    Body(child, true, depth + 1);
                    if (at < tokens.Count && tokens[at] == ";") at++;
                    node.Children.Add(child); continue;
                }
                if (name == "delete") throw new InvalidDataException("Source-config delete directives are unsupported.");
                bool array = false, append = false;
                if (at < tokens.Count && tokens[at] == "[") { at++; Expect("]"); array = true; }
                string equals = Take();
                if (equals == "+" || equals == "+=") { append = true; if (equals == "+") Expect("="); }
                else if (equals != "=") throw new InvalidDataException("Expected source property assignment for " + name);
                List<string> values = new List<string>();
                int braces = 0;
                bool terminated = false;
                while (at < tokens.Count)
                {
                    string value = Take();
                    if (value == ";" && braces == 0) { terminated = true; break; }
                    if (value == "{") braces++;
                    if (value == "}") { braces--; if (braces < 0) throw new InvalidDataException("Unbalanced source property."); }
                    values.Add(value);
                }
                if (!terminated) throw new InvalidDataException("Unterminated source property.");
                if (name.Equals("inventorySlot", StringComparison.OrdinalIgnoreCase))
                {
                    IEnumerable<string> slots = array ? values.Where(v => v != "{" && v != "}" && v != ",") : values;
                    node.Metadata.Slots = slots.Select(StringValue).ToArray(); node.Metadata.AppendSlots = append;
                }
                else if (name.Equals("itemsCargoSize", StringComparison.OrdinalIgnoreCase))
                {
                    if (!array || append) throw new InvalidDataException("Unsupported source cargo dimensions.");
                    List<int> size = new List<int>();
                    foreach (string value in values.Where(v => v != "{" && v != "}" && v != ","))
                    {
                        int dimension;
                        if (!Int32.TryParse(value, out dimension) || dimension < 0) throw new InvalidDataException("Invalid source cargo dimensions.");
                        size.Add(dimension);
                    }
                    if (size.Count == 0) size.AddRange(new int[] {0, 0});
                    if (size.Count != 2) throw new InvalidDataException("Invalid source cargo dimension count.");
                    node.Metadata.CargoSize = size.ToArray();
                }
            }
            if (nested) throw new InvalidDataException("Truncated source class body.");
        }
    }
    private class Metadata { public string[] Slots; public bool AppendSlots; public int[] CargoSize; }
    private class Rap : IDisposable
    {
        private BinaryReader r;
        public Rap(byte[] bytes)
        {
            r = new BinaryReader(new MemoryStream(bytes));
            if (bytes.Length < 18 || bytes[0] != 0 || bytes[1] != 114 || bytes[2] != 97 || bytes[3] != 80)
                throw new InvalidDataException("config.bin is not a supported raP config.");
        }
        public void Dispose() { r.Dispose(); }
        private uint Count()
        {
            uint value = 0;
            for (int shift = 0; shift <= 28; shift += 7)
            {
                byte part = r.ReadByte();
                value |= (uint)(part & 127) << shift;
                if ((part & 128) == 0)
                {
                    if (value > 1000000) throw new InvalidDataException("Config count is too large.");
                    return value;
                }
            }
            throw new InvalidDataException("Invalid config integer.");
        }
        private void SkipValue(byte kind, int depth)
        {
            if (kind == 0 || kind == 4) ReadZ(r);
            else if (kind == 1 || kind == 2) r.ReadUInt32();
            else if (kind == 3) SkipArray(depth + 1);
            else throw new InvalidDataException("Unknown config value kind: " + kind);
        }
        private void SkipArray(int depth)
        {
            if (depth > 64) throw new InvalidDataException("Config nesting is too deep.");
            uint count = Count();
            for (uint i = 0; i < count; i++) SkipValue(r.ReadByte(), depth);
        }
        private string[] SlotArray()
        {
            uint count = Count();
            List<string> slots = new List<string>();
            for (uint i = 0; i < count; i++)
            {
                byte kind = r.ReadByte();
                if (kind == 0 || kind == 4) slots.Add(ReadZ(r));
                else throw new InvalidDataException("Unexpected inventorySlot array value.");
            }
            return slots.ToArray();
        }
        private int[] CargoArray()
        {
            uint count = Count();
            List<int> dimensions = new List<int>();
            for (uint i = 0; i < count; i++)
            {
                byte kind = r.ReadByte();
                if (kind == 2) dimensions.Add(r.ReadInt32());
                else if (kind == 1)
                {
                    float value = r.ReadSingle();
                    if (Single.IsNaN(value) || Single.IsInfinity(value) || value != Math.Truncate(value))
                        throw new InvalidDataException("Non-integer cargo dimensions.");
                    dimensions.Add(checked((int)value));
                }
                else throw new InvalidDataException("Unexpected itemsCargoSize array value.");
            }
            if (dimensions.Count == 0) return new int[] {0, 0};
            if (dimensions.Count != 2 || dimensions.Any(n => n < 0))
                throw new InvalidDataException("Cargo dimensions must contain two nonnegative values.");
            return dimensions.ToArray();
        }
        public Metadata Properties(uint offset)
        {
            r.BaseStream.Position = offset;
            ReadZ(r);
            uint count = Count();
            Metadata metadata = new Metadata();
            for (uint i = 0; i < count; i++)
            {
                byte kind = r.ReadByte();
                if (kind == 0) { ReadZ(r); r.ReadUInt32(); }
                else if (kind == 1)
                {
                    byte valueKind = r.ReadByte(); string name = ReadZ(r);
                    if (name.Equals("inventorySlot", StringComparison.OrdinalIgnoreCase) && (valueKind == 0 || valueKind == 4))
                        metadata.Slots = new string[] {ReadZ(r)};
                    else SkipValue(valueKind, 0);
                }
                else if (kind == 2 || kind == 5)
                {
                    if (kind == 5) r.ReadUInt32();
                    string name = ReadZ(r);
                    if (name.Equals("inventorySlot", StringComparison.OrdinalIgnoreCase))
                    {
                        metadata.Slots = SlotArray(); metadata.AppendSlots = kind == 5;
                    }
                    else if (name.Equals("itemsCargoSize", StringComparison.OrdinalIgnoreCase))
                    {
                        if (kind == 5) throw new InvalidDataException("Appending cargo dimensions is unsupported.");
                        metadata.CargoSize = CargoArray();
                    }
                    else SkipArray(0);
                }
                else if (kind == 3 || kind == 4) ReadZ(r);
                else throw new InvalidDataException("Unknown config entry kind: " + kind);
            }
            return metadata;
        }
        public List<Node> Children(uint offset)
        {
            if (offset >= r.BaseStream.Length) throw new InvalidDataException("Invalid config class offset.");
            r.BaseStream.Position = offset;
            ReadZ(r); // inheritance of this body
            uint count = Count();
            List<Node> nodes = new List<Node>();
            for (uint i = 0; i < count; i++)
            {
                byte kind = r.ReadByte();
                if (kind == 0)
                {
                    string name = ReadZ(r);
                    uint position = r.ReadUInt32();
                    long restore = r.BaseStream.Position;
                    if (position >= r.BaseStream.Length) throw new InvalidDataException("Invalid child class offset.");
                    r.BaseStream.Position = position;
                    string parent = ReadZ(r);
                    r.BaseStream.Position = restore;
                    nodes.Add(new Node { Name = name, Parent = parent, Offset = position });
                }
                else if (kind == 1)
                {
                    byte valueKind = r.ReadByte(); ReadZ(r); SkipValue(valueKind, 0);
                }
                else if (kind == 2 || kind == 5)
                {
                    if (kind == 5) r.ReadUInt32();
                    ReadZ(r); SkipArray(0);
                }
                else if (kind == 3 || kind == 4) ReadZ(r); // forward declaration / delete
                else throw new InvalidDataException("Unknown config entry kind: " + kind);
            }
            return nodes;
        }
    }
    private static string ReadZ(BinaryReader r)
    {
        List<byte> bytes = new List<byte>();
        for (int i = 0; i < 1048576; i++)
        {
            byte b = r.ReadByte();
            if (b == 0) return Encoding.UTF8.GetString(bytes.ToArray());
            bytes.Add(b);
        }
        throw new InvalidDataException("Invalid null-terminated string.");
    }
    private static List<ConfigResource> ReadConfigs(string path)
    {
        List<ConfigResource> result = new List<ConfigResource>();
        using (BinaryReader reader = new BinaryReader(File.Open(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite)))
        {
            List<Entry> entries = new List<Entry>();
            bool self = false;
            for (int i = 0; i < 100000; i++)
            {
                string name = ReadZ(reader); uint method = reader.ReadUInt32(); uint original = reader.ReadUInt32();
                reader.ReadUInt32(); reader.ReadUInt32(); uint size = reader.ReadUInt32();
                if (name == "" && method == 0x56657273)
                {
                    string key;
                    while ((key = ReadZ(reader)) != "")
                    {
                        string value = ReadZ(reader);
                        if (key.Equals("prefix", StringComparison.OrdinalIgnoreCase) && value.Equals("GRZ_InventorySizes", StringComparison.OrdinalIgnoreCase)) self = true;
                    }
                    continue;
                }
                if (name == "") break;
                entries.Add(new Entry { Name = name.Replace('/', '\\'), Method = method, Original = original, Size = size });
            }
            if (self) return result;
            // Configs nested under Models/... are independent addon definitions.
            HashSet<string> binaryDirectories = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (Entry entry in entries)
                if (entry.Name.Equals("config.bin", StringComparison.OrdinalIgnoreCase) || entry.Name.EndsWith("\\config.bin", StringComparison.OrdinalIgnoreCase))
                    binaryDirectories.Add(entry.Name.Substring(0, entry.Name.Length - "config.bin".Length));
            long offset = reader.BaseStream.Position;
            foreach (Entry entry in entries)
            {
                bool binary = entry.Name.Equals("config.bin", StringComparison.OrdinalIgnoreCase) || entry.Name.EndsWith("\\config.bin", StringComparison.OrdinalIgnoreCase);
                bool source = entry.Name.Equals("config.cpp", StringComparison.OrdinalIgnoreCase) || entry.Name.EndsWith("\\config.cpp", StringComparison.OrdinalIgnoreCase);
                if (source && binaryDirectories.Contains(entry.Name.Substring(0, entry.Name.Length - "config.cpp".Length))) source = false;
                if (binary || source)
                {
                    if (entry.Method != 0) throw new InvalidDataException("Compressed " + (binary ? "config.bin" : "config.cpp") + " is not supported: " + entry.Name);
                    if (entry.Size == 0 || entry.Size > 32 * 1024 * 1024 || offset + entry.Size > reader.BaseStream.Length) throw new InvalidDataException("Invalid config size: " + entry.Name);
                    reader.BaseStream.Position = offset;
                    byte[] data = reader.ReadBytes((int)entry.Size);
                    if (data.Length != entry.Size) throw new InvalidDataException("Truncated config: " + entry.Name);
                    result.Add(new ConfigResource { Name = entry.Name, Data = data });
                }
                offset += entry.Size;
            }
        }
        return result;
    }
    private static void MergeSource(Dictionary<string, Definition> table, List<SourceNode> nodes, string[] providers)
    {
        foreach (SourceNode node in nodes)
        {
            Definition def;
            if (!table.TryGetValue(node.Name, out def)) { def = new Definition { Name = node.Name, Parent = node.Parent }; table.Add(node.Name, def); }
            else if (node.Parent != "") def.Parent = node.Parent;
            if (node.Parent != "") { def.ParentChoices.Add(node.Parent); if (providers.Length > 0) def.AddonParentChoices.Add(node.Parent); }
            Metadata meta = node.Metadata;
            if (meta.Slots != null)
            {
                if (meta.AppendSlots && def.Slots != null) def.Slots = def.Slots.Concat(meta.Slots).Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
                else { def.Slots = meta.Slots; def.AppendSlots = meta.AppendSlots; }
            }
            if (meta.CargoSize != null) def.CargoSize = meta.CargoSize;
            foreach (string provider in providers) def.Providers.Add(provider);
        }
    }
    private static void Merge(Dictionary<string, Definition> table, List<Node> nodes, string[] providers, Rap rap)
    {
        foreach (Node node in nodes)
        {
            Definition def;
            if (!table.TryGetValue(node.Name, out def))
            {
                def = new Definition { Name = node.Name, Parent = node.Parent };
                table.Add(node.Name, def);
            }
            else if (node.Parent != "")
            {
                def.Parent = node.Parent;
            }
            if (node.Parent != "")
            {
                def.ParentChoices.Add(node.Parent);
                if (providers.Length > 0) def.AddonParentChoices.Add(node.Parent);
            }
            Metadata metadata = rap.Properties(node.Offset);
            if (metadata.Slots != null)
            {
                if (metadata.AppendSlots && def.Slots != null) def.Slots = def.Slots.Concat(metadata.Slots).Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
                else { def.Slots = metadata.Slots; def.AppendSlots = metadata.AppendSlots; }
            }
            if (metadata.CargoSize != null) def.CargoSize = metadata.CargoSize;
            foreach (string provider in providers) def.Providers.Add(provider);
        }
    }
    private static bool IsPotentialTarget(string name, Dictionary<string, Definition> table, HashSet<string> roots)
    {
        // Only relevant clothing/weapon ancestry needs an unambiguous parent.
        // Engine/addon redefinitions of unrelated classes (e.g. Man) are normal.
        Stack<string> pending = new Stack<string>();
        HashSet<string> visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        pending.Push(name);
        while (pending.Count > 0)
        {
            string current = pending.Pop();
            if (!visited.Add(current)) continue;
            if (roots.Contains(current)) return true;
            Definition def;
            if (table.TryGetValue(current, out def))
                foreach (string parent in def.ParentChoices) pending.Push(parent);
        }
        return false;
    }
    private static void CheckTargetParents(Dictionary<string, Definition> table, HashSet<string> roots)
    {
        foreach (Definition def in table.Values)
            if (def.AddonParentChoices.Count > 1 && IsPotentialTarget(def.Name, table, roots))
                throw new InvalidDataException("Conflicting base classes for clothing/gun class " + def.Name + "; do not scan inactive/alternative mods together.");
    }
    private static bool IsTarget(string name, Dictionary<string, Definition> table, HashSet<string> roots)
    {
        HashSet<string> visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        while (name != "" && visited.Add(name))
        {
            if (roots.Contains(name)) return true;
            Definition def;
            if (!table.TryGetValue(name, out def)) return false;
            name = def.Parent;
        }
        return false;
    }
    private static string[] EffectiveSlots(Definition def, Dictionary<string, Definition> table, HashSet<string> visited)
    {
        if (!visited.Add(def.Name)) throw new InvalidDataException("Circular clothing slot inheritance: " + def.Name);
        if (def.Slots != null && !def.AppendSlots) return def.Slots;
        Definition parent;
        string[] inherited = table.TryGetValue(def.Parent, out parent) ? EffectiveSlots(parent, table, visited) : new string[0];
        return def.Slots == null ? inherited : inherited.Concat(def.Slots).Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
    }
    private static int[] EffectiveCargo(Definition def, Dictionary<string, Definition> table, HashSet<string> visited)
    {
        if (!visited.Add(def.Name)) throw new InvalidDataException("Circular clothing cargo inheritance: " + def.Name);
        if (def.CargoSize != null) return def.CargoSize;
        Definition parent;
        return table.TryGetValue(def.Parent, out parent) ? EffectiveCargo(parent, table, visited) : new int[] {0, 0};
    }
    private static void Visit(Definition def, Dictionary<string, Definition> targets, HashSet<string> visiting, HashSet<string> done, List<Definition> ordered)
    {
        if (done.Contains(def.Name)) return;
        if (!visiting.Add(def.Name)) throw new InvalidDataException("Circular inheritance at " + def.Name);
        Definition parent;
        if (targets.TryGetValue(def.Parent, out parent)) Visit(parent, targets, visiting, done, ordered);
        visiting.Remove(def.Name); done.Add(def.Name); ordered.Add(def);
    }
    private static string Render(string category, List<Definition> selected, string property, string size, Dictionary<string, int[]> cargoOverrides = null)
    {
        Dictionary<string, Definition> targets = selected.ToDictionary(d => d.Name, StringComparer.OrdinalIgnoreCase);
        List<Definition> ordered = new List<Definition>();
        HashSet<string> visiting = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        HashSet<string> done = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (Definition def in selected.OrderBy(d => d.Name, StringComparer.OrdinalIgnoreCase)) Visit(def, targets, visiting, done, ordered);
        StringBuilder s = new StringBuilder("class " + category + "\n{\n");
        foreach (string parent in selected.Select(d => d.Parent).Where(p => p != "" && !targets.ContainsKey(p)).Distinct(StringComparer.OrdinalIgnoreCase).OrderBy(p => p, StringComparer.OrdinalIgnoreCase))
            s.Append("    class " + parent + ";\n");
        foreach (Definition def in ordered)
        {
            string dimensions = cargoOverrides == null ? size : String.Join(", ", cargoOverrides[def.Name]);
            s.Append("    class " + def.Name + (def.Parent == "" ? "" : ": " + def.Parent) + "\n    {\n        " + property + "[] = {" + dimensions + "};\n    };\n");
        }
        s.Append("};\n");
        return s.ToString();
    }
    private static void Header(BinaryWriter w, string name, uint method, uint size)
    {
        Z(w, name); w.Write(method); w.Write(size); w.Write((uint)0); w.Write((uint)0); w.Write(size);
    }
    private static void Z(BinaryWriter w, string s) { w.Write(Encoding.UTF8.GetBytes(s)); w.Write((byte)0); }
    public static byte[] PackFiles(string prefix, SortedDictionary<string, byte[]> files)
    {
        byte[] body;
        using (MemoryStream ms = new MemoryStream())
        {
            using (BinaryWriter writer = new BinaryWriter(ms, Encoding.UTF8, true))
            {
                Header(writer, "", 0x56657273, 0); Z(writer, "prefix"); Z(writer, prefix); Z(writer, "");
                foreach (KeyValuePair<string, byte[]> file in files) Header(writer, file.Key, 0, (uint)file.Value.Length);
                Header(writer, "", 0, 0);
                foreach (byte[] data in files.Values) writer.Write(data);
                writer.Flush(); body = ms.ToArray();
            }
        }
        using (SHA1 sha = SHA1.Create())
        using (MemoryStream result = new MemoryStream())
        {
            result.Write(body, 0, body.Length); result.WriteByte(0);
            byte[] digest = sha.ComputeHash(body); result.Write(digest, 0, digest.Length);
            return result.ToArray();
        }
    }
    private static byte[] Pack(string config)
    {
        SortedDictionary<string, byte[]> files = new SortedDictionary<string, byte[]>(StringComparer.Ordinal);
        files.Add("config.cpp", new UTF8Encoding(false).GetBytes(config));
        return PackFiles("GRZ_InventorySizes", files);
    }
    public static bool IsGroundGait(string path)
    {
        string name = path.Replace('\\', '/').ToLowerInvariant();
        if (!name.Contains("/anm/player/moves/") || !name.EndsWith(".anm", StringComparison.Ordinal)) return false;
        string file = name.Substring(name.LastIndexOf('/') + 1);
        foreach (string excluded in new string[] {"jump", "fall", "climb", "swim", "reload", "shoot", "melee", "evade", "stop", "start", "trans", "turn", "death", "uncon", "_pne_", "_prn_"})
            if (file.Contains(excluded)) return false;
        return System.Text.RegularExpressions.Regex.IsMatch(file, @"(?:^|_)(?:walk|run|sprint|spr)[a-z0-9]*(?:_|\.)");
    }
    public static string BuildMovementData(string[] archives, string output)
    {
        HashSet<string> clips = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (string archive in archives.Distinct(StringComparer.OrdinalIgnoreCase))
        {
            using (BinaryReader reader = new BinaryReader(File.Open(archive, FileMode.Open, FileAccess.Read, FileShare.ReadWrite)))
            {
                List<string> names = new List<string>(); string prefix = "";
                for (int i = 0; i < 100000; i++)
                {
                    string name = ReadZ(reader); uint method = reader.ReadUInt32();
                    reader.ReadUInt32(); reader.ReadUInt32(); reader.ReadUInt32(); reader.ReadUInt32();
                    if (name == "" && method == 0x56657273)
                    {
                        string key; while ((key = ReadZ(reader)) != "") { string value = ReadZ(reader); if (key.Equals("prefix", StringComparison.OrdinalIgnoreCase)) prefix = value; }
                        continue;
                    }
                    if (name == "") break;
                    names.Add(name.Replace('\\', '/').TrimStart('/'));
                }
                prefix = prefix.Replace('\\', '/').Trim('/');
                foreach (string entry in names)
                {
                    string path = entry;
                    if (prefix != "" && !entry.StartsWith(prefix + "/", StringComparison.OrdinalIgnoreCase)) path = prefix + "/" + entry;
                    path = path.ToLowerInvariant();
                    if (IsGroundGait(path)) clips.Add(path);
                }
            }
        }
        if (clips.Count == 0) throw new InvalidDataException("No native walk/run/sprint animation paths found. Select the complete DayZ game installation; no speed-data PBO was produced.");
        string json = "{\"Multiplier\":1.10,\"Animations\":[" + String.Join(",", clips.OrderBy(x => x, StringComparer.Ordinal).Select(x => "\"" + x + "\"")) + "]}\n";
        string config = "class CfgPatches { class GRZ_MovementSpeedData { units[] = {}; weapons[] = {}; requiredVersion = 0.1; requiredAddons[] = {\"GRZ_Movement\"}; }; };\n";
        SortedDictionary<string, byte[]> files = new SortedDictionary<string, byte[]>(StringComparer.Ordinal);
        files.Add("config.cpp", Encoding.UTF8.GetBytes(config)); files.Add("gaits.json", Encoding.UTF8.GetBytes(json));
        Directory.CreateDirectory(output);
        string pathOut = Path.Combine(output, "GRZ_MovementSpeedData.pbo");
        File.WriteAllBytes(pathOut, PackFiles("GRZ_MovementSpeedData", files));
        File.WriteAllText(Path.Combine(output, "gaits.json"), json, new UTF8Encoding(false));
        return pathOut;
    }
    public static string[] ExportConfigs(string archive, string output)
    {
        List<string> paths = new List<string>();
        foreach (ConfigResource resource in ReadConfigs(archive))
        {
            Directory.CreateDirectory(output);
            string suffix = System.Text.RegularExpressions.Regex.Replace(resource.Name, "[^A-Za-z0-9_.-]", "_");
            string path = Path.Combine(output, Path.GetFileNameWithoutExtension(archive) + "-" + suffix);
            File.WriteAllBytes(path, resource.Data); paths.Add(path);
        }
        return paths.ToArray();
    }
    public static string ExportConfig(string archive, string output)
    {
        return ExportConfigs(archive, output).FirstOrDefault() ?? "";
    }
    public static Report Build(string[] archives, string output)
    {
        Dictionary<string, Definition> vehicles = new Dictionary<string, Definition>(StringComparer.OrdinalIgnoreCase);
        Dictionary<string, Definition> weapons = new Dictionary<string, Definition>(StringComparer.OrdinalIgnoreCase);
        int read = 0;
        List<string> failures = new List<string>();
        foreach (string archive in archives.Distinct(StringComparer.OrdinalIgnoreCase).OrderBy(p => String.Equals(Path.GetFileName(Path.GetDirectoryName(p)), "dta", StringComparison.OrdinalIgnoreCase) ? 0 : 1).ThenBy(p => p, StringComparer.OrdinalIgnoreCase))
        {
            try
            {
                List<ConfigResource> configs = ReadConfigs(archive);
                read++;
                foreach (ConfigResource resource in configs)
                {
                    byte[] bytes = resource.Data;
                    bool binary = bytes.Length >= 4 && bytes[0] == 0 && bytes[1] == 114 && bytes[2] == 97 && bytes[3] == 80;
                    if (!binary)
                    {
                        SourceNode parsedSource = new SourceConfig(bytes).Read();
                        SourceNode patches = parsedSource.Children.FirstOrDefault(n => n.Name.Equals("CfgPatches", StringComparison.OrdinalIgnoreCase));
                        string[] providers = patches == null ? new string[0] : patches.Children.Select(n => n.Name).ToArray();
                        foreach (SourceNode node in parsedSource.Children)
                        {
                            if (node.Name.Equals("CfgVehicles", StringComparison.OrdinalIgnoreCase)) MergeSource(vehicles, node.Children, providers);
                            if (node.Name.Equals("CfgWeapons", StringComparison.OrdinalIgnoreCase)) MergeSource(weapons, node.Children, providers);
                        }
                        continue;
                    }
                    using (Rap rap = new Rap(bytes))
                    {
                        List<Node> roots = rap.Children(16);
                        Node patches = roots.FirstOrDefault(n => n.Name.Equals("CfgPatches", StringComparison.OrdinalIgnoreCase));
                        string[] providers = patches == null ? new string[0] : rap.Children(patches.Offset).Select(n => n.Name).ToArray();
                        foreach (Node node in roots)
                        {
                            if (node.Name.Equals("CfgVehicles", StringComparison.OrdinalIgnoreCase)) Merge(vehicles, rap.Children(node.Offset), providers, rap);
                            if (node.Name.Equals("CfgWeapons", StringComparison.OrdinalIgnoreCase)) Merge(weapons, rap.Children(node.Offset), providers, rap);
                        }
                    }
                }
            }
            catch (Exception ex) { failures.Add(Path.GetFileName(archive) + ": " + ex.Message); }
        }
        if (failures.Count > 0) throw new InvalidDataException("No partial override was produced.\n" + String.Join("\n", failures));
        HashSet<string> clothingRoots = new HashSet<string>(new string[] {"Clothing", "Clothing_Base"}, StringComparer.OrdinalIgnoreCase);
        HashSet<string> gunRoots = new HashSet<string>(new string[] {"RifleCore", "PistolCore", "LauncherCore", "Rifle_Base", "Pistol_Base", "Launcher_Base", "Weapon_Base"}, StringComparer.OrdinalIgnoreCase);
        CheckTargetParents(vehicles, clothingRoots);
        CheckTargetParents(weapons, gunRoots);
        List<Definition> allClothing = vehicles.Values.Where(d => IsTarget(d.Name, vehicles, clothingRoots)).ToList();
        HashSet<string> allowedSlots = new HashSet<string>(new string[] {"Body", "Vest", "Legs", "Hips"}, StringComparer.OrdinalIgnoreCase);
        List<Definition> clothing = allClothing.Where(d => EffectiveSlots(d, vehicles, new HashSet<string>(StringComparer.OrdinalIgnoreCase)).Any(slot => allowedSlots.Contains(slot))).ToList();
        HashSet<string> expanded = new HashSet<string>(clothing.Select(d => d.Name), StringComparer.OrdinalIgnoreCase);
        Dictionary<string, int[]> cargoOverrides = new Dictionary<string, int[]>(StringComparer.OrdinalIgnoreCase);
        foreach (Definition def in allClothing)
            cargoOverrides[def.Name] = expanded.Contains(def.Name) ? new int[] {10, 10} : EffectiveCargo(def, vehicles, new HashSet<string>(StringComparer.OrdinalIgnoreCase));
        // The engine defines abstract anchors in dta/bin.pbo without CfgPatches.
        // Concrete/addon gun classes are all patched; those anchors stay intact.
        HashSet<string> engineAnchors = new HashSet<string>(new string[] {"Weapon_Base", "DefaultWeapon", "PistolCore", "RifleCore", "LauncherCore"}, StringComparer.OrdinalIgnoreCase);
        List<Definition> guns = weapons.Values.Where(d => IsTarget(d.Name, weapons, gunRoots) && !(d.Providers.Count == 0 && engineAnchors.Contains(d.Name))).ToList();
        if (clothing.Count == 0 || guns.Count == 0) throw new InvalidDataException("Clothing or gun definitions are missing. Select the complete DayZ game installation.");
        HashSet<string> dependencies = new HashSet<string>(new string[] {"DZ_Data", "DZ_Scripts"}, StringComparer.OrdinalIgnoreCase);
        foreach (Definition def in allClothing.Concat(guns))
        {
            if (!System.Text.RegularExpressions.Regex.IsMatch(def.Name, "^[A-Za-z_][A-Za-z0-9_]*$") || (def.Parent != "" && !System.Text.RegularExpressions.Regex.IsMatch(def.Parent, "^[A-Za-z_][A-Za-z0-9_]*$")))
                throw new InvalidDataException("Invalid class identifier: " + def.Name);
            if (def.Providers.Count == 0) throw new InvalidDataException("Defining CfgPatches addon missing for " + def.Name);
            foreach (string provider in def.Providers) if (!provider.Equals("GRZ_InventorySizes", StringComparison.OrdinalIgnoreCase)) dependencies.Add(provider);
        }
        string[] deps = dependencies.OrderBy(n => n, StringComparer.OrdinalIgnoreCase).ToArray();
        foreach (string dependency in deps)
            if (!System.Text.RegularExpressions.Regex.IsMatch(dependency, "^[A-Za-z_][A-Za-z0-9_]*$"))
                throw new InvalidDataException("Invalid addon identifier: " + dependency);
        string config = "// Generated from installed DayZ class definitions. Only inventory dimensions change.\nclass CfgPatches\n{\n    class GRZ_InventorySizes\n    {\n        units[] = {};\n        weapons[] = {};\n        requiredVersion = 0.1;\n        requiredAddons[] = {" + String.Join(", ", deps.Select(n => "\"" + n + "\"")) + "};\n    };\n};\n";
        config += Render("CfgVehicles", allClothing, "itemsCargoSize", "10, 10", cargoOverrides);
        config += Render("CfgWeapons", guns, "itemSize", "10, 2");
        Directory.CreateDirectory(output);
        string source = Path.Combine(output, "config.cpp");
        string pbo = Path.Combine(output, "GRZ_InventorySizes.pbo");
        File.WriteAllText(source, config, new UTF8Encoding(false));
        File.WriteAllBytes(pbo, Pack(config));
        return new Report { ArchivesRead = read, Clothing = clothing.Select(d => d.Name).OrderBy(n => n).ToArray(), NormalClothing = allClothing.Where(d => !expanded.Contains(d.Name)).Select(d => d.Name).OrderBy(n => n).ToArray(), Guns = guns.Select(d => d.Name).OrderBy(n => n).ToArray(), RequiredAddons = deps, PboPath = pbo, ConfigPath = source };
    }
}
