class GRZHitmarkerVisual
{
    Widget Root;
    protected TextWidget m_DamageLabel;
    protected CanvasWidget m_Lines;
    protected float m_LineSize = -1;
    protected int m_LineColor;

    void GRZHitmarkerVisual(Widget parent)
    {
        Root = g_Game.GetWorkspace().CreateWidgets("GRZ_Hitmarker/gui/layouts/marker.layout", parent);
        if (!Root)
            return;
        m_DamageLabel = TextWidget.Cast(Root.FindAnyWidget("Damage"));
        m_Lines = CanvasWidget.Cast(Root.FindAnyWidget("MarkerLines"));
        Root.Show(false);
    }

    void Draw(GRZHitmarkerSettings settings, bool head, bool killed, float damage, float centerX, float centerY, float alpha = 1.0, bool preview = false)
    {
        if (!Root)
            return;
        int index = settings.BodyColor;
        if (head)
            index = settings.HeadColor;
        int color = GRZHitmarkerSettings.Color(index);
        int outline = 0xFF000000;
        if (index == 11)
            outline = 0xFFFFFFFF;
        bool highlight = killed && settings.HighlightKills;
        float size = 64.0 * settings.SizePercent / 100.0;
        if (highlight)
            size *= 1.25;
        Root.SetScreenSize(size, size);
        Root.SetScreenPos(centerX - size * 0.5, centerY - size * 0.5);
        Root.SetAlpha(alpha);
        Root.Show(preview || settings.ShowHitmarkers || settings.ShowDamage || highlight);
        if (m_Lines)
        {
            m_Lines.Show(settings.ShowHitmarkers || preview);
            if (m_LineSize != size || m_LineColor != color)
            {
                m_LineSize = size;
                m_LineColor = color;
                m_Lines.Clear();
                float outer = size * 0.18;
                float inner = size * 0.32;
                float lineWidth = Math.Max(1, size / 32.0);
                DrawSegment(outer, outer, inner, inner, lineWidth, color, outline);
                DrawSegment(size - outer, outer, size - inner, inner, lineWidth, color, outline);
                DrawSegment(outer, size - outer, inner, size - inner, lineWidth, color, outline);
                DrawSegment(size - outer, size - outer, size - inner, size - inner, lineWidth, color, outline);
            }
        }
        string detail;
        if (highlight && !preview)
            detail = "KILL";
        if (settings.ShowDamage && !preview)
        {
            if (detail != "")
                detail += " | ";
            detail += Math.Round(damage).ToString();
        }
        m_DamageLabel.SetText(detail);
        m_DamageLabel.SetTextExactSize(18);
        m_DamageLabel.SetOutline(1, 0xFF000000);
        m_DamageLabel.Show(detail != "");
    }

    protected void DrawSegment(float x1, float y1, float x2, float y2, float lineWidth, int color, int outline)
    {
        m_Lines.DrawLine(x1, y1, x2, y2, lineWidth + 1.5, outline);
        m_Lines.DrawLine(x1, y1, x2, y2, lineWidth, color);
    }

    void Dispose()
    {
        if (Root)
        {
            Root.Unlink();
            Root = null;
        }
    }
}

class GRZHitmarkerSoundVoice
{
    ref SoundParams Params;
    ref SoundObjectBuilder Builder;
    ref SoundObject Sound;
    AbstractWave Wave;
    float Age;
}

class GRZHitmarkerAudio
{
    protected static bool s_Ticking;
    protected static ref array<ref GRZHitmarkerSoundVoice> s_Voices = new array<ref GRZHitmarkerSoundVoice>;

    static void Play(bool head, int volume)
    {
        if (g_Game.IsDedicatedServer() || !g_Game.IsAppActive() || volume <= 0)
            return;
        string soundSet = "GRZ_HitmarkerBody_SoundSet";
        if (head)
            soundSet = "GRZ_HitmarkerHead_SoundSet";
        GRZHitmarkerSoundVoice voice = new GRZHitmarkerSoundVoice;
        voice.Params = new SoundParams(soundSet);
        if (!voice.Params.IsValid())
        {
            Error("GRZ Hitmarker: invalid sound set " + soundSet);
            return;
        }
        voice.Builder = new SoundObjectBuilder(voice.Params);
        voice.Sound = voice.Builder.BuildSoundObject();
        if (!voice.Sound)
        {
            Error("GRZ Hitmarker: could not build sound " + soundSet);
            return;
        }
        voice.Sound.SetKind(WaveKind.WAVEEFFECTEX);
        voice.Wave = g_Game.GetSoundScene().Play2D(voice.Sound, voice.Builder);
        if (!voice.Wave)
        {
            Error("GRZ Hitmarker: 2D playback failed for " + soundSet);
            return;
        }
        voice.Wave.Loop(false);
        voice.Wave.SetVolumeRelative(Math.Clamp(volume / 100.0, 0, 1));
        voice.Wave.Play();
        voice.Wave.SetVolumeRelative(Math.Clamp(volume / 100.0, 0, 1));
        if (s_Voices.Count() >= 12)
        {
            if (s_Voices[0].Wave)
                s_Voices[0].Wave.Stop();
            s_Voices.Remove(0);
        }
        s_Voices.Insert(voice);
        if (!s_Ticking)
        {
            s_Ticking = true;
            g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(Tick, 100, true);
        }
    }

    static void Tick()
    {
        Update(0.1);
        if (s_Voices.Count() == 0)
        {
            s_Ticking = false;
            g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(Tick);
        }
    }

    static void Update(float seconds)
    {
        for (int i = s_Voices.Count() - 1; i >= 0; i--)
        {
            s_Voices[i].Age += seconds;
            if (s_Voices[i].Age > 3.0)
            {
                if (s_Voices[i].Wave)
                    s_Voices[i].Wave.Stop();
                s_Voices.Remove(i);
            }
        }
    }

    static void Clear()
    {
        foreach (GRZHitmarkerSoundVoice voice : s_Voices)
        {
            if (voice.Wave)
                voice.Wave.Stop();
        }
        s_Voices.Clear();
        s_Ticking = false;
        g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(Tick);
    }
}

modded class MissionGameplay
{
    protected ref GRZHitmarkerVisual m_GRZHitmarker;
    protected ref GRZHitmarkerEvent m_GRZLastHit;
    protected float m_GRZHitAge = 10;
    protected bool m_GRZHitListening;

    override void OnInit()
    {
        super.OnInit();
        if (!m_GRZHitListening)
        {
            GRZHitmarkerBus.Hits.Insert(GRZReceiveHitmarker);
            m_GRZHitListening = true;
        }
    }

    protected void GRZReceiveHitmarker(GRZHitmarkerEvent hit)
    {
        if (!hit || !g_Game.IsAppActive())
            return;
        m_GRZLastHit = hit;
        m_GRZHitAge = 0;
        GRZHitmarkerSettings settings = GRZHitmarkerStore.Current();
        if (settings.PlaySound)
            GRZHitmarkerAudio.Play(hit.IsHeadHit, settings.VolumePercent);
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        m_GRZHitAge += timeslice;
        IngameHud hud = IngameHud.Cast(GetHud());
        if (!m_GRZHitmarker && hud && hud.GetHudPanelWidget())
            m_GRZHitmarker = new GRZHitmarkerVisual(hud.GetHudPanelWidget());
        if (!m_GRZHitmarker || !m_GRZHitmarker.Root)
            return;
        if (!m_GRZLastHit || m_GRZHitAge > 0.6 || !g_Game.IsAppActive() || !hud || hud.GetHudVisibility().IsContextFlagActive(IngameHudVisibility.HUD_HIDE_FLAGS))
        {
            m_GRZHitmarker.Root.Show(false);
            return;
        }
        float x, y, width, height;
        hud.GetHudPanelWidget().GetScreenPos(x, y);
        hud.GetHudPanelWidget().GetScreenSize(width, height);
        float alpha = Math.Clamp((0.6 - m_GRZHitAge) / 0.25, 0, 1);
        m_GRZHitmarker.Draw(GRZHitmarkerStore.Current(), m_GRZLastHit.IsHeadHit, m_GRZLastHit.IsKill, m_GRZLastHit.DamageAmount, x + width * 0.5, y + height * 0.5, alpha);
    }

    override void OnMissionFinish()
    {
        GRZHitmarkerBus.Hits.Remove(GRZReceiveHitmarker);
        m_GRZHitListening = false;
        GRZHitmarkerAudio.Clear();
        if (m_GRZHitmarker)
            m_GRZHitmarker.Dispose();
        m_GRZHitmarker = null;
        m_GRZLastHit = null;
        super.OnMissionFinish();
    }
}
