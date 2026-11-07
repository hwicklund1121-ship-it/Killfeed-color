using System;
using System.IO;
using System.Text;
using System.Collections.Generic;

public sealed class GRZDropGraphEntry {
    public string Path;
    public string Archive;
    public uint Method;
    public uint OriginalSize;
    public uint StoredSize;
    public long Offset;
    public bool Graph;
}

public static class GRZDropGraphPboV1 {
    const uint Compressed = 0x43707273;
    static string CString(BinaryReader reader) {
        var data = new List<byte>();
        for (int i = 0; i < 65536; ++i) {
            byte value = reader.ReadByte();
            if (value == 0) return Encoding.UTF8.GetString(data.ToArray());
            data.Add(value);
        }
        throw new InvalidDataException("Oversized PBO header string.");
    }
    public static GRZDropGraphEntry[] Index(string archive) {
        var all = new List<GRZDropGraphEntry>();
        string prefix = "";
        using (var r = new BinaryReader(File.OpenRead(archive))) {
            bool ended = false;
            for (int i = 0; i < 100000; ++i) {
                string name = CString(r);
                uint method = r.ReadUInt32();
                uint original = r.ReadUInt32();
                r.ReadUInt32(); r.ReadUInt32();
                uint stored = r.ReadUInt32();
                if (name.Length == 0 && method == 0x56657273) {
                    bool extEnded = false;
                    for (int k = 0; k < 1000; ++k) {
                        string key = CString(r);
                        if (key.Length == 0) { extEnded = true; break; }
                        string value = CString(r);
                        if (key.Equals("prefix", StringComparison.OrdinalIgnoreCase)) prefix = value;
                    }
                    if (!extEnded) throw new InvalidDataException("PBO extension header limit exceeded.");
                    continue;
                }
                if (name.Length == 0) { ended = true; break; }
                all.Add(new GRZDropGraphEntry { Path = name, Archive = System.IO.Path.GetFileName(archive), Method = method, OriginalSize = original, StoredSize = stored });
            }
            if (!ended) throw new InvalidDataException("PBO header entry limit exceeded.");
            prefix = prefix.Replace('\\', '/').Trim('/').ToLowerInvariant();
            long offset = r.BaseStream.Position;
            var selected = new List<GRZDropGraphEntry>();
            foreach (var entry in all) {
                entry.Offset = offset;
                offset = checked(offset + entry.StoredSize);
                if (offset > r.BaseStream.Length) throw new InvalidDataException("PBO entry exceeds archive length.");
                string path = entry.Path.Replace('\\', '/').TrimStart('/').ToLowerInvariant();
                if (prefix.Length > 0 && !path.StartsWith(prefix + "/", StringComparison.Ordinal)) path = prefix + "/" + path;
                foreach (string part in path.Split('/')) {
                    if (part == ".." || part == "." || part.IndexOf(':') >= 0) throw new InvalidDataException("Invalid virtual path.");
                }
                entry.Path = path;
                string ext = System.IO.Path.GetExtension(path);
                bool graphExt = ext == ".agr" || ext == ".agf" || ext == ".asi" || ext == ".ast" || ext == ".asy";
                entry.Graph = path.StartsWith("dz/anims/workspaces/player/", StringComparison.Ordinal) && graphExt;
                if (entry.Graph || (path.StartsWith("dz/anims/anm/player/", StringComparison.Ordinal) && ext == ".anm")) selected.Add(entry);
            }
            return selected.ToArray();
        }
    }
    public static byte[] Read(string archive, GRZDropGraphEntry entry) {
        if (!entry.Graph) throw new InvalidDataException("Only native player graph payloads may be exported.");
        if (entry.StoredSize > 16 * 1024 * 1024 || entry.OriginalSize > 16 * 1024 * 1024) throw new InvalidDataException("Graph entry exceeds 16 MiB limit.");
        byte[] bytes;
        using (var r = new BinaryReader(File.OpenRead(archive))) {
            r.BaseStream.Position = entry.Offset;
            bytes = r.ReadBytes((int)entry.StoredSize);
        }
        if (bytes.Length != entry.StoredSize) throw new EndOfStreamException("Truncated graph entry.");
        if (entry.Method == 0) return bytes;
        if (entry.Method != Compressed) throw new InvalidDataException("Unsupported PBO compression method.");
        var output = new byte[entry.OriginalSize];
        int src = 0, dst = 0;
        uint checksum = 0;
        while (dst < output.Length) {
            if (src >= bytes.Length - 4) throw new InvalidDataException("Truncated compressed graph.");
            int flags = bytes[src++];
            for (int bit = 0; bit < 8 && dst < output.Length; ++bit) {
                if ((flags & (1 << bit)) != 0) {
                    if (src >= bytes.Length - 4) throw new InvalidDataException("Truncated compressed literal.");
                    byte value = bytes[src++]; output[dst++] = value;
                    checksum = unchecked(checksum + value);
                } else {
                    if (src + 2 > bytes.Length - 4) throw new InvalidDataException("Truncated compressed reference.");
                    int first = bytes[src++], second = bytes[src++];
                    int distance = first | ((second & 0xf0) << 4);
                    int count = (second & 0xf) + 3;
                    if (distance == 0 || dst + count > output.Length) throw new InvalidDataException("Invalid compressed reference.");
                    for (int k = 0; k < count; ++k) {
                        int index = dst - distance;
                        byte value = index < 0 ? (byte)32 : output[index];
                        output[dst++] = value; checksum = unchecked(checksum + value);
                    }
                }
            }
        }
        if (src + 4 != bytes.Length || BitConverter.ToUInt32(bytes, src) != checksum) throw new InvalidDataException("Graph decompression checksum mismatch.");
        return output;
    }
}
