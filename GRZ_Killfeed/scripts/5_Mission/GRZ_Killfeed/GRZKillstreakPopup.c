class GRZKillstreakPopupData
{
    string PlayerName;
    int Count;
}

class GRZKillstreakPopup
{
    protected Widget m_Root;
    protected ref array<ref GRZKillstreakPopupData> m_Queue = new array<ref GRZKillstreakPopupData>;
    protected float m_Age = 5;
    protected AbstractWave m_Chime;
    protected EffectSound m_FallbackChime;

    void Enqueue(string playerName, int count)
    {
        if (count <= 0 || count % 10 != 0)
            return;
        GRZKillstreakPopupData data = new GRZKillstreakPopupData;
        data.PlayerName = playerName;
        data.Count = count;
        if (m_Queue.Count() >= 16)
            m_Queue.Remove(0);
        m_Queue.Insert(data);
    }

    protected void PlayChime()
    {
        if (m_Chime)
        {
            m_Chime.Stop();
            m_Chime = null;
        }
        SoundParams params = new SoundParams("GRZ_Killstreak_Notification_SoundSet");
        SoundObjectBuilder builder = new SoundObjectBuilder(params);
        SoundObject sound = builder.BuildSoundObject();
        if (sound)
        {
            sound.SetKind(WaveKind.WAVEUI);
            m_Chime = g_Game.GetSoundScene().Play2D(sound, builder);
            if (m_Chime)
            {
                m_Chime.Loop(false);
                m_Chime.Play();
                return;
            }
        }
        // Keep native effect-manager playback as a fallback for sound scenes
        // that do not return a 2D wave. Neither route changes volume settings.
        if (m_FallbackChime)
            SEffectManager.DestroyEffect(m_FallbackChime);
        m_FallbackChime = SEffectManager.CreateSound("GRZ_Killstreak_Notification_SoundSet", g_Game.GetCurrentCameraPosition(), 0, 0, false, false);
        if (m_FallbackChime)
        {
            m_FallbackChime.SetSoundWaveKind(WaveKind.WAVEUI);
            m_FallbackChime.SetSoundAutodestroy(true);
            if (m_FallbackChime.SoundPlay())
                return;
        }
        Print("[GRZ Killstreaks] Notification chime could not start.");
    }

    void Update(float timeslice, Widget hudRoot)
    {
        if (!hudRoot)
            return;
        if (!m_Root)
        {
            m_Root = g_Game.GetWorkspace().CreateWidgets("GRZ_Killfeed/gui/layouts/streak.layout", hudRoot);
            if (!m_Root)
                return;
            m_Root.SetSort(500);
        }
        m_Age += timeslice;
        if (m_Age >= 5 && m_Queue.Count() > 0)
        {
            GRZKillstreakPopupData data = m_Queue[0];
            m_Queue.Remove(0);
            TextWidget title = TextWidget.Cast(m_Root.FindAnyWidget("StreakTitle"));
            TextWidget detail = TextWidget.Cast(m_Root.FindAnyWidget("StreakDetail"));
            title.SetText("GRZ KILLSTREAKS");
            detail.SetText(data.PlayerName + " - " + data.Count.ToString() + " killstreak!");
            m_Age = 0;
            PlayChime();
        }
        if (m_Age >= 5)
        {
            m_Root.Show(false);
            return;
        }
        float x, y, width, height;
        hudRoot.GetScreenPos(x, y);
        hudRoot.GetScreenSize(width, height);
        float scale = Math.Clamp(height / 1080.0, 0.75, 2.0);
        float popupWidth = Math.Min(560 * scale, width - 32);
        m_Root.SetScreenSize(popupWidth, 88 * scale);
        m_Root.SetScreenPos(x + (width - popupWidth) * 0.5, y + 48 * scale);
        TextWidget.Cast(m_Root.FindAnyWidget("StreakTitle")).SetTextExactSize(Math.Round(22 * scale));
        TextWidget fittedDetail = TextWidget.Cast(m_Root.FindAnyWidget("StreakDetail"));
        fittedDetail.SetTextExactSize(Math.Round(20 * scale));
        int textWidth, textHeight;
        fittedDetail.GetTextSize(textWidth, textHeight);
        if (textWidth > popupWidth * 0.94)
            fittedDetail.SetTextExactSize(Math.Max(12, Math.Floor(20 * scale * popupWidth * 0.94 / textWidth)));
        float alpha = Math.Min(Math.Clamp(m_Age / 0.2, 0, 1), Math.Clamp((5 - m_Age) / 0.5, 0, 1));
        m_Root.SetAlpha(alpha);
        m_Root.Show(true);
    }

    void Dispose()
    {
        m_Queue.Clear();
        if (m_Chime)
            m_Chime.Stop();
        m_Chime = null;
        if (m_FallbackChime)
            SEffectManager.DestroyEffect(m_FallbackChime);
        m_FallbackChime = null;
        if (m_Root)
            m_Root.Unlink();
        m_Root = null;
    }
}
