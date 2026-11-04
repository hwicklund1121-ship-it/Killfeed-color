// Keep native state updates and synchronized random draws, then remove sway only.
modded class DayZPlayerImplementAiming
{
    override protected void ApplyBreathingPattern(out float x_axis, out float y_axis, float pAmplitude, float pTotalTime, float weight)
    {
        super.ApplyBreathingPattern(x_axis, y_axis, pAmplitude, pTotalTime, weight);
        x_axis = 0;
        y_axis = 0;
    }

    override protected void ApplyHorizontalNoise(out float x_axis, out float y_axis, float smooth_time, float max_velocity_low, float max_velocity_high, float velocity_modifier, float max_distance, float weight, float pDt)
    {
        super.ApplyHorizontalNoise(x_axis, y_axis, smooth_time, max_velocity_low, max_velocity_high, velocity_modifier, max_distance, weight, pDt);
        x_axis = 0;
        y_axis = 0;
    }
}
