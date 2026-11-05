modded class LoadingScreen
{
    protected float m_GRZSlideElapsed;
    protected int m_GRZSlideIndex;

    void LoadingScreen(DayZGame game)
    {
        if (!m_ImageBackground)
            return;
        m_ImageBackground.LoadImageFile(0, "GRZ_LoadingScreen/data/loading_1.paa");
        m_ImageBackground.LoadImageFile(1, "GRZ_LoadingScreen/data/loading_2.paa");
        m_ImageBackground.LoadImageFile(2, "GRZ_LoadingScreen/data/loading_3.paa");
        m_GRZSlideIndex = Math.RandomInt(0, 3);
        GRZApplyLoadingSlide();
    }

    protected void GRZApplyLoadingSlide()
    {
        if (!m_ImageBackground)
            return;
        m_ImageBackground.SetImage(m_GRZSlideIndex);
        m_ImageBackground.SetMaskProgress(1.0);
        // The native asynchronous image wipe would obscure the supplied artwork.
        // The standard progress bar remains connected by Show().
        ProgressAsync.SetUserData(null);
        if (m_ImageLogoMid)
            m_ImageLogoMid.Show(false);
        if (m_ImageLogoCorner)
            m_ImageLogoCorner.Show(false);
    }

    override void Show()
    {
        bool wasVisible = IsLoading();
        super.Show();
        if (!wasVisible)
        {
            m_GRZSlideElapsed = 0;
            m_GRZSlideIndex = Math.RandomInt(0, 3);
        }
        GRZApplyLoadingSlide();
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        if (!IsLoading())
            return;
        m_GRZSlideElapsed += timeslice;
        if (m_GRZSlideElapsed >= 10.0)
        {
            m_GRZSlideElapsed = 0;
            m_GRZSlideIndex = (m_GRZSlideIndex + 1) % 3;
            GRZApplyLoadingSlide();
        }
    }
};
