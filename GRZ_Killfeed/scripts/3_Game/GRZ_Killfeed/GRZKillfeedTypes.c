class GRZKillfeedRPC
{
    static const int REQUEST_MENU = 1647280101;
    static const int MENU_STATE = 1647280102;
    static const int SAVE_STYLE = 1647280103;
    static const int KILL = 1647280104;
}

class GRZNameStyle
{
    static const int PALETTE_COUNT = 12;
    static const int EFFECT_COUNT = 6;
    int ColorA;
    int ColorB = 1;
    int AnimationMode;
    int Speed = 1;

    bool IsValid()
    {
        return ColorA >= 0 && ColorA < PALETTE_COUNT && ColorB >= 0 && ColorB < PALETTE_COUNT && AnimationMode >= 0 && AnimationMode < EFFECT_COUNT && Speed >= 0 && Speed < 3;
    }

    int ColorAt(float seconds)
    {
        float frequency = 1.0;
        if (Speed == 0)
            frequency = 0.5;
        if (Speed == 2)
            frequency = 2.0;
        float wave = (Math.Sin(seconds * frequency * Math.PI2) + 1.0) * 0.5;
        int a = Palette(ColorA);
        int b = Palette(ColorB);
        if (AnimationMode == 1) // Opacity pulse.
            return WithAlpha(a, 90 + wave * 165);
        if (AnimationMode == 2) // Blink without hiding the name entirely.
        {
            if (wave < 0.5)
                return WithAlpha(a, 65);
            return a;
        }
        if (AnimationMode == 3) // Smooth two-color flash.
            return Mix(a, b, wave);
        if (AnimationMode == 4) // Alternating colors.
        {
            if (wave < 0.5)
                return a;
            return b;
        }
        if (AnimationMode == 5)
        {
            float position = seconds * frequency;
            int step = Math.Floor(position);
            int current = step % 7 + 1;
            int next = (step + 1) % 7 + 1;
            return Mix(Palette(current), Palette(next), position - step);
        }
        return a;
    }

    static int Palette(int index)
    {
        switch (index)
        {
            case 1: return 0xFFFF5353;
            case 2: return 0xFFFFA347;
            case 3: return 0xFFFFE36A;
            case 4: return 0xFF31D66E;
            case 5: return 0xFF4285FF;
            case 6: return 0xFF5B44CF;
            case 7: return 0xFFC56DFF;
            case 8: return 0xFF68E8FF;
            case 9: return 0xFFFF80C8;
            case 10: return 0xFF9E9E9E;
            case 11: return 0xFF000000;
        }
        return 0xFFFFFFFF;
    }

    static string ColorName(int index)
    {
        switch (index)
        {
            case 1: return "Red";
            case 2: return "Orange";
            case 3: return "Yellow";
            case 4: return "Green";
            case 5: return "Blue";
            case 6: return "Indigo";
            case 7: return "Violet";
            case 8: return "Cyan";
            case 9: return "Pink";
            case 10: return "Gray";
            case 11: return "Black";
        }
        return "White";
    }

    static string EffectName(int index)
    {
        switch (index)
        {
            case 1: return "Pulse";
            case 2: return "Blink";
            case 3: return "Color fade";
            case 4: return "Color swap";
            case 5: return "Rainbow";
        }
        return "Steady";
    }

    static int WithAlpha(int color, int alpha)
    {
        return ARGB(alpha, (color >> 16) & 255, (color >> 8) & 255, color & 255);
    }

    static int Mix(int a, int b, float amount)
    {
        int r = ((a >> 16) & 255) * (1 - amount) + ((b >> 16) & 255) * amount;
        int g = ((a >> 8) & 255) * (1 - amount) + ((b >> 8) & 255) * amount;
        int blue = (a & 255) * (1 - amount) + (b & 255) * amount;
        return ARGB(255, r, g, blue);
    }
}

class GRZKillfeedMessage
{
    string Killer;
    string Victim;
    string WeaponJson;
    int Distance;
    ref GRZKillfeedGunData GunData = new GRZKillfeedGunData;
    ref GRZNameStyle KillerStyle = new GRZNameStyle;
    ref GRZNameStyle VictimStyle = new GRZNameStyle;
}

class GRZKillfeedGunData
{
    ref array<string> Types = new array<string>;
    ref array<int> Parents = new array<int>;
    ref array<int> Slots = new array<int>;

    bool IsValid()
    {
        if (!Types || !Parents || !Slots || Types.Count() > 32 || Types.Count() != Parents.Count() || Types.Count() != Slots.Count())
            return false;
        for (int i = 0; i < Types.Count(); i++)
        {
            if (Types[i] == "" || Types[i].Length() > 128)
                return false;
            if (i > 0 && (Parents[i] < 0 || Parents[i] >= i))
                return false;
        }
        return true;
    }
}

class GRZKillfeedBus
{
    static ref ScriptInvoker Kills = new ScriptInvoker;
    static ref ScriptInvoker Menu = new ScriptInvoker;
    static ref ScriptInvoker AppDeactivated = new ScriptInvoker;
}

class GRZKillfeedConfig
{
    ref array<string> AllowedSteamIds = new array<string>;
}

class GRZKillfeedSavedStyle
{
    string SteamId;
    ref GRZNameStyle Style = new GRZNameStyle;
}

class GRZKillfeedSavedStyles
{
    ref array<ref GRZKillfeedSavedStyle> Players = new array<ref GRZKillfeedSavedStyle>;
}
