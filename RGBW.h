#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class RGBW
{
private:
    uint8_t r_;
    uint8_t g_;
    uint8_t b_;
    uint8_t w_;

    uint8_t Clamp(double value) const
    {
        return min(255., max(0., value));
    }

public:

    static RGBW FromHSV(uint16_t hue, uint8_t saturation, uint8_t value)
    {
        uint32_t color = Adafruit_NeoPixel::ColorHSV(hue, saturation, value);
        return RGBW(color);
    }

    static RGBW AllChannels(uint8_t value)
    {
        return RGBW(value, value, value, value);
    }

    RGBW()
        : RGBW(0, 0, 0, 0)
    {
    }

    RGBW(byte r, byte g, byte b)
        : RGBW(r, g, b, 0)
    {
    }

    RGBW(byte r, byte g, byte b, byte w)
        : r_(r), g_(g), b_(b), w_(w)
    {
    }

    RGBW(uint32_t rgbw)
        : RGBW(
              (rgbw >> 16) & 0xFFUL,
              (rgbw >> 8) & 0xFFUL,
              rgbw & 0xFFUL,
              (rgbw >> 24) & 0xFFUL)
    {
    }

    RGBW(const RGBW &other)
        : RGBW(other.r_, other.g_, other.b_, other.w_)
    {
    }

    inline RGBW Blend(const RGBW &topColor, double opacity) const
    {
        if (opacity > 0.99)
            return topColor;

        if (opacity < 0.01)
            return *this;

        double bottomOpacity = 1.0 - opacity;
        return RGBW(
            r_ * bottomOpacity + topColor.r_ * opacity,
            g_ * bottomOpacity + topColor.g_ * opacity,
            b_ * bottomOpacity + topColor.b_ * opacity,
            w_ * bottomOpacity + topColor.w_ * opacity);
    }

    RGBW PerChannelMax(const RGBW &other) const
    {
        return RGBW(
            max(r_, other.r_),
            max(g_, other.g_),
            max(b_, other.b_),
            max(w_, other.w_));
    }

    RGBW AdjustBrightness(double brightnessPercent) const
    {
        return RGBW(
            Clamp(r_ * brightnessPercent),
            Clamp(g_ * brightnessPercent),
            Clamp(b_ * brightnessPercent),
            Clamp(w_ * brightnessPercent)
        );
    }

    uint32_t GetRaw() const
    {
        return ((uint32_t)w_ << 24) | ((uint32_t)r_ << 16) | ((uint32_t)g_ << 8) | (uint32_t)b_;
    }

    uint8_t &R()
    {
        return r_;
    }

    uint8_t &G()
    {
        return g_;
    }

    uint8_t &B()
    {
        return b_;
    }

    uint8_t &W()
    {
        return w_;
    }

    const uint8_t &R() const
    {
        return r_;
    }

    const uint8_t &G() const
    {
        return g_;
    }

    const uint8_t &B() const
    {
        return b_;
    }

    const uint8_t &W() const
    {
        return w_;
    }
};
