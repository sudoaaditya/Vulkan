#pragma once

// Previous default values for the Raging Sea controls, kept for reference / reverting.
// Not included anywhere; copy the values back into gSeaUiState (vk.cpp) if needed.

// ---- Old wave defaults (used by both the night and the first daytime version) ----
namespace OldWaveValues {
    const float timeScale         = 0.32f;
    const float windDirectionX    = 1.0f;
    const float windDirectionY    = 0.35f;
    const float primaryWavelength = 5.2f;
    const float primaryAmplitude  = 0.18f;
    const float waveSpeed         = 0.65f;
    const float choppiness        = 0.58f;
    const float detailHeight      = 0.06f;
    const float detailFrequency   = 4.0f;
    const float detailSpeed       = 1.4f;
    const float detailLayers      = 3.0f;
}

// ---- Original night-time (moon + stars) look ----
// The shader no longer has moon/star code; these are the values from the night version.
namespace OldNightValues {
    const float depthColor[3]      = {0.004f, 0.05f, 0.10f};
    const float surfaceColor[3]    = {0.02f, 0.16f, 0.25f};
    const float skyBottomColor[3]  = {0.03f, 0.06f, 0.16f};
    const float skyTopColor[3]     = {0.005f, 0.015f, 0.05f};
    const float moonDirection[3]   = {-0.25f, 0.88f, -0.40f};
    const float moonSize           = 0.992f;
    const float moonColor[3]       = {0.90f, 0.94f, 1.00f};
    const float moonIntensity      = 1.35f;
    const float moonGlow           = 0.20f;
    const float starDensity        = 0.42f;
    const float starIntensity      = 1.10f;
    const float starTwinkle        = 0.45f;
    const float skyExposure        = 1.00f;
    const float colorOffset        = 0.10f;
    const float colorMultiplier    = 1.05f;
    const float fresnelPower       = 5.6f;
    const float reflectionStrength = 0.95f;
    const float specularPower      = 110.0f;
    const float foamHeight         = 0.32f;
    const float foamIntensity      = 0.22f;
}

// ---- First daytime version (before dimming, i.e. the "white washed" look) ----
namespace OldDayValues {
    const float depthColor[3]      = {0.00f, 0.055f, 0.11f};
    const float surfaceColor[3]    = {0.02f, 0.28f, 0.36f};
    const float skyBottomColor[3]  = {0.70f, 0.82f, 0.92f};
    const float skyTopColor[3]     = {0.18f, 0.42f, 0.78f};
    const float sunDirection[3]    = {-0.15f, 0.22f, -1.0f};
    const float sunColor[3]        = {1.00f, 0.92f, 0.78f};
    const float sunIntensity       = 6.0f;
    const float sunGlow            = 0.35f;
    const float glitterStrength    = 0.35f;
    const float skyAmbient         = 1.0f;
    const float sssStrength        = 0.12f;
    const float horizonHaze        = 0.6f;
    const float fogDensity         = 0.8f;
    const float skyExposure        = 1.00f;
    const float reflectionStrength = 0.95f;
    const float specularPower      = 110.0f;
}
