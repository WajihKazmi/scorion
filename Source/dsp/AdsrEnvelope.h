#pragma once

#include <cmath>
#include <algorithm>

class AdsrEnvelope
{
public:
    enum class Stage { Idle, Attack, Decay, Sustain, Release };

    void prepare (double sampleRate) noexcept
    {
        sampleRate_ = sampleRate;
        reset();
    }

    void setAttack (float seconds) noexcept { attackSec_ = std::max (0.001f, seconds); }
    void setDecay (float seconds) noexcept { decaySec_ = std::max (0.001f, seconds); }
    void setSustain (float level) noexcept { sustain_ = std::clamp (level, 0.0f, 1.0f); }
    void setRelease (float seconds) noexcept { releaseSec_ = std::max (0.001f, seconds); }

    void noteOn() noexcept
    {
        stage_ = Stage::Attack;
        if (attackSec_ <= 0.001f)
        {
            level_ = 1.0f;
            stage_ = Stage::Decay;
        }
    }

    void noteOff() noexcept
    {
        if (stage_ != Stage::Idle)
        {
            releaseStart_ = std::max (level_, 0.0001f);
            releasePos_ = 0.0f;
            stage_ = Stage::Release;
        }
    }

    void reset() noexcept
    {
        stage_ = Stage::Idle;
        level_ = 0.0f;
        releaseStart_ = 0.0f;
        releasePos_ = 0.0f;
    }

    bool isActive() const noexcept { return stage_ != Stage::Idle; }
    Stage getStage() const noexcept { return stage_; }
    float getLevel() const noexcept { return level_; }

    float next() noexcept
    {
        switch (stage_)
        {
            case Stage::Attack:
            {
                level_ += 1.0f / (float) (attackSec_ * sampleRate_);
                if (level_ >= 1.0f)
                {
                    level_ = 1.0f;
                    stage_ = Stage::Decay;
                }
                break;
            }
            case Stage::Decay:
            {
                level_ -= (1.0f - sustain_) / (float) (decaySec_ * sampleRate_);
                if (level_ <= sustain_)
                {
                    level_ = sustain_;
                    stage_ = Stage::Sustain;
                }
                break;
            }
            case Stage::Sustain:
                level_ = sustain_;
                break;
            case Stage::Release:
            {
                releasePos_ += 1.0f / (float) (releaseSec_ * sampleRate_);
                if (releasePos_ >= 1.0f)
                {
                    level_ = 0.0f;
                    stage_ = Stage::Idle;
                }
                else
                {
                    level_ = releaseStart_ * (1.0f - releasePos_);
                }
                break;
            }
            case Stage::Idle:
            default:
                level_ = 0.0f;
                break;
        }
        return level_;
    }

private:
    double sampleRate_ = 44100.0;
    float attackSec_ = 0.01f;
    float decaySec_ = 0.2f;
    float sustain_ = 0.7f;
    float releaseSec_ = 0.3f;
    float level_ = 0.0f;
    float releaseStart_ = 0.0f;
    float releasePos_ = 0.0f;
    Stage stage_ = Stage::Idle;
};
