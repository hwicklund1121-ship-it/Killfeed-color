// Shared validation for the UI, server RPC saves and loaded killfeed styles.
class GRZPrefixPolicy
{
    static string Normalize(string word)
    {
        word.ToLower();
        word.Replace("_", "");
        word.Replace("-", "");
        word.Replace("0", "o");
        word.Replace("1", "i");
        word.Replace("3", "e");
        word.Replace("4", "a");
        word.Replace("5", "s");
        word.Replace("6", "g");
        word.Replace("7", "t");
        word.Replace("8", "b");
        word.Replace("9", "g");
        return word;
    }

    static bool Reserved(string word)
    {
        string normalized = Normalize(word);
        // Contains matching also covers co-owner, head-admin and staff-team.
        array<string> reserved = {"admin", "owner", "mod", "staff", "support", "helper", "manager", "dev", "founder", "operator", "official", "gamemaster", "superuser", "sysop", "leadership"};
        foreach (string role : reserved)
        {
            if (normalized.IndexOf(role) >= 0)
                return true;
        }
        // Numeric decoration must not turn GM into a usable staff badge.
        string letters;
        word.ToLower();
        for (int i = 0; i < word.Length(); i++)
        {
            string character = word.Substring(i, 1);
            if ("abcdefghijklmnopqrstuvwxyz".IndexOf(character) >= 0)
                letters += character;
        }
        return letters == "gm" || normalized == "gm";
    }
}
