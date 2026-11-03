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
    private static byte[] ReadConfig(string path)
    {
        using (BinaryReader r = new BinaryReader(File.Open(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite)))
        {
            List<Entry> entries = new List<Entry>();
            for (int i = 0; i < 100000; i++)
            {
                string name = ReadZ(r);
                uint method = r.ReadUInt32();
                uint original = r.ReadUInt32();
                r.ReadUInt32(); r.ReadUInt32();
                uint size = r.ReadUInt32();
                if (name == "" && method == 0x56657273)
                {
                    string key;
                    while ((key = ReadZ(r)) != "")
                    {
                        string value = ReadZ(r);
                        if (key.Equals("prefix", StringComparison.OrdinalIgnoreCase) && value.Equals("GRZ_InventorySizes", StringComparison.OrdinalIgnoreCase))
                            return null; // never depend on a prior generated copy of ourselves
                    }
                    continue;
                }
                if (name == "") break;
                entries.Add(new Entry { Name = name.Replace('/', '\\'), Method = method, Original = original, Size = size });
            }
            long offset = r.BaseStream.Position;
            bool sourceOnly = false;
            foreach (Entry entry in entries)
            {
                if (entry.Name.Equals("config.cpp", StringComparison.OrdinalIgnoreCase)) sourceOnly = true;
                if (entry.Name.Equals("config.bin", StringComparison.OrdinalIgnoreCase))
                {
                    if (entry.Method != 0)
                        throw new InvalidDataException("Compressed config.bin is not supported by this builder; archive: " + Path.GetFileName(path));
                    if (entry.Size == 0 || entry.Size > 32 * 1024 * 1024 || offset + entry.Size > r.BaseStream.Length)
                        throw new InvalidDataException("Invalid config size in " + Path.GetFileName(path));
                    r.BaseStream.Position = offset;
                    return r.ReadBytes((int)entry.Size);
                }
                offset += entry.Size;
            }
            if (sourceOnly)
                throw new InvalidDataException("Source-only config.cpp requires a binarized config for cataloging; archive: " + Path.GetFileName(path));
            return null; // resource-only archive has no class definitions
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
    private static byte[] Pack(string config)
    {
        byte[] payload = new UTF8Encoding(false).GetBytes(config);
        byte[] body;
        using (MemoryStream ms = new MemoryStream())
        {
            using (BinaryWriter w = new BinaryWriter(ms, Encoding.UTF8, true))
            {
                Header(w, "", 0x56657273, 0); Z(w, "prefix"); Z(w, "GRZ_InventorySizes"); Z(w, "");
                Header(w, "config.cpp", 0, (uint)payload.Length); Header(w, "", 0, 0); w.Write(payload); w.Flush();
                body = ms.ToArray();
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
                byte[] bytes = ReadConfig(archive);
                read++;
                if (bytes == null) continue;
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
